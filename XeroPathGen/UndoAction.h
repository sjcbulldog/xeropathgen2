#pragma once

#include <cassert>

class PathsDataModel;
class RobotPath;
class PathConstraint;

class UndoAction
{
public:
	virtual void apply() = 0;
};

