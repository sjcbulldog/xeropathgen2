#include "UndoSetUnits.h"
#include "PathsDataModel.h"

void UndoSetUnits::apply()
{
	model_.setUnits(units_, false);
}