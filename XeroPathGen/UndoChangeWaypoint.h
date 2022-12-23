#pragma once
#include "UndoAction.h"
#include "Pose2dWithRotation.h"
#include <memory>

class RobotPath;

class UndoChangeWaypoint : public UndoAction
{
public:
	UndoChangeWaypoint(int index, const Pose2dWithRotation& waypoint, std::shared_ptr<RobotPath> path) {
		path_ = path;
		index_ = index;
		waypoint_ = waypoint;
	}

	void apply() override;

private:
	std::shared_ptr<RobotPath> path_;
	int index_;
	Pose2dWithRotation waypoint_;
};

