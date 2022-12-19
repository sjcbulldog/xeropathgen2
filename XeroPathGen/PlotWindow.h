#pragma once

#include "TrajectoryGroup.h"
#include "TrajectoryPlotWindow.h"
#include "NodesListWindow.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QFrame>

class PlotWindow : public QWidget
{
public:
	PlotWindow(QWidget* parent, QList<int> sizes);

	std::shared_ptr<TrajectoryGroup> group() { return group_; }
	void setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group);
	QList<int> getSplitterPosition();

	const QStringList& nodeList() const {
		return plot_->nodeList();
	}

	void setNodeList(const QStringList& list) {
		plot_->setNodeList(list);
	}

	static QVector<QString> TrajVariableName;

private:
	std::shared_ptr<TrajectoryGroup> group_;
	QHBoxLayout* layout_;
	QSplitter* left_right_splitter_;
	NodesListWindow* nodes_;
	QFrame* nodes_frame_;
	TrajectoryPlotWindow* plot_;
};

