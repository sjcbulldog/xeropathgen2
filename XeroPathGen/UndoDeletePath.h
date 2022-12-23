#pragma once
#include "UndoAction.h"
#include <memory>

class PathsDataModel;
class RobotPath;

class UndoDeletePath : public UndoAction
{
public:
	UndoDeletePath(std::shared_ptr<RobotPath> path, PathsDataModel& model) : model_(model) {
		path_ = path;
	}

	void apply() override;

private:
	std::shared_ptr<RobotPath> path_;
	PathsDataModel& model_;
};

