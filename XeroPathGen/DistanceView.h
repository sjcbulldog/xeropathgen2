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


#include "Pose2dWithRotation.h"
#include <QtCore/QVector>

class DistanceView
{
public:
	DistanceView(const QVector<Pose2dWithRotation>& points, double delta);
	double length() const {
		return distances_.back();
	}

	double getPosition(int index) const {
		return distances_[index];
	}

	Pose2dWithRotation operator[](double dist) const;
	Pose2dWithRotation operator[](int index) const;
	int size() const {
		return points_.size();
	}

private:
	QVector<double> distances_;
	QVector<Pose2dWithRotation> points_;
};

