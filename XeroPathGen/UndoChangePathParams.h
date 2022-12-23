#pragma once
#include "UndoAction.h"
#include "PathParameters.h"
#include <memory>

class RobotPath;

class UndoChangePathParams : public UndoAction
{
public:
	UndoChangePathParams(const PathParameters& params, std::shared_ptr<RobotPath> path) {
		path_ = path;
		params_ = params;
	}

	void apply() override;

private:
	std::shared_ptr<RobotPath> path_;
	PathParameters params_;
};

