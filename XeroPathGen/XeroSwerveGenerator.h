#pragma once

#include "PathConstraint.h"
#include "PathTrajectory.h"
#include "Pose2dWithRotation.h"
#include "Pose2dWithTrajectory.h"
#include "SplinePair.h"
#include "DistanceView.h"
#include <QtCore/QVector>

class XeroSwerveGenerator
{
public:
	XeroSwerveGenerator(double diststep, double timestep, double maxdx, double maxdy, double maxtheta, std::shared_ptr<RobotParams> robot);
	virtual ~XeroSwerveGenerator();

	std::shared_ptr<PathTrajectory> generate(const QVector<Pose2dWithRotation>& waypoints, const QVector<std::shared_ptr<PathConstraint>>& constraints,
		double startvel, double endvel, double maxvel, double maxaccel, double maxjerk);
};

