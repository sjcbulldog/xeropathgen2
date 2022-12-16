#include "PathParametersWindow.h"
#include "NoEditDelegate.h"
#include <QtWidgets/QMessageBox>

PathParametersWindow::PathParametersWindow(QWidget* parent) : QTreeWidget(parent)
{
	length_item_ = nullptr;
	duration_item_ = nullptr;

	setHeaderHidden(true);
	setColumnCount(2);

	setItemDelegateForColumn(0, new NoEditDelegate(this));
	connect(this, &QTreeWidget::itemChanged, this, &PathParametersWindow::pathParamChanged);
}

void PathParametersWindow::setTrajectory(std::shared_ptr<PathTrajectory> traj)
{
	if (duration_item_ != nullptr) {
		duration_item_->setText(1, QString::number(traj->getEndTime(), 'f', 2));
	}

	if (length_item_ != nullptr) {
		length_item_->setText(1, QString::number(traj->getEndDistance(), 'f', 2));
	}
}

void PathParametersWindow::pathParamChanged(QTreeWidgetItem* item, int column)
{
	assert(column == 1);

	bool ok;
	double value = item->text(1).toDouble(&ok);

	if (!ok)
	{
		QMessageBox::critical(this, "Invalid Number", "The string '" + item->text(1) + "' is not a valid number");
		refresh();
	}
	else
	{
		PathParameters params = path_->params();

		if (item->text(0) == StartVelocityTag)
		{
			params = PathParameters(value, params.endVelocity(), params.maxVelocity(), params.maxAccel());
		}
		else if (item->text(0) == EndVelocityTag)
		{
			params = PathParameters(params.startVelocity(), value, params.maxVelocity(), params.maxAccel());
		}
		else if (item->text(0) == MaxVelocityTag)
		{
			params = PathParameters(params.startVelocity(), params.endVelocity(), value, params.maxAccel());
		}
		else if (item->text(0) == MaxAccelerationTag)
		{
			params = PathParameters(params.startVelocity(), params.endVelocity(), params.maxVelocity(),value);
		}
		path_->setParams(params);
	}
}

QTreeWidgetItem* PathParametersWindow::newItem(const QString& title, bool editable)
{
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText(0, title);
	if (editable) {
		item->setFlags(item->flags() | Qt::ItemIsEditable);
	}

	return item;
}

void PathParametersWindow::refresh()
{
	clear();
	length_item_ = nullptr;
	duration_item_ = nullptr;

	if (path_ != nullptr)
	{
		QTreeWidgetItem* item;

		item = newItem(StartVelocityTag);
		item->setText(1, QString::number(path_->params().startVelocity()));
		addTopLevelItem(item);

		item = newItem(EndVelocityTag);
		item->setText(1, QString::number(path_->params().endVelocity()));
		addTopLevelItem(item);

		item = newItem(MaxVelocityTag);
		item->setText(1, QString::number(path_->params().maxVelocity()));
		addTopLevelItem(item);

		item = newItem(MaxAccelerationTag);
		item->setText(1, QString::number(path_->params().maxAccel()));
		addTopLevelItem(item);

		length_item_ = newItem(LengthTag, false);
		length_item_->setText(1, "<unknown>");
		addTopLevelItem(length_item_);

		duration_item_ = newItem(DurationTag, false);
		duration_item_->setText(1, "<unknown>");
		addTopLevelItem(duration_item_);

		resizeColumnToContents(0);
		resizeColumnToContents(1);
	}
}