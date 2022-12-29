#include "UndoDeleteGroup.h"
#include "PathsDataModel.h"

void UndoDeleteGroup::apply()
{
	model_.blockSignals(true);
	model_.insertGroup(group_, index_);
	model_.blockSignals(false);
}