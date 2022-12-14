#include "Pose2dWithRotation.h"


Pose2dWithRotation::Pose2dWithRotation()
{
}

Pose2dWithRotation::Pose2dWithRotation(double x, double y) : Pose2d(x, y)
{
}

Pose2dWithRotation::Pose2dWithRotation(double x, double y, const Rotation2d& rot) : Pose2d(x, y, rot)
{

}

Pose2dWithRotation::Pose2dWithRotation(const Translation2d& pos) : Pose2d(pos)
{

}

Pose2dWithRotation::Pose2dWithRotation(const Rotation2d& rot) : Pose2d(rot)
{

}

Pose2dWithRotation::Pose2dWithRotation(const Translation2d& pos, const Rotation2d& rot) : Pose2d(pos, rot)
{

}

Pose2dWithRotation::Pose2dWithRotation(const Translation2d& pos, const Rotation2d& rot, const Rotation2d& swrot) : Pose2d(pos, rot)
{
	swrot_ = swrot;
}

Pose2dWithRotation::Pose2dWithRotation(const Pose2d& other) : Pose2d(other)
{
}

Pose2dWithRotation::Pose2dWithRotation(const Pose2dWithRotation& other) : Pose2d(other)
{
	swrot_ = other.swrot_;
}

Pose2dWithRotation::~Pose2dWithRotation()
{
}