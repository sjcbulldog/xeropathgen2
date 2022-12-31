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

#include "TrajectoryGroup.h"
#include "TrajectoryQtChartPlotWindow.h"
#include "NodesListWindow.h"
#include "BasePlotWindow.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QFrame>

class PlotWindow : public QWidget
{
public:
	PlotWindow(bool custom, QWidget* parent, QList<int> sizes);

	std::shared_ptr<TrajectoryGroup> group() { return group_; }

	bool isSplitterPositionValid() const {
		return isSplitterPositionValid_;
	}
	void setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group);
	QList<int> getSplitterPosition();

	const QStringList& nodeList() const {
		return plot_->nodeList();
	}

	void setNodeList(const QStringList& list) {
		plot_->setNodeList(list);
	}

	void setCustomPlot();
	void setQChartPlot();

	static QVector<QString> TrajVariableName;

protected:
	void showEvent(QShowEvent*) override;

private:
	std::shared_ptr<TrajectoryGroup> group_;
	QHBoxLayout* layout_;
	QSplitter* left_right_splitter_;
	NodesListWindow* nodes_;
	QFrame* nodes_frame_;
	BasePlotWindow* plot_;
	bool isSplitterPositionValid_;
	bool custom_plot_;
};

