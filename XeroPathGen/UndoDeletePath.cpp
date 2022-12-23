#include "UndoDeletePath.h"
#include "PathsDataModel.h"

void UndoDeletePath::apply()
{
	model_.addPath(path_, false);
}