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
