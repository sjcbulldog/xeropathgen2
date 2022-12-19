#include "XeroSwerveGenerator.h"
#include "DistanceVelocityConstraint.h"
#include "TrajectoryUtils.h"
#include "TrajectoryNames.h"
#include <QtCore/QVector>

XeroSwerveGenerator::XeroSwerveGenerator(double diststep, double timestep, double maxdx, double maxdy, double maxtheta, std::shared_ptr<RobotParams> robot)
	: CheesyGenerator(diststep, timestep, maxdx, maxdy, maxtheta, robot)
{
}

XeroSwerveGenerator::~XeroSwerveGenerator()
{
}

QVector<double> XeroSwerveGenerator::generateDistances(const QVector<std::shared_ptr<SplinePair>> &splines)
{
	QVector<double> dists;
	int steps = 1000;

	for (size_t i = 0; i < splines.size(); i++)
	{
		double dist = 0;
		auto pair = splines[i];
		bool first = true;
		Translation2d pos, prevpos;
		for (float t = 0.0f; t <= 1.0f; t += 1.0f / steps)
		{
			pos = pair->evalPosition(t);
			if (first)
				first = false;
			else
			{
				dist += pos.distance(prevpos);
			}

			prevpos = pos;
		}

		dists.push_back(dist);
	}

	return dists;
}

bool XeroSwerveGenerator::isSegmentValid(std::shared_ptr<PathTrajectory> traj, double start, double end)
{
	for (double t = start; t <= end; t += getTimeStep())
	{

	}

	return false;
}

void XeroSwerveGenerator::adjustConstraints(QVector<std::shared_ptr<PathConstraint>>& constraints, const QVector<bool>& segvalid)
{
}

double XeroSwerveGenerator::linearToRotational(double value)
{
	return value;
}

std::shared_ptr<PathTrajectory> 
XeroSwerveGenerator::generate(std::shared_ptr<RobotPath> path)
{
	bool isValid = false;

	rot_max_accel_ = linearToRotational(path->params().maxAccel());
	rot_max_velocity_ = linearToRotational(path->params().maxVelocity());

	//
	//.Step 1: generate the spliens for the waypoints
	//
	auto splines = generateSplines(path->waypoints());

	//
	// Step 2: find the distance between the waypoints based on the splines
	//
	auto distances = generateDistances(splines);

	//
	// Step 3: create a set of constraints to limit the velocity between waypoints.  These will be used
	//         to limit the velocity between waypoints to provide enough velocity for any rotation
	//
	QVector<std::shared_ptr<PathConstraint>> rotconst;
	for (int i = 0; i < path->waypoints().size() - 1; i++) {
		std::shared_ptr<DistanceVelocityConstraint> c = std::make_shared<DistanceVelocityConstraint>(path, distances[i], distances[i + 1], path->params().maxVelocity());
		rotconst.push_back(c);
	}

	//
	// Step 4: generate a set of points that represent the path where the curvature, x, and y do not 
	//         differ to an amount greater than maxDx_, maxDy_, maxDTheta_
	//         (taken from the cheesy poofs code)
	//
	QVector<Pose2dWithRotation> paramtraj = TrajectoryUtils::parameterize(splines, getMaxDx(), getMaxDy(), getMaxDTheta());

	//
	// Step 3: generate a set of points that are equi-distant apart (diststep_).
	//
	DistanceView distview(paramtraj, getDistStep());

	QVector<std::shared_ptr<PathConstraint>> constraints;
	constraints.append(path->constraints());
	constraints.append(rotconst);
	const PathParameters& params = path->params();
	QVector<Pose2dWithTrajectory> pts;
	std::shared_ptr<PathTrajectory> traj;
	QVector<double> times;

	while (!isValid) {

		//
		// Step 4: generate a timing view that meets the constraints of the system
		//
		pts = timeParameterize(distview, constraints, params.startVelocity(),
			params.endVelocity(), params.maxVelocity(), params.maxAccel());

		//
		// Step 5: convert the timeview view to a uniform timeing view
		//
		QVector<Pose2dWithTrajectory> uniform = convertToUniformTime(pts, getTimeStep());

		//
		// Step 6: Convert to a trajectory to make it easier to use
		//
		traj = std::make_shared<PathTrajectory>(TrajectoryName::Main, uniform);

		//
		// Step 7: Compute the times of the waypoints
		// 
		times.clear();
		times.push_back(0.0);
		for (int i = 0; i < distances.size(); i++) {
			double time;
			
			if (!traj->getTimeForDistance(distances[i], time)) {
				traj = nullptr;
				break;
			}

			times.push_back(time);
		}

		//
		// Step 8: Evaluate each of the time periods based on the desired change in
		//         robot angle.  For instance, the time period from times[1] to times[0]
		//         is the time between waypoint 1 and wayhpoint 0.
		//
		isValid = true;
		QVector<bool> worksBySegment;
		for (int i = 0; i < times.size() - 1; i++) {
			bool b = isSegmentValid(traj, times[i], times[i + 1]);
			worksBySegment.push_back(b);
			if (!b) {
				isValid = false;
			}
		}

		//
		// Step 9: Adjust the constraints
		//
		if (!isValid) {
			adjustConstraints(rotconst, worksBySegment);
		}
	}

	return traj;
}