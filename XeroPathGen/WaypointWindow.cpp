#include "WaypointWindow.h"
#include <QtWidgets/QMessageBox>

WaypointWindow::WaypointWindow(QWidget* parent) : QTreeWidget(parent)
{
	index_ = -1;

	setHeaderHidden(true);
	setColumnCount(2);

	setItemDelegateForColumn(0, new NoEditDelegate(this));
	connect(this, &QTreeWidget::itemChanged, this, &WaypointWindow::waypointParamChanged);
}

void WaypointWindow::waypointParamChanged(QTreeWidgetItem* item, int column)
{
	assert(column == 1);
	assert(item->text(0) != IndexTag);

	bool ok;
	double value = item->text(1).toDouble(&ok);

	if (!ok) 
	{
		QMessageBox::critical(this, "Invalid Number", "The string '" + item->text(1) + "' is not a valid number");
		refresh();
	}
	else
	{
		const Pose2dWithRotation pt = path_->getPoint(index_);
		Pose2dWithRotation newpt;

		if (item->text(0) == XTag) 
		{
			newpt = Pose2dWithRotation(Translation2d(value, pt.getTranslation().getY()), pt.getRotation(), pt.swrot());
		}
		else if (item->text(0) == YTag) 
		{
			newpt = Pose2dWithRotation(Translation2d(pt.getTranslation().getX(), value), pt.getRotation(), pt.swrot());
		}
		else if (item->text(0) == HeadingTag) 
		{
			newpt = Pose2dWithRotation(Translation2d(pt.getTranslation().getX(), pt.getTranslation().getY()), Rotation2d::fromDegrees(value), pt.swrot());
		}
		else if (item->text(0) == RotationTag) 
		{
			newpt = Pose2dWithRotation(Translation2d(pt.getTranslation().getX(), pt.getTranslation().getY()), pt.getRotation(), Rotation2d::fromDegrees(value));
		}
		path_->replacePoint(index_, newpt);
	}
}

QTreeWidgetItem* WaypointWindow::newItem(const QString& title, bool editable)
{
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText(0, title);
	if (editable) {
		item->setFlags(item->flags() | Qt::ItemIsEditable);
	}

	return item;
}

void WaypointWindow::refresh()
{
	clear();

	if (path_ != nullptr && index_ >= 0 && index_ < path_->size()) 
	{
		QTreeWidgetItem* item;
		const auto& pt = path_->getPoint(index_);

		item = newItem(IndexTag, false);
		item->setText(1, QString::number(index_));
		addTopLevelItem(item);

		item = newItem(XTag);
		item->setText(1, QString::number(pt.getTranslation().getX(), 'f', 2));
		addTopLevelItem(item);

		item = newItem(YTag);
		item->setText(1, QString::number(pt.getTranslation().getY(), 'f', 2));
		addTopLevelItem(item);

		item = newItem(HeadingTag);
		item->setText(1, QString::number(pt.getRotation().toDegrees(), 'f', 2));
		addTopLevelItem(item);

		item = newItem(RotationTag);
		item->setText(1, QString::number(pt.swrot().toDegrees(), 'f', 2));
		addTopLevelItem(item);

		item = newItem(DistanceTag, false);
		item->setText(1, QString::number(dist_, 'f', 2));
		addTopLevelItem(item);
	}
}