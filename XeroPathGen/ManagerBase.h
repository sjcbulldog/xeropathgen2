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
#pragma once

#include "Translation2d.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QVersionNumber>
#include <cassert>

class ManagerBase
{
public:
	ManagerBase();
	virtual ~ManagerBase();

	const QString& getDefaultDir() const {
		return default_dir_;
	}

	virtual void copyDefaults(const QString& subdir);

	virtual bool initialize();

	void setDirectory(const QString& dir) {
		dirs_.clear();
		dirs_.push_back(dir);
	}

	void addDirectory(const QString& dir) {
		dirs_.push_back(dir);
	}

	void addDefaultDirectory(const QString& dir) {
		addDirectory(dir);
		default_dir_ = dir;
	}


	void dumpSearchPath(const char* name);

protected:
	virtual bool processPath(const QString& path);
	virtual bool processJSONFile(QFile& file) = 0;
	virtual bool checkCount() = 0;

	const QString& getFirstDir() {
		assert(dirs_.size() > 0);
		return dirs_.front();
	}

	bool getJsonVersionValue(QFile& file, QJsonDocument& doc, const char* name, QVersionNumber& ver);
	bool getJSONStringValue(QFile &file, QJsonDocument& doc, const char* name, QString& value);
	bool getJSONStringValue(QFile& file, QJsonObject& obj, const char* name, QString& value);
	bool getJSONDoubleValue(QFile& file, QJsonDocument& doc, const char* name, double& value);
	bool getJSONDoubleValue(QFile& file, QJsonObject& doc, const char* name, double& value);
	bool getJSONIntegerValue(QFile& file, QJsonDocument& doc, const char* name, int& value);
	bool getJSONPointValue(QFile& file, const QJsonValue& value, const char *name, Translation2d& t2d);
	bool getJSONPointValue(QFile& file, QJsonDocument& doc, const char* name, Translation2d& t2d);

private:
	QString default_dir_;
	std::list<QString> dirs_;
};

