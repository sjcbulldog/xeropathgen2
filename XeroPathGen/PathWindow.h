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

	void trajectoryGenerationError(std::shared_ptr<RobotPath> path, bool error);

	void removeGroupFromDisplay(const QString& grname);
	void removePathFromDisplay(const QString& grname, const QString& pathname);
	void changePathNameInDisplay(const QString& grname, const QString& pathname, const QString& newname);
	void changeGroupNameInDisplay(const QString& pathname, const QString& newname);

	void insertGroupInDisplay(const QString& grname, int index);
	void insertPathInDisplay(std::shared_ptr<RobotPath> path, int index);

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
		item->setForeground(0, unknown_);

		return item;
	}

	void itemRenamed(QTreeWidgetItem* item, int column);

	QTreeWidgetItem* getGroupItem(const QString& name);
	QTreeWidgetItem* getPathItem(const QString& grname, const QString& pathname);

private:
	QTreeWidgetItem* menuItem_;
	PathsDataModel &model_;
	QString units_;
	std::shared_ptr<RobotParams> robot_;
	std::shared_ptr<RobotPath> selected_path_;

	QBrush unknown_;
	QBrush good_;
	QBrush bad_;
};
