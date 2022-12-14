#pragma once

#include "RobotPath.h"
#include "GeneratorType.h"
#include "TrajectoryGroup.h"
#include <QtCore/QObject>
#include <memory>

class Generator : public QObject
{
	Q_OBJECT

public:
	Generator(std::shared_ptr<TrajectoryGroup> group);

	void generateTrajectory();

signals:
	void trajectoryComplete(std::shared_ptr<TrajectoryGroup> group);

private:
	std::shared_ptr<TrajectoryGroup> group_;
};