//
// Copyright 2022 Jack W. Griffin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
//
#pragma once

#include "SplinePair.h"
#include "Pose2dWithRotation.h"
#include "Pose2dWithTrajectory.h"
#include "PathConstraint.h"
#include "DistanceView.h"
#include "SwerveWheels.h"
#include "PathTrajectory.h"
#include <QtCore/QVector>
#include <QtCore/QMutex>
#include <memory>

class RobotParams;

class GeneratorBase
{
public:
	GeneratorBase(const QString &logfile, QMutex& loglock, int which, double diststep, double timestep, double maxdx, double maxdy, double maxtheta, std::shared_ptr<RobotParams> robot);

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
	int findIndex(const QVector<Pose2dWithTrajectory>& traj, double time);


	Translation2d getWheelPerpendicularVector(Wheel w, double magnitude);
	int findIndexFromLocation(std::shared_ptr<PathTrajectory> traj, int start, const Translation2d& loc);

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

	bool modifySegmentForRotation(std::shared_ptr<RobotPath> path, std::shared_ptr<PathTrajectory> traj, double percent, int start, int end, double startRot, double startRotVel, double endRot, double endRotVel);
	bool modifyForRotation(std::shared_ptr<RobotPath> path, std::shared_ptr<PathTrajectory> traj, double percent);

	void logMessage(const QString& msg);

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

	const QString &logfile_;
	QMutex &loglock_;
	int which_;
};

