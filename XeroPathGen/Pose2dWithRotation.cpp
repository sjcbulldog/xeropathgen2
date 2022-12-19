#include "Pose2dWithRotation.h"


Pose2dWithRotation::Pose2dWithRotation()
{
	curvature_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(double x, double y) : Pose2d(x, y)
{
	curvature_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(double x, double y, const Rotation2d& rot) : Pose2d(x, y, rot)
{
	curvature_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(const Translation2d& pos) : Pose2d(pos)
{
	curvature_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(const Rotation2d& rot) : Pose2d(rot)
{
	curvature_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(const Translation2d& pos, const Rotation2d& rot) : Pose2d(pos, rot)
{
	curvature_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(const Translation2d& pos, const Rotation2d& rot, const Rotation2d& swrot, double cur) : Pose2d(pos, rot)
{
	swrot_ = swrot;
	curvature_ = cur;
}

Pose2dWithRotation::Pose2dWithRotation(const Pose2d& other) : Pose2d(other)
{
	curvature_ = 0.0;
}

Pose2dWithRotation::Pose2dWithRotation(const Pose2dWithRotation& other) : Pose2d(other)
{
	swrot_ = other.swrot_;
	curvature_ = 0.0;
}

Pose2dWithRotation::~Pose2dWithRotation()
{
}

Pose2dWithRotation Pose2dWithRotation::interpolate(const Pose2dWithRotation& other, double percent) const
{
	Translation2d t2d = getTranslation().interpolate(other.getTranslation(), percent);
	Rotation2d r2d = getRotation().interpolate(other.getRotation(), percent);
	Rotation2d sw = swrot().interpolate(other.swrot(), percent);
	double ncurv = (other.curvature() - curvature()) * percent + curvature();

	return Pose2dWithRotation(t2d, r2d, sw, ncurv);
}