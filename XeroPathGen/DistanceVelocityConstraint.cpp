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
#include "DistanceVelocityConstraint.h"

DistanceVelocityConstraint::DistanceVelocityConstraint(std::shared_ptr<RobotPath> path, double after, double before, double velocity) : PathConstraint(path)
{
	after_distance_ = after;
	before_distance_ = before;
	velocity_ = velocity;
}

DistanceVelocityConstraint::~DistanceVelocityConstraint()
{
}

std::shared_ptr<PathConstraint> DistanceVelocityConstraint::clone(std::shared_ptr<RobotPath> tpath)
{
	if (tpath == nullptr) {
		tpath = path();
	}

	auto ret = std::make_shared<DistanceVelocityConstraint>(tpath, after_distance_, before_distance_, velocity_);
	return ret;
}

double DistanceVelocityConstraint::getMaxVelocity(const Pose2dWithTrajectory& state, std::shared_ptr<RobotParams> robot)
{
	if (state.position() > after_distance_ && state.position() < before_distance_)
		return velocity_;

	return std::numeric_limits<double>::max();
}

MinMaxAcceleration DistanceVelocityConstraint::getMinMaxAccel(const Pose2dWithTrajectory& state, double velocity, std::shared_ptr<RobotParams> robot)
{
	(void)velocity;
	(void)state;
	return MinMaxAcceleration();
}
