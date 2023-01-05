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
#include "qcustomplot.h"
#include "BasePlotWindow.h"

class TrajectoryCustomPlotWindow : public QCustomPlot, public BasePlotWindow
{
public:
	TrajectoryCustomPlotWindow(QWidget* parent = nullptr);

	void clear() override;

	void setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group) override;

	void setNodeList(const QStringList& list) override;


protected:
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void dragLeaveEvent(QDragLeaveEvent* event) override;
	void dropEvent(QDropEvent* ev) override;

private:
	void insertNode(const QString& node);
	void setupTimeAxis();
	QCPAxis* createAxis(const QString& name);

	void prepareCustomMenu(const QPoint& pos);
	void removeAll();
	void removeOne(const QString& name);

private:
	bool time_axis_;
	QMap<AxisType, QCPAxis*> y_axis_;
	QMap<QString, QCPGraph*> graphs_;
	bool left_right_;
	int color_index_;

	static QVector<QColor> node_colors_;
};

