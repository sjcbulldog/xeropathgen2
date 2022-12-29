#pragma once

#include "UndoAction.h"
#include <memory>

class RobotPath;
class PathConstraint;

class UndoDeleteConstraint : public UndoAction
{
public:
	UndoDeleteConstraint(std::shared_ptr<PathConstraint> c, int index, std::shared_ptr<RobotPath> path) {
		path_ = path;
		constraint_ = c;
		index_ = index;
	}

	void apply() override;

	std::shared_ptr<PathConstraint> constraint() {
		return constraint_;
	}

	int index() const {
		return index_;
	}

private:
	std::shared_ptr<RobotPath> path_;
	std::shared_ptr<PathConstraint> constraint_;
	int index_;
};

