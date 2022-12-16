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

	menuItem_ = nullptr;
}

void ConstraintEditorWindow::refresh()
{
	clear();

	if (path_ != nullptr)
	{
		for (auto c : path_->constraints())
		{
			QTreeWidgetItem* item = new QTreeWidgetItem();
			item->setText(0, c->toString());
			addTopLevelItem(item);
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
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText(0, c->toString());
	addTopLevelItem(item);
	path_->addConstraint(c);
}

void ConstraintEditorWindow::addDistanceVelocityConstraint()
{
	QString label = "Velocity (" + path_->units() + "/s)";
	double velocity = QInputDialog::getDouble(this, "Enter Velocity Limit", label);

	label = "Distance (" + path_->units() + ")";
	double after = QInputDialog::getDouble(this, "Apply Velocity After Distance", label);
	double before = QInputDialog::getDouble(this, "Apply Velocity Before Distance", label);

	auto c = std::make_shared<DistanceVelocityConstraint>(path_, after, before, velocity);
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText(0, c->toString());
	addTopLevelItem(item);
	path_->addConstraint(c);
}

void ConstraintEditorWindow::deleteConstraint()
{
	path_->deleteConstraint(menuItem_->text(0));
	delete menuItem_;
}