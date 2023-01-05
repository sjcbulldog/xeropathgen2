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
#include "WaypointWindow.h"
#include "RobotPath.h"
#include <QtWidgets/QMessageBox>

WaypointWindow::WaypointWindow(QWidget* parent) : QTreeWidget(parent)
{
	index_ = -1;
	dist_ = 0.0;

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

		if (item->text(0) == RobotPath::XTag)
		{
			newpt = Pose2dWithRotation(Translation2d(value, pt.getTranslation().getY()), pt.getRotation(), pt.getSwrot());
			newpt.setRotVelocity(pt.getSwrotVelocity());
		}
		else if (item->text(0) == RobotPath::YTag)
		{
			newpt = Pose2dWithRotation(Translation2d(pt.getTranslation().getX(), value), pt.getRotation(), pt.getSwrot());
			newpt.setRotVelocity(pt.getSwrotVelocity());
		}
		else if (item->text(0) == RobotPath::HeadingTag) 
		{
			newpt = Pose2dWithRotation(Translation2d(pt.getTranslation().getX(), pt.getTranslation().getY()), Rotation2d::fromDegrees(value), pt.getSwrot());
			newpt.setRotVelocity(pt.getSwrotVelocity());
		}
		else if (item->text(0) == RobotPath::RotationTag) 
		{
			newpt = Pose2dWithRotation(Translation2d(pt.getTranslation().getX(), pt.getTranslation().getY()), pt.getRotation(), Rotation2d::fromDegrees(value));
			newpt.setRotVelocity(pt.getSwrotVelocity());
		}
		else if (item->text(0) == RobotPath::SwerveRotationVelocityTag)
		{
			newpt = Pose2dWithRotation(Translation2d(pt.getTranslation().getX(), pt.getTranslation().getY()), pt.getRotation(), pt.getSwrot());
			newpt.setRotVelocity(value);
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

		item = newItem(RobotPath::XTag);
		item->setText(1, QString::number(pt.getTranslation().getX(), 'f', 2));
		addTopLevelItem(item);

		item = newItem(RobotPath::YTag);
		item->setText(1, QString::number(pt.getTranslation().getY(), 'f', 2));
		addTopLevelItem(item);

		item = newItem(RobotPath::HeadingTag);
		item->setText(1, QString::number(pt.getRotation().toDegrees(), 'f', 2));
		addTopLevelItem(item);

		item = newItem(RobotPath::RotationTag);
		item->setText(1, QString::number(pt.getSwrot().toDegrees(), 'f', 2));
		addTopLevelItem(item);

		item = newItem(RobotPath::SwerveRotationVelocityTag);
		item->setText(1, QString::number(pt.getSwrotVelocity(), 'f', 2));
		addTopLevelItem(item);

		item = newItem(DistanceTag, false);
		item->setText(1, QString::number(dist_, 'f', 2));
		addTopLevelItem(item);
	}
}