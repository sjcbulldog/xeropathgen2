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
#include "Logger.h"
#include "LoggerWindow.h"
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>
#include <QtCore/QStandardPaths>

Logger::Logger()
{
	QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
	filename_ = dirs.front() + "/logfile.txt";
	new_session_ = true;
}

void Logger::addMessage(const QString& msg)
{
	if (msg.contains("No file name specified"))
		return;

	messages_.push_back(msg);

	if (logwin_ != nullptr) {
		logwin_->addMessage(msg);
	}

	if (new_session_) {
		QFileInfo info(filename_);
		if (info.exists()) {
			QFile file(filename_);
			file.remove();
		}
		new_session_ = false;
	}

	QFile file(filename_);
	if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
	{
		QTextStream strm(&file);
		strm << msg << "\n";
	}
}
