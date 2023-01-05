//
// Copyright 2022 Jack W. Griffin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
//
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

