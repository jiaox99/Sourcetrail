#include "CommandlineCommandServer.h"

#include "CommandLineParser.h"
#include "CommandlineHelper.h"

namespace po = boost::program_options;

namespace commandline
{
CommandlineCommandServer::CommandlineCommandServer(CommandLineParser* parser)
	: CommandlineCommand("server", "serve REST API with a certain project", parser)
{}

CommandlineCommandServer::~CommandlineCommandServer() {}

void CommandlineCommandServer::setup()
{
	po::options_description options("Server options");
	options.add_options()("help,h", "Print this help message")(
		"port,p", po::value<int>()->default_value(9984), "Port to serve REST API on")(
		"project-file", po::value<std::string>(), "Project file to index (.srctrlprj)");

	m_options.add(options);
	m_positional.add("project-file", 1);
}

CommandlineCommand::ReturnStatus CommandlineCommandServer::parse(std::vector<std::string>& args)
{
	po::variables_map vm;
	try
	{
		m_parser->setIsServerMode();
		po::store(
			po::command_line_parser(args).options(m_options).positional(m_positional).run(), vm);
		po::notify(vm);

		parseConfigFile(vm, m_options);
	}
	catch (po::error& e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
		std::cerr << m_options << std::endl;
		return ReturnStatus::CMD_FAILURE;
	}

	if (vm.count("help") || args.size() == 0 || args[0] == "help")
	{
		printHelp();
		return ReturnStatus::CMD_QUIT;
	}

	if (vm.count("port"))
	{
		m_parser->setRESTServerPort(vm["port"].as<int>());
	}
	

	if (vm.count("project-file"))
	{
		m_parser->noRefresh();
		m_parser->setProjectFile(FilePath(vm["project-file"].as<std::string>()));
	}

	return ReturnStatus::CMD_OK;
}

}