#pragma once
#include "UndoAction.h"

class PathsDataModel;
class PathGroup;

class UndoDeleteGroup : public UndoAction
{
public:
	UndoDeleteGroup(PathGroup* gr, PathsDataModel& model) : model_(model) {
		group_ = gr;
	}

	void apply();

private:
	PathsDataModel& model_;
	PathGroup* group_;
};

