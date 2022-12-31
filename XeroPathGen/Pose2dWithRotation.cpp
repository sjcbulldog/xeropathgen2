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
#include "Pose2dWithRotation.h"


Pose2dWithRotation::Pose2dWithRotation()
{
	curvature_ = 0.0;
	rotvel_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(double x, double y) : Pose2d(x, y)
{
	curvature_ = 0.0;
	rotvel_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(double x, double y, const Rotation2d& rot) : Pose2d(x, y, rot)
{
	curvature_ = 0.0;
	rotvel_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(const Translation2d& pos) : Pose2d(pos)
{
	curvature_ = 0.0;
	rotvel_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(const Rotation2d& rot) : Pose2d(rot)
{
	curvature_ = 0.0;
	rotvel_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(const Translation2d& pos, const Rotation2d& rot) : Pose2d(pos, rot)
{
	curvature_ = 0.0;
	rotvel_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(const Translation2d& pos, const Rotation2d& rot, const Rotation2d& swrot, double cur) : Pose2d(pos, rot)
{
	swrot_ = swrot;
	curvature_ = cur;
	rotvel_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(const Pose2d& other) : Pose2d(other)
{
	curvature_ = 0.0;
	rotvel_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(const Pose2dWithRotation& other) : Pose2d(other)
{
	swrot_ = other.swrot_;
	rotvel_ = other.rotvel_;
	curvature_ = 0.0;
}

Pose2dWithRotation::~Pose2dWithRotation()
{
}

Pose2dWithRotation Pose2dWithRotation::interpolate(const Pose2dWithRotation& other, double percent) const
{
	Translation2d t2d = getTranslation().interpolate(other.getTranslation(), percent);
	Rotation2d r2d = getRotation().interpolate(other.getRotation(), percent);
	Rotation2d sw = getSwrot().interpolate(other.getSwrot(), percent);
	double ncurv = (other.curvature() - curvature()) * percent + curvature();

	return Pose2dWithRotation(t2d, r2d, sw, ncurv);
}