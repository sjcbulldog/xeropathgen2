#include "CheesyGenerator.h"
#include "RobotPath.h"
#include "PathGroup.h"
#include "DistanceView.h"
#include "Pose2dConstrained.h"
#include "PathTrajectory.h"
#include "TrajectoryNames.h"
#include "TrajectoryUtils.h"
#include "RobotParams.h"
#include "TrapezoidalProfile.h"
#include "DistanceVelocityConstraint.h"
#include <cmath>

CheesyGenerator::CheesyGenerator(const QString &logfile, QMutex& loglock, int which, double diststep, double timestep, double maxdx, double maxdy, double maxtheta, std::shared_ptr<RobotParams> robot, bool xeromode)
		: GeneratorBase(logfile, loglock, which, diststep, timestep, maxdx, maxdy, maxtheta, robot)
{
	xeromode_ = xeromode;
}

CheesyGenerator::~CheesyGenerator()
{
}

std::shared_ptr<PathTrajectory>
CheesyGenerator::generateSwervePerWaypointRotate(std::shared_ptr<RobotPath> path)
{
	QString logtext;
	std::shared_ptr<PathTrajectory> traj;

	logMessage(path->fullname() + ": generating splines");
	QVector<std::shared_ptr<SplinePair>> splines = generateSplines(path->waypoints());
	QVector<double> dists = TrajectoryUtils::getDistancesForSplines(splines);
	assert(dists.size() == path->waypoints().size());
	bool generating = true;

	QVector<std::shared_ptr<PathConstraint>> extras;
	QVector<double> percents;

	for (int i = 0; i < path->size() - 1; i++) {
		percents.push_back(1);
	}

	int iteration = 1;
	bool running = true;
	while (running) {
		logMessage(path->fullname() + ": iteration " + QString::number(iteration++));

		logtext.clear();
		for (double d : percents) {
			if (logtext.length() > 0) {
				logtext += ", ";
			}
			logtext += QString::number(d, 'f', 2);
		}
		logMessage(path->fullname() + ": per seg percentages: " + logtext);

		running = false;

		//
		// Generate the linear trajectory based on the percent of robot velocity used for each
		// of the segments between the various waypoints
		//
		extras.clear();
		for (int i = 0; i < path->size() - 1; i++) {
			auto c = std::make_shared<DistanceVelocityConstraint>(path, dists[i], dists[i + 1], path->params().maxVelocity() * percents[i]);
			extras.push_back(c);
		}

		traj = generateInternal(path, extras);

		//
		// Now evaluate if any rotation requested is feasible.  If not, we lower the percentage of velocity for
		// any given segment available to the linear trajectory to reserve more for the rotation.
		//
		QVector<bool> status;

		for (int i = 0; i < path->size() - 1; i++)
		{
			double startTime, endTime;
			int startIndex, endIndex;
			double startRot = path->getPoint(i).swrot().toDegrees();
			double endRot = path->getPoint(i + 1).swrot().toDegrees();

			if (!traj->getTimeForDistance(dists[i], startTime))
			{
				//
				// Something is wrong, this should never happen
				//
				return nullptr;
			}
			startIndex = traj->getIndex(startTime);

			if (!traj->getTimeForDistance(dists[i + 1], endTime))
			{
				//
				// Something is wrong, this should never happen
				//
				return nullptr;
			}

			endIndex = traj->getIndex(endTime);

			if (traj->size() - endIndex < 5) {
				//
				// If the detected end point is within 100 ms of the end of the path
				// just push the computation to the end of the path.  This is a round off
				// error that has to do with computing the distances using the splines and then
				// the computations required to get from splines to a trajectory.
				//
				endIndex = traj->size();
			}

			//
			// We now need the trajectory points for the times range
			//

			if (!modifySegmentForRotation(path, traj, 1.0 - percents[i], startIndex, endIndex, startRot, endRot))
			{
				percents[i] -= 0.01;
				running = true;

				if (percents[i] <= 0.0) {
					//
					// A single segment cannot reach the desired goal
					//
					return nullptr;
				}
				status.push_back(false);
			}
			else
			{
				status.push_back(true);
				status.push_back(true);
			}

			logtext.clear();
			for (bool b : status) {
				if (logtext.length() > 0) {
					logtext += ", ";
				}

				logtext += (b ? "success" : "failed");
			}
		}
	}

	return traj;
}

std::shared_ptr<PathTrajectory>
CheesyGenerator::generateSwerveSingleRotate(std::shared_ptr<RobotPath> path)
{
	std::shared_ptr<PathTrajectory> traj;
	double percent = 1.0;
	double maxvel = robotMaxVelocity();
	QVector<std::shared_ptr<PathConstraint>> extras;

	while (percent > 0.0)
	{
		extras.clear();
		auto c = std::make_shared<DistanceVelocityConstraint>(path, 0.0, std::numeric_limits<double>::max(), percent * maxvel);
		extras.push_back(c);
		traj = generateInternal(path, extras);

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
	QVector<std::shared_ptr<PathConstraint>> extras;

	traj = generateInternal(path, extras);

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

	QString startmsg = "Starting CheesyGenerator:";
	startmsg += "path " + path->fullname();
	if (robot()->getDriveType() == RobotParams::DriveType::TankDrive) {
		startmsg += ", drive = tank";
	}
	else {
		startmsg += ", drive = swerve";
	}

	if (xeromode_) {
		startmsg += ", per waypoint rotation mode";
	}
	else {
		startmsg += ", single rotation mode";
	}
	logMessage(startmsg);

	//
	// Compute the robot parameters in terms of the units used by the
	// path and cache them for future access.
	//
	computeRobotParameters(path);

	if (robot()->getDriveType() == RobotParams::DriveType::TankDrive)
	{
		traj = generateTankDrive(path);
	}
	else
	{
		if (xeromode_) {
			traj = generateSwervePerWaypointRotate(path);
		}
		else {
			traj = generateSwerveSingleRotate(path);
		}
	}

	if (traj == nullptr) {
		logMessage(path->fullname() + " - path generation failed");
	}
	else {
		logMessage(path->fullname() + " - path generation successful");
	}

	return traj;
}