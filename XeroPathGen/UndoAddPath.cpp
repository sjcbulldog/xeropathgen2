#include "UndoAddPath.h"
#include "PathsDataModel.h"

void UndoAddPath::apply()
{
	model_.deletePath(path_->pathGroup()->name(), path_->name(), false);
}