#pragma once
#include "UndoAction.h"
#include <memory>

class RobotPath;

class UndoInsertPoint : public UndoAction
{
public:
	UndoInsertPoint(int index, std::shared_ptr<RobotPath> path) {
		index_ = index;
		path_ = path;
	}

	void apply() override;

	std::shared_ptr<RobotPath> path() {
		return path_;
	}

private:
	int index_;
	std::shared_ptr<RobotPath> path_;
};

