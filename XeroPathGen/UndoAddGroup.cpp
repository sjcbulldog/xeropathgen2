#include "UndoAddGroup.h"
#include "PathsDataModel.h"
#include <cassert>

void UndoAddGroup::apply()
{
	assert(model_.getPathGroupByName(grname_)->paths().size() == 0);

	model_.deleteGroup(grname_, false);
}