#include "PathWindow.h"
#include <QtWidgets/QMenu>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMessageBox>
#include <QtGui/QActionGroup>

PathWindow::PathWindow(PathsDataModel& model, QWidget* parent) : QTreeWidget(parent), model_(model)
{
	unknown_ = QBrush(QColor(0, 0, 0));
	bad_ = QBrush(QColor(255, 0, 0));
	good_ = QBrush(QColor(0, 153, 0));

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

void PathWindow::trajectoryGenerationError(std::shared_ptr<RobotPath> path, bool error)
{
	QTreeWidgetItem* groupItem = nullptr ;
	QTreeWidgetItem* pathItem = nullptr;

	for (int i = 0; i < topLevelItemCount(); i++) {
		QTreeWidgetItem* item = topLevelItem(i);
		if (item->text(0) == path->pathGroup()->name()) {
			groupItem = item;
			break;
		}
	}

	if (groupItem == nullptr) {
		return;
	}

	for(int i = 0 ; i < groupItem->childCount() ; i++) {
		QTreeWidgetItem* item = groupItem->child(i);
		if (item->text(0) == path->name()) {
			pathItem = item;
			break;
		}
	}

	if (pathItem == nullptr) {
		return;
	}

	if (error) {
		pathItem->setForeground(0, bad_);
	}
	else {
		pathItem->setForeground(0, good_);
	}
}


void PathWindow::itemRenamed(QTreeWidgetItem* item, int column)
{
	assert(column == 0);

	QString oldname = item->data(0, Qt::UserRole).toString();
	QString newname = item->text(0);
	if (oldname == newname) {
		//
		// This is trigerred via an item changed signal.  This could be the background changing for the item
		// in which case the names are the same.  Here we do nothing to the data model.
		//
		return;
	}

	if (!isValidName(newname))
	{
		QMessageBox::critical(this, "Invalid Name", "The name '" + newname + "' is not a valid name.  A name must consist of letters and numbers only.");
		item->setText(0, oldname);
		return;
	}

	if (item->parent() == nullptr) {
		if (model_.hasGroup(newname))
		{
			QMessageBox::critical(this, "Invalid Name", "There is already a group named '" + newname + "'.");
			item->setText(0, oldname);
			return;
		}

		model_.blockSignals(true);
		model_.renameGroup(oldname, newname);
		item->setData(0, Qt::UserRole, newname);
		model_.blockSignals(false);
	}
	else
	{
		QString grname = item->parent()->text(0);
		if (model_.hasPath(grname, newname))
		{
			QMessageBox::critical(this, "Invalid Name", "There is already a path named '" + newname + "' in the group '" + grname + "'.");
			item->setText(0, oldname);
			return;
		}

		model_.blockSignals(true);
		model_.renamePath(grname, oldname, newname);
		item->setData(0, Qt::UserRole, newname);
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
			selected_path_ = model_.getPathByName(current->parent()->text(0), current->text(0));
			emit pathSelected(selected_path_);
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

	act = new QAction(tr("Change Path Units"));
	connect(act, &QAction::triggered, this, &PathWindow::changePathUnits);
	menu.addAction(act);

	QMenu* genmenu = menu.addMenu("Generator");
	QActionGroup* group = new QActionGroup(this);
	auto gens = getGeneratorTypes();
	for (const GeneratorDescriptor& desc : gens) {
		act = new QAction(desc.desc_);
		genmenu->addAction(act);
		group->addAction(act);
		act->setCheckable(true);

		if (desc.type_ == model_.generatorType()) {
			act->setChecked(true);
		}

		connect(act, &QAction::triggered, [this, desc]() { model_.setGeneratorType(desc.type_); });
	}

	menu.exec(this->mapToGlobal(pos));
	menuItem_ = nullptr;
}

void PathWindow::changePathUnits()
{
	QStringList units = UnitConverter::getAllLengthUnits();
	QString newunits = QInputDialog::getText(this, "New Units", "Length Units");
	if (!units.contains(newunits)) 
	{
		QString possible = units.join(",");
		QString msg = "The units '" + newunits + "' are not valid.  Only the units " + possible + " are supported.";
		QMessageBox::critical(this, "Bad Length Units", msg);
	}
	else 
	{
		model_.setUnits(newunits);
	}
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

	double maxvel = (robot_ == nullptr) ? 1.0 : robot_->getMaxVelocity();
	double maxaccel = (robot_ == nullptr) ? 1.0 : robot_->getMaxAccel();
	PathParameters params(0.0, 0.0, maxvel, maxaccel);

	auto path = std::make_shared<RobotPath>(group, units_, name, params);

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