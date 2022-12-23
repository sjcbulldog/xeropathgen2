#include "TrajectoryUtils.h"
#include "RobotPath.h"

QVector<Pose2dWithRotation> TrajectoryUtils::parameterize(const QVector<std::shared_ptr<SplinePair>>& splines,
	double maxDx, double maxDy, double maxDTheta)
{
	QVector<Pose2dWithRotation> results;

	results.push_back(splines[0]->getStartPose());
	for (int i = 0; i < splines.size(); i++)
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

void TrajectoryUtils::computeCurvature(std::shared_ptr<PathTrajectory> traj)
{
	double curv;

	auto pts = *traj;
	for (int i = 0; i < traj->size(); i++) {
		if (i == 0 || i == traj->size() - 1)
		{
			curv = 0.0;
		}
		else
		{
			curv = Pose2dWithRotation::curvature(pts[i - 1].pose(), pts[i].pose(), pts[i + 1].pose());
			pts[i].pose().setCurvature(curv);
		}
	}
}


void TrajectoryUtils::computeCurvature(QVector<Pose2dWithRotation>& pts)
{
	double curv;

	for (int i = 0; i < pts.size(); i++) {
		if (i == 0 || i == pts.size() - 1)
		{
			curv = 0.0;
		}
		else
		{
			curv = Pose2dWithRotation::curvature(pts[i - 1], pts[i], pts[i + 1]);
			pts[i].setCurvature(curv);
		}
	}
}

QVector<double> TrajectoryUtils::getDistancesForSplines(const QVector<std::shared_ptr<SplinePair>>& splines)
{
	QVector<double> dists;

	if (splines.length() > 0) {
		int steps = 10000;
		double dist = 0;

		dists.push_back(0.0);
		for (int i = 0; i < splines.size(); i++)
		{
			auto pair = splines[i];
			bool first = true;
			Translation2d pos, prevpos;
			for (float t = 0.0f; t <= 1.0f; t += 1.0f / steps)
			{
				pos = pair->evalPosition(t);
				if (first)
					first = false;
				else
				{
					dist += pos.distance(prevpos);
				}

				prevpos = pos;
			}

			dists.push_back(dist);
		}
	}
	return dists;
}
