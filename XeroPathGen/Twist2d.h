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

/// \brief This class represents a location and an angle
class Twist2d
{
public:
	Twist2d();
	Twist2d(double dx, double dy, double dtheta);
	~Twist2d();

	double getX() const {
		return dx_;
	}

	double getY() const {
		return dy_;
	}

	double getTheta() const {
		return dtheta_;
	}

	static Twist2d kIdentity;

	Twist2d scaled(double scale) const;
	double normal() const;
	double curvature() const;

private:
	double dx_;
	double dy_;
	double dtheta_;
};

