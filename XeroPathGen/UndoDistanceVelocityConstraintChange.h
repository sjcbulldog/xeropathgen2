#pragma once
#include "UndoAction.h"
#include <memory>

class DistanceVelocityConstraint;

class UndoDistanceVelocityConstraintChange : public UndoAction
{
public:
	UndoDistanceVelocityConstraintChange(double velocity, double after, double before, std::shared_ptr< DistanceVelocityConstraint> c) {
		constraint_ = c;
		before_ = before;
		after_ = after;
		velocity_ = velocity;
	}

	void apply() override;

	std::shared_ptr<DistanceVelocityConstraint> constraint() {
		return constraint_;
	}


private:
	std::shared_ptr<DistanceVelocityConstraint> constraint_;
	double before_;
	double after_;
	double velocity_;
};

