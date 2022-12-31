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

#include <QtCore/QStringList>
#include <memory>

class TrajectoryGroup;

class BasePlotWindow
{
public:
	virtual ~BasePlotWindow() = default;

	virtual void clear() = 0;

	virtual void setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group) {
		group_ = group;
	}

	std::shared_ptr<TrajectoryGroup> group() {
		return group_;
	}

	const QStringList& nodeList() const {
		return nodes_;
	}

	virtual void setNodeList(const QStringList& list) {
		nodes_ = list;
	}

	virtual void clearNodeList() {
		nodes_.clear();
	}

	virtual void addNode(const QString& node) {
		nodes_.push_back(node);
	}

	virtual void removeNode(const QString& node) {
		nodes_.removeAll(node);
	}

protected:
	enum class AxisType
	{
		Distance,
		Speed,
		Acceleration,
		Curvature,
		Angle,
		AngleVelocity,
	};

	AxisType mapVariableToAxis(const QString& var);

private:
	QStringList nodes_;
	std::shared_ptr<TrajectoryGroup> group_;
};

