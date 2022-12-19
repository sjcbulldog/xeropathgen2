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

PlotWindow::PlotWindow(QWidget* parent, QList<int> sizes) : QWidget(parent)
{
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

	plot_ = new TrajectoryPlotWindow(TrajVariableName, this);

	left_right_splitter_->addWidget(nodes_frame_);
	left_right_splitter_->addWidget(plot_);

	left_right_splitter_->setSizes(sizes);
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