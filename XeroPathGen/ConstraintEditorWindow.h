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

private:
	std::shared_ptr<RobotPath> path_;
};

