#pragma once

#include "TrajectoryGroup.h"
#include <QtWidgets/QTreeWidget>

class NodesListWindow : public QTreeWidget
{
public:
	NodesListWindow(const QVector<QString>& varnames, QWidget* parent);

	void setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group);

protected:
	QMimeData* mimeData(const QList<QTreeWidgetItem*>& items) const;

private:
	const QVector<QString>& varnames_;
	std::shared_ptr<TrajectoryGroup> group_;
};

