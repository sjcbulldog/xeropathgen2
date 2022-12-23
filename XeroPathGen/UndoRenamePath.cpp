#include "UndoRenamePath.h"
#include "PathsDataModel.h"

void UndoRenamePath::apply()
{
	model_.renamePath(grname_, newname_, oldname_, false);
}
