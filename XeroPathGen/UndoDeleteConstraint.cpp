#include "UndoDeleteConstraint.h"
#include "RobotPath.h"

void UndoDeleteConstraint::apply()
{
	path_->addConstraint(constraint_, false);
}