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

	void setGeneratorType(GeneratorType type) {
		emit beforeChange();
		gen_type_ = type;
		dirty_ = true;
		emit trajectoryGeneratorChanged();
	}

	GeneratorType generatorType() const {
		return gen_type_;
	}

	void reset();

	const QString& setDefaultUnits(const QString units) {
		default_units_ = units;
	}

	const QString& defaultUnits() const {
		return default_units_;
	}

	const QString& units() const {
		return units_;
	}

	void setUnits(const QString& units) {
		emit beforeChange();
		convert(units);
		emit unitsChanged(units_);
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
	QVector<double> getDistancesForPath(std::shared_ptr<RobotPath> path);

	QVector<std::shared_ptr<RobotPath>> getAllPaths();

private:
	void computeSplinesForPath(std::shared_ptr<RobotPath> path);
	void computeSplines(const QString &grname, const QString &pathname);

	bool readPathGroup(QFile& file, const QJsonObject& obj, QString &msg);

	QJsonObject modelToObject();

	void setDirty() {
		dirty_ = true;
	}

	void beforePathChanged(const QString& grname, const QString& pathname);
	void afterPathChanged(const QString& grname, const QString& pathname);

signals:
	void beforeChange();
	void groupAdded(const QString& grname);
	void groupDeleted(const QString& grname);
	void groupRenamed(const QString& oldname, const QString& newname);
	void pathAdded(std::shared_ptr<RobotPath> path);
	void pathDeleted(const QString& grname, const QString& pathname);
	void pathRenamed(const QString &grname, const QString& oldname, const QString& newname);
	void unitsChanged(const QString& units);
	void trajectoryGeneratorChanged();

private:
	QString filename_;						// The filename for the path JSON file
	QString path_output_dir_;				// The directory for the output of trajectory (.csv) files
	QString units_;							// The units for the current data model
	QString default_units_;					// The units to use if a data file being read does not have units
	QList<PathGroup *> groups_;				// The path groups (auto modes) that are stored here
	bool dirty_;							// If true, unsaved changes exist
	GeneratorType gen_type_;				// The generator type that goes with these paths
	GenerationMgr& gen_mgr_;				// The generator manager to actually do the work of generating trajectories

	// A mapping of paths to splines
	QMap<std::shared_ptr<RobotPath>, QVector<std::shared_ptr<SplinePair>>> splines_;

	// A mapping of paths to distances
	QMap<std::shared_ptr<RobotPath>, QVector<double>> distances_;
};
