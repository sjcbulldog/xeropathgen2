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
#include "Twist2d.h"
#include "MathUtils.h"
#include <cmath>

Twist2d Twist2d::kIdentity(0.0, 0.0, 0.0);

Twist2d::Twist2d()
{
	dx_ = 0.0;
	dy_ = 0.0;
	dtheta_ = 0.0;
}

Twist2d::Twist2d(double dx, double dy, double dtheta)
{
	dx_ = dx;
	dy_ = dy;
	dtheta_ = dtheta;
}

Twist2d::~Twist2d()
{
}

Twist2d Twist2d::scaled(double scale) const
{
	return Twist2d(dx_ * scale, dy_ * scale, dtheta_ * scale);
}

double Twist2d::normal() const
{
	if (dy_ == 0.0)
		return std::abs(dx_);

	return std::hypot(dx_, dy_);
}

double Twist2d::curvature() const
{
	double dn = normal();

	if (std::abs(dtheta_) < MathUtils::kEpsilon && dn < MathUtils::kEpsilon)
		return 0.0;

	return dtheta_ / dn;
}
