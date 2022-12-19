#pragma once

#include "PathConstraint.h"
#include "PathTrajectory.h"
#include "Pose2dWithRotation.h"
#include "Pose2dWithTrajectory.h"
#include "SplinePair.h"
#include "DistanceView.h"
#include "CheesyGenerator.h"
#include <QtCore/QVector>

class XeroSwerveGenerator : public CheesyGenerator
{
public:
	XeroSwerveGenerator(double diststep, double timestep, double maxdx, double maxdy, double maxtheta, std::shared_ptr<RobotParams> robot);
	virtual ~XeroSwerveGenerator();

	std::shared_ptr<PathTrajectory> generate(std::shared_ptr<RobotPath> path);

private:
	QVector<double> generateDistances(const QVector<std::shared_ptr<SplinePair>> &splines);

	bool isSegmentValid(std::shared_ptr<PathTrajectory> traj, double start, double end);
	void adjustConstraints(QVector<std::shared_ptr<PathConstraint>>& constraints, const QVector<bool>& segvalid);

	double linearToRotational(double value);

private:
	double rot_max_accel_;
	double rot_max_velocity_;
};

