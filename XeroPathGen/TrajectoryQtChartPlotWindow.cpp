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
#include "TrajectoryQtChartPlotWindow.h"
#include "RobotPath.h"
#include <QtCore/QMimeData>
#include <QtCharts/QLineSeries>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>

TrajectoryQtChartPlotWindow::TrajectoryQtChartPlotWindow(const QVector<QString>& varnames, QWidget* parent) : QChartView(parent), varnames_(varnames)
{
	setRubberBand(QChartView::NoRubberBand);
	chart()->setAnimationOptions(QChart::NoAnimation);
	chart()->setDropShadowEnabled(true);

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &TrajectoryQtChartPlotWindow::customContextMenuRequested, this, &TrajectoryQtChartPlotWindow::prepareCustomMenu);

	time_axis_ = nullptr;
	left_right_ = true;
}

void TrajectoryQtChartPlotWindow::setNodeList(const QStringList& list)
{
	BasePlotWindow::setNodeList(list);

	removeAll();
	for (const QString& node : list) {
		insertNode(node);
	}
}

void TrajectoryQtChartPlotWindow::prepareCustomMenu(const QPoint& pos)
{
	QMenu menu(this);
	QAction* act;

	//
	// This is a group item: delete group, add path
	//
	act = new QAction(tr("Remove All"));
	connect(act, &QAction::triggered, this, &TrajectoryQtChartPlotWindow::removeAll);
	menu.addAction(act);

	for (const QString& node : nodeList()) {
		act = new QAction("Remove '" + node + "'");
		connect(act, &QAction::triggered, [this, node]() { this->removeOne(node); });
		menu.addAction(act);
	}

	menu.exec(this->mapToGlobal(pos));
}

void TrajectoryQtChartPlotWindow::removeOne(const QString &node)
{
	if (nodeList().contains(node))
	{
		removeNode(node);
		for (auto series : chart()->series()) {
			if (series->name() == node) {
				chart()->removeSeries(series);
				break;
			}
		}
	}
}

void TrajectoryQtChartPlotWindow::removeAll()
{
	clearNodeList();
	clear();
}

void TrajectoryQtChartPlotWindow::keyPressEvent(QKeyEvent* event)
{
	switch (event->key()) {
	case Qt::Key_Plus:
		chart()->zoomIn();
		break;
	case Qt::Key_Minus:
		chart()->zoomOut();
		break;
	case Qt::Key_Home:
		chart()->zoomReset();
		break;
	}
}

bool TrajectoryQtChartPlotWindow::viewportEvent(QEvent* event)
{
	return QChartView::viewportEvent(event);
}

void TrajectoryQtChartPlotWindow::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::MiddleButton)
	{
		QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
		last_mouse_ = event->pos();
		event->accept();
	}

	QChartView::mousePressEvent(event);
}

void TrajectoryQtChartPlotWindow::mouseMoveEvent(QMouseEvent* event)
{
	// pan the chart with a middle mouse drag
	if (event->buttons() & Qt::MiddleButton)
	{
		auto dPos = event->pos() - last_mouse_;
		chart()->scroll(-dPos.x(), dPos.y());

		last_mouse_ = event->pos();
		event->accept();
	}

	QChartView::mouseMoveEvent(event);
}

void TrajectoryQtChartPlotWindow::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::MiddleButton)
	{
		QApplication::restoreOverrideCursor();
	}
}

void TrajectoryQtChartPlotWindow::setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group)
{
	BasePlotWindow::setTrajectoryGroup(group);
	clear();

	if (group != nullptr) {
		QFont font = chart()->titleFont();
		font.setPointSize(16);
		font.setBold(true);
		chart()->setTitleFont(font);
		chart()->setTitle("Path: " + group->path()->name());
	}

	QStringList putback = nodeList();
	clearNodeList();

	for (const QString& node : putback) {
		insertNode(node);
	}
}

void TrajectoryQtChartPlotWindow::clear()
{
	// Series
	chart()->removeAllSeries();

	// Titles
	chart()->setTitle("");

	// Legend
	QLegend* legend = chart()->legend();
	legend->setVisible(false);

	// Axis
	chart()->removeAxis(time_axis_);
	delete time_axis_;
	time_axis_ = nullptr;

	for (auto yaxis : y_axis_.values()) {
		chart()->removeAxis(yaxis);
		delete yaxis;
	}
	y_axis_.clear();
}


void TrajectoryQtChartPlotWindow::dragEnterEvent(QDragEnterEvent* ev)
{
	setBackgroundRole(QPalette::Highlight);
	ev->setDropAction(Qt::DropAction::CopyAction);
	ev->acceptProposedAction();
}

void TrajectoryQtChartPlotWindow::dragMoveEvent(QDragMoveEvent* ev)
{
	ev->acceptProposedAction();
}

void TrajectoryQtChartPlotWindow::dragLeaveEvent(QDragLeaveEvent* ev)
{
	setBackgroundRole(QPalette::Window);
}

void TrajectoryQtChartPlotWindow::dropEvent(QDropEvent* ev)
{
	const char *fmttext = "text/plain";
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

QValueAxis* TrajectoryQtChartPlotWindow::createYAxis(const QString &node)
{
	QValueAxis* axis;

	int index = node.indexOf('-');
	QString name = node.mid(0, index);
	QString typestr = node.mid(index + 1);
	AxisType type = mapVariableToAxis(typestr);

	if (y_axis_.contains(type)) {
		axis = y_axis_.value(type);
	}
	else {
		axis = new QValueAxis();

		axis->setLabelsVisible(true);
		axis->setVisible(true);
		axis->setTickCount(10);

		if (type == AxisType::Distance) {
			axis->setTitleText("distance (m)");
		}
		else if (type == AxisType::Speed) {
			axis->setTitleText("velocity (m/s)");
		}
		else if (type == AxisType::Acceleration) {
			axis->setTitleText("acceleration (m/s/s)");
		}
		else if (type == AxisType::Angle) {
			axis->setTitleText("angle (degrees)");
			axis->setMin(-180.0);
			axis->setMax(180.0);
		}
		else if (type == AxisType::AngleVelocity) {
			axis->setTitleText("angle velocity (degrees/second)");
		}
		else if (type == AxisType::Curvature) {
			axis->setTitleText("curvature");
		}
		axis->setTitleVisible(true);

		y_axis_.insert(type, axis);
		if (left_right_) {
			chart()->addAxis(axis, Qt::AlignLeft);
		}
		else {
			chart()->addAxis(axis, Qt::AlignRight);
		}

		left_right_ = !left_right_;
	}

	return axis;
}

void TrajectoryQtChartPlotWindow::setupLegend()
{
	QFont font;

	QLegend* legend = chart()->legend();
	legend->setVisible(true);
	legend->setAlignment(Qt::AlignBottom);
	legend->setMarkerShape(QLegend::MarkerShape::MarkerShapeCircle);
	font = legend->font();
	font.setPointSize(8);
	font.setBold(true);
	legend->setFont(font);
}

void TrajectoryQtChartPlotWindow::setupTimeAxis()
{
	time_axis_ = new QValueAxis();
	time_axis_->setLabelsVisible(true);
	time_axis_->setVisible(true);
	time_axis_->setTickCount(10);
	time_axis_->setTitleText("time (s)");
	time_axis_->setTitleVisible(true);
	chart()->addAxis(time_axis_, Qt::AlignBottom);
}

void TrajectoryQtChartPlotWindow::insertNode(const QString& node)
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

	setupLegend();

	if (time_axis_ == nullptr)
	{
		setupTimeAxis();
	}

	QValueAxis* axis = createYAxis(node);

	double minv = std::numeric_limits<double>::max();
	double maxv = std::numeric_limits<double>::min();

	QLineSeries* series = new QLineSeries();
	series->setUseOpenGL(true);
	series->setName(node);

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

		series->append(time, value);
	}

	chart()->addSeries(series);
	series->attachAxis(time_axis_);
	series->attachAxis(axis);
	axis->applyNiceNumbers();
}
