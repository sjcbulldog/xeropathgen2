#pragma once

#include "SplinePair.h"
#include "Pose2dWithRotation.h"
#include "Pose2dWithTrajectory.h"
#include "PathConstraint.h"
#include "DistanceView.h"
#include "SwerveWheels.h"
#include "PathTrajectory.h"
#include <QtCore/QVector>
#include <memory>

class RobotParams;

class GeneratorBase
{
public:
	GeneratorBase(double diststep, double timestep, double maxdx, double maxdy, double maxtheta, std::shared_ptr<RobotParams> robot);

	std::shared_ptr<RobotParams> robot() {
		return robot_;
	}

protected:
	double getMaxDx() const { return maxDx_; }
	double getMaxDy() const { return maxDy_; }
	double getMaxDTheta() const { return maxDTheta_; }
	double getDistStep() const { return diststep_; }
	double getTimeStep() const { return timestep_; }

	std::shared_ptr<PathTrajectory> generateInternal(std::shared_ptr<RobotPath> path, QVector<std::shared_ptr<PathConstraint>>& extras);
	void computeRobotParameters(std::shared_ptr<RobotPath> path);

	QVector<std::shared_ptr<SplinePair>> generateSplines(const QVector<Pose2dWithRotation>& points);

	QVector<Pose2dWithTrajectory> timeParameterize(const DistanceView& view, const QVector<std::shared_ptr<PathConstraint>>& constraints,
		double startvel, double endvel, double maxvel, double maxaccel);

	QVector<Pose2dWithTrajectory> convertToUniformTime(const QVector<Pose2dWithTrajectory>& traj, double step);
	size_t findIndex(const QVector<Pose2dWithTrajectory>& traj, double time);


	Translation2d getWheelPerpendicularVector(Wheel w, double magnitude);
	size_t findIndexFromLocation(std::shared_ptr<PathTrajectory> traj, size_t start, const Translation2d& loc);

	double robotMaxAccel() const {
		return robot_max_accel_;
	}

	double robotMaxVelocity() const {
		return robot_max_velocity_;
	}

	double robotWidth() const {
		return robot_width_;
	}

	double robotLength() const {
		return robot_length_;
	}

	bool modifySegmentForRotation(std::shared_ptr<RobotPath> path, std::shared_ptr<PathTrajectory> traj, double percent, int start, int end, double startRot, double endRot);
	bool modifyForRotation(std::shared_ptr<RobotPath> path, std::shared_ptr<PathTrajectory> traj, double percent);

private:
	std::shared_ptr<RobotParams> robot_;

	double robot_max_accel_;
	double robot_max_velocity_;
	double robot_width_;
	double robot_length_;

	double maxDx_;
	double maxDy_;
	double maxDTheta_;
	double diststep_;
	double timestep_;
};

