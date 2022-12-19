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
