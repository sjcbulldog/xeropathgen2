#pragma once
#include "UndoAction.h"
#include "GeneratorType.h"

class PathsDataModel;

class UndoSetGeneratorType : public UndoAction
{
public:
	UndoSetGeneratorType(GeneratorType gentype, PathsDataModel &model) : model_(model) {
		gentype_ = gentype;
	}

	void apply() override;

private:
	PathsDataModel& model_;
	GeneratorType gentype_;
};
