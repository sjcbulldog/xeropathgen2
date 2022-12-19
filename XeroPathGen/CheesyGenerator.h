#pragma once

#include "PathConstraint.h"
#include "PathTrajectory.h"
#include "Pose2dWithRotation.h"
#include "Pose2dWithTrajectory.h"
#include "SplinePair.h"
#include "DistanceView.h"
#include "SwerveWheels.h"
#include <QtCore/QVector>

class CheesyGenerator
{
public:
	CheesyGenerator(double diststep, double timestep, double maxdx, double maxdy, double maxtheta, std::shared_ptr<RobotParams> robot);
	virtual ~CheesyGenerator();

	std::shared_ptr<PathTrajectory> generate(std::shared_ptr<RobotPath> path);

protected:
	QVector<std::shared_ptr<SplinePair>> generateSplines(const QVector<Pose2dWithRotation>& points);

	QVector<Pose2dWithTrajectory> timeParameterize(const DistanceView& view, const QVector<std::shared_ptr<PathConstraint>>& constraints,
					double startvel, double endvel, double maxvel, double maxaccel);

	QVector<Pose2dWithTrajectory> convertToUniformTime(const QVector<Pose2dWithTrajectory>& traj, double step);
	size_t findIndex(const QVector<Pose2dWithTrajectory>& traj, double time);

	double getMaxDx() const { return maxDx_; }
	double getMaxDy() const { return maxDy_; }
	double getMaxDTheta() const { return maxDTheta_; }
	double getDistStep() const { return diststep_; }
	double getTimeStep() const { return timestep_; }
	std::shared_ptr<RobotParams> robot() const { return robot_; }

	std::shared_ptr<PathTrajectory> generateInternal(std::shared_ptr<RobotPath> path, double maxvel);
	std::shared_ptr<PathTrajectory> generateSwerve(std::shared_ptr<RobotPath> path);

	Translation2d getWheelPerpendicularVector(Wheel w, double magnitude);
	size_t findIndexFromLocation(std::shared_ptr<PathTrajectory> traj, size_t start, const Translation2d& loc);
	QVector<QPair<size_t, double>> getRotationTransitions(std::shared_ptr<RobotPath> path, std::shared_ptr<PathTrajectory> traj);
	bool modifyForRotation(std::shared_ptr<RobotPath> path, std::shared_ptr<PathTrajectory> traj, double percent);

private:
	double maxDx_;
	double maxDy_;
	double maxDTheta_;
	double diststep_;
	double timestep_;
	std::shared_ptr<RobotParams> robot_;

	double robot_max_accel_;
	double robot_max_velocity_;
	double robot_width_;
	double robot_length_;
};

