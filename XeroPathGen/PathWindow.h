#pragma once

#include "PathsDataModel.h"
#include <QtWidgets/QTreeWidget>

class PathWindow : public QTreeWidget
{
	Q_OBJECT

public:
	PathWindow(PathsDataModel&model, QWidget* parent);
	void refresh();

signals:
	void groupSelected(const QString& grname);
	void pathSelected(const QString& grname, const QString& pathname);

private:
	void prepareCustomMenu(const QPoint& pos);
	void selectedItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

	void addGroup();
	void deleteGroup();
	void addPath();
	void deletePath();

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
};
