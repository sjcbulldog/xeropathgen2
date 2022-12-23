#include "UndoChangeCentripetalForceConstraint.h"
#include "CentripetalConstraint.h"

void UndoChangeCentripetalForceConstraint::apply()
{
	constraint_->setMaxCenForce(maxcen_, false);
}