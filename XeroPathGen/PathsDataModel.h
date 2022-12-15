#pragma once

#include "GenerationMgr.h"
#include "GeneratorType.h"
#include "PathGroup.h"
#include "SplinePair.h"
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QFile>
#include <QtCore/QJsonObject>

class PathsDataModel : public QObject
{
	Q_OBJECT

public:
	PathsDataModel(GenerationMgr &genmgr);
	virtual ~PathsDataModel();

	GeneratorType type() const {
		return gen_type_;
	}

	void reset();

	const QString& units() const {
		return units_;
	}

	bool isDirty() const {
		return dirty_;
	}

	void clearDirty() {
		dirty_ = false;
	}

	const QString& filename() const {
		return filename_;
	}
	
	bool hasFilename() const {
		return filename_.length() > 0;
	}

	void setFilename(const QString& filename) {
		filename_ = filename;
	}

	const QString& outputDir() const {
		return path_output_dir_;
	}

	bool hasOutpuDir() const {
		return path_output_dir_.length() > 0;
	}

	void setOutputDir(const QString& dir) {
		path_output_dir_ = dir;
	}

	bool save(QString &msg);
	bool saveToFile(const QString& filename, QString& msg);
	bool saveAs(const QString& filename, QString &msg);
	bool load(const QString& filename, QString& msg);

	void convert(const QString& units);

	bool hasGroup(const QString& grname) const ;
	void addGroup(const QString& grname);
	void deleteGroup(const QString& grname);
	QStringList groupNames() const ;
	const PathGroup* getPathGroupByName(const QString& grname);
	void renameGroup(const QString& oldname, const QString& newname);

	bool hasPath(const QString& grname, const QString& pathname) const;
	void addPath(std::shared_ptr<RobotPath> path);
	void deletePath(const QString& grname, const QString& pathname);
	QStringList pathNames(const QString& grname) const;
	std::shared_ptr<RobotPath> getPathByName(const QString& grname, const QString& pathname);
	void renamePath(const QString &grname, const QString& oldname, const QString& newname);

	QVector<std::shared_ptr<SplinePair>> getSplinesForPath(std::shared_ptr<RobotPath> path);

	QVector<std::shared_ptr<RobotPath>> getAllPaths();

private:
	void computeSplinesForPath(std::shared_ptr<RobotPath> path);
	void computeSplines(const QString &grname, const QString &pathname);

	bool readPathGroup(QFile& file, const QJsonObject& obj, QString &msg);

	QJsonObject modelToObject();

	void setDirty() {
		dirty_ = true;
	}

signals:
	void groupAdded(const QString& grname);
	void groupDeleted(const QString& grname);
	void groupRenamed(const QString& oldname, const QString& newname);
	void pathAdded(std::shared_ptr<RobotPath> path);
	void pathDeleted(const QString& grname, const QString& pathname);
	void pathRenamed(const QString &grname, const QString& oldname, const QString& newname);

private:
	QString filename_;
	QString path_output_dir_;
	QString units_;
	QList<PathGroup *> groups_;
	QMap<RobotPath*, QVector<std::shared_ptr<SplinePair>>> splines_;
	bool dirty_;
	GeneratorType gen_type_;
	GenerationMgr& gen_mgr_;
};
