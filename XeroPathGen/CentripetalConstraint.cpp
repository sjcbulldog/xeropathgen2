#include "CentripetalConstraint.h"
#include "UndoChangeCentripetalForceConstraint.h"

void CentripetalConstraint::setMaxCenForce(double c, bool undoentry) {
	if (undoentry) {
		path()->beforeConstraintChanged(std::make_shared<UndoChangeCentripetalForceConstraint>(maxcen_, shared_from_this()));
	}
	maxcen_ = c;
	path()->afterConstraintChanged();
}