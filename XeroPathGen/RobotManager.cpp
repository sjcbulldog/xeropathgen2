//
// Copyright 2019 Jack W. Griffin
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
#include "RobotManager.h"
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <stdexcept>


RobotManager::RobotManager()
{
}

RobotManager::~RobotManager()
{
}

bool RobotManager::deleteRobot(const QString& robot)
{
	bool ret = false;

	auto it = std::find_if(robots_.begin(), robots_.end(), [robot](std::shared_ptr<RobotParams> one) { return one->getName() == robot; });
	if (it != robots_.end())
	{
		auto robotobj = *it;
		QFile file(robotobj->getFilename());
		file.remove();
		robots_.erase(it);
		ret = true;
	}

	return ret;
}

std::shared_ptr<RobotParams> RobotManager::importRobot(QFile &file)
{
	processJSONFile(file) ;

	auto robot = robots_.back();
	robot->setFilename("");
	if (!add(robot))
	{
		deleteRobot(robot->getName());
		throw std::runtime_error("cannot save robot file");
	}
	return robot;
}

bool RobotManager::exists(const QString& name)
{
	for (auto robot : robots_)
	{
		if (robot->getName() == name)
			return true;
	}

	return false;
}

std::shared_ptr<RobotParams> RobotManager::getDefaultRobot()
{
	if (robots_.size() == 0)
		return nullptr;

	return robots_.front();
}

std::shared_ptr<RobotParams> RobotManager::getRobotByName(const QString& name)
{
	for (auto field : robots_)
	{
		if (field->getName() == name)
			return field;
	}

	return nullptr;
}

std::list<QString> RobotManager::getNames()
{
	std::list<QString> names;

	for (auto field : robots_)
		names.push_back(field->getName());

	return names;
}

std::list<std::shared_ptr<RobotParams>> RobotManager::getRobots()
{
	std::list<std::shared_ptr<RobotParams>> list;
	for (auto field : robots_)
		list.push_back(field);

	return list;
}

bool RobotManager::processJSONFile(QFile& file)
{
	QString name_value, verstr, lengthunits, weightunits;
	double ewidth_value;
	double elength_value;
	double rwidth_value;
	double rlength_value;
	double rweight_value;
	double velocity_value;
	double accel_value;
	double cent_value;
	double timestep_value;
	int drivetype_value;
	QString text;
	int version;
	size_t len;
	bool ok;

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		throw std::runtime_error("cannot open file for reading");

	text = file.readAll();
	QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8());
	if (doc.isNull())
		throw std::runtime_error("invalid JSON text in file");

	if (!doc.isObject())
	{
		qWarning() << "JSON file '" << file.fileName() << "' does not hold a JSON object";
		throw std::runtime_error("valid JSON, but not correct format for a robot file");
	}

	if (!getJSONStringValue(file, doc, RobotParams::VersionTag, verstr))
	{
		qWarning() << "JSON file '" << file.fileName() << "' does not have a '_version' value, assuming version 1";
	}
	else
	{
		version = verstr.toInt(&ok);
		if (!ok) {
			qWarning() << "JSON file '" << file.fileName() << "' has a '_version' string field, that is not a valid integer";
			throw std::runtime_error("valid JSON, but not correct format for a robot file");
		}

		if (version != 1)
		{
			qWarning() << "JSON file '" << file.fileName() << "' has a '_version' field '" << version << "' that is not valid";
			throw std::runtime_error("valid JSON, but not correct format for a robot file");
		}
	}

	if (!getJSONStringValue(file, doc, RobotParams::NameTag, name_value))
		throw std::runtime_error("valid JSON, but not correct format for a robot file");

	if (exists(name_value))
	{
		QString msg = "robot with the name '" + name_value + "' already exists";
		throw std::runtime_error(msg.toStdString());
	}

	if (!getJSONDoubleValue(file, doc, RobotParams::EffectiveWidthTag, ewidth_value))
		throw std::runtime_error("valid JSON, but not correct format for a robot file");

	if (!getJSONDoubleValue(file, doc, RobotParams::EffectiveLengthTag, elength_value))
		throw std::runtime_error("valid JSON, but not correct format for a robot file");

	if (!getJSONDoubleValue(file, doc, RobotParams::RobotWidthTag, rwidth_value))
		throw std::runtime_error("valid JSON, but not correct format for a robot file");

	if (!getJSONDoubleValue(file, doc, RobotParams::RobotWeightTag, rweight_value))
		rweight_value = 180;

	if (!getJSONDoubleValue(file, doc, RobotParams::RobotLengthTag, rlength_value))
		throw std::runtime_error("valid JSON, but not correct format for a robot file");

	if (!getJSONDoubleValue(file, doc, RobotParams::MaxVelocityTag, velocity_value))
		throw std::runtime_error("valid JSON, but not correct format for a robot file");

	if (!getJSONDoubleValue(file, doc, RobotParams::MaxAccelerationTag, accel_value))
		throw std::runtime_error("valid JSON, but not correct format for a robot file");

	if (!getJSONDoubleValue(file, doc, RobotParams::MaxCentripetalTag, cent_value))
	{
		cent_value = 1200;
	}

	if (!getJSONDoubleValue(file, doc, RobotParams::TimeStepTag, timestep_value))
		throw std::runtime_error("valid JSON, but not correct format for a robot file");

	if (!getJSONIntegerValue(file, doc, RobotParams::DriveTypeTag, drivetype_value))
		throw std::runtime_error("valid JSON, but not correct format for a robot file");

	std::shared_ptr<RobotParams> robot = std::make_shared<RobotParams>(name_value);
	robot->setFilename(file.fileName());
	robot->setTimestep(timestep_value);
	robot->setWheelBaseWidth(ewidth_value);
	robot->setWheelBaseLength(elength_value);
	robot->setBumberWidth(rwidth_value);
	robot->setBumberLength(rlength_value);
	robot->setRobotWeight(rweight_value);
	robot->setMaxVelocity(velocity_value);
	robot->setMaxAcceleration(accel_value);
	robot->setDriveType(static_cast<RobotParams::DriveType>(drivetype_value));

	if (!getJSONStringValue(file, doc, RobotParams::LengthUnitsTag, lengthunits))
		robot->setLengthUnits(RobotParams::DefaultLengthUnits);
	else
		robot->setLengthUnits(lengthunits);

	if (!getJSONStringValue(file, doc, RobotParams::WeightUnitsTag, weightunits))
		robot->setWeightUnits(RobotParams::DefaultWeightUnits);
	else
		robot->setWeightUnits(weightunits);

	robots_.push_back(robot);
	return true;
}

bool RobotManager::getNextFile(QFileInfo &file)
{
	size_t i = 1;
	while (i < 1024)
	{
		QString path = getFirstDir();
		path += "//";
		path += QString::number(i) + ".json";

		QFileInfo info(path);
		if (!info.exists())
		{
			file = info;
			return true;
		}

		i++;
	}
	return false;
}

bool RobotManager::add(std::shared_ptr<RobotParams> robot)
{
	robots_.push_back(robot);
	return save(robot);
}

bool RobotManager::save(std::shared_ptr<RobotParams> robot)
{
	QFile* file;
	bool ret;

	if (robot->getFilename().length() > 0)
	{
		file = new QFile(robot->getFilename());
	}
	else
	{
		QFileInfo fileinfo;

		if (!getNextFile(fileinfo))
			return false;

		file = new QFile(fileinfo.absoluteFilePath());
		robot->setFilename(fileinfo.absoluteFilePath());
	}

	ret = save(robot, *file);
	delete file;

	return ret;
}

std::shared_ptr<RobotParams> RobotManager::load(QFile& file)
{
	if (!processJSONFile(file)) {
		return nullptr;
	}

	auto ret = robots_.back();
	robots_.pop_back();

	return ret;
}

bool RobotManager::save(std::shared_ptr<RobotParams> robot, QFile &file)
{
	QJsonObject obj;
	QFileInfo info(file);
	QDir dir = info.absoluteDir();

	if (!dir.exists())
		dir.mkpath(dir.path());

	obj[RobotParams::VersionTag] = "1";
	obj[RobotParams::NameTag] = robot->getName();
	obj[RobotParams::DriveTypeTag] = static_cast<int>(robot->getDriveType());
	obj[RobotParams::TimeStepTag] = robot->getTimestep();
	obj[RobotParams::EffectiveWidthTag] = robot->getWheelBaseWidth();
	obj[RobotParams::EffectiveLengthTag] = robot->getWheelBaseLength();
	obj[RobotParams::RobotWidthTag] = robot->getBumberWidth();
	obj[RobotParams::RobotLengthTag] = robot->getBumberLength();
	obj[RobotParams::RobotWeightTag] = robot->getRobotWeight();
	obj[RobotParams::MaxVelocityTag] = robot->getMaxVelocity();
	obj[RobotParams::MaxAccelerationTag] = robot->getMaxAccel();
	obj[RobotParams::LengthUnitsTag] = robot->getLengthUnits();
	obj[RobotParams::WeightUnitsTag] = robot->getWeightUnits();

	QJsonDocument doc(obj);
	file.open(QFile::WriteOnly);
	file.write(doc.toJson());
	file.close();
	return true;
}
