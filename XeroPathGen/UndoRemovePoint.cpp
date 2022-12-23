#include "UndoRemovePoint.h"
#include "RobotPath.h"

void UndoRemovePoint::apply()
{
	path_->insertPoint(index_, waypoint_, false);
}