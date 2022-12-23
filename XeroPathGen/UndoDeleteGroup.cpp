#include "UndoDeleteGroup.h"
#include "PathsDataModel.h"

void UndoDeleteGroup::apply()
{
	model_.addGroup(group_);
}