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
#include "TrajectoryCustomPlotWindow.h"
#include "TrajectoryGroup.h"

TrajectoryCustomPlotWindow::TrajectoryCustomPlotWindow(QWidget* parent) : QCustomPlot(parent)
{
	left_right_ = true;
	time_axis_ = false;

	color_index_ = 0;

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &TrajectoryCustomPlotWindow::customContextMenuRequested, this, &TrajectoryCustomPlotWindow::prepareCustomMenu);

	setAcceptDrops(true);

	axisRect(0)->removeAxis(yAxis);
	axisRect(0)->removeAxis(yAxis2);

	setAutoAddPlottableToLegend(true);
	legend->setVisible(true);

	setInteraction(QCP::iRangeDrag);
	setInteraction(QCP::iRangeZoom);
}

//
// This method clears the plot window.
//
void TrajectoryCustomPlotWindow::clear()
{
	clearGraphs();
	graphs_.clear();

	for (auto axis : y_axis_.values())
	{
		axisRect(0)->removeAxis(axis);
	}
	y_axis_.clear();

	color_index_ = 0;
}

void TrajectoryCustomPlotWindow::prepareCustomMenu(const QPoint& pos)
{
	QMenu menu(this);
	QAction* act;

	//
	// This is a group item: delete group, add path
	//
	act = new QAction(tr("Remove All"));
	connect(act, &QAction::triggered, this, &TrajectoryCustomPlotWindow::removeAll);
	menu.addAction(act);

	for (const QString& node : nodeList()) {
		act = new QAction("Remove '" + node + "'");
		connect(act, &QAction::triggered, [this, node]() { this->removeOne(node); });
		menu.addAction(act);
	}

	menu.exec(this->mapToGlobal(pos));
}

void TrajectoryCustomPlotWindow::removeOne(const QString& node)
{
	removeNode(node);
	if (graphs_.contains(node))
	{
		auto gr = graphs_[node];
		graphs_.remove(node);
		removeGraph(gr);
	}
	replot();
}

void TrajectoryCustomPlotWindow::removeAll()
{
	clear();
	clearNodeList();
	replot();
}

void TrajectoryCustomPlotWindow::setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group)
{
	BasePlotWindow::setTrajectoryGroup(group);
	clear();

	plotLayout()->addElement(0, 0, new QCPTextElement(this, group->path()->fullname()));

	QStringList putback = nodeList();
	clearNodeList();

	for (const QString& node : putback) {
		insertNode(node);
	}
}

void TrajectoryCustomPlotWindow::setNodeList(const QStringList& list)
{
	for (const QString& node : list) {
		insertNode(node);
	}
}


void TrajectoryCustomPlotWindow::dragEnterEvent(QDragEnterEvent* ev)
{
	setBackgroundRole(QPalette::Highlight);
	ev->setDropAction(Qt::DropAction::CopyAction);
	ev->acceptProposedAction();
}

void TrajectoryCustomPlotWindow::dragMoveEvent(QDragMoveEvent* ev)
{
	ev->acceptProposedAction();
}

void TrajectoryCustomPlotWindow::dragLeaveEvent(QDragLeaveEvent* ev)
{
	setBackgroundRole(QPalette::Window);
}

void TrajectoryCustomPlotWindow::dropEvent(QDropEvent* ev)
{
	const char* fmttext = "text/plain";
	QString text;

	const QMimeData* data = ev->mimeData();
	QStringList fmts = data->formats();
	if (data->hasFormat(fmttext))
	{
		QByteArray encoded = data->data(fmttext);
		QString text = QString::fromUtf8(encoded);
		QStringList list = text.split(",");
		for (const QString& node : list) {
			insertNode(node.trimmed());
		}
	}
	setBackgroundRole(QPalette::Window);
}

void TrajectoryCustomPlotWindow::setupTimeAxis()
{
	QPen pen = xAxis->basePen();
	pen.setWidth(1.5);
	xAxis->setBasePen(pen);
	xAxis->setLabel("time (s)");
	time_axis_ = true;
}

QCPAxis* TrajectoryCustomPlotWindow::createAxis(const QString& node)
{
	QCPAxis* axis = nullptr;

	int index = node.indexOf('-');
	QString name = node.mid(0, index);
	QString typestr = node.mid(index + 1);
	AxisType type = mapVariableToAxis(typestr);

	//
	// We create a new axis
	// 
	if (left_right_) {
		axis = axisRect()->addAxis(QCPAxis::AxisType::atLeft);
	}
	else {
		axis = axisRect()->addAxis(QCPAxis::AxisType::atRight);
	}

	QPen pen = axis->basePen();
	pen.setWidth(1.5);
	axis->setBasePen(pen);

	left_right_ = !left_right_;

	if (type == AxisType::Distance) {
		axis->setLabel("distance (m)");
	}
	else if (type == AxisType::Speed) {
		axis->setLabel("velocity (m/s)");
	}
	else if (type == AxisType::Acceleration) {
		axis->setLabel("acceleration (m/s/s)");
	}
	else if (type == AxisType::Angle) {
		axis->setLabel("angle (degrees)");
		axis->setRange(-180.0, 180.0);
	}
	else if (type == AxisType::AngleVelocity) {
		axis->setLabel("angle velocity (degrees/second)");
	}
	else if (type == AxisType::Curvature) {
		axis->setLabel("curvature");
	}

	y_axis_[type] = axis;

	return axis;
}

void TrajectoryCustomPlotWindow::insertNode(const QString& node)
{
	if (nodeList().contains(node))
		return;

	addNode(node);

	if (group() == nullptr)
		return;

	int index = node.indexOf('-');
	QString name = node.mid(0, index);
	QString type = node.mid(index + 1);

	auto traj = group()->getTrajectory(name);
	if (traj == nullptr)
		return;

	if (!time_axis_)
	{
		setupTimeAxis();
	}
	xAxis->setRange(0.0, traj->getEndTime());

	QCPAxis* myyaxis = createAxis(node);

	double minv = std::numeric_limits<double>::max();
	double maxv = std::numeric_limits<double>::min();

	QVector<double> x, y;

	for (int i = 0; i < traj->size(); i++) {
		const Pose2dWithTrajectory& pt = (*traj)[i];
		double time = pt.getField(RobotPath::TimeTag);
		double value = pt.getField(type);

		if (value > maxv) {
			maxv = value;
		}

		if (value < minv) {
			minv = value;
		}

		x.push_back(time);
		y.push_back(value);
	}

	auto gr = addGraph(xAxis, myyaxis);
	QPen pen(node_colors_.at(color_index_));
	pen.setWidth(2.0);
	gr->setPen(pen);
	gr->setName(node);
	graphs_.insert(node, gr);
	double range = maxv - minv;
	myyaxis->setRange(minv - 0.05 * range, maxv + 0.05 * range);
	gr->setData(x, y);
	replot();

	color_index_++;
	if (color_index_ == node_colors_.size())
		color_index_ = 0;
}

QVector<QColor> TrajectoryCustomPlotWindow::node_colors_ =
{
	QColor(0, 0, 0),
	QColor(255, 0, 0),
	QColor(0, 255, 0),
	QColor(0, 0, 255),
	QColor(255, 165, 0),
	QColor(0, 255, 255),
	QColor(255, 0, 255)
};