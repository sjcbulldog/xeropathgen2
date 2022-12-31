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

