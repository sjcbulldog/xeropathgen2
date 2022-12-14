#pragma once

#include "GeneratorType.h"
#include "RobotPath.h"
#include <memory>

class TrajectoryGroup 
{
public:
	TrajectoryGroup(GeneratorType type, std::shared_ptr<RobotPath> path);

	std::shared_ptr<RobotPath> path() {
		return path_;
	}

private:
	GeneratorType type_;
	std::shared_ptr<RobotPath> path_;
};

