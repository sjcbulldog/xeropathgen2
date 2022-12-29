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

	void deleteConstraintFromDisplay(std::shared_ptr<PathConstraint> c);
	void updateConstraintInDisplay(void *);
	void insertConstraint(std::shared_ptr<PathConstraint> c, int index);

protected:
	void prepareCustomMenu(const QPoint& pos);

private:
	void addCentripetalConstraint();
	void addDistanceVelocityConstraint();
	void deleteConstraint();

	void constraintDoubleClicked(QTreeWidgetItem* item, int column);
	void addConstraintToTree(std::shared_ptr<PathConstraint> c);


private:
	std::shared_ptr<RobotPath> path_;
	QTreeWidgetItem* menuItem_;
	QVector<std::shared_ptr<PathConstraint>> constraints_;
};

