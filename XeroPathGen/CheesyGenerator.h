#pragma once

#include "GeneratorBase.h"
#include "PathConstraint.h"
#include "PathTrajectory.h"
#include "Pose2dWithRotation.h"
#include "Pose2dWithTrajectory.h"
#include "SplinePair.h"
#include "DistanceView.h"
#include "SwerveWheels.h"
#include <QtCore/QVector>

class CheesyGenerator : public GeneratorBase
{
public:
	CheesyGenerator(double diststep, double timestep, double maxdx, double maxdy, double maxtheta, std::shared_ptr<RobotParams> robot, bool xeromod);
	virtual ~CheesyGenerator();

	std::shared_ptr<PathTrajectory> generate(std::shared_ptr<RobotPath> path);

protected:
	std::shared_ptr<PathTrajectory> generateSwerveSingleRotate(std::shared_ptr<RobotPath> path);
	std::shared_ptr<PathTrajectory> generateSwervePerWaypointRotate(std::shared_ptr<RobotPath> path);
	std::shared_ptr<PathTrajectory> generateTankDrive(std::shared_ptr<RobotPath> path);

private:
	bool xeromode_;
};

