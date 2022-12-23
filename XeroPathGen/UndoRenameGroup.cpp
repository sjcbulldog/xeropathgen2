#include "UndoRenameGroup.h"
#include "PathsDataModel.h"

void UndoRenameGroup::apply()
{
	model_.renameGroup(newname_, oldname_, false);
}