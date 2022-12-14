#pragma once

#include "RobotPath.h"
#include "NoEditDelegate.h"
#include <QtWidgets/QTreeWidget>

class WaypointWindow : public QTreeWidget
{
	Q_OBJECT

public:
	WaypointWindow(QWidget* parent);

	void setPath(std::shared_ptr<RobotPath> path) {
		path_ = path;
	}

	void setWaypoint(int index) {
		index_ = index;
	}

	void refresh();

private:
	static constexpr const char* XTag = "X";
	static constexpr const char* YTag = "Y";
	static constexpr const char* HeadingTag = "Heading";
	static constexpr const char* RotationTag = "Rotation";
	static constexpr const char* IndexTag = "Index";

private:
	QTreeWidgetItem* newItem(const QString& title, bool editable = true);
	void waypointParamChanged(QTreeWidgetItem* item, int column);

private:
	std::shared_ptr<RobotPath> path_;
	int index_;
};

