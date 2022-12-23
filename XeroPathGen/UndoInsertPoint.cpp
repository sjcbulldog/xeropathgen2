#include "UndoInsertPoint.h"
#include "RobotPath.h"

void UndoInsertPoint::apply()
{
	path_->removePoint(index_, false);
}