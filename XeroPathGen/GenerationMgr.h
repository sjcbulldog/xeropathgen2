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
#include "Generator.h"
#include "TrajectoryGroup.h"
#include "RobotParams.h"
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QThread>

class GenerationMgr : public QObject
{
	Q_OBJECT

public:
	GenerationMgr();

	void setTimestep(double t) {
		timestep_ = t;
	}

	double timestep() const {
		return timestep_;
	}

	void setRobot(std::shared_ptr<RobotParams> robot) {
		robot_ = robot;
	}

	void addPath(GeneratorType type, std::shared_ptr<RobotPath> path);
	void removePath(std::shared_ptr<RobotPath> path);

	std::shared_ptr<TrajectoryGroup> getTrajectoryGroup(std::shared_ptr<RobotPath> path);

	bool isEmpty() {
		bool ret = true;
		pending_queue_mutex_.lock();
		ret = pending_queue_.size() == 0 && worker_ == nullptr;
		pending_queue_mutex_.unlock();
		return ret;
	}

	void clear();

	void removeAllTrajectories() {
		trajectory_group_mutex_.lock();
		trajectories_.clear();
		trajectory_group_mutex_.unlock();
	}

signals:
	void generationComplete(std::shared_ptr<RobotPath> path);

private:
	void schedulePath();
	void pathFinished(std::shared_ptr<TrajectoryGroup> path);

private:
	QMutex pending_queue_mutex_;
	QList<QPair<GeneratorType, std::shared_ptr<RobotPath>>> pending_queue_;

	QMutex trajectory_group_mutex_;
	QMap<std::shared_ptr<RobotPath>, std::shared_ptr<TrajectoryGroup>> trajectories_;

	QMutex active_queue_mutex_;
	GeneratorType active_type_;
	std::shared_ptr<RobotPath> active_path_;

	Generator* worker_;
	QThread *thread_;

	std::shared_ptr<RobotParams> robot_;
	double timestep_;

	QString logfile_;
	QMutex loglock_;
};
