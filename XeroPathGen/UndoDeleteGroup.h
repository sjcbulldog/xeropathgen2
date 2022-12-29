#pragma once
#include "UndoAction.h"

class PathsDataModel;
class PathGroup;

class UndoDeleteGroup : public UndoAction
{
public:
	UndoDeleteGroup(PathGroup* gr, int index, PathsDataModel& model) : model_(model) {
		group_ = gr;
		index_ = index;
	}

	void apply();

	PathGroup* group() {
		return group_;
	}
	
	int index() {
		return index_;
	}

private:
	PathsDataModel& model_;
	PathGroup* group_;
	int index_;
};

