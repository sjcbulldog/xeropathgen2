#pragma once
#include "UndoAction.h"
#include <memory>

class RobotPath;
class PathConstraint;

class UndoAddConstraint : public UndoAction
{
public:
	UndoAddConstraint(std::shared_ptr<PathConstraint> c, std::shared_ptr<RobotPath> path) {
		path_ = path;
		constraint_ = c;
	}

	void apply() override;

	std::shared_ptr<PathConstraint> getConstraint() {
		return constraint_;
	}

private:
	std::shared_ptr<RobotPath> path_;
	std::shared_ptr<PathConstraint> constraint_;
};

