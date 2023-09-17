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
#include "RobotPath.h"
#include "PathGroup.h"
#include "CentripetalConstraint.h"
#include "DistanceVelocityConstraint.h"
#include "UndoAction.h"
#include "UndoRenamePath.h"
#include "UndoChangePathParams.h"
#include "UndoChangeWaypoint.h"
#include "UndoRemovePoint.h"
#include "UndoInsertPoint.h"
#include "UndoAddConstraint.h"
#include "UndoDeleteConstraint.h"
#include <QtCore/QJsonArray>
#include <limits>

RobotPath::RobotPath(const PathGroup* gr, const QString& units, const QString &name, const PathParameters &params)
{
	group_ = gr;
	name_ = name;
	params_ = params;
	units_ = units;
}

RobotPath::RobotPath(const PathGroup* gr, const QString &name, const RobotPath& other)
{
	group_ = gr;
	name_ = name;
	params_ = other.params_;
	units_ = other.units_;

	for (const Pose2dWithRotation& pt : other.waypoints()) {
		addWayPoint(pt);
	}

	for (const std::shared_ptr<PathConstraint> con : other.constraints()) {
		addConstraint(con->clone(shared_from_this()));
	}
}

QString RobotPath::fullname() const {
	return group_->name() + "-" + name_;
}

void RobotPath::setName(const QString& name, bool undoentry) 
{
	name_ = name;
	emitAfterPathChangedSignal();
}

void RobotPath::setParams(const PathParameters& p, bool undoentry) 
{
	if (undoentry) {
		emitBeforePathChangedSignal(std::make_shared<UndoChangePathParams>(params_, shared_from_this()));
	}
	params_ = p;
	emitAfterPathChangedSignal();
}

void RobotPath::replacePoint(int index, const Pose2dWithRotation& pt, bool undoentry) {
	if (undoentry) {
		emitBeforePathChangedSignal(std::make_shared<UndoChangeWaypoint>(index, waypoints_[index], shared_from_this()));
	}
	waypoints_[index] = pt;
	emitAfterPathChangedSignal();
}

void RobotPath::removePoint(int index, bool undoentry) 
{
	if (undoentry) {
		emitBeforePathChangedSignal(std::make_shared<UndoRemovePoint>(index, waypoints_[index], shared_from_this()));
	}
	waypoints_.remove(index, 1);
	emitAfterPathChangedSignal();

}

void RobotPath::insertPoint(int index, const Pose2dWithRotation& pt, bool undoentry) 
{
	if (undoentry) {
		emitBeforePathChangedSignal(std::make_shared<UndoInsertPoint>(index + 1, shared_from_this()));
	}
	waypoints_.insert(index + 1, pt);
	emitAfterPathChangedSignal();
}

void RobotPath::addConstraint(std::shared_ptr<PathConstraint> c, bool undoentry) 
{
	if (undoentry) {
		emitBeforePathChangedSignal(std::make_shared<UndoAddConstraint>(c, shared_from_this()));
	}
	constraints_.push_back(c);
	emitAfterPathChangedSignal();
}

void RobotPath::insertConstraint(std::shared_ptr<PathConstraint> c, int index)
{
	constraints_.insert(index, c);
	emitAfterPathChangedSignal();
}

void RobotPath::deleteConstraint(std::shared_ptr<PathConstraint> c, bool undoentry) {
	auto it = std::find(constraints_.begin(), constraints_.end(), c);
	if (it != constraints_.end()) {
		if (undoentry) {
			int index = constraints_.indexOf(c);
			emitBeforePathChangedSignal(std::make_shared<UndoDeleteConstraint>(c, index, shared_from_this()));
		}
		constraints_.erase(it);
		emitAfterPathChangedSignal();
	}
}

void RobotPath::emitBeforePathChangedSignal(std::shared_ptr<UndoAction> action)
{
	emit beforePathChanged(action);
}

void RobotPath::emitAfterPathChangedSignal()
{
	emit afterPathChanged(group_->name(), name_);
}

void RobotPath::convert(const QString& from, const QString& to)
{
	for (auto con : constraints_) {
		con->convert(from, to);
	}

	params_.convert(from, to);

	bool save = signalsBlocked();
	blockSignals(true);

	for (int i = 0; i < size(); i++) {
		const Pose2dWithRotation pt = getPoint(i);
		double newx = UnitConverter::convert(pt.getTranslation().getX(), from, to);
		double newy = UnitConverter::convert(pt.getTranslation().getY(), from, to);
		Pose2dWithRotation newpt(Translation2d(newx, newy), pt.getRotation(), pt.getSwrot());

		replacePoint(i, newpt);
	}

	units_ = to;
	
	blockSignals(save);

	emitAfterPathChangedSignal();
}


double RobotPath::getDoubleParam(const QJsonObject& obj, const QString& tag, QString &msg)
{
	if (!obj.contains(tag))
	{
		msg = "Robot Path was missing '" + tag + "' entry";
		return std::numeric_limits<double>::max();
	}

	if (!obj.value(tag).isDouble())
	{
		msg = "Robot Path has '" + tag + "' entry but it was not a number";
		return std::numeric_limits<double>::max();
	}

	return obj.value(tag).toDouble();
}

QString RobotPath::getStringParam(const QJsonObject& obj, const QString& tag, QString& msg)
{
	if (!obj.contains(tag))
	{
		msg = "Robot Path was missing '" + tag + "' entry";
		return QString();
	}

	if (!obj.value(tag).isString())
	{
		msg = "Robot Path has '" + tag + "' entry but it was not a string";
		return QString();
	}

	return obj.value(tag).toString();
}


QJsonObject RobotPath::getObjectParam(const QJsonObject& obj, const QString& tag, QString& msg)
{
	if (!obj.contains(tag))
	{
		msg = "Robot Path was missing '" + tag + "' entry";
		return QJsonObject();
	}

	if (!obj.value(tag).isObject())
	{
		msg = "Robot Path has '" + tag + "' entry but it was not a json object";
		return QJsonObject();
	}

	return obj.value(tag).toObject();
}

QJsonArray RobotPath::getArrayParam(const QJsonObject& obj, const QString& tag, QString& msg)
{
	if (!obj.contains(tag))
	{
		msg = "Robot Path was missing '" + tag + "' entry";
		return QJsonArray();
	}

	if (!obj.value(tag).isArray())
	{
		msg = "Robot Path has '" + tag + "' entry but it was not a json array";
		return QJsonArray();
	}

	return obj.value(tag).toArray();
}

bool RobotPath::readConstraints(std::shared_ptr<RobotPath> path, const QJsonArray& obj, QString& msg)
{
	for (int i = 0; i < obj.count(); i++) 
	{
		if (!obj.at(i).isObject()) {
			msg = "Robot Path has constraints array entry that is not a json object";
			return false;
		}

		QJsonObject cobj = obj.at(i).toObject();

		std::shared_ptr<PathConstraint> constraint;

		constraint = CentripetalConstraint::fromJSON(path, cobj, msg);
		if (msg.length() > 0) {
			return false;
		}

		if (constraint == nullptr) {
			constraint = DistanceVelocityConstraint::fromJSON(path, cobj, msg);
			if (msg.length() > 0) {
				return false;
			}
		}

		if (constraint == nullptr) {
			if (!cobj.contains("type")) {
				msg = "Robot Path has constraint object without 'type' field";
				return false;
			}

			QString typestr = cobj.value("type").toString();
			msg = "Robot Path has constraint of type '" + typestr + "' which is not a valid type";
			return false;
		}

		path->addConstraint(constraint);
	}

	return true;
}

bool RobotPath::readPoints(std::shared_ptr<RobotPath> path, const QJsonArray& obj, QString& msg)
{
	for (int i = 0; i < obj.count(); i++) 
	{
		if (!obj.at(i).isObject()) 
		{
			msg = "entry " + QString::number(i + 1) + "in '" + RobotPath::PointsTag + "' array is not a JSON object";
			return false;
		}

		QJsonObject ptobj = obj.at(i).toObject();
		
		double x, y, heading, swrot, swrotvel;

		x = RobotPath::getDoubleParam(ptobj, RobotPath::XTag, msg);
		if (msg.length() > 0)
		{
			return false;
		}

		y = RobotPath::getDoubleParam(ptobj, RobotPath::YTag, msg);
		if (msg.length() > 0)
		{
			return false;
		}

		heading = RobotPath::getDoubleParam(ptobj, RobotPath::HeadingTag, msg);
		if (msg.length() > 0)
		{
			return false;
		}

		swrot = RobotPath::getDoubleParam(ptobj, RobotPath::SwerveRotationTag, msg);
		if (msg.length() > 0)
		{
			swrot = 0.0;
			msg.clear();
		}

		swrotvel = RobotPath::getDoubleParam(ptobj, RobotPath::SwerveRotationVelocityTag, msg);
		if (msg.length() > 0)
		{
			swrotvel = 0.0;
			msg.clear();
		}
		else
		{
			qDebug() << "";
		}

		Pose2dWithRotation way(Translation2d(x, y), Rotation2d::fromDegrees(heading), Rotation2d::fromDegrees(swrot));
		way.setRotVelocity(swrotvel);
		path->addWayPoint(way);
	}

	return true;
}

QJsonObject RobotPath::toJSONObject()
{
	QJsonObject obj;

	obj.insert(RobotPath::NameTag, name_);
	obj.insert(RobotPath::StartVelocityTag, params_.startVelocity());
	obj.insert(RobotPath::EndVelocityTag, params_.endVelocity());
	obj.insert(RobotPath::MaxVelocityTag, params_.maxVelocity());
	obj.insert(RobotPath::MaxAccelerationTag, params_.maxAccel());

	QJsonArray constraints;
	for (auto c : constraints_) {
		QJsonObject cobj = c->toJSON();
		constraints.append(cobj);
	}
	obj.insert(RobotPath::ConstraintsTag, constraints);

	QJsonArray waypoints;
	for (const Pose2dWithRotation& pt : waypoints_) {
		QJsonObject wobj;

		wobj.insert(RobotPath::XTag, pt.getTranslation().getX());
		wobj.insert(RobotPath::YTag, pt.getTranslation().getY());
		wobj.insert(RobotPath::HeadingTag, pt.getRotation().toDegrees());
		wobj.insert(RobotPath::SwerveRotationTag, pt.getSwrot().toDegrees());
		wobj.insert(RobotPath::SwerveRotationVelocityTag, pt.getSwrotVelocity());

		waypoints.append(wobj);
	}
	obj.insert(RobotPath::PointsTag, waypoints);
	return obj;
}

std::shared_ptr<RobotPath> RobotPath::fromJSONObject(const PathGroup *group, const QString& units, const QJsonObject& obj, QString &msg)
{
	double startvel, endvel, maxvel, maxaccel;
	QString name;

	name = RobotPath::getStringParam(obj, RobotPath::NameTag, msg);
	if (msg.length() > 0)
	{
		return nullptr;
	}

	startvel = RobotPath::getDoubleParam(obj, RobotPath::StartVelocityTag, msg);
	if (msg.length() > 0) 
	{
		return nullptr;
	}

	endvel = RobotPath::getDoubleParam(obj, RobotPath::EndVelocityTag, msg);
	if (msg.length() > 0)
	{
		return nullptr;
	}

	maxvel = RobotPath::getDoubleParam(obj, RobotPath::MaxVelocityTag, msg);
	if (msg.length() > 0)
	{
		return nullptr;
	}

	maxaccel = RobotPath::getDoubleParam(obj, RobotPath::MaxAccelerationTag, msg);
	if (msg.length() > 0)
	{
		return nullptr;
	}

	std::shared_ptr<RobotPath> path = std::make_shared<RobotPath>(group, units, name, PathParameters(startvel, endvel, maxvel, maxaccel));

	QJsonArray child;
	
	child = RobotPath::getArrayParam(obj, RobotPath::ConstraintsTag, msg);
	if (msg.length() > 0) {
		return nullptr;
	}
	else {
		if (!RobotPath::readConstraints(path, child, msg))
		{
			return nullptr;
		}
	}

	child = RobotPath::getArrayParam(obj, RobotPath::PointsTag, msg);
	if (msg.length() > 0) {
		return nullptr;
	}
	else {
		if (!RobotPath::readPoints(path, child, msg))
		{
			return nullptr;
		}
	}

	return path;
}