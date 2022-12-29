#include "UndoDeleteConstraint.h"
#include "RobotPath.h"

void UndoDeleteConstraint::apply()
{
	path_->insertConstraint(constraint_, index_);
}