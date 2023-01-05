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
#include "NodesListWindow.h"
#include <QtCore/QMimeData>

NodesListWindow::NodesListWindow(const QVector<QString>& varnames, QWidget* parent) : QTreeWidget(parent), varnames_(varnames)
{
	setHeaderHidden(true);
	setDragDropMode(DragDropMode::DragOnly);
	setDragEnabled(true);
	setSelectionMode(SelectionMode::SingleSelection);
}

void NodesListWindow::setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group)
{
	QString nodeName;
	QTreeWidgetItem* item;

	group_ = group;

	clear();

	if (group_ != nullptr) {
		for (const QString& name : group_->trajectoryNames()) {
			auto traj = group_->getTrajectory(name);

			for (const QString& valuename : varnames_) {
				nodeName = name + "-" + valuename;
				item = new QTreeWidgetItem();
				item->setText(0, nodeName);
				addTopLevelItem(item);
			}
		}
	}
}

QMimeData* NodesListWindow::mimeData(const QList<QTreeWidgetItem*>& items) const
{
	QMimeData* data = nullptr;

	if (items.size() > 0)
	{
		QString text;
		for (auto item : items) {
			if (text.length() > 0) {
				text += ",";
			}
			text += item->text(0);
		}

		data = new QMimeData();
		data->setText(text);
	}

	return data;
}
