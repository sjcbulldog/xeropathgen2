#include "CheesyGenerator.h"
#include "RobotPath.h"
#include "DistanceView.h"
#include "Pose2dConstrained.h"
#include "PathTrajectory.h"
#include "TrajectoryNames.h"
#include "TrajectoryUtils.h"
#include "RobotParams.h"
#include "TrapezoidalProfile.h"
#include <cmath>

CheesyGenerator::CheesyGenerator(double diststep, double timestep, double maxdx, double maxdy, double maxtheta, std::shared_ptr<RobotParams> robot)
{
	diststep_ = diststep;
	timestep_ = timestep;
	maxDx_ = maxdx;
	maxDy_ = maxdy;
	maxDTheta_ = maxtheta;
	robot_ = robot;
}

CheesyGenerator::~CheesyGenerator()
{
}

QVector<std::shared_ptr<SplinePair>>
CheesyGenerator::generateSplines(const QVector<Pose2dWithRotation>& points)
{
	QVector<std::shared_ptr<SplinePair>> splines;

	for (int i = 0; i < points.size() - 1; i++) {
		const Pose2dWithRotation& p1 = points[i];
		const Pose2dWithRotation& p2 = points[i + 1];
		auto pair = std::make_shared<SplinePair>(p1, p2);
		splines.push_back(pair);
	}

	return splines;
}

QVector<Pose2dWithTrajectory>
CheesyGenerator::timeParameterize(const DistanceView& view, const QVector<std::shared_ptr<PathConstraint>>& constraints, 
					double startvel, double endvel, double maxvel, double maxaccel)
{
	QVector<Pose2dConstrained> points;
	Pose2dConstrained predecessor;
	const static double kEpsilon = 1e-6;

	predecessor.setPosition(0.0);
	predecessor.setPose(view[static_cast<size_t>(0)]);
	predecessor.setVelocity(startvel);
	predecessor.setAccelMin(-maxaccel);
	predecessor.setAccelMax(maxaccel);

	//
	// Forward pass
	//
	for (size_t i = 0; i < view.size(); i++)
	{
		Pose2dConstrained state;
		state.setPose(view[i]);
		state.setPosition(view.getPosition(i));

		double dist = predecessor.pose().distance(state.pose());
		state.setPosition(predecessor.position() + dist);

		while (true)
		{
			double calcvel = std::sqrt(predecessor.velocity() * predecessor.velocity() + 2.0 * predecessor.accelMax() * dist);
			state.setVelocity(std::min(maxvel, calcvel));

			if (std::isnan(state.velocity()))
				throw std::runtime_error("invalid maximum velocity");

			state.setAccelMin(-maxaccel);
			state.setAccelMax(maxaccel);

			for (auto constraint : constraints)
			{
				double convel = constraint->getMaxVelocity(state, robot_);
				state.setVelocity(std::min(state.velocity(), convel));
			}

			if (state.velocity() < 0.0)
				throw std::runtime_error("invalid maximum velocity - constraint set to negative");

			if (dist < kEpsilon)
				break;

			double actaccel = (state.velocity() * state.velocity() - predecessor.velocity() * predecessor.velocity()) / (2.0 * dist);
			if (state.accelMax() < actaccel - kEpsilon)
			{
				predecessor.setAccelMax(state.accelMax());
				if (i != 0)
					points[i - 1] = predecessor;
			}
			else
			{
				if (actaccel > predecessor.accelMin() + kEpsilon)
				{
					predecessor.setAccelMax(actaccel);
					if (i != 0)
						points[i - 1] = predecessor;
				}
				break;
			}
		}
		points.push_back(state);
		predecessor = state;
	}

	//
	// Backward pass
	//
	size_t last = view.size() - 1;
	Pose2dConstrained sucessor;
	sucessor.setPose(view[last]);
	sucessor.setPosition(points[last].position());
	sucessor.setVelocity(endvel);
	sucessor.setAccelMin(-maxaccel);
	sucessor.setAccelMax(maxaccel);

	for (size_t i = points.size() - 1; i < points.size(); i--)
	{
		Pose2dConstrained state = points[i];
		double dist = state.position() - sucessor.position();				// Will be negative

		while (true)
		{
			double tmp = sucessor.velocity() * sucessor.velocity() + 2.0 * sucessor.accelMin() * dist;
			double newmaxvel = std::sqrt(tmp);
			if (newmaxvel >= state.velocity())
				break;

			if (std::isnan(newmaxvel))
				throw std::runtime_error("invalid new maximum velocity");

			state.setVelocity(newmaxvel);
			points[i] = state;

			if (dist > kEpsilon)
				break;

			double actaccel = (state.velocity() * state.velocity() - sucessor.velocity() * sucessor.velocity()) / (2.0 * dist);
			if (state.accelMin() > actaccel + kEpsilon)
			{
				sucessor.setAccelMin(state.accelMin());
				if (i != points.size() - 1)
					points[i + 1] = sucessor;
			}
			else
			{
				sucessor.setAccelMin(actaccel);
				if (i != points.size() - 1)
					points[i + 1] = sucessor;

				break;
			}
		}
		sucessor = state;
	}

	double t = 0.0;
	double s = 0.0;
	double v = 0.0;
	QVector<Pose2dWithTrajectory> result;

	for (size_t i = 0; i < points.size(); i++)
	{
		const Pose2dConstrained& state = points[i];
		double ds = state.position() - s;
		double accel = (state.velocity() * state.velocity() - v * v) / (2.0 * ds);
		double dt = 0;
		if (i > 0)
		{
			result[i - 1].setAcceleration(accel);
			if (std::fabs(accel) > kEpsilon)
			{
				dt = (state.velocity() - v) / accel;
			}
			else
			{
				dt = ds / v;
			}
		}
		t += dt;
		if (std::isinf(t) || std::isnan(t))
			throw std::runtime_error("invalid time value integrating forward");

		v = state.velocity();
		s = state.position();

		Pose2dWithTrajectory trajpt(view[i], t, s, v, accel);
		result.push_back(trajpt);
	}
	return result;
}

size_t CheesyGenerator::findIndex(const QVector<Pose2dWithTrajectory>& traj, double time)
{
	if (time < traj[0].time())
		return 0;

	if (time > traj[traj.size() - 1].time())
		return traj.size() - 1;

	size_t low = 0;
	size_t high = traj.size() - 1;

	while (high - low > 1)
	{
		size_t center = (high + low) / 2;
		if (time > traj[center].time())
		{
			low = center;
		}
		else
		{
			high = center;
		}
	}

	return low;
}

QVector<Pose2dWithTrajectory> CheesyGenerator::convertToUniformTime(const QVector<Pose2dWithTrajectory>& traj, double step)
{
	QVector<Pose2dWithTrajectory> result;

	for (double time = 0.0; time < traj[traj.size() - 1].time(); time += step)
	{
		Pose2dWithTrajectory newpt;

		size_t low = findIndex(traj, time);
		if (low == traj.size() - 1)
		{
			newpt = traj[traj.size() - 1];
		}
		else
		{
			double percent = (time - traj[low].time()) / (traj[low + 1].time() - traj[low].time());
			newpt = traj[low].interpolate(traj[low + 1], percent);
		}

		result.push_back(newpt);
	}

	return result;
}

std::shared_ptr<PathTrajectory> 
CheesyGenerator::generateInternal(std::shared_ptr<RobotPath> path, double maxvel)
{
	double maxDxPath = UnitConverter::convert(maxDx_, robot_->getLengthUnits(), path->units());
	double maxDyPath = UnitConverter::convert(maxDy_, robot_->getLengthUnits(), path->units());
	double distSteppath = UnitConverter::convert(diststep_, robot_->getLengthUnits(), path->units());

	//
	// Step 1: generate a set of splines that represent the path
	//         (taken from the cheesy poofs code)
	//
	QVector<std::shared_ptr<SplinePair>> splines = generateSplines(path->waypoints());

	//
	// Step 2: generate a set of points that represent the path where the curvature, x, and y do not 
	//         differ to an amount greater than maxDx_, maxDy_, maxDTheta_
	//         (taken from the cheesy poofs code)
	//
	QVector<Pose2dWithRotation> paramtraj = TrajectoryUtils::parameterize(splines, maxDxPath, maxDyPath, maxDTheta_);

	//
	// Step 3: generate a set of points that are equi-distant apart (diststep_).
	//
	DistanceView distview(paramtraj, distSteppath);

	//
	// Step 4: generate a timing view that meets the constraints of the system
	//
	const PathParameters& params = path->params();
	QVector<Pose2dWithTrajectory> pts = timeParameterize(distview, path->constraints(), params.startVelocity(),
										params.endVelocity(), maxvel, params.maxAccel());

	//
	// Step 5: convert the timeview view to a uniform timeing view
	//
	QVector<Pose2dWithTrajectory> uniform = convertToUniformTime(pts, timestep_);


	//
	// Step 6: compute the curvature for the path
	//
	auto traj = std::make_shared<PathTrajectory>(TrajectoryName::Main, uniform);
	TrajectoryUtils::computeCurvature(traj);

	//
	// Return a trajectory
	//
	return traj;
}

size_t CheesyGenerator::findIndexFromLocation(std::shared_ptr<PathTrajectory> traj, size_t start, const Translation2d& loc)
{
	static double tol = 0.05;

	for (size_t i = start; i < traj->size(); i++)
	{
		auto pt = (*traj)[i];
		if (std::abs(pt.x() - loc.getX()) < tol && std::abs(pt.y() - loc.getY()) < tol)
		{
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

QVector<QPair<size_t, double>> CheesyGenerator::getRotationTransitions(std::shared_ptr<RobotPath> path, std::shared_ptr<PathTrajectory> traj)
{
	QVector<std::pair<size_t, double>> ret;
	size_t index = 0;

	if (traj) {
		for (int i = 0; i < path->size(); i++)
		{
			const Pose2dWithRotation& pose = path->getPoint(i);
			size_t which = findIndexFromLocation(traj, index, pose.getTranslation());
			if (which == std::numeric_limits<size_t>::max()) {
				//
				// Something failed, signal the caller
				//
				return QVector<std::pair<size_t, double>>();
			}

			ret.push_back(QPair<size_t, double>(which, pose.swrot().toDegrees()));

			index = which + 1;
		}
	}

	return ret;
}

Translation2d CheesyGenerator::getWheelPerpendicularVector(Wheel w, double magnitude)
{
	double dx = 0.0, dy = 0.0;

	switch (w)
	{
	case Wheel::FL:
		dx = robot_length_ / 2.0;
		dy = robot_width_ / 2.0;
		break;

	case Wheel::FR:
		dx = robot_length_ / 2.0;
		dy = -robot_width_ / 2.0;
		break;

	case Wheel::BL:
		dx = -robot_length_ / 2.0;
		dy = robot_width_ / 2.0;
		break;

	case Wheel::BR:
		dx = -robot_length_ / 2.0;
		dy = -robot_width_ / 2.0;
		break;
	}

	double dist = std::sqrt(dx * dx + dy * dy);
	Translation2d pt(-dy / dist * magnitude, dx / dist * magnitude);
	return pt;
}

bool CheesyGenerator::modifyForRotation(std::shared_ptr<RobotPath> path, std::shared_ptr<PathTrajectory> traj, double percent)
{
	std::shared_ptr<TrapezoidalProfile> tp;
	QVector<Pose2dWithTrajectory> flpts, frpts, blpts, brpts;
	bool first = true;

	Translation2d prevfl, prevfr, prevbl, prevbr;
	double fldist = 0, frdist = 0, bldist = 0, brdist = 0;

	//
	// Get the indexes in the trajectory where the rotation of the swerve must change
	//
	QVector<QPair<size_t, double>> rotpoints = getRotationTransitions(path, traj);

	size_t rotindex = 0;
	double startrotangle = 0.0;			// Starting angle for this segment of rotation
	double startrottime = 0.0;			// Starting time for this segment of rotation

	for (size_t i = 0; i < traj->size(); i++)
	{
		const Pose2dWithTrajectory& pt = (*traj)[i];
		double time = pt.time();

		Rotation2d angle;

		//
		// Compute the desired angle at the current point on the trajectory
		//
		if (tp == nullptr)
		{
			//
			// We need a rotational profile to move us from the current point to the
			// next point in the rotpoints vector.  This is a vector of trajectory indices
			// and an associated angle.  If this is entry 0 or N - 1, we must also account for
			// the start and end delay
			//
			double duration;
			double diff;

			if (rotindex >= rotpoints.size() - 1)
			{
				duration = traj->getEndTime() - time;
				diff = 0.0;
				startrotangle = rotpoints[rotpoints.size() - 1].second;
			}
			else
			{
				duration = (*traj)[rotpoints[rotindex + 1].first].time() - (*traj)[rotpoints[rotindex].first].time();
				diff = rotpoints[rotindex + 1].second - rotpoints[rotindex].second;
				startrotangle = rotpoints[rotindex].second;
			}

			double accel = TrajectoryUtils::linearToRotational(robot_, path->params().maxAccel() * percent);
			double maxvel = TrajectoryUtils::linearToRotational(robot_, path->params().maxVelocity() * percent);

			if (diff > 180.0)
				diff -= 360.0;
			else if (diff <= -180.0)
				diff += 360.0;

			tp = std::make_shared<TrapezoidalProfile>(accel, -accel, maxvel);
			if (!tp->update(diff, 0.0, 0.0))
				return false;

			if (tp->getTotalTime() > duration)
				return false;

			startrottime = time;
		}

		if (rotindex == rotpoints.size() - 1)
		{
			angle = Rotation2d::fromDegrees(rotpoints[rotpoints.size() - 1].second);
		}
		else
		{
			angle = Rotation2d::fromDegrees(MathUtils::boundDegrees(startrotangle + tp->getDistance(time - startrottime)));
		}

		//
		// This is the linear velocity needed to rotate the robot per the
		// rotational speed profile.  This needs to be combined with the
		// translational velocity to set the final velocity for each wheel
		//
		double rv = TrajectoryUtils::rotationalToLinear(robot_, tp->getVelocity(time));

		//
		// This is the linear acceleration needed to rotate the robot per the
		// rotational speed profile.  This needs to be combined with the
		// translational acceleration to set the final acceleration for each wheel.
		//
		double ra = TrajectoryUtils::rotationalToLinear(robot_, tp->getAccel(time));

		Translation2d rotflvel = getWheelPerpendicularVector(Wheel::FL, rv).rotateBy(angle);
		Translation2d rotfrvel = getWheelPerpendicularVector(Wheel::FR, rv).rotateBy(angle);
		Translation2d rotblvel = getWheelPerpendicularVector(Wheel::BL, rv).rotateBy(angle);
		Translation2d rotbrvel = getWheelPerpendicularVector(Wheel::BR, rv).rotateBy(angle);

		if (rotflvel.normalize() > robot_max_velocity_)
			return false;

		if (rotfrvel.normalize() > robot_max_velocity_)
			return false;

		if (rotblvel.normalize() > robot_max_velocity_)
			return false;

		if (rotbrvel.normalize() > robot_max_velocity_)
			return false;

		Translation2d rotflacc = getWheelPerpendicularVector(Wheel::FL, ra).rotateBy(angle);
		Translation2d rotfracc = getWheelPerpendicularVector(Wheel::FR, ra).rotateBy(angle);
		Translation2d rotblacc = getWheelPerpendicularVector(Wheel::BL, ra).rotateBy(angle);
		Translation2d rotbracc = getWheelPerpendicularVector(Wheel::BR, ra).rotateBy(angle);

		if (rotflacc.normalize() > robot_max_accel_)
			return false;

		if (rotfracc.normalize() > robot_max_accel_)
			return false;

		if (rotblacc.normalize() > robot_max_accel_)
			return false;

		if (rotbracc.normalize() > robot_max_accel_)
			return false;

		Rotation2d heading = pt.rotation();
		Translation2d pathvel = Translation2d(heading, pt.velocity()).rotateBy(Rotation2d::fromDegrees(-angle.toDegrees()));
		Translation2d pathacc = Translation2d(heading, pt.acceleration()).rotateBy(Rotation2d::fromDegrees(-angle.toDegrees()));

		Translation2d flv = rotflvel + pathvel;
		Translation2d frv = rotfrvel + pathvel;
		Translation2d blv = rotblvel + pathvel;
		Translation2d brv = rotbrvel + pathvel;

		Translation2d fla = rotflacc + pathacc;
		Translation2d fra = rotfracc + pathacc;
		Translation2d bla = rotblacc + pathacc;
		Translation2d bra = rotbracc + pathacc;

		Translation2d flpos = Translation2d(robot_length_ / 2.0, robot_width_ / 2.0).rotateBy(angle).translateBy(pt.translation());
		Translation2d frpos = Translation2d(robot_length_ / 2.0, -robot_width_ / 2.0).rotateBy(angle).translateBy(pt.translation());
		Translation2d blpos = Translation2d(-robot_length_ / 2.0, robot_width_ / 2.0).rotateBy(angle).translateBy(pt.translation());
		Translation2d brpos = Translation2d(-robot_length_ / 2.0, -robot_width_ / 2.0).rotateBy(angle).translateBy(pt.translation());

		if (!first)
		{
			fldist += flpos.distance(prevfl);
			frdist += frpos.distance(prevfr);
			bldist += blpos.distance(prevbl);
			brdist += brpos.distance(prevbr);
		}
		else
		{
			first = false;
		}

		Pose2d flpose(flpos, flv.toRotation());
		Pose2dWithTrajectory fltraj(flpose, time, fldist, flv.normalize(), fla.normalize());
		flpts.push_back(fltraj);

		Pose2d frpose(frpos, frv.toRotation());
		Pose2dWithTrajectory frtraj(frpose, time, frdist, frv.normalize(), fra.normalize());
		frpts.push_back(frtraj);

		Pose2d blpose(blpos, blv.toRotation());
		Pose2dWithTrajectory bltraj(blpose, time, bldist, blv.normalize(), bla.normalize());
		blpts.push_back(bltraj);

		Pose2d brpose(brpos, brv.toRotation());
		Pose2dWithTrajectory brtraj(brpose, time, brdist, brv.normalize(), bra.normalize());
		brpts.push_back(brtraj);

		prevfl = flpos;
		prevfr = frpos;
		prevbl = blpos;
		prevbr = brpos;

		(*traj)[i].setSwRotation(angle);

		//
		// See if we are done with the current profile
		//
		if (time - startrottime >= tp->getTotalTime()) {
			rotindex++;
			tp = nullptr;
		}
	}

	return true;
}

std::shared_ptr<PathTrajectory>
CheesyGenerator::generateSwerve(std::shared_ptr<RobotPath> path)
{
	std::shared_ptr<PathTrajectory> traj;
	double percent = 1.0;
	double maxvel = robot_max_velocity_;

	while (percent > 0.0)
	{
		traj = generateInternal(path, percent * maxvel);

		if (modifyForRotation(path, traj, 1.0 - percent)) {
			break;
		}

		percent -= 0.01;
	}

	return (percent <= 0.0) ? nullptr : traj;
}

std::shared_ptr<PathTrajectory>
CheesyGenerator::generateTankDrive(std::shared_ptr<RobotPath> path)
{
	std::shared_ptr<PathTrajectory> traj;

	traj = generateInternal(path, path->params().maxVelocity());

	//
	// Now, set the "swerve" rotation, which is meaningless for a tank drive, to the
	// heading so that it still represents the direction the robot is pointing
	//
	for (int i = 0; i < traj->size(); i++) {
		(*traj)[i].setSwRotation((*traj)[i].rotation());
	}

	return traj;
}

std::shared_ptr<PathTrajectory>
CheesyGenerator::generate(std::shared_ptr<RobotPath> path)
{
	double percent = 1.0;
	std::shared_ptr<PathTrajectory> traj;

	robot_width_ = UnitConverter::convert(robot_->getWheelBaseWidth(), robot_->getLengthUnits(), path->units());
	robot_length_ = UnitConverter::convert(robot_->getWheelBaseLength(), robot_->getLengthUnits(), path->units());
	robot_max_velocity_ = UnitConverter::convert(robot_->getMaxVelocity(), robot_->getLengthUnits(), path->units());
	robot_max_accel_ = UnitConverter::convert(robot_->getMaxAccel(), robot_->getLengthUnits(), path->units());

	if (robot_->getDriveType() == RobotParams::DriveType::TankDrive)
	{
		traj = generateTankDrive(path);
	}
	else
	{
		traj = generateSwerve(path);
	}

	return traj;
}