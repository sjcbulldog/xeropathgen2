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
#include "UnitConverter.h"
#include "Translation2d.h"
#include <string>
#include <QDir>
#include <QFile>
#include <QFileInfo>

class GameField
{
public:
	GameField(const QDir& path, const QString& name, const QString& image, const QString &units,
		Translation2d topleft, Translation2d bottomright,
		Translation2d size) {
		path_ = path;
		name_ = name;
		image_ = image;
		units_ = units;
		top_left_ = topleft;
		bottom_right_ = bottomright;
		size_ = size;
	}

	~GameField() {
	}

	const QString& getName() const {
		return name_;
	}

	QString getImageFile() {
		QFileInfo info(path_, image_);
		return info.absoluteFilePath();
	}

	Translation2d getTopLeft() const {
		return top_left_;
	}

	Translation2d getBottomRight() const {
		return bottom_right_;
	}

	Translation2d getSize() const {
		return size_;
	}

	void convert(const QString& units) {
		if (units != units_)
		{
			double x = UnitConverter::convert(size_.getX(), units_, units);
			double y = UnitConverter::convert(size_.getY(), units_, units);

			size_ = Translation2d(x, y);

			units_ = units;
		}
	}

public:


private:
	QString name_;
	QString image_;
	QString units_;
	Translation2d top_left_;
	Translation2d bottom_right_;
	Translation2d size_;
	QDir path_;
};

