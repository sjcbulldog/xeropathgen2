#include "ConstraintEditorWindow.h"
#include "CentripetalConstraint.h"
#include "DistanceVelocityConstraint.h"
#include <QtWidgets/QMenu>
#include <QtWidgets/QInputDialog>

ConstraintEditorWindow::ConstraintEditorWindow(QWidget* parent) : QTreeWidget(parent)
{
	setHeaderHidden(true);
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QTreeWidget::customContextMenuRequested, this, &ConstraintEditorWindow::prepareCustomMenu);
	connect(this, &QTreeWidget::itemDoubleClicked, this, &ConstraintEditorWindow::constraintDoubleClicked);

	menuItem_ = nullptr;
}

void ConstraintEditorWindow::addConstraintToTree(std::shared_ptr<PathConstraint> c)
{
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText(0, c->toString());
	addTopLevelItem(item);
	constraints_.push_back(c);
}

void ConstraintEditorWindow::refresh()
{
	clear();
	constraints_.clear();

	if (path_ != nullptr)
	{
		for (auto c : path_->constraints())
		{
			addConstraintToTree(c);
		}
	}
}

void ConstraintEditorWindow::prepareCustomMenu(const QPoint& pos)
{
	if (path_ == nullptr)
		return;

	QMenu menu(this);
	QAction* act;

	menuItem_ = itemAt(pos);
	if (menuItem_ == nullptr) {
		//
		// We are not on an item: add constraint
		//
		act = new QAction(tr("Add Centripetal Force Constraint"));
		connect(act, &QAction::triggered, this, &ConstraintEditorWindow::addCentripetalConstraint);
		menu.addAction(act);

		act = new QAction(tr("Add Distance/Velocity Constraint"));
		connect(act, &QAction::triggered, this, &ConstraintEditorWindow::addDistanceVelocityConstraint);
		menu.addAction(act);
	}
	else if (menuItem_->parent() == nullptr) {
		//
		// This is a group item: delete group, add path
		//
		act = new QAction(tr("Delete Constraint"));
		connect(act, &QAction::triggered, this, &ConstraintEditorWindow::deleteConstraint);
		menu.addAction(act);
	}

	menu.exec(this->mapToGlobal(pos));
	menuItem_ = nullptr;
}

void ConstraintEditorWindow::addCentripetalConstraint()
{
	double d = QInputDialog::getDouble(this, "Enter Maximum Centripetal Force in Newtons", "Force (N)");
	auto c = std::make_shared<CentripetalConstraint>(path_, d);
	path_->addConstraint(c);
	addConstraintToTree(c);
}

void ConstraintEditorWindow::addDistanceVelocityConstraint()
{
	bool ok;
	Qt::WindowFlags flags;

	QString label = "Velocity (" + path_->units() + "/s)";
	double maxvel = UnitConverter::convert(10.0, "m", path()->units());
	double velocity = QInputDialog::getDouble(this, "Enter Velocity Limit", label, 0.0, 0.0, maxvel, 3, &ok, flags, 0.01);

	label = "After Distance (" + path_->units() + ")";
	double maxdist = UnitConverter::convert(50.0, "m", path()->units());
	double after = QInputDialog::getDouble(this, "Apply Velocity After Distance", label, 0.0, 0.0, maxdist, 3, &ok, flags, 0.01);
	label = "Before Distance (" + path_->units() + ")";
	double before = QInputDialog::getDouble(this, "Apply Velocity Before Distance", label, after, after, maxdist, 3, &ok, flags, 0.01);

	auto c = std::make_shared<DistanceVelocityConstraint>(path_, after, before, velocity);
	path_->addConstraint(c);
	addConstraintToTree(c);
}

void ConstraintEditorWindow::deleteConstraint()
{
	int index = indexOfTopLevelItem(menuItem_);
	path_->deleteConstraint(constraints_.at(index));
	constraints_.removeAt(index);
	delete menuItem_;
}

void ConstraintEditorWindow::constraintDoubleClicked(QTreeWidgetItem* item, int column)
{
	QString str;
	Qt::WindowFlags flags;
	bool ok;

	int index = indexOfTopLevelItem(item);

	std::shared_ptr<PathConstraint> c = constraints_[index];

	std::shared_ptr<CentripetalConstraint> cent = std::dynamic_pointer_cast<CentripetalConstraint>(c);
	if (cent != nullptr) {
		double d = QInputDialog::getDouble(this, "Enter Maximum Centripetal Force in Newtons", "Force (N)", cent->maxCenForce(), 0.0, 500.0, 3, &ok, flags, 0.01);
		cent->setMaxCenForce(d);
		item->setText(0, cent->toString());
	}
	else {
		std::shared_ptr<DistanceVelocityConstraint> dist = std::dynamic_pointer_cast<DistanceVelocityConstraint>(c);
		if (dist != nullptr) {
			double maxvel = UnitConverter::convert(10.0, "m", path()->units());
			QString label = "Velocity (" + path_->units() + "/s)";
			double velocity = QInputDialog::getDouble(this, "Enter Velocity Limit", label, dist->getVelocity(), 0.0, maxvel, 3, &ok, flags, 0.01);

			label = "After Distance (" + path_->units() + ")";
			double maxdist = UnitConverter::convert(50.0, "m", path()->units());
			double after = QInputDialog::getDouble(this, "Apply Velocity After Distance", label, dist->getAfter(), 0.0, maxdist, 2, &ok, flags, 0.01);

			label = "Before Distance (" + path_->units() + ")";
			double before = QInputDialog::getDouble(this, "Apply Velocity Before Distance", label, dist->getBefore(), after, maxdist, 2, &ok, flags, 0.01);

			dist->update(after, before, velocity);
			item->setText(0, dist->toString());
		}
	}
}

void ConstraintEditorWindow::deleteConstraintFromDisplay(std::shared_ptr<PathConstraint> c)
{
	int which = -1;

	for (int i = 0; i < constraints_.size(); i++) {
		if (constraints_[i] == c) {
			which = i;
			break;
		}
	}

	if (which != -1) {
		QTreeWidgetItem* item = topLevelItem(which);
		delete item;
	}
}

void ConstraintEditorWindow::updateConstraintInDisplay(void *ptr)
{
	int which = -1;

	for (int i = 0; i < constraints_.size(); i++) {
		if (constraints_[i].get() == ptr) {
			which = i;
			break;
		}
	}

	if (which != -1) {
		topLevelItem(which)->setText(0, constraints_[which]->toString());
	}
}

void ConstraintEditorWindow::insertConstraint(std::shared_ptr<PathConstraint> c, int index)
{
	constraints_.insert(index, c);
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText(0, c->toString());
	insertTopLevelItem(index, item);
}
