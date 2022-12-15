#include "RobotPath.h"
#include "PathGroup.h"
#include <QtCore/QJsonArray>
#include <limits>

RobotPath::RobotPath(const PathGroup* gr, const QString& units, const QString &name, const PathParameters &params)
{
	group_ = gr;
	name_ = name;
	params_ = params;
	units_ = units;
}

void RobotPath::emitPathChangedSignal()
{
	emit pathChanged(group_->name(), name_);
}

void RobotPath::convert(const QString& from, const QString& to)
{
	for (auto con : constraints_) {
		con->convert(from, to);
	}

	params_.convert(from, to);

	bool save = signalsBlocked();
	blockSignals(true);

	for (size_t i = 0; i < size(); i++) {
		const Pose2dWithRotation pt = getPoint(i);
		double newx = UnitConverter::convert(pt.getTranslation().getX(), from, to);
		double newy = UnitConverter::convert(pt.getTranslation().getY(), from, to);
		Pose2dWithRotation newpt(Translation2d(newx, newy), pt.getRotation(), pt.swrot());

		replacePoint(i, newpt);
	}
	
	blockSignals(save);

	emitPathChangedSignal();
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
		// TODO
		assert(false);
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
		
		double x, y, heading, swrot;

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

		path->addWayPoint(Pose2dWithRotation(Translation2d(x, y), Rotation2d::fromDegrees(heading), Rotation2d::fromDegrees(swrot)));
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
		wobj.insert(RobotPath::SwerveRotationTag, pt.swrot().toDegrees());

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