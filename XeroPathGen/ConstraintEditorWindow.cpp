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
	index_ = 0;
}

void ConstraintEditorWindow::addConstraintToTree(std::shared_ptr<PathConstraint> c)
{
	constraint_index_map_.insert(index_, c);
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText(0, c->toString());
	item->setData(0, Qt::UserRole, QVariant(index_));
	addTopLevelItem(item);

	index_++;
}

void ConstraintEditorWindow::refresh()
{
	clear();
	constraint_index_map_.clear();
	index_ = 0;

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

	label = "Distance (" + path_->units() + ")";
	double maxdist = UnitConverter::convert(50.0, "m", path()->units());
	double after = QInputDialog::getDouble(this, "Apply Velocity After Distance", label, 0.0, 0.0, maxdist, 3, &ok, flags, 0.01);
	double before = QInputDialog::getDouble(this, "Apply Velocity Before Distance", label, after, after, maxdist, 3, &ok, flags, 0.01);

	auto c = std::make_shared<DistanceVelocityConstraint>(path_, after, before, velocity);
	path_->addConstraint(c);
	addConstraintToTree(c);
}

void ConstraintEditorWindow::deleteConstraint()
{
	int index = menuItem_->data(0, Qt::UserRole).toInt();
	path_->deleteConstraint(constraint_index_map_.value(index));
	constraint_index_map_.remove(index);
	delete menuItem_;
}

void ConstraintEditorWindow::constraintDoubleClicked(QTreeWidgetItem* item, int column)
{
	QString str;
	Qt::WindowFlags flags;
	bool ok;

	int index = item->data(0, Qt::UserRole).toInt();
	if (index < 0 || index >= index_ || !constraint_index_map_.contains(index)) {
		return;
	}

	std::shared_ptr<PathConstraint> c = constraint_index_map_.value(index);

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

			label = "Distance (" + path_->units() + ")";
			double maxdist = UnitConverter::convert(50.0, "m", path()->units());
			double after = QInputDialog::getDouble(this, "Apply Velocity After Distance", label, dist->getAfter(), 0.0, maxdist, 2, &ok, flags, 0.01);
			double before = QInputDialog::getDouble(this, "Apply Velocity Before Distance", label, dist->getBefore(), after, maxdist, 2, &ok, flags, 0.01);

			dist->update(after, before, velocity);
			item->setText(0, dist->toString());
		}
	}
}
