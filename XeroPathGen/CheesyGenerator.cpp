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
		if (traj == nullptr) {
			return traj;
		}

		//
		// Now evaluate if any rotation requested is feasible.  If not, we lower the percentage of velocity for
		// any given segment available to the linear trajectory to reserve more for the rotation.
		//
		QVector<bool> status;

		for (int i = 0; i < path->size() - 1; i++)
		{
			double startTime, endTime;
			int startIndex, endIndex;
			double startRot = path->getPoint(i).getSwrot().toDegrees();
			double endRot = path->getPoint(i + 1).getSwrot().toDegrees();
			double startRotVel = path->getPoint(i).getSwrotVelocity();
			double endRotVel = path->getPoint(i + 1).getSwrotVelocity();

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

			if (!modifySegmentForRotation(path, traj, 1.0 - percents[i], startIndex, endIndex, startRot, startRotVel, endRot, endRotVel))
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
		(*traj)[i].pose().setSwrot((*traj)[i].rotation());
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