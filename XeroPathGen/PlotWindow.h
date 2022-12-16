#pragma once

#include "TrajectoryGroup.h"
#include "TrajectoryPlotWindow.h"
#include "NodesListWindow.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QBoxLayout>

class PlotWindow : public QWidget
{
public:
	PlotWindow(QWidget* parent);

	void setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group);

	static QVector<QString> TrajVariableName;

private:
	std::shared_ptr<TrajectoryGroup> group_;
	QHBoxLayout* layout_;
	QSplitter* left_right_splitter_;
	NodesListWindow* nodes_;
	TrajectoryPlotWindow* plot_;
};

