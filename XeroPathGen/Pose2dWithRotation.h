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
	Pose2dWithRotation(const Translation2d& pos, const Rotation2d& rot, const Rotation2d& swrot);
	Pose2dWithRotation(const Pose2d& other);
	Pose2dWithRotation(const Pose2dWithRotation& other);
	virtual ~Pose2dWithRotation();

	const Rotation2d& swrot() const {
		return swrot_;
	}

	void setSwrot(const Rotation2d& swrot) {
		swrot_ = swrot;
	}

private:
	Rotation2d swrot_;
};

