#include "Generator.h"
#include "RobotPath.h"
#include "PathGroup.h"
#include <QtCore/QThread>

Generator::Generator(std::shared_ptr<TrajectoryGroup> group)
{
	group_ = group;
}

void Generator::generateTrajectory()
{
	emit trajectoryComplete(group_);
}
