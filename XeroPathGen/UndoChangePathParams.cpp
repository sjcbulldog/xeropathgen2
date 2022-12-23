#include "UndoChangePathParams.h"
#include "RobotPath.h"

void UndoChangePathParams::apply()
{
	path_->setParams(params_, false);
}