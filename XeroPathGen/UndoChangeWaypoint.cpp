#include "UndoChangeWaypoint.h"
#include "RobotPath.h"

void UndoChangeWaypoint::apply()
{
	path_->replacePoint(index_, waypoint_, false);
}