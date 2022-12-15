#include "PathsDataModel.h"
#include "RobotPath.h"
#include <QtCore/QFile>
#include <QtCore/QJsonParseError>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <stdexcept>

PathsDataModel::PathsDataModel(GenerationMgr& genmgr) : gen_mgr_(genmgr)
{
	reset();
	gen_type_ = GeneratorType::CheesyPoofs;
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

void PathsDataModel::addGroup(const QString& grname)
{
	assert(hasGroup(grname) == false);
	PathGroup *gr = new PathGroup(grname);
	groups_.push_back(gr);

	setDirty();
	emit groupAdded(grname);
}

void PathsDataModel::deleteGroup(const QString& grname)
{
	auto it = std::find_if(groups_.begin(), groups_.end(), [&grname](const PathGroup* g) { return g->name() == grname; });
	if (it != groups_.end()) {
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

void PathsDataModel::renameGroup(const QString& oldname, const QString& newname)
{
	auto it = std::find_if(groups_.begin(), groups_.end(), [&oldname](const PathGroup* g) { return g->name() == oldname; });
	if (it == groups_.end()) {
		QString msg = "group '" + oldname + "' does not exist";
		throw std::runtime_error(msg.toStdString());
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

void PathsDataModel::addPath(std::shared_ptr<RobotPath> path)
{
	auto it = std::find_if(groups_.begin(), groups_.end(), [&path](const PathGroup* g) { return g->name() == path->pathGroup()->name(); });
	if (it != groups_.end()) {
		(*it)->addPath(path);
		connect(path.get(), &RobotPath::pathChanged, this, &PathsDataModel::computeSplines);
		setDirty();
		gen_mgr_.addPath(gen_type_, path);
		emit pathAdded(path);
	}
	else {
		QString msg = "group '" + path->pathGroup()->name() + "' does not exist";
		throw std::runtime_error(msg.toStdString());
	}
}

void PathsDataModel::deletePath(const QString& grname, const QString& pathname)
{
	auto it = std::find_if(groups_.begin(), groups_.end(), [&grname](const PathGroup* g) { return g->name() == grname; });
	if (it != groups_.end()) {
		auto path = (*it)->getPathByName(pathname);
		if (path != nullptr) {
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

void PathsDataModel::renamePath(const QString& grname, const QString& oldname, const QString& newname)
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

	path->setName(newname);
	setDirty();
	emit pathRenamed(grname, oldname, newname);
}

void PathsDataModel::computeSplines(const QString& grname, const QString& pathname)
{
	setDirty();

	auto path = getPathByName(grname, pathname);
	assert(path != nullptr);

	gen_mgr_.addPath(gen_type_, path);
	computeSplinesForPath(path);
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

	splines_.insert(path.get(), splines);
}

QVector<std::shared_ptr<SplinePair>> PathsDataModel::getSplinesForPath(std::shared_ptr<RobotPath> path)
{
	if (!splines_.contains(path.get())) {
		computeSplinesForPath(path);
	}

	QVector<std::shared_ptr<SplinePair>> splines = splines_[path.get()];
	return splines;
}

bool PathsDataModel::load(const QString& filename, QString& msg)
{
	QFile file(filename);
	QString text;

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

	if (version > 2)
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
		// problem, but since most of our recent path files are in meters, we assume meters.
		//
		units_ = "m";
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

	addGroup(name);
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
		addPath(path);
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

	obj[RobotPath::VersionTag] = "2";
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

		units_ = units;
	}
}