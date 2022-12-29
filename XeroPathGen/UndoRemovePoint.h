#pragma once
#include "UndoAction.h"
#include "Pose2dWithRotation.h"
#include <memory>

class RobotPath;

class UndoRemovePoint : public UndoAction
{
public:
	UndoRemovePoint(int index, const Pose2dWithRotation& waypoint, std::shared_ptr<RobotPath> path) {
		path_ = path;
		waypoint_ = waypoint;
		index_ = index;
	}

	void apply() override;

	std::shared_ptr<RobotPath> path() {
		return path_;
	}


private:
	int index_;
	Pose2dWithRotation waypoint_;
	std::shared_ptr<RobotPath> path_;
};

