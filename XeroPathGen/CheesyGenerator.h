#pragma once

#include "PathConstraint.h"
#include "PathTrajectory.h"
#include "Pose2dWithRotation.h"
#include "Pose2dWithTrajectory.h"
#include "SplinePair.h"
#include "DistanceView.h"
#include <QtCore/QVector>

class CheesyGenerator
{
public:
	CheesyGenerator(double diststep, double timestep, double maxdx, double maxdy, double maxtheta);
	virtual ~CheesyGenerator();

	std::shared_ptr<PathTrajectory> generate(const QVector<Pose2dWithRotation>& waypoints, const QVector<std::shared_ptr<PathConstraint>>& constraints,
		double startvel, double endvel, double maxvel, double maxaccel, double maxjerk);

private:
	QVector<std::shared_ptr<SplinePair>> generateSplines(const QVector<Pose2dWithRotation>& points);

	QVector<Pose2dWithTrajectory> timeParameterize(const DistanceView& view, const QVector<std::shared_ptr<PathConstraint>>& constraints,
					double startvel, double endvel, double maxvel, double maxaccel);

	QVector<Pose2dWithTrajectory> convertToUniformTime(const QVector<Pose2dWithTrajectory>& traj, double step);
	size_t findIndex(const QVector<Pose2dWithTrajectory>& traj, double time);

private:
	double maxDx_;
	double maxDy_;
	double maxDTheta_;
	double diststep_;
	double timestep_;
};

