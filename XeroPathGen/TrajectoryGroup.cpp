#include "TrajectoryGroup.h"

TrajectoryGroup::TrajectoryGroup(GeneratorType type, std::shared_ptr<RobotPath> path)
{
	type_ = type;
	path_ = path;
}