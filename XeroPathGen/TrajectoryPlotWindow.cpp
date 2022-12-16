#include "TrajectoryPlotWindow.h"
#include "RobotPath.h"
#include <QtCore/QMimeData>
#include <QtCharts/QLineSeries>

TrajectoryPlotWindow::TrajectoryPlotWindow(const QVector<QString>& varnames, QWidget* parent) : QChartView(parent), varnames_(varnames)
{
	setRenderHint(QPainter::Antialiasing);
	setRubberBand(QChartView::RectangleRubberBand);
	chart()->setAnimationOptions(QChart::SeriesAnimations);
	chart()->setDropShadowEnabled(true);

	time_axis_ = nullptr;
	left_right_ = true;
}

void TrajectoryPlotWindow::setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group)
{
	if (group == nullptr || group_ == nullptr || group->path() != group_->path())
	{
		nodes_.clear();
	}

	group_ = group;

	clear();

	if (group_ != nullptr) {
		QFont font = chart()->titleFont();
		font.setPointSize(16);
		font.setBold(true);
		chart()->setTitleFont(font);
		chart()->setTitle("Path: " + group->path()->name());
	}

	QStringList putback = nodes_;
	nodes_.clear();

	for (const QString& node : putback) {
		insertNode(node);
	}
}

void TrajectoryPlotWindow::clear()
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

TrajectoryPlotWindow::AxisType TrajectoryPlotWindow::mapVariableToAxis(const QString & var)
{
	AxisType ret = AxisType::Distance;

	if (var == RobotPath::HeadingTag || var == RobotPath::RotationTag)
	{
		ret = AxisType::Angle;
	}
	else if (var == RobotPath::CurvatureTag)
	{
		ret = AxisType::Curvature;
	}
	else if (var == RobotPath::VelocityTag)
	{
		ret = AxisType::Speed;
	}
	else if (var == RobotPath::AccelerationTag)
	{
		ret = AxisType::Acceleration;
	}

	return ret;
}

void TrajectoryPlotWindow::dragEnterEvent(QDragEnterEvent* ev)
{
	setBackgroundRole(QPalette::Highlight);
	ev->setDropAction(Qt::DropAction::CopyAction);
	ev->acceptProposedAction();
}

void TrajectoryPlotWindow::dragMoveEvent(QDragMoveEvent* ev)
{
	ev->acceptProposedAction();
}

void TrajectoryPlotWindow::dragLeaveEvent(QDragLeaveEvent* ev)
{
	setBackgroundRole(QPalette::Window);
}

void TrajectoryPlotWindow::dropEvent(QDropEvent* ev)
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

QValueAxis* TrajectoryPlotWindow::createYAxis(const QString &node)
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

void TrajectoryPlotWindow::setupLegend()
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

void TrajectoryPlotWindow::setupTimeAxis()
{
	time_axis_ = new QValueAxis();
	time_axis_->setLabelsVisible(true);
	time_axis_->setVisible(true);
	time_axis_->setTickCount(10);
	time_axis_->setTitleText("time (s)");
	time_axis_->setTitleVisible(true);
	chart()->addAxis(time_axis_, Qt::AlignBottom);
}

void TrajectoryPlotWindow::insertNode(const QString& node)
{
	if (nodes_.contains(node))
		return;

	nodes_.push_back(node);

	int index = node.indexOf('-');
	QString name = node.mid(0, index);
	QString type = node.mid(index + 1);

	auto traj = group_->getTrajectory(name);
	if (traj == nullptr)
		return;

	if (time_axis_ == nullptr) 
	{
		setupTimeAxis();
	}

	QValueAxis* axis = createYAxis(node);

	double minv = std::numeric_limits<double>::max();
	double maxv = std::numeric_limits<double>::min();

	QLineSeries* series = new QLineSeries();
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
}
