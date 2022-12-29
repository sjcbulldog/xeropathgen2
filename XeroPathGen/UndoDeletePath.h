#pragma once
#include "UndoAction.h"
#include <memory>

class PathsDataModel;
class RobotPath;

class UndoDeletePath : public UndoAction
{
public:
	UndoDeletePath(std::shared_ptr<RobotPath> path, int index, PathsDataModel& model) : model_(model) {
		path_ = path;
		index_ = index;
	}

	void apply() override;

	std::shared_ptr<RobotPath> path() {
		return path_;
	}

	int index() {
		return index_;
	}

private:
	std::shared_ptr<RobotPath> path_;
	PathsDataModel& model_;
	int index_;
};

