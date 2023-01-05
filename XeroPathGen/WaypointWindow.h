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
#include "NoEditDelegate.h"
#include "PathTrajectory.h"
#include <QtWidgets/QTreeWidget>

class WaypointWindow : public QTreeWidget
{
	Q_OBJECT

public:
	WaypointWindow(QWidget* parent);

	bool isSelectedIndex(int index) const {
		return index == index_;
	}

	void setPath(std::shared_ptr<RobotPath> path) {
		if (path_ != path) {
			path_ = path;
			index_ = -1;
		}
	}

	std::shared_ptr<RobotPath> path() {
		return path_;
	}

	void setWaypoint(int index, double dist) {
		index_ = index;
		dist_ = dist;
		refresh();
	}

	int getWaypoint() const {
		return index_;
	}

	void refresh();

private:
	static constexpr const char* IndexTag = "Index";
	static constexpr const char* DistanceTag = "distance";

private:
	QTreeWidgetItem* newItem(const QString& title, bool editable = true);
	void waypointParamChanged(QTreeWidgetItem* item, int column);

private:
	std::shared_ptr<RobotPath> path_;
	int index_;
	double dist_;
};

