#include "PlotWindow.h"

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
};

PlotWindow::PlotWindow(QWidget* parent) : QWidget(parent)
{
	layout_ = new QHBoxLayout();
	setLayout(layout_);
	left_right_splitter_ = new QSplitter(this);
	layout_->addWidget(left_right_splitter_);

	nodes_ = new NodesListWindow(TrajVariableName, this);
	plot_ = new TrajectoryPlotWindow(TrajVariableName, this);

	left_right_splitter_->addWidget(nodes_);
	left_right_splitter_->addWidget(plot_);
}

void PlotWindow::setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group)
{
	QString nodeName;
	QTreeWidgetItem* item;

	group_ = group;

	nodes_->setTrajectoryGroup(group);
	plot_->setTrajectoryGroup(group);
}