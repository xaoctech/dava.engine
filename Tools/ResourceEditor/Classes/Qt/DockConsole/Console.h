#ifndef __QT_CONSOLE_H__
#define __QT_CONSOLE_H__

#include <QObject>

#include "Base/StaticSingleton.h"
#include "FileSystem/Logger.h"

class Console : public QObject, public DAVA::StaticSingleton<Console>
{
	Q_OBJECT

public:
	Console();
	~Console();

	void Clear();
	void Echo(const DAVA::String &text);

	int GetLineCount() const;
	const DAVA::String& GetLine(size_t index) const;

signals:
	void OnClear();
	void OnEcho();

protected:
	DAVA::Vector<DAVA::String> log;
};

class LoggerToConsole : public DAVA::LoggerOutput
{
public:
	LoggerToConsole();
	~LoggerToConsole();

	virtual void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) const;
	virtual void Output(DAVA::Logger::eLogLevel ll, const DAVA::char16* text) const;
};

#endif // __QT_CONSOLE_H__
