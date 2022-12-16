#pragma once

#include "RobotPath.h"
#include <QtWidgets/QTreeWidget>

class ConstraintEditorWindow : public QTreeWidget
{
public:
	ConstraintEditorWindow(QWidget* parent);

	std::shared_ptr<RobotPath> path() {
		return path_;
	}

	void setPath(std::shared_ptr<RobotPath> path) {
		path_ = path;
		refresh();
	}

	void refresh();

protected:
	void prepareCustomMenu(const QPoint& pos);

private:
	void addCentripetalConstraint();
	void addDistanceVelocityConstraint();
	void deleteConstraint();

private:
	std::shared_ptr<RobotPath> path_;
	QTreeWidgetItem* menuItem_;
};

