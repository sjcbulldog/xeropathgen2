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

#include "RobotPath.h"
#include "GeneratorType.h"
#include "TrajectoryGroup.h"
#include "RobotParams.h"
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <memory>

class Generator : public QObject
{
	Q_OBJECT

public:
	Generator(const QString& logfile, QMutex& mutex, double timestep, std::shared_ptr<RobotParams> robot, std::shared_ptr<TrajectoryGroup> group);

	void generateTrajectory();

signals:
	void trajectoryComplete(std::shared_ptr<TrajectoryGroup> group);

private:
	void addTankDriveTrajectories();

private:
	int which_;
	double timestep_;
	std::shared_ptr<TrajectoryGroup> group_;
	std::shared_ptr<RobotParams> robot_;

	const QString& logfile_;
	QMutex& loglock_;

	static int global_which_;
};