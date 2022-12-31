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
#include "PlotWindow.h"
#include "TrajectoryCustomPlotWindow.h"

QVector<QString> PlotWindow::TrajVariableName =
{
	RobotPath::XTag,
	RobotPath::YTag,
	RobotPath::PositionTag,
	RobotPath::AccelerationTag,
	RobotPath::VelocityTag,
	RobotPath::HeadingTag,
	RobotPath::CurvatureTag,
	RobotPath::RotationTag,
	RobotPath::SwerveRotationVelocityTag
};

PlotWindow::PlotWindow(bool custom, QWidget* parent, QList<int> sizes) : QWidget(parent)
{
	custom_plot_ = custom;

	layout_ = new QHBoxLayout();
	setLayout(layout_);
	left_right_splitter_ = new QSplitter(this);
	layout_->addWidget(left_right_splitter_);

	nodes_frame_ = new QFrame();

	QHBoxLayout* lay = new QHBoxLayout();
	nodes_frame_->setLayout(lay);
	nodes_frame_->setFrameShape(QFrame::Box);
	nodes_frame_->setFrameShadow(QFrame::Sunken);

	nodes_ = new NodesListWindow(TrajVariableName, this);
	lay->addWidget(nodes_);

	left_right_splitter_->addWidget(nodes_frame_);

	if (custom_plot_) {
		auto plot = new TrajectoryCustomPlotWindow(this);
		left_right_splitter_->addWidget(plot);
		plot_ = plot;
	}
	else {
		auto plot = new TrajectoryQtChartPlotWindow(TrajVariableName, this);
		left_right_splitter_->addWidget(plot);
		plot_ = plot;
	}

	left_right_splitter_->setSizes(sizes);

	isSplitterPositionValid_ = false;
}


void PlotWindow::setCustomPlot()
{
	if (!custom_plot_) {
		custom_plot_ = true;

		QStringList nodes = plot_->nodeList();
		auto grp = group();

		auto sizes = left_right_splitter_->sizes();

		auto oldplot = dynamic_cast<TrajectoryQtChartPlotWindow*>(plot_);
		oldplot->setParent(nullptr);
		oldplot->deleteLater();

		auto plot = new TrajectoryCustomPlotWindow(this);

		left_right_splitter_->addWidget(plot);
		left_right_splitter_->setSizes(sizes);

		plot_ = plot;

		plot_->setNodeList(nodes);
		plot_->setTrajectoryGroup(grp);
	}
}

void PlotWindow::setQChartPlot()
{
	if (custom_plot_) {
		custom_plot_ = false;

		QStringList nodes = plot_->nodeList();
		auto grp = group();

		auto sizes = left_right_splitter_->sizes();

		auto oldplot = dynamic_cast<TrajectoryCustomPlotWindow*>(plot_);
		oldplot->setParent(nullptr);
		oldplot->deleteLater();

		auto plot = new TrajectoryQtChartPlotWindow(TrajVariableName, this);

		left_right_splitter_->addWidget(plot);
		left_right_splitter_->setSizes(sizes);

		plot_ = plot;

		plot_->setNodeList(nodes);
		plot_->setTrajectoryGroup(grp);
	}
}

void PlotWindow::showEvent(QShowEvent*)
{
	isSplitterPositionValid_ = true;
}

QList<int> PlotWindow::getSplitterPosition()
{
	auto sizes = left_right_splitter_->sizes();
	return sizes;
}

void PlotWindow::setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group)
{
	QString nodeName;
	QTreeWidgetItem* item;

	group_ = group;

	nodes_->setTrajectoryGroup(group);
	plot_->setTrajectoryGroup(group);
}