#pragma once
#include "UndoAction.h"
#include <memory>

class RobotPath;
class PathsDataModel;

class UndoAddPath : public UndoAction
{
public:
	UndoAddPath(std::shared_ptr<RobotPath> path, PathsDataModel& model) : model_(model) {
		path_ = path;
	}

	void apply() override;

	std::shared_ptr<RobotPath> path() {
		return path_;
	}

private:
	PathsDataModel& model_;
	std::shared_ptr<RobotPath> path_;
};
