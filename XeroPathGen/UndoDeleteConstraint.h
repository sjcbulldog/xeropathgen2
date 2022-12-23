#pragma once

#include "UndoAction.h"
#include <memory>

class RobotPath;
class PathConstraint;

class UndoDeleteConstraint : public UndoAction
{
public:
	UndoDeleteConstraint(std::shared_ptr<PathConstraint> c, std::shared_ptr<RobotPath> path) {
		path_ = path;
		constraint_ = c;
	}

	void apply() override;

private:
	std::shared_ptr<RobotPath> path_;
	std::shared_ptr<PathConstraint> constraint_;
};

