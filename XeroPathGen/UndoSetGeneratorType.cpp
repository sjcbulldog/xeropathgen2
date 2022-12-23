#include "UndoSetGeneratorType.h"
#include "PathsDataModel.h"

void UndoSetGeneratorType::apply()
{
	model_.setGeneratorType(gentype_, false);
}