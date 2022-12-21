#pragma once

#include "PathsDataModel.h"
#include <QtWidgets/QTreeWidget>

class PathWindow : public QTreeWidget
{
	Q_OBJECT

public:
	PathWindow(PathsDataModel&model, QWidget* parent);
	void refresh();

	void setUnits(const QString& units) {
		units_ = units;
	}

	void setRobot(std::shared_ptr<RobotParams> robot) {
		robot_ = robot;
	}

	std::shared_ptr<RobotPath> selectedPath() {
		return selected_path_;
	}

signals:
	void groupSelected(const QString& grname);
	void pathSelected(std::shared_ptr<RobotPath> path);

private:
	void prepareCustomMenu(const QPoint& pos);
	void selectedItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

	void addGroup();
	void deleteGroup();
	void addPath();
	void deletePath();
	void changePathUnits();

	QString newGroupName();
	QString newPathName(const QString& grname);
	bool isValidName(const QString& name);

	QTreeWidgetItem* newItem(const QString& text) {
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, text);
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		item->setData(0, Qt::UserRole, text);

		return item;
	}

	void itemRenamed(QTreeWidgetItem* item, int column);

private:
	QTreeWidgetItem* menuItem_;
	PathsDataModel &model_;
	QString units_;
	std::shared_ptr<RobotParams> robot_;
	std::shared_ptr<RobotPath> selected_path_;
};
