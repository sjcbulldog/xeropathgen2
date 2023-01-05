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
#include <limits>

class MinMaxAcceleration
{
public:
	MinMaxAcceleration() {
		maxaccel_ = std::numeric_limits<double>::max();
		minaccel_ = -std::numeric_limits<double>::max();
	}

	MinMaxAcceleration(double minacc, double maxacc)
	{
		minaccel_ = minacc;
		maxaccel_ = maxacc;
	}

	virtual ~MinMaxAcceleration() {
	}

	double getMinAccel() const {
		return minaccel_;
	}

	double getMaxAccel() const {
		return maxaccel_;
	}

	bool isValid() const {
		return minaccel_ < maxaccel_;
	}

private:
	double maxaccel_;
	double minaccel_;
};
