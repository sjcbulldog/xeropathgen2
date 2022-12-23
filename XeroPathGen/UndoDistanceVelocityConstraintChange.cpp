#include "UndoDistanceVelocityConstraintChange.h"
#include "DistanceVelocityConstraint.h"

void UndoDistanceVelocityConstraintChange::apply() {
	constraint_->setVelocity(velocity_, false);
	constraint_->setBefore(before_, false);
	constraint_->setAfter(after_, false);
}