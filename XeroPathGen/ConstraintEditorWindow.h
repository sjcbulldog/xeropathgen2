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

