#include "CheesyGenerator.h"
#include "RobotPath.h"
#include "DistanceView.h"
#include "Pose2dConstrained.h"
#include "PathTrajectory.h"
#include "TrajectoryNames.h"
#include "TrajectoryUtils.h"
#include "RobotParams.h"
#include "TrapezoidalProfile.h"
#include "DistanceVelocityConstraint.h"
#include <cmath>

CheesyGenerator::CheesyGenerator(double diststep, double timestep, double maxdx, double maxdy, double maxtheta, std::shared_ptr<RobotParams> robot, bool xeromode) 
		: GeneratorBase(diststep, timestep, maxdx, maxdy, maxtheta, robot)
{
	xeromode_ = xeromode;
}

CheesyGenerator::~CheesyGenerator()
{
}

std::shared_ptr<PathTrajectory>
CheesyGenerator::generateSwerve(std::shared_ptr<RobotPath> path)
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
		traj = generateSwerve(path);
	}

	return traj;
}