#pragma once

#include "RobotPath.h"
#include "PathTrajectory.h"
#include <QtWidgets/QTreeWidget>

class PathParametersWindow : public QTreeWidget
{
	Q_OBJECT

public:
	PathParametersWindow(QWidget* widget);

	std::shared_ptr<RobotPath> path() {
		return path_;
	}

	void setPath(std::shared_ptr<RobotPath> path) {
		path_ = path;
		refresh();
	}

	void setTrajectory(std::shared_ptr<PathTrajectory> traj);

	void refresh();

private:
	QTreeWidgetItem* newItem(const QString& title, bool editable = true);
	void pathParamChanged(QTreeWidgetItem* item, int column);

private:
	static constexpr const char* StartVelocityTag = "Start Velocity";
	static constexpr const char* EndVelocityTag = "End Velocity";
	static constexpr const char* MaxVelocityTag = "Max Velocity";
	static constexpr const char* MaxAccelerationTag = "Max Acceleration";
	static constexpr const char* LengthTag = "Length";
	static constexpr const char* DurationTag = "Duration";

private:
	std::shared_ptr<RobotPath> path_;

	QTreeWidgetItem *length_item_;
	QTreeWidgetItem *duration_item_;
};

