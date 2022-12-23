#pragma once
#include "UndoAction.h"
#include <memory>

class CentripetalConstraint;

class UndoChangeCentripetalForceConstraint : public UndoAction
{
public:
	UndoChangeCentripetalForceConstraint(double maxcen, std::shared_ptr<CentripetalConstraint> c) {
		constraint_ = c;
		maxcen_ = maxcen;
	}

	void apply() override;

private:
	double maxcen_;
	std::shared_ptr<CentripetalConstraint> constraint_;
};

