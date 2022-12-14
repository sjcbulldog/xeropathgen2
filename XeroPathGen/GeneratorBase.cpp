//
// Copyright 2022 Jack W. Griffin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
//
#include "GeneratorBase.h"
#include "Pose2dConstrained.h"
#include "TrapezoidalProfile.h"
#include "TrajectoryUtils.h"
#include "RobotPath.h"
#include "TrajectoryNames.h"
#include <QtCore/QStandardPaths>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

GeneratorBase::GeneratorBase(const QString &logfile, QMutex& loglock, int which, double diststep, double timestep, double maxdx, double maxdy, double maxtheta, std::shared_ptr<RobotParams> robot)
	: logfile_(logfile), loglock_(loglock), which_(which)
{
	robot_ = robot;
	diststep_ = diststep;
	timestep_ = timestep;
	maxDx_ = maxdx;
	maxDy_ = maxdy;
	maxDTheta_ = maxtheta;
}

void GeneratorBase::logMessage(const QString& msg)
{
	loglock_.lock();

	QFile file(logfile_);
	if (file.open(QIODeviceBase::WriteOnly | QIODeviceBase::Append | QIODeviceBase::Text))
	{
		QTextStream strm(&file);
		strm << QString::number(which_) << ":" << msg << "\n";
	}

	loglock_.unlock();
}

std::shared_ptr<PathTrajectory>
GeneratorBase::generateInternal(std::shared_ptr<RobotPath> path, QVector<std::shared_ptr<PathConstraint>>& extras)
{
	double maxDxPath = UnitConverter::convert(maxDx_, robot()->getLengthUnits(), path->units());
	double maxDyPath = UnitConverter::convert(maxDy_, robot()->getLengthUnits(), path->units());
	double distSteppath = UnitConverter::convert(diststep_, robot()->getLengthUnits(), path->units());

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

	if (distview.size() == 1 && distview.length() < 1e-4) {
		//
		// We have a simple, two point path with the two points concurrent.  Just
		// return a null trajectory, as this does not make any sense
		//
		return nullptr;
	}

	//
	// Step 4: generate a timing view that meets the constraints of the system
	//
	const PathParameters& params = path->params();
	QVector<std::shared_ptr<PathConstraint>> constraints;
	constraints.append(path->constraints());
	constraints.append(extras);
	QVector<Pose2dWithTrajectory> pts = timeParameterize(distview, constraints, params.startVelocity(),
		params.endVelocity(), params.maxVelocity(), params.maxAccel());

	//
	// Step 5: convert the timeview view to a uniform timing view
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


void GeneratorBase::computeRobotParameters(std::shared_ptr<RobotPath> path)
{
	robot_width_ = UnitConverter::convert(robot_->getWheelBaseWidth(), robot_->getLengthUnits(), path->units());
	robot_length_ = UnitConverter::convert(robot_->getWheelBaseLength(), robot_->getLengthUnits(), path->units());
	robot_max_velocity_ = UnitConverter::convert(robot_->getMaxVelocity(), robot_->getLengthUnits(), path->units());
	robot_max_accel_ = UnitConverter::convert(robot_->getMaxAccel(), robot_->getLengthUnits(), path->units());
}

QVector<std::shared_ptr<SplinePair>>
GeneratorBase::generateSplines(const QVector<Pose2dWithRotation>& points)
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
GeneratorBase::timeParameterize(const DistanceView& view, const QVector<std::shared_ptr<PathConstraint>>& constraints,
	double startvel, double endvel, double maxvel, double maxaccel)
{
	QVector<Pose2dConstrained> points;
	Pose2dConstrained predecessor;
	const static double kEpsilon = 1e-6;

	predecessor.setPosition(0.0);
	predecessor.setPose(view[static_cast<int>(0)]);
	predecessor.setVelocity(startvel);
	predecessor.setAccelMin(-maxaccel);
	predecessor.setAccelMax(maxaccel);

	//
	// Forward pass
	//
	for (int i = 0; i < view.size(); i++)
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
	int last = view.size() - 1;
	Pose2dConstrained sucessor;
	sucessor.setPose(view[last]);
	sucessor.setPosition(points[last].position());
	sucessor.setVelocity(endvel);
	sucessor.setAccelMin(-maxaccel);
	sucessor.setAccelMax(maxaccel);

	for (int i = points.size() - 1; i >= 0 ; i--)
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

	for (int i = 0; i < points.size(); i++)
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


int GeneratorBase::findIndex(const QVector<Pose2dWithTrajectory>& traj, double time)
{
	if (time < traj[0].time())
		return 0;

	if (time > traj[traj.size() - 1].time())
		return traj.size() - 1;

	int low = 0;
	int high = traj.size() - 1;

	while (high - low > 1)
	{
		int center = (high + low) / 2;
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

QVector<Pose2dWithTrajectory> GeneratorBase::convertToUniformTime(const QVector<Pose2dWithTrajectory>& traj, double step)
{
	QVector<Pose2dWithTrajectory> result;

	for (double time = 0.0; time < traj[traj.size() - 1].time(); time += step)
	{
		Pose2dWithTrajectory newpt;

		int low = findIndex(traj, time);
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


int GeneratorBase::findIndexFromLocation(std::shared_ptr<PathTrajectory> traj, int start, const Translation2d& loc)
{
	static double tol = 0.05;

	for (int i = start; i < traj->size(); i++)
	{
		auto pt = (*traj)[i];
		if (std::abs(pt.x() - loc.getX()) < tol && std::abs(pt.y() - loc.getY()) < tol)
		{
			return i;
		}
	}

	return std::numeric_limits<int>::max();
}

Translation2d GeneratorBase::getWheelPerpendicularVector(Wheel w, double magnitude)
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

bool GeneratorBase::modifyForRotation(std::shared_ptr<RobotPath> path, std::shared_ptr<PathTrajectory> traj, double percent)
{
	QVector<std::shared_ptr<SplinePair>> splines = generateSplines(path->waypoints());
	QVector<double> dists = TrajectoryUtils::getDistancesForSplines(splines);
	assert(dists.size() == path->waypoints().size());

	for (int i = 0; i < path->size() - 1; i++) 
	{
		double startTime, endTime;
		int startIndex, endIndex;
		double startRot = path->getPoint(i).getSwrot().toDegrees();
		double startRotVel = path->getPoint(i).getSwrotVelocity();
		double endRot = path->getPoint(i + 1).getSwrot().toDegrees();
		double endRotVel = path->getPoint(i + 1).getSwrotVelocity();
		
		if (!traj->getTimeForDistance(dists[i], startTime))
		{
			//
			// Something is wrong, this should never happen
			//
			return false;
		}
		startIndex = traj->getIndex(startTime);

		if (!traj->getTimeForDistance(dists[i + 1], endTime))
		{
			//
			// Something is wrong, this should never happen
			//
			return false;
		}

		endIndex = traj->getIndex(endTime);

		if (traj->size() - endIndex < 5) {
			//
			// If the detected end point is within 100 ms of the end of the path
			// just push the computation to the end of the path.  This is a round off
			// error that has to do with computing the distances using the splines and then
			// the computations required to get from splines to a trajectory.
			//
			endIndex = traj->size() ;
		}

		//
		// We now need the trajectory points for the times range
		//
		if (!modifySegmentForRotation(path, traj, percent , startIndex, endIndex, startRot, startRotVel, endRot, endRotVel))
		{
			return false;
		}
	}

	return true;
}

bool GeneratorBase::modifySegmentForRotation(std::shared_ptr<RobotPath> path, std::shared_ptr<PathTrajectory> traj, double percent, int start, int end, double startRot, double startRotVel, double endRot, double endRotVel)
{
	QString logmsg;

	assert(start >= 0 && start < traj->size());
	assert(end >= 0 && end <= traj->size());
	assert(end > start);

	std::shared_ptr<TrapezoidalProfile> tp;
	QVector<Pose2dWithTrajectory> flpts, frpts, blpts, brpts;
	bool first = true;

	Translation2d prevfl, prevfr, prevbl, prevbr;
	double fldist = 0, frdist = 0, bldist = 0, brdist = 0;

	//
	// Get the time interval between the points
	//
	double startTime, endTime;
	startTime = (*traj)[start].time();
	if (end == traj->size())
		endTime = traj->getEndTime();
	else
		endTime = (*traj)[end].time();

	double deltat = endTime - startTime;

	double maxaccel = TrajectoryUtils::linearToRotational(robot_, path->params().maxAccel() * percent);
	double maxvel = TrajectoryUtils::linearToRotational(robot_, path->params().maxVelocity() * percent);

	double diff = MathUtils::boundDegrees(endRot - startRot);

	logmsg = "modifySegmentForRotation:";
	logmsg += "time = " + QString::number(startTime, 'f', 2) + " - " + QString::number(endTime, 'f', 2);
	logmsg += ", linear accel " + QString::number(path->params().maxAccel() * percent, 'f', 2);
	logmsg += ", linear velocity " + QString::number(path->params().maxVelocity() * percent, 'f', 2);
	logmsg += ", rot accel " + QString::number(maxaccel, 'f', 2);
	logmsg += ", rot velocity " + QString::number(maxvel, 'f', 2);
	logMessage(logmsg);

	tp = std::make_shared<TrapezoidalProfile>(maxaccel, -maxaccel, maxvel);
	if (!tp->update(diff, startRotVel, endRotVel)) {
		logMessage("modifySegmentForRotation: cannot create TrapezoidalProfile - failed");
		return false;
	}

	logMessage("trapezoidal profile: " + tp->toString());

	if (tp->getTotalTime() > deltat) {
		//
		// With the percentage of the velocity and acceleration given to 
		// rotation, we don't have time to complete the rotation.
		//
		logMessage("modifySegmentForRotation: trapeazoidal profile cannot complete in defined interval");
		return false;
	}

	for (int i = start; i < end; i++)
	{
		const Pose2dWithTrajectory& pt = (*traj)[i];
		double time = pt.time();

		//
		// Get the rotation of the swerve drive at this point in time in the segment
		//
		Rotation2d angle;
		double rotvel = 0.0;
		if (time - startTime > tp->getTotalTime()) {
			angle = Rotation2d::fromDegrees(endRot);
			rotvel = endRotVel;
		}
		else {
			angle = Rotation2d::fromDegrees(MathUtils::boundDegrees(startRot + tp->getDistance(time - startTime)));
			rotvel = tp->getVelocity(time - startTime);
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

		Translation2d rotflacc = getWheelPerpendicularVector(Wheel::FL, ra).rotateBy(angle);
		Translation2d rotfracc = getWheelPerpendicularVector(Wheel::FR, ra).rotateBy(angle);
		Translation2d rotblacc = getWheelPerpendicularVector(Wheel::BL, ra).rotateBy(angle);
		Translation2d rotbracc = getWheelPerpendicularVector(Wheel::BR, ra).rotateBy(angle);

		bool ok = true;
		logmsg.clear();
		if (rotflvel.normalize() > robot_max_velocity_)
		{
			logmsg.clear();
			logmsg += "FL velocity failed - required " + QString::number(rotflvel.normalize(), 'f', 2) + ", max " + QString::number(robot_max_velocity_, 'f', 2);
			logMessage(logmsg);
			ok = false;
		}

		if (rotfrvel.normalize() > robot_max_velocity_)
		{
			logmsg.clear();
			logmsg += "FR velocity failed - required " + QString::number(rotfrvel.normalize(), 'f', 2) + ", max " + QString::number(robot_max_velocity_, 'f', 2);
			logMessage(logmsg);
			ok = false;
		}

		if (rotblvel.normalize() > robot_max_velocity_)
		{
			logmsg.clear();
			logmsg += "BL velocity failed - required " + QString::number(rotblvel.normalize(), 'f', 2) + ", max " + QString::number(robot_max_velocity_, 'f', 2);
			logMessage(logmsg);
			ok = false;
		}

		if (rotbrvel.normalize() > robot_max_velocity_)
		{
			logmsg.clear();
			logmsg += "BR velocity failed - required " + QString::number(rotbrvel.normalize(), 'f', 2) + ", max " + QString::number(robot_max_velocity_, 'f', 2);
			logMessage(logmsg);
			ok = false;
		}

		if (rotflacc.normalize() > robot_max_accel_)
		{
			logmsg.clear();
			logmsg += "FL acceleration failed - required " + QString::number(rotflacc.normalize(), 'f', 2) + ", max " + QString::number(robot_max_accel_, 'f', 2);
			logMessage(logmsg);
			ok = false;
		}

		if (rotfracc.normalize() > robot_max_accel_)
		{
			logmsg.clear();
			logmsg += "FR acceleration failed - required " + QString::number(rotfracc.normalize(), 'f', 2) + ", max " + QString::number(robot_max_accel_, 'f', 2);
			logMessage(logmsg);
			ok = false;
		}

		if (rotblacc.normalize() > robot_max_accel_)
		{
			logmsg.clear();
			logmsg += "BL acceleration failed - required " + QString::number(rotblacc.normalize(), 'f', 2) + ", max " + QString::number(robot_max_accel_, 'f', 2);
			logMessage(logmsg);
			ok = false;
		}

		if (rotbracc.normalize() > robot_max_accel_)
		{
			logmsg.clear();
			logmsg += "BR acceleration failed - required " + QString::number(rotbracc.normalize(), 'f', 2) + ", max " + QString::number(robot_max_accel_, 'f', 2);
			logMessage(logmsg);
			ok = false;
		}

		if (!ok) {
			return false;
		}

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

		(*traj)[i].pose().setSwrot(angle);
		(*traj)[i].setRotVel(rotvel);
	}

	return true;
}
