#pragma once

#include "RobotPath.h"
#include "GeneratorType.h"
#include "TrajectoryGroup.h"
#include "RobotParams.h"
#include <QtCore/QObject>
#include <memory>

class Generator : public QObject
{
	Q_OBJECT

public:
	Generator(double timestep, std::shared_ptr<RobotParams> robot, std::shared_ptr<TrajectoryGroup> group);

	void generateTrajectory();

signals:
	void trajectoryComplete(std::shared_ptr<TrajectoryGroup> group);

private:
	void addTankDriveTrajectories();

private:
	double timestep_;
	std::shared_ptr<TrajectoryGroup> group_;
	std::shared_ptr<RobotParams> robot_;
};