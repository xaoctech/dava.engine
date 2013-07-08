#include "DockConsole/Console.h"

Console::Console()
{
	DAVA::Logger::AddCustomOutput(new LoggerToConsole());
	log.reserve(5000);
}

Console::~Console()
{ }

void Console::Clear()
{
	if(log.size() > 0)
	{
		log.clear();
		emit OnClear();
	}
}

void Console::Echo(const DAVA::String &text)
{
	bool echo = false;

	size_t begin = 0;
	size_t end = text.find('\n', 0);

	while(end != DAVA::String::npos)
	{
		log.push_back(text.substr(begin, end - begin));
		echo = true;

		begin = end + 1;
		end = text.find('\n', begin);
	}

	if(begin < text.size())
	{
		log.push_back(text.substr(begin));
		echo = true;
	}

	if(echo)
	{
		emit OnEcho();
	}
}

int Console::GetLineCount() const
{
	return log.size();
}

const DAVA::String& Console::GetLine(size_t index) const
{
	static DAVA::String empty = "";
	
	if(index < log.size())
	{
		return log[index];
	}
	
	return empty;
}

LoggerToConsole::LoggerToConsole()
{ }

LoggerToConsole::~LoggerToConsole()
{ }

void LoggerToConsole::Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) const
{
	Console::Instance()->Echo(text);
}

void LoggerToConsole::Output(DAVA::Logger::eLogLevel ll, const DAVA::char16* text) const
{

}
