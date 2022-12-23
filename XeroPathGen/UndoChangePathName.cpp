#include "UndoChangePathName.h"
#include "RobotPath.h"

void UndoChangePathName::apply()
{
	path_->setName(name_, false);
}