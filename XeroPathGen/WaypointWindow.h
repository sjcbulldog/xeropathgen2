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
	static constexpr const char* XTag = "X";
	static constexpr const char* YTag = "Y";
	static constexpr const char* HeadingTag = "Heading";
	static constexpr const char* RotationTag = "Rotation";
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

