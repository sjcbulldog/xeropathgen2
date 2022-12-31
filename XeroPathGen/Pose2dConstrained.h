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
#include "Pose2dWithTrajectory.h"

class Pose2dConstrained : public Pose2dWithTrajectory
{
public:
	Pose2dConstrained() {
		accel_min_ = std::numeric_limits<double>::min();
		accel_max_ = std::numeric_limits<double>::max();
	}

	virtual ~Pose2dConstrained() {
	}

	double accelMin() const {
		return accel_min_;
	}

	double accelMax() const {
		return accel_max_;
	}

	void setAccelMin(double d) {
		accel_min_ = d;
	}

	void setAccelMax(double d) {
		accel_max_ = d;
	}

private:
	double accel_min_;
	double accel_max_;
};

