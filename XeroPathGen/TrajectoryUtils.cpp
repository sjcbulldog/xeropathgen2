#include "TrajectoryUtils.h"

QVector<Pose2dWithRotation> TrajectoryUtils::parameterize(const QVector<std::shared_ptr<SplinePair>>& splines,
	double maxDx, double maxDy, double maxDTheta)
{
	QVector<Pose2dWithRotation> results;

	results.push_back(splines[0]->getStartPose());
	for (size_t i = 0; i < splines.size(); i++)
		getSegmentArc(splines[i], results, 0.0, 1.0, maxDx, maxDy, maxDTheta);

	return results;
}

void TrajectoryUtils::getSegmentArc(std::shared_ptr<SplinePair> pair, QVector<Pose2dWithRotation>& results,
	double t0, double t1, double maxDx, double maxDy, double maxDTheta)
{
	const Translation2d& p0 = pair->evalPosition(t0);
	const Translation2d& p1 = pair->evalPosition(t1);
	const Rotation2d& r0 = pair->evalHeading(t0);
	const Rotation2d& r1 = pair->evalHeading(t1);
	Pose2d transformation = Pose2d(Translation2d(p0, p1).rotateBy(r0.inverse()), r1.rotateBy(r0.inverse()));
	Twist2d twist = Pose2d::logfn(transformation);
	if (twist.getY() > maxDy || twist.getX() > maxDx || twist.getTheta() > maxDTheta) {
		getSegmentArc(pair, results, t0, (t0 + t1) / 2, maxDx, maxDy, maxDTheta);
		getSegmentArc(pair, results, (t0 + t1) / 2, t1, maxDx, maxDy, maxDTheta);
	}
	else {
		results.push_back(pair->evalPose(t1));
	}
}

double TrajectoryUtils::rotationalToLinear(std::shared_ptr<RobotParams> robot, double rv)
{
	double diameter = std::sqrt(robot->getWheelBaseWidth() * robot->getWheelBaseWidth() + robot->getWheelBaseLength() * robot->getWheelBaseLength());
	double circum = diameter * MathUtils::kPI;
	return rv * circum / 360.0;
}

double TrajectoryUtils::linearToRotational(std::shared_ptr<RobotParams> robot, double gr)
{
	double diameter = std::sqrt(robot->getWheelBaseWidth() * robot->getWheelBaseWidth() + robot->getWheelBaseLength() * robot->getWheelBaseLength());
	double circum = diameter * MathUtils::kPI;
	return gr * 360.0 / circum;
}
