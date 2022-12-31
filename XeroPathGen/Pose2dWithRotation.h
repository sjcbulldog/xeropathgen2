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

#include "Pose2d.h"

class Pose2dWithRotation : public Pose2d
{
public:
	Pose2dWithRotation();
	Pose2dWithRotation(double x, double y);
	Pose2dWithRotation(double x, double y, const Rotation2d& rot);
	Pose2dWithRotation(const Translation2d& pos);
	Pose2dWithRotation(const Rotation2d& pos);
	Pose2dWithRotation(const Translation2d& pos, const Rotation2d& rot);
	Pose2dWithRotation(const Translation2d& pos, const Rotation2d& rot, const Rotation2d& swrot, double cur = 0.0);
	Pose2dWithRotation(const Pose2d& other);
	Pose2dWithRotation(const Pose2dWithRotation& other);
	virtual ~Pose2dWithRotation();

	Pose2dWithRotation interpolate(const Pose2dWithRotation& other, double percent) const;

	const Rotation2d& getSwrot() const {
		return swrot_;
	}

	void setSwrot(const Rotation2d& swrot) {
		swrot_ = swrot;
	}

	double getSwrotVelocity() const {
		return rotvel_;
	}

	void setRotVelocity(double v) {
		rotvel_ = v;
	}

	void setCurvature(double d) {
		curvature_ = d;
	}

	double curvature() const {
		return curvature_;
	}

	static double triangleArea(const Pose2dWithRotation& a, const Pose2dWithRotation& b, const Pose2dWithRotation& c)
	{
		return ((b.getTranslation().getX() - a.getTranslation().getX()) * (c.getTranslation().getY() - a.getTranslation().getY())) -
			((b.getTranslation().getY() - a.getTranslation().getY()) * (c.getTranslation().getX() - a.getTranslation().getX()));
	}

	static double curvature(const Pose2dWithRotation& a, const Pose2dWithRotation& b, const Pose2dWithRotation& c)
	{
		double area = triangleArea(a, b, c);

		double len1 = a.distance(b);
		double len2 = b.distance(c);
		double len3 = a.distance(c);

		return 4.0 * area / (len1 * len2 * len3);
	}

private:
	Rotation2d swrot_;				// The rotation at a given waypoint
	double rotvel_;					// The rotational velocity at a given waypoint
	double curvature_;				// The computed curvature at a given waypoint
};

