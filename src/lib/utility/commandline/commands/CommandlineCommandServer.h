#ifndef COMMANDLINE_COMMAND_SERVER_H
#define COMMANDLINE_COMMAND_SERVER_H

#include "CommandlineCommand.h"

namespace commandline
{
class CommandlineCommandServer: public CommandlineCommand
{
public:
	CommandlineCommandServer(CommandLineParser* parser);
	virtual ~CommandlineCommandServer();

	virtual void setup();
	virtual ReturnStatus parse(std::vector<std::string>& args);

	virtual bool hasHelp() const
	{
		return true;
	}
};
}	 // namespace commandline
#endif	  // COMMANDLINE_COMMAND_SERVER_H