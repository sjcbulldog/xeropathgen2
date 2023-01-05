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
#include "BasePlotWindow.h"
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QtWidgets/QWidget>

class TrajectoryQtChartPlotWindow : public QChartView, public BasePlotWindow
{
public:
	TrajectoryQtChartPlotWindow(const QVector<QString> &varnames, QWidget* parent);

	void clear() override;
	void setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group) override;

	void setNodeList(const QStringList& list) override;

protected:
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void dragLeaveEvent(QDragLeaveEvent* event) override;
	void dropEvent(QDropEvent* ev) override;

	bool viewportEvent(QEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void keyPressEvent(QKeyEvent* event);

private:

	void insertNode(const QString& node);
	QValueAxis* createYAxis(const QString& node);
	void setupLegend();
	void setupTimeAxis();

	void prepareCustomMenu(const QPoint& pos);
	void removeAll();
	void removeOne(const QString& name);

private:
	QValueAxis* time_axis_;
	QMap<AxisType, QValueAxis*> y_axis_;
	const QVector<QString>& varnames_;
	bool left_right_;
	QPoint last_mouse_;
};

