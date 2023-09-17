//
// Copyright 2022 Jack W. Griffin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
//
#include "PathsDataModel.h"
#include "RobotPath.h"
#include "TrajectoryUtils.h"
#include "UndoAddGroup.h"
#include "UndoDeleteGroup.h"
#include "UndoRenameGroup.h"
#include "UndoAddPath.h"
#include "UndoDeletePath.h"
#include "UndoRenamePath.h"
#include <QtCore/QFile>
#include <QtCore/QJsonParseError>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <stdexcept>

PathsDataModel::PathsDataModel(GenerationMgr& genmgr) : gen_mgr_(genmgr)
{
	reset();
	gen_type_ = GeneratorType::CheesyPoofs;
	default_units_ = "m";
	gen_type_ = GeneratorType::None;
	generation_enabled_ = true;
}

PathsDataModel::~PathsDataModel()
{
	for (auto group : groups_) {
		delete group;
	}

	groups_.clear();
}

void PathsDataModel::reset()
{
	filename_.clear();
	groups_.clear();
	splines_.clear();
	undo_stack_.clear();
	dirty_ = false;
}

QStringList PathsDataModel::groupNames() const
{
	QStringList names;

	for (PathGroup* gr : groups_) {
		names.push_back(gr->name());
	}

	return names;
}

bool PathsDataModel::hasGroup(const QString& grname) const
{
	auto it = std::find_if(groups_.begin(), groups_.end(), [&grname](const PathGroup *g) { return g->name() == grname; });
	return it != groups_.end();
}

void PathsDataModel::addGroup(const QString& grname, bool undoentry)
{
	if (undoentry) {
		addUndoStackEntry(std::make_shared<UndoAddGroup>(grname, *this));
	}
	assert(hasGroup(grname) == false);
	PathGroup *gr = new PathGroup(grname);
	groups_.push_back(gr);

	setDirty();
	emit groupAdded(grname);
}

void PathsDataModel::insertGroup(PathGroup *gr, int index)
{
	groups_.insert(index, gr);

	setDirty();
	emit groupAdded(gr->name());
}

void PathsDataModel::deleteGroup(const QString& grname, bool undoentry)
{
	auto it = std::find_if(groups_.begin(), groups_.end(), [&grname](const PathGroup* g) { return g->name() == grname; });
	if (it != groups_.end()) {
		if (undoentry) {
			int index = std::distance(groups_.begin(), it);
			addUndoStackEntry(std::make_shared<UndoDeleteGroup>(*it, index, *this));
		}
		groups_.erase(it);
	}

	setDirty();
	emit groupDeleted(grname);
}

QStringList PathsDataModel::pathNames(const QString& grname) const
{
	QStringList names;

	auto it = std::find_if(groups_.begin(), groups_.end(), [&grname](const PathGroup* g) { return g->name() == grname; });
	if (it != groups_.end()) {
		names = (*it)->pathNames();
	}

	return names;
}

const PathGroup* PathsDataModel::getPathGroupByName(const QString& grname)
{
	auto it = std::find_if(groups_.begin(), groups_.end(), [&grname](const PathGroup* g) { return g->name() == grname; });
	if (it == groups_.end()) {
		return nullptr;
	}

	return *it;
}

void PathsDataModel::renameGroup(const QString& oldname, const QString& newname, bool undoentry)
{
	auto it = std::find_if(groups_.begin(), groups_.end(), [&oldname](const PathGroup* g) { return g->name() == oldname; });
	if (it == groups_.end()) {
		QString msg = "group '" + oldname + "' does not exist";
		throw std::runtime_error(msg.toStdString());
	}

	if (undoentry) {
		addUndoStackEntry(std::make_shared<UndoRenameGroup>(newname, oldname, *this));
	}

	(*it)->setName(newname);
	setDirty();
	emit groupRenamed(oldname, newname);
}

bool PathsDataModel::hasPath(const QString& grname, const QString& pathname) const
{
	bool ret = false;

	auto it = std::find_if(groups_.begin(), groups_.end(), [&grname](const PathGroup* g) { return g->name() == grname; });
	if (it != groups_.end()) {
		ret = ((*it)->getPathByName(pathname) != nullptr);
	}

	return ret;
}

void PathsDataModel::addPath(std::shared_ptr<RobotPath> path, bool undoentry)
{
	auto it = std::find_if(groups_.begin(), groups_.end(), [&path](const PathGroup* g) { return g->name() == path->pathGroup()->name(); });
	if (it != groups_.end()) {

		if (undoentry) {
			addUndoStackEntry(std::make_shared<UndoAddPath>(path, *this));
		}

		(*it)->addPath(path);
		setDirty();
		connect(path.get(), &RobotPath::beforePathChanged, this, &PathsDataModel::beforePathChanged);
		connect(path.get(), &RobotPath::afterPathChanged, this, &PathsDataModel::afterPathChanged);
		this->generateTrajectory(path);
		emit pathAdded(path);
	}
	else {
		QString msg = "group '" + path->pathGroup()->name() + "' does not exist";
		throw std::runtime_error(msg.toStdString());
	}
}

void PathsDataModel::insertPath(std::shared_ptr<RobotPath> path, int index)
{
	auto it = std::find_if(groups_.begin(), groups_.end(), [&path](const PathGroup* g) { return g->name() == path->pathGroup()->name(); });
	if (it != groups_.end()) {
		(*it)->insertPath(path, index);
		setDirty();
		connect(path.get(), &RobotPath::beforePathChanged, this, &PathsDataModel::beforePathChanged);
		connect(path.get(), &RobotPath::afterPathChanged, this, &PathsDataModel::afterPathChanged);
		this->generateTrajectory(path);
		emit pathAdded(path);
	}
}

void PathsDataModel::deletePath(const QString& grname, const QString& pathname, bool undoentry)
{
	auto it = std::find_if(groups_.begin(), groups_.end(), [&grname](const PathGroup* g) { return g->name() == grname; });
	if (it != groups_.end()) {
		auto path = (*it)->getPathByName(pathname);
		if (path != nullptr) {
			if (undoentry) {
				int index = (*it)->getPathIndexByName(pathname);
				addUndoStackEntry(std::make_shared<UndoDeletePath>(path, index, *this));
			}
			gen_mgr_.removePath(path);
		}

		(*it)->deletePath(pathname);
		setDirty();
		emit pathDeleted(grname, pathname);
	}
	else {
		QString msg = "group '" + grname + "' does not exist";
		throw std::runtime_error(msg.toStdString());
	}
}

std::shared_ptr<RobotPath> PathsDataModel::getPathByName(const QString& grname, const QString& pathname)
{
	auto it = std::find_if(groups_.begin(), groups_.end(), [&grname](const PathGroup* g) { return g->name() == grname; });
	if (it == groups_.end()) {
		return nullptr;
	}

	return (*it)->getPathByName(pathname);
}

void PathsDataModel::renamePath(const QString& grname, const QString& oldname, const QString& newname, bool undoentry)
{
	auto it = std::find_if(groups_.begin(), groups_.end(), [&grname](const PathGroup* g) { return g->name() == grname; });
	if (it == groups_.end()) {
		QString msg = "group '" + grname + "' does not exist";
		throw std::runtime_error(msg.toStdString());
	}

	auto path = (*it)->getPathByName(oldname);
	if (path == nullptr) {
		QString msg = "path '" + oldname + "' in group '" + grname + "' does not exist";
		throw std::runtime_error(msg.toStdString());
	}

	if (undoentry) {
		addUndoStackEntry(std::make_shared<UndoRenamePath>(grname, oldname, newname, *this));
	}
	path->setName(newname);
	setDirty();
	emit pathRenamed(grname, oldname, newname);
}

QVector<std::shared_ptr<RobotPath>> PathsDataModel::getAllPaths()
{
	QVector<std::shared_ptr<RobotPath>> paths;

	for (auto gr : groups_) {
		for (auto path : gr->paths()) {
			paths.push_back(path);
		}
	}

	return paths;
}

void PathsDataModel::beforePathChanged(std::shared_ptr<UndoAction> undo)
{
	addUndoStackEntry(undo);
}

void PathsDataModel::afterPathChanged(const QString& grname, const QString& pathname)
{
	setDirty();
	computeSplines(grname, pathname);
}

void PathsDataModel::enableGeneration(bool b)
{
	if (b)
	{
		generation_enabled_ = true;
		for (auto path : deferred_) {
			gen_mgr_.addPath(gen_type_, path);
		}
	}
	else
	{
		generation_enabled_ = false;
	}
}

void PathsDataModel::generateTrajectory(std::shared_ptr<RobotPath> path)
{
	if (generation_enabled_) {
		gen_mgr_.addPath(gen_type_, path);
	}
	else {
		deferred_.push_back(path);
	}
}

void PathsDataModel::computeSplines(const QString& grname, const QString& pathname)
{
	auto path = getPathByName(grname, pathname);
	assert(path != nullptr);

	generateTrajectory(path);
	computeSplinesForPath(path);
	distances_.remove(path);
}

void PathsDataModel::computeSplinesForPath(std::shared_ptr<RobotPath> path)
{
	QVector<std::shared_ptr<SplinePair>> splines;

	for (int i = 0; i < path->size() - 1; i++) {
		const Pose2dWithRotation& p1 = path->getPoint(i);
		const Pose2dWithRotation& p2 = path->getPoint(i + 1);
		auto pair = std::make_shared<SplinePair>(p1, p2);
		splines.push_back(pair);
	}

	splines_.insert(path, splines);
}

QVector<std::shared_ptr<SplinePair>> PathsDataModel::getSplinesForPath(std::shared_ptr<RobotPath> path)
{
	if (!splines_.contains(path)) {
		computeSplinesForPath(path);
	}

	QVector<std::shared_ptr<SplinePair>> splines = splines_[path];
	return splines;
}

bool PathsDataModel::load(const QString& filename, QString& msg)
{
	QFile file(filename);
	QString text;

	undo_stack_.clear();

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		msg = "cannot open file '" + file.fileName() + "' for reading";
		return false;
	}

	text = file.readAll();
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &err);
	if (doc.isNull())
	{
		msg = "cannot parse file '" + file.fileName() + "' as JSON - " + err.errorString();
		return false;
	}

	if (!doc.isObject())
	{
		msg = "JSON file '" + file.fileName() + "' does not hold a JSON object";
		return false;
	}

	int version = 1;
	QJsonObject obj = doc.object();
	if (obj.contains(RobotPath::VersionTag))
	{
		QJsonValue vobj = obj[RobotPath::VersionTag];
		if (!vobj.isString())
		{
			msg = "JSON file '" + file.fileName() + "' has '_version' value that is not a string, invalid path file";
			return false;
		}

		bool ok;
		version = vobj.toString().toInt(&ok);

		if (!ok) 
		{
			msg = "JSON file '" + file.fileName() + "' has a '_version' value that is a string, but is not a legal version number, invalid path file";
			return false;
		}
	}

	if (version > 3)
	{
		//
		// When we create a version 2, here is where the logic will go to read both version 1 and version 2 files.
		//
		msg = "JSON file '" + file.fileName() + "' has '_version' value that is greater than the version this program supported - look for a newer version of this program";
		return false;
	}

	if (obj.contains(RobotPath::UnitsTag))
	{
		QJsonValue vobj = obj[RobotPath::UnitsTag];
		if (vobj.isString())
		{
			units_ = vobj.toString();
		}
	}
	
	if (units_.length() == 0) {
		//
		// Older versions of the file did not store units in the paths file.  This is a
		// problem so we set the units to the default.  For now the default is meters, but eventually
		// we want to make this user editable
		//
		units_ = default_units_;
	}

	if (obj.contains(RobotPath::GeneratorTag))
	{
		QJsonValue vobj = obj[RobotPath::GeneratorTag];
		if (vobj.isString())
		{
			gen_type_ = keyToType(vobj.toString());
		}
	}

	if (gen_type_ == GeneratorType::None)
	{
		gen_type_ = GeneratorType::ErrorCodeXeroSwerve;
	}

	if (obj.contains(RobotPath::OutputTag))
	{
		QJsonValue vobj = obj[RobotPath::OutputTag];
		if (vobj.isString())
		{
			path_output_dir_ = vobj.toString();
		}
	}

	if (!obj.contains(RobotPath::GroupsTag))
	{
		msg = "JSON file '" + file.fileName() + "' does not contain '" + RobotPath::GroupsTag + "'array";
		return false;
	}

	QJsonValue grval = obj[RobotPath::GroupsTag];
	if (!grval.isArray())
	{
		msg = "JSON file '" + file.fileName() + "contains a '" + RobotPath::GroupsTag + "'value, but it is not an array";
		return false;
	}

	QJsonArray groups = grval.toArray();
	for(int i = 0 ; i < groups.size() ; i++)
	{
		QJsonValue groupval = groups.at(i);
		if (!groupval.isObject())
		{
			msg = "JSON file '" + file.fileName() + "', section '" + RobotPath::GroupsTag + ", element " + QString::number(i + 1), ", is not a JSON object";
			return false;
		}

		QJsonObject obj2 = groupval.toObject();
		if (!readPathGroup(file, obj2, msg))
			return false;
	}

	filename_ = filename;
	dirty_ = false;
	return true;
}

bool PathsDataModel::readPathGroup(QFile& file, const QJsonObject& obj, QString &msg)
{
	QString name = RobotPath::getStringParam(obj, RobotPath::NameTag, msg);
	if (msg.length() > 0)
	{
		return false;
	}

	addGroup(name, false);
	const PathGroup* grobj = getPathGroupByName(name);

	if (!obj.contains(RobotPath::PathsTag))
	{
		msg = "JSON file '" + file.fileName() + "', object in 'groups' array missing 'paths' value";
		return false;
	}

	QJsonValue val = obj[RobotPath::PathsTag];
	if (!val.isArray())
	{
		msg = "JSON file '" + file.fileName() + "', object in 'group' array has 'paths' value, but its not an array";
		return false;
	}

	QJsonArray pathsarray = val.toArray();
	for (auto it = pathsarray.begin(); it != pathsarray.end(); it++)
	{
		val = *it;
		if (!val.isObject())
		{
			msg = "JSON file '" + file.fileName() + "', object in 'paths' array is not an object";
			return false;
		}
		QJsonObject obj2 = val.toObject();
		auto path = RobotPath::fromJSONObject(grobj, units_, obj2, msg);
		if (msg.length() > 0) {
			return false;
		}
		addPath(path, false);
	}

	return true;
}

bool PathsDataModel::saveAs(const QString& filename, QString& msg)
{
	if (saveToFile(filename, msg)) {
		filename_ = filename;
		return true;
	}

	return false;
}

bool PathsDataModel::save(QString& msg)
{
	return saveToFile(filename_, msg);
}

bool PathsDataModel::saveToFile(const QString& filename, QString& msg)
{
	QJsonObject obj = modelToObject();
	QJsonDocument doc(obj);
	QFile file(filename);
	if (!file.open(QIODevice::OpenModeFlag::Truncate | QIODevice::OpenModeFlag::WriteOnly))
		return false;

	file.write(doc.toJson());
	file.close();
	dirty_ = false;

	return true;
}

QJsonObject PathsDataModel::modelToObject()
{
	QJsonObject obj;

	QJsonArray a;

	for (auto gr : groups_) {
		QJsonObject grobj;
		QJsonArray patharray;

		for (auto path : gr->paths()) {
			QJsonObject pathobj = path->toJSONObject();
			patharray.append(pathobj);
		}

		grobj.insert(RobotPath::NameTag, gr->name());
		grobj.insert(RobotPath::PathsTag, patharray);

		a.append(grobj);
	}

	obj[RobotPath::VersionTag] = "3";
	obj[RobotPath::UnitsTag] = units_;
	obj[RobotPath::GeneratorTag] = typeToKey(gen_type_);
	obj[RobotPath::OutputTag] = path_output_dir_;
	obj[RobotPath::GroupsTag] = a;

	return obj;
}

void PathsDataModel::convert(const QString& units)
{
	if (units != units_) {
		for (auto gr : groups_) {
			for (auto path : gr->paths()) {
				path->convert(units_, units);
			}
		}

		distances_.clear();
		splines_.clear();
		units_ = units;
	}
}

QVector<double> PathsDataModel::getDistancesForPath(std::shared_ptr<RobotPath> path)
{
	if (!distances_.contains(path))
	{
		QVector<std::shared_ptr<SplinePair>> splines = getSplinesForPath(path);
		auto dists = TrajectoryUtils::getDistancesForSplines(splines);
		distances_.insert(path, dists);
	}
	return distances_.value(path);
}

void PathsDataModel::addUndoStackEntry(std::shared_ptr<UndoAction> undo)
{
	undo_stack_.push_back(undo);
}

std::shared_ptr<UndoAction> PathsDataModel::popUndoStack()
{
	if (undo_stack_.size() == 0) {
		return nullptr;
	}

	auto action = undo_stack_.back();
	undo_stack_.pop_back();
	return action;
}

void PathsDataModel::mirrorPathAboutX(double dim, std::shared_ptr<RobotPath> path)
{
	for (int i = 0; i < path->waypoints().size(); i++) {
		const Pose2dWithRotation& pt = path->getPoint(i);

		Translation2d ntr(dim - pt.getTranslation().getX(), pt.getTranslation().getY());
		Rotation2d trt = Rotation2d::fromDegrees(180.0 - pt.getRotation().toDegrees());
		Rotation2d rot = Rotation2d::fromDegrees(180.0 - pt.getSwrot().toDegrees());


		const Pose2dWithRotation npt(ntr, trt, rot);
		path->replacePoint(i, npt, false);
	}
}

void PathsDataModel::mirrorPathAboutY(double dim, std::shared_ptr<RobotPath> path)
{
	for (int i = 0; i < path->waypoints().size(); i++) {
		const Pose2dWithRotation& pt = path->getPoint(i);

		Translation2d ntr(pt.getTranslation().getX(), dim - pt.getTranslation().getY());
		Rotation2d trt = Rotation2d::fromDegrees(-pt.getRotation().toDegrees());
		Rotation2d rot = Rotation2d::fromDegrees(-pt.getSwrot().toDegrees());


		const Pose2dWithRotation npt(ntr, trt, rot);
		path->replacePoint(i, npt, false);
	}
}