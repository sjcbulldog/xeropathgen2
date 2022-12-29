#include "UndoDeletePath.h"
#include "PathsDataModel.h"

void UndoDeletePath::apply()
{
	model_.blockSignals(true);
	model_.insertPath(path_, index_);
	model_.blockSignals(false);
}