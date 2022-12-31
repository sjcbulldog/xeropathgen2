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
#include "RobotParams.h"
#include "PathTrajectory.h"
#include <QtCore/QVector>

class TrajectoryUtils
{
public:
	TrajectoryUtils() = delete;
	~TrajectoryUtils() = delete;

	static QVector<Pose2dWithRotation> parameterize(const QVector<std::shared_ptr<SplinePair>>& splines,
		double maxDx, double maxDy, double maxDTheta);

	static double linearToRotational(std::shared_ptr<RobotParams> robot, double v);
	static double rotationalToLinear(std::shared_ptr<RobotParams> robot, double v);

	static void computeCurvature(std::shared_ptr<PathTrajectory> traj);

	static void computeCurvature(QVector<Pose2dWithRotation>& points);

	static QVector<double> getDistancesForSplines(const QVector<std::shared_ptr<SplinePair>>& splines);

private:
	static void getSegmentArc(std::shared_ptr<SplinePair> pair, QVector<Pose2dWithRotation>& results,
		double t0, double t1, double maxDx, double maxDy, double maxDTheta);
};

