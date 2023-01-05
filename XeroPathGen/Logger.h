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

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <fstream>


class LoggerWindow;

class Logger
{
public:
	Logger();

	const QStringList& messages() const {
		return messages_;
	}

	void setLogWindow(LoggerWindow* win) {
		logwin_ = win;
	}

	void addMessage(const QString& msg);

private:
	LoggerWindow* logwin_;
	QStringList messages_;
	QString filename_;
	bool new_session_;
};

