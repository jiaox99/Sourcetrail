#ifndef COMMAND_LINE_PARSER_H
#define COMMAND_LINE_PARSER_H

#include <memory>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "FilePath.h"
#include "RefreshInfo.h"

namespace po = boost::program_options;

namespace commandline
{
class CommandlineCommand;

class CommandLineParser
{
public:
	CommandLineParser(const std::string& version);
	~CommandLineParser();

	void preparse(int argc, char** argv);
	void preparse(std::vector<std::string>& args);
	void parse();

	bool runWithoutGUI() const;
	bool exitApplication() const;

	bool hasError() const;
	std::wstring getError();

	void fullRefresh();
	void incompleteRefresh();
	inline void noRefresh() { m_refreshMode = REFRESH_NONE; }
	void setShallowIndexingRequested(bool enabled = true);

	const FilePath& getProjectFilePath() const;
	void setProjectFile(const FilePath& filepath);

	RefreshMode getRefreshMode() const;
	bool getShallowIndexingRequested() const;

	inline void setRESTServerPort(int port) { m_port = port; }
	inline int getRESTServerPort() const { return m_port; }

private:
	void processProjectfile();
	void printHelp() const;

	boost::program_options::options_description m_options;
	boost::program_options::positional_options_description m_positional;

	std::vector<std::shared_ptr<CommandlineCommand>> m_commands;
	std::vector<std::string> m_args;

	const std::string m_version;
	FilePath m_projectFile;
	RefreshMode m_refreshMode = REFRESH_UPDATED_FILES;
	bool m_shallowIndexingRequested = false;

	bool m_quit = false;
	bool m_withoutGUI = false;
	int m_port = 0;

	std::wstring m_errorString;
};

}	 // namespace commandline

#endif	  // COMMAND_LINE_PARSER_H
