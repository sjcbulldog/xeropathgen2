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
	QString label = "Velocity (" + path_->units() + "/s)";
	double velocity = QInputDialog::getDouble(this, "Enter Velocity Limit", label);

	label = "Distance (" + path_->units() + ")";
	double after = QInputDialog::getDouble(this, "Apply Velocity After Distance", label);
	double before = QInputDialog::getDouble(this, "Apply Velocity Before Distance", label);

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
	int index = item->data(0, Qt::UserRole).toInt();
	if (index < 0 || index >= index_ || !constraint_index_map_.contains(index)) {
		return;
	}

	std::shared_ptr<PathConstraint> c = constraint_index_map_.value(index);

	std::shared_ptr<CentripetalConstraint> cent = std::dynamic_pointer_cast<CentripetalConstraint>(c);
	if (cent != nullptr) {
		double d = QInputDialog::getDouble(this, "Enter Maximum Centripetal Force in Newtons", "Force (N)");
		cent->setMaxCenForce(d);
		item->setText(0, cent->toString());
	}

	std::shared_ptr<DistanceVelocityConstraint> dist = std::dynamic_pointer_cast<DistanceVelocityConstraint>(c);
	if (dist != nullptr) {
		QString label = "Velocity (" + path_->units() + "/s)";
		double velocity = QInputDialog::getDouble(this, "Enter Velocity Limit", label);

		label = "Distance (" + path_->units() + ")";
		double after = QInputDialog::getDouble(this, "Apply Velocity After Distance", label);
		double before = QInputDialog::getDouble(this, "Apply Velocity Before Distance", label);

		dist->update(after, before, velocity);
		item->setText(0, dist->toString());
	}
}
