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

#include "Pose2dWithTrajectory.h"
#include <QtCore/QVector>
#include <QtCore/QString>

class PathTrajectory
{
public:
	PathTrajectory(const QString &name, const QVector<Pose2dWithTrajectory>& pts) {
		name_ = name;
		points_ = pts;
	}
	typedef QVector<Pose2dWithTrajectory>::iterator iterator;
	typedef QVector<Pose2dWithTrajectory>::const_iterator const_iterator;

	iterator begin() {
		return points_.begin();
	}

	iterator end() {
		return points_.end();
	}

	const_iterator begin() const {
		return points_.begin();
	}

	const_iterator end() const {
		return points_.end();
	}

	int size() const {
		return points_.size();
	}

	const Pose2dWithTrajectory& operator[](int index) const {
		return points_[index];
	}

	Pose2dWithTrajectory& operator[](int index) {
		return points_[index];
	}

	const QString& name() const {
		return name_;
	}

	int getIndex(double time);

	bool getTimeForDistance(double dist, double& time);

	double getEndTime() const {
		if (points_.size() == 0)
			return 0.0;

		return points_[points_.size() - 1].time();
	}

	double getEndDistance() const {
		if (points_.size() == 0)
			return 0.0;

		return points_[points_.size() - 1].position();
	}

	double getDistance(int index);

private:
	QString name_;
	QVector<Pose2dWithTrajectory> points_;
	QVector<double> distances_;
};