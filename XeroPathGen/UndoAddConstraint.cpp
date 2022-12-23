#include "UndoAddConstraint.h"
#include "RobotPath.h"

void UndoAddConstraint::apply() 
{
	path_->deleteConstraint(constraint_, false);
}