#include "NodesListWindow.h"
#include <QtCore/QMimeData>

NodesListWindow::NodesListWindow(const QVector<QString>& varnames, QWidget* parent) : QTreeWidget(parent), varnames_(varnames)
{
	setHeaderHidden(true);
	setDragDropMode(DragDropMode::DragOnly);
	setDragEnabled(true);
	setSelectionMode(SelectionMode::SingleSelection);
}

void NodesListWindow::setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group)
{
	QString nodeName;
	QTreeWidgetItem* item;

	group_ = group;

	clear();

	if (group_ != nullptr) {
		for (const QString& name : group_->trajectoryNames()) {
			auto traj = group_->getTrajectory(name);

			for (const QString& valuename : varnames_) {
				nodeName = name + "-" + valuename;
				item = new QTreeWidgetItem();
				item->setText(0, nodeName);
				addTopLevelItem(item);
			}
		}
	}
}

QMimeData* NodesListWindow::mimeData(const QList<QTreeWidgetItem*>& items) const
{
	QMimeData* data = nullptr;

	if (items.size() > 0)
	{
		QString text;
		for (auto item : items) {
			if (text.length() > 0) {
				text += ",";
			}
			text += item->text(0);
		}

		data = new QMimeData();
		data->setText(text);
	}

	return data;
}
