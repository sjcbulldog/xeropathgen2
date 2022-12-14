#include "PathWindow.h"
#include <QtWidgets/QMenu>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMessageBox>

PathWindow::PathWindow(PathsDataModel& model, QWidget* parent) : QTreeWidget(parent), model_(model)
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QTreeWidget::customContextMenuRequested, this, &PathWindow::prepareCustomMenu);
	connect(this, &QTreeWidget::currentItemChanged, this, &PathWindow::selectedItemChanged);
	connect(this, &QTreeWidget::itemChanged, this, &PathWindow::itemRenamed);

	connect(&model, &PathsDataModel::groupAdded, this, &PathWindow::refresh);
	connect(&model, &PathsDataModel::groupDeleted, this, &PathWindow::refresh);
	connect(&model, &PathsDataModel::pathAdded, this, &PathWindow::refresh);
	connect(&model, &PathsDataModel::pathDeleted, this, &PathWindow::refresh);

	menuItem_ = nullptr;
	setHeaderHidden(true);
}

void PathWindow::itemRenamed(QTreeWidgetItem* item, int column)
{
	assert(column == 0);

	QString oldname = item->data(0, Qt::UserRole).toString();

	if (!isValidName(item->text(0)))
	{
		QMessageBox::critical(this, "Invalid Name", "The name '" + item->text(0) + "' is not a valid name.  A name must consist of letters and numbers only.");
		item->setText(0, oldname);
		return;
	}

	if (item->parent() == nullptr) {
		if (model_.hasGroup(item->text(0)))
		{
			QMessageBox::critical(this, "Invalid Name", "There is already a group named '" + item->text(0) + "'.");
			item->setText(0, oldname);
			return;
		}

		model_.blockSignals(true);
		model_.renameGroup(oldname, item->text(0));
		model_.blockSignals(false);
	}
	else
	{
		QString grname = item->parent()->text(0);
		if (model_.hasPath(grname, item->text(0)))
		{
			QMessageBox::critical(this, "Invalid Name", "There is already a path named '" + item->text(0) + "' in the group '" + grname + "'.");
			item->setText(0, oldname);
			return;
		}

		model_.blockSignals(true);
		model_.renamePath(grname, oldname, item->text(0));
		model_.blockSignals(false);
	}
}

void PathWindow::selectedItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	if (current != nullptr) 
	{
		if (current->parent() == nullptr) 
		{
			emit groupSelected(current->text(0));
		}
		else
		{
			emit pathSelected(current->parent()->text(0), current->text(0));
		}
	}
}

void PathWindow::refresh()
{
	clear();

	for (const QString& groupName : model_.groupNames())
	{
		QTreeWidgetItem* gritem = newItem(groupName);

		for (const QString& pathName : model_.pathNames(groupName))
		{
			QTreeWidgetItem* pathitem = newItem(pathName);
			gritem->addChild(pathitem);
		}
		addTopLevelItem(gritem);
	}
}

void PathWindow::prepareCustomMenu(const QPoint& pos)
{
	QMenu menu(this);
	QAction* act;

	menuItem_ = itemAt(pos);
	if (menuItem_ == nullptr) {
		//
		// We are not on an item: add group
		//
		act = new QAction(tr("Add New Path Group"));
		connect(act, &QAction::triggered, this, &PathWindow::addGroup);
		menu.addAction(act);
	}
	else if (menuItem_->parent() == nullptr) {
		//
		// This is a group item: delete group, add path
		//
		act = new QAction(tr("Add New Path"));
		connect(act, &QAction::triggered, this, &PathWindow::addPath);
		menu.addAction(act);

		act = new QAction(tr("Delete Group"));
		connect(act, &QAction::triggered, this, &PathWindow::deleteGroup);
		menu.addAction(act);
	}
	else {
		//
		// This is a path item: delete path
		//
		act = new QAction(tr("Delete Path"));
		connect(act, &QAction::triggered, this, &PathWindow::deletePath);
		menu.addAction(act);
	}

	menu.exec(this->mapToGlobal(pos));
	menuItem_ = nullptr;
}

void PathWindow::addGroup()
{
	QString grname = newGroupName();

	QTreeWidgetItem* item = newItem(grname);
	addTopLevelItem(item);

	model_.blockSignals(true);
	model_.addGroup(grname);
	model_.blockSignals(false);
}

void PathWindow::deleteGroup()
{
	assert(menuItem_ != nullptr);

	QString grname = menuItem_->text(0);

	delete menuItem_;
	menuItem_ = nullptr;

	model_.blockSignals(true);
	model_.deleteGroup(grname);
	model_.blockSignals(false);
}

void PathWindow::addPath()
{
	assert(menuItem_ != nullptr);

	QString name = newPathName(menuItem_->text(0));

	QTreeWidgetItem* pitem = newItem(name);
	menuItem_->addChild(pitem);

	const PathGroup* group = model_.getPathGroupByName(menuItem_->text(0));

	PathParameters params;
	auto path = std::make_shared<RobotPath>(group, name, params);

	//
	// The initialize waypoints are 0, 0, and 0, 1 m, but the 1 meter is
	// converted to the units being used by the path data model
	//
	double xval = UnitConverter::convert(1.0, "m", model_.units());

	path->addWayPoint(Pose2dWithRotation(Translation2d(0, 0), Rotation2d::fromDegrees(0.0), Rotation2d::fromDegrees(0.0)));
	path->addWayPoint(Pose2dWithRotation(Translation2d(xval , 0), Rotation2d::fromDegrees(0.0), Rotation2d::fromDegrees(0.0)));

	model_.blockSignals(true);
	model_.addPath(path);
	model_.blockSignals(false);
}

void PathWindow::deletePath()
{
	assert(menuItem_ != nullptr);
	QString grname = menuItem_->parent()->text(0);
	QString pname = menuItem_->text(0);

	delete menuItem_;

	model_.blockSignals(true);
	model_.deletePath(grname, pname);
	model_.blockSignals(false);
}

bool PathWindow::isValidName(const QString &name)
{
	bool valid = true;
	for (QChar ch : name) {
		if (!ch.isLetterOrNumber()) {
			valid = false;
			break;
		}
	}

	return valid;
}

QString PathWindow::newGroupName()
{
	QString name = "NewGroup";
	int count = 1;

	if (model_.hasGroup(name)) {
		while (true) {
			name = "NewGroup" + QString::number(count);
			if (!model_.hasGroup(name)) {
				break;
			}
			count++;
		}
	}

	return name;
}

QString PathWindow::newPathName(const QString& grname)
{
	QString name = "NewPath";
	int count = 1;

	if (model_.hasPath(grname, name)) {
		while (true) {
			name = "NewPath" + QString::number(count);
			if (!model_.hasPath(grname, name)) {
				break;
			}
			count++;
		}
	}

	return name;
}