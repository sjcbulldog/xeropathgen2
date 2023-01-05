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

#include "MinMaxAcceleration.h"
#include "Pose2dWithTrajectory.h"
#include "RobotParams.h"
#include <QtCore/QJsonObject>
#include <memory>

class RobotPath;

class PathConstraint
{
public:
	PathConstraint(std::shared_ptr<RobotPath> path) {
		path_ = path;
	}

	virtual ~PathConstraint() {
	}

	virtual double getMaxVelocity(const Pose2dWithTrajectory& state, std::shared_ptr<RobotParams> robot) = 0;
	virtual MinMaxAcceleration getMinMaxAccel(const Pose2dWithTrajectory& state, double velocity, std::shared_ptr<RobotParams> robot) = 0;
	virtual void convert(const QString& from, const QString &to) = 0;
	virtual QJsonObject toJSON() const = 0;
	virtual QString toString() const = 0;

	std::shared_ptr<RobotPath> path() const {
		return path_;
	}

private:
	std::shared_ptr<RobotPath> path_;
};

