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

#include "GeneratorBase.h"
#include "PathConstraint.h"
#include "PathTrajectory.h"
#include "Pose2dWithRotation.h"
#include "Pose2dWithTrajectory.h"
#include "SplinePair.h"
#include "DistanceView.h"
#include "SwerveWheels.h"
#include <QtCore/QVector>

class CheesyGenerator : public GeneratorBase
{
public:
	CheesyGenerator(const QString &logfile, QMutex &loglock, int which, double diststep, double timestep, double maxdx, double maxdy, double maxtheta, std::shared_ptr<RobotParams> robot, bool xeromod);
	virtual ~CheesyGenerator();

	std::shared_ptr<PathTrajectory> generate(std::shared_ptr<RobotPath> path);

protected:
	std::shared_ptr<PathTrajectory> generateSwerveSingleRotate(std::shared_ptr<RobotPath> path);
	std::shared_ptr<PathTrajectory> generateSwervePerWaypointRotate(std::shared_ptr<RobotPath> path);
	std::shared_ptr<PathTrajectory> generateTankDrive(std::shared_ptr<RobotPath> path);

private:
	bool xeromode_;
};

