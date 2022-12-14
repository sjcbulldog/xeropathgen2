#pragma once

#include <QtWidgets/QStyledItemDelegate>

class NoEditDelegate : public QStyledItemDelegate
{
public:
	NoEditDelegate(QObject* parent) : QStyledItemDelegate(parent) {
	}

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
		return nullptr;
	}
};

