//----------------------------------------------------------------------------------------------
//	Filename:	main.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
// TODO:
// Double check ILight-derived classes ... some methods have not been tested properly.
// Polish object factories
// Move factories to CorePlugins.dll
// Finish scene loaders
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// Illumina PRT compilation modes:
//	ILLUMINA_SHM : Compilation for local, multithreaded, shared memory systems (no network support)
//		ILLUMINA_SHMVIEWER : Compile as a shared memory viewer (to be phased out; separate project)
//  ILLUMINA_DSM : Compilation for multiple-client support (cloud deployment)
//  ILLUMINA_P2P : Compilation for peer-to-peer support (client collaboration)
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
//	Set Illumina PRT compilation mode (SHM, DSM or P2P)
//----------------------------------------------------------------------------------------------
#define ILLUMINA_P2P
// #define ILLUMINA_SHM

#if (!defined ILLUMINA_SHM) || (!defined ILLUMINA_P2P)
	#define ILLUMINA_DSM
#else
	/* I hate myself for this */
	// #define ILLUMINA_SHMVIEWER
	#if (defined ILLUMINA_SHMVIEWER)
		#include "SHMViewer.h"
	#endif
/**/
#endif

//----------------------------------------------------------------------------------------------
//	Set Illumina PRT version
//----------------------------------------------------------------------------------------------
namespace Illumina { namespace Core { const int Major = 0; const int Minor = 7; const int Build = 0; } }

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
//	Include basic headers for OpenMP and io, string and file streams
//----------------------------------------------------------------------------------------------
// #include <omp.h>
#include <iostream>
#include <sstream>
#include <fstream>

//----------------------------------------------------------------------------------------------
//	Include boost header files for managing program options and file paths
//----------------------------------------------------------------------------------------------
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//	Shared-memory compilation mode
//----------------------------------------------------------------------------------------------
//	This compilation modes targets Illumina at a standalone multithreaded system.
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#if (defined(ILLUMINA_SHM))

#include "Logger.h"
#include "Environment.h"
#include "Export.h"

#include "Multithreaded.h"
#include "MultithreadedFrameless.h"

//----------------------------------------------------------------------------------------------
// Should follow Core/System/Platform.h (due to windows.h conflicts)
//----------------------------------------------------------------------------------------------
class SimpleListener 
	: public IlluminaMTListener
{
	void OnBeginFrame(IIlluminaMT *p_pIlluminaMT) 
	{ 
		ICamera* pCamera = p_pIlluminaMT->GetEnvironment()->GetCamera();
		// pCamera->MoveTo(pCamera->GetObserver() + pCamera->GetFrame().W * 1.0f);
	};
};

//----------------------------------------------------------------------------------------------
void IlluminaPRT(Logger *p_pLogger, int p_nVerboseFrequency, 
	int p_nIterations, int p_nThreads, int p_nFPS, 
	int p_nJobs, int p_nSize, int p_nFlags, 
	std::string p_strScript)
{
	IlluminaMTFrameless illumina;
	//IlluminaMT illumina;

	illumina.SetFlags(p_nFlags);
	illumina.SetLogger(p_pLogger);
	illumina.SetLoggerUpdate(p_nVerboseFrequency);
	illumina.SetScript(p_strScript);
	illumina.SetThreadCount(p_nThreads);
	illumina.SetIterations(p_nIterations);
	illumina.SetJobs(p_nJobs, p_nSize);
	illumina.SetFrameBudget(0);

	SimpleListener listener;
	illumina.AttachListener(&listener);

	illumina.Initialise();
	illumina.Render();
	illumina.Shutdown();
}

//----------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	#if (defined ILLUMINA_SHMVIEWER)
		// Create viewer instance
		// Note: Pull arguments from command line or something!
		SHMViewer viewer(512, 512, "IlluminaPRT_OutputSink");

		/* Test code ... uncomment and run to test SHMViewer with SharedMemoryDevice... 
		Spectrum luminance;
		IDevice* p = viewer.SetDummyOutput();

		p->BeginFrame();
		for (int y = 0; y < 512; y++)
			for (int x = 0; x < 512; x++)
			{
				//luminance.Set(x + y / 1024, x / 1024, 1024 - x / 1024);
				luminance.Set(1.f, 0.5f, 0.75f);
				p->Set(x, y, luminance);
				// p->Set(x, y, Spectrum(float(x + y) / 1024));
			}
		p->EndFrame();
		/* */

		// Failed to open?
		if (!viewer.Open())
		{
			std::cerr << "IlluminaPRT Sharedmemory viewer failed to open!" << std::endl;
			exit(0);
		}

		while(true) {
			viewer.Update();
			boost::this_thread::sleep(boost::posix_time::millisec(20));
		}

		viewer.Close();
		return 0;
	#endif

	std::cout << "Illumina Renderer : Version " << Illumina::Core::Major << "." << Illumina::Core::Minor << "." << Illumina::Core::Build << " http://www.illuminaprt.codeplex.com " << std::endl;
	std::cout << "Copyright (C) 2010-2012 Keith Bugeja" << std::endl << std::endl;

	// default options
	int nVerboseFrequency = 1,
		nIterations = 1,
		nThreads = 1,
		nSize = 32,
		nJobs = 0x10000,
		nFPS = 5,
		nFlags = 0xFF;

	bool bVerbose = false;
	std::string strScript("default.ilm");

	// Declare the supported options.
	boost::program_options::options_description description("Allowed Settings");

	description.add_options()
		("help", "show this message")
		("verbose", boost::program_options::value<bool>(), "show extended information")
		("statfreq", boost::program_options::value<int>(), "show frame statistics every nth frame (requires verbose)")
		("script", boost::program_options::value<std::string>(), "script file to render")
		("workdir", boost::program_options::value<std::string>(), "working directory")
		("iterations", boost::program_options::value<int>(), "iterations to execute")
		("threads", boost::program_options::value<int>(), "number of rendering threads")
		("tilesize", boost::program_options::value<int>(), "initial length of tile edge")
		("tilejobs", boost::program_options::value<int>(), "number of jobs before tile subdivision")
		("flags", boost::program_options::value<int>(), "rendering flags")
		("fps", boost::program_options::value<int>(), "frame rendering frequency (hint)")
		;

	// Declare variable map
	boost::program_options::variables_map variableMap;

	// Parse command line options
	try 
	{
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), variableMap);
		boost::program_options::notify(variableMap);
	} 
	catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::unknown_option> > &exception) 
	{
		std::cout << "Unknown option [" << exception.get_option_name() << "] : Please use --help to display help message." << std::endl;
		return 1;
	}
	catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::invalid_option_value> > &exception) 
	{
		std::cout << "Error parsing input for [" << exception.get_option_name() << "] : Invalid argument value." << std::endl;
		return 1;
	}

	// --help
	if (variableMap.count("help"))
	{
		std::cout << description << std::endl;
		return 1;
	}

	// --verbose
	if (variableMap.count("verbose"))
	{
		bVerbose = variableMap["verbose"].as<bool>();
		std::cout << "Verbose mode [" << (bVerbose ? "ON]" : "OFF]") << std::endl;
	}

	// --statfreq
	if (variableMap.count("statfreq"))
	{
		try {
			nVerboseFrequency = variableMap["statfreq"].as<int>();
		} catch (...) { nVerboseFrequency = 1; } 
		std::cout << "Render statistics output frequency [" << nIterations << "]" << std::endl;
	}

	// --iterations
	if (variableMap.count("iterations"))
	{
		try {
			nIterations = variableMap["iterations"].as<int>();
		} catch (...) { nIterations = 1; } 
		std::cout << "Iterations [" << nIterations << "]" << std::endl;
	}

	// --script
	if (variableMap.count("script"))
	{
		strScript = variableMap["script"].as<std::string>();
		std::cout << "Script [" << strScript << "]" << std::endl;
	}

	// --workdir
	boost::filesystem::path cwdPath;

	if (variableMap.count("workdir"))
	{
		cwdPath = boost::filesystem::path(variableMap["workdir"].as<std::string>());
	}
	else
	{
		// Setting working directory
		boost::filesystem::path scriptPath(strScript);
		cwdPath = boost::filesystem::path(scriptPath.parent_path());
	}

	try {
		boost::filesystem::current_path(cwdPath);
		std::cout << "Working directory [" << cwdPath.string() << "]" << std::endl;;
	} catch (...) { std::cerr << "Error : Unable to set working directory to " << cwdPath.string() << std::endl; }

	// --threads
	if (variableMap.count("threads"))
	{
		try {
			nThreads = variableMap["threads"].as<int>();
		} catch (...) { nThreads = 1; } 
		std::cout << "Threads [" << nThreads << "]" << std::endl;
	}

	// --width
	if (variableMap.count("tilesize"))
	{
		try {
			nSize = variableMap["tilesize"].as<int>();
		} catch (...) { nSize = 16; } 
		std::cout << "Tile size [" << nSize << " x " << nSize << "]" << std::endl;
	}

	// --threads
	if (variableMap.count("tilejobs"))
	{
		try {
			nJobs = variableMap["tilejobs"].as<int>();
		} catch (...) { nJobs = 0x10000; } 
		std::cout << "Jobs before tile subdivision [" << nJobs << "]" << std::endl;
	}

	// --budget
	if (variableMap.count("fps"))
	{
		try {
			nFPS = variableMap["fps"].as<int>();
		} catch (...) { nFPS = 0; } 
		std::cout << "FPS [" << nFPS << "]" << std::endl;
	}

	// --flags
	if (variableMap.count("flags"))
	{
		try {
			nFlags = variableMap["flags"].as<int>();
		} catch (...) { nFlags = 0x01 | 0x02 | 0x04 | 0x08; } 
		std::cout << "Flags [" << nFlags << "]" << std::endl;
	}

	// Initialise new logger
	Logger logger; logger.SetLoggingFilter(bVerbose ? LL_All : LL_ErrorLevel);

	// -- start rendering
	IlluminaPRT(&logger, nVerboseFrequency, nIterations, nThreads, nFPS, nJobs, nSize, nFlags, strScript);

	// Exit
	return 1;
}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//	Peer-to-peer compilation mode
//----------------------------------------------------------------------------------------------
//  This mode enables single, typically shared memory multithreaded instances to collaborate
//	by sharing results (e.g., IC samples).
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#elif (defined ILLUMINA_P2P)

#include "Logger.h"
#include "Environment.h"
#include "Export.h"
#include "Peer.h"

#include "Multithreaded.h"
#include "MultithreadedFrameless.h"

#include "MultithreadedP2P.h"

//----------------------------------------------------------------------------------------------
void IlluminaPRT(
	Logger *p_pLogger, int p_nVerboseFrequency, int p_nIterations, int p_nFPS,
	int p_nRenderThreads, int p_nJobsPerFrame, int p_nTileSize, int p_nFlags, std::string p_strScript,
	int p_nPort, bool p_bAutomaticDiscovery, std::string p_strPeerIP, int p_nPeerPort)
{
	Peer localHost;

	localHost.Configure(p_nPort, p_nPeerPort, 2, 1);
	localHost.Initialise();

	HostId remoteHost = HostId::MakeHostId(p_strPeerIP, p_nPeerPort);

	// IlluminaMTFrameless illumina;
	IlluminaMT illumina;

	illumina.SetFlags(p_nFlags);
	illumina.SetLogger(p_pLogger);
	illumina.SetLoggerUpdate(p_nVerboseFrequency);
	illumina.SetScript(p_strScript);
	illumina.SetThreadCount(p_nRenderThreads);
	illumina.SetIterations(p_nIterations);
	illumina.SetJobs(p_nJobsPerFrame, p_nTileSize);
	illumina.SetFrameBudget(0);

	P2PListener2Way listener;
	listener.SetPeer(&localHost, p_nPort == 7001 ? P2PListener2Way::P2PReceive : P2PListener2Way::P2PSendReceive);
	HostDirectory &hostDir = listener.GetHostDirectory();
	hostDir.Add(remoteHost);
	illumina.AttachListener(&listener);

	illumina.Initialise();
	illumina.Render();	
	illumina.Shutdown();

	std::cout << "Press any key to continue..." << std::endl;
	std::getchar();
}

//----------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	std::cout << "Illumina Renderer [P2P]: Version " << Illumina::Core::Major << "." << Illumina::Core::Minor << "." << Illumina::Core::Build << " http://www.illuminaprt.codeplex.com " << std::endl;
	std::cout << "Copyright (C) 2010-2012 Keith Bugeja" << std::endl << std::endl;

	// default options
	int nVerboseFrequency = 1,
		nIterations = 1,
		nThreads = 1,
		nSize = 32,
		nJobs = 0x10000,
		nFPS = 5,
		nFlags = 0xFF;

	bool bVerbose = false,
		bDiscovery = false;

	int nRemotePort = 6666,
		nPort = 6666;

	std::string strScript("default.ilm"),
		strRemoteAddress("127.0.0.1");

	// Declare the supported options.
	boost::program_options::options_description description("Allowed Settings");

	description.add_options()
		("help", "show this message")
		("verbose", boost::program_options::value<bool>(), "show extended information")
		("statfreq", boost::program_options::value<int>(), "show frame statistics every nth frame (requires verbose)")
		("script", boost::program_options::value<std::string>(), "script file to render")
		("workdir", boost::program_options::value<std::string>(), "working directory")
		("iterations", boost::program_options::value<int>(), "iterations to execute")
		("threads", boost::program_options::value<int>(), "number of rendering threads")
		("tilesize", boost::program_options::value<int>(), "initial length of tile edge")
		("tilejobs", boost::program_options::value<int>(), "number of jobs before tile subdivision")
		("flags", boost::program_options::value<int>(), "rendering flags")
		("fps", boost::program_options::value<int>(), "frame presentation frequency (hint)")
		("port", boost::program_options::value<int>(), "listening port")
		("discovery", boost::program_options::value<bool>(), "try automatic discovery in P2P network")
		("remoteaddr", boost::program_options::value<std::string>(), "remote address of peer in P2P network")
		("remoteport", boost::program_options::value<int>(), "remote port of peer in P2P network")
		;

	// Declare variable map
	boost::program_options::variables_map variableMap;

	// Parse command line options
	try 
	{
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), variableMap);
		boost::program_options::notify(variableMap);
	} 
	catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::unknown_option> > &exception) 
	{
		std::cout << "Unknown option [" << exception.get_option_name() << "] : Please use --help to display help message." << std::endl;
		return 1;
	}
	catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::invalid_option_value> > &exception) 
	{
		std::cout << "Error parsing input for [" << exception.get_option_name() << "] : Invalid argument value." << std::endl;
		return 1;
	}

	// --help
	if (variableMap.count("help"))
	{
		std::cout << description << std::endl;
		return 1;
	}

	// --verbose
	if (variableMap.count("verbose"))
	{
		bVerbose = variableMap["verbose"].as<bool>();
		std::cout << "Verbose mode [" << (bVerbose ? "ON]" : "OFF]") << std::endl;
	}

	// --statfreq
	if (variableMap.count("statfreq"))
	{
		try {
			nVerboseFrequency = variableMap["statfreq"].as<int>();
		} catch (...) { nVerboseFrequency = 1; } 
		std::cout << "Render statistics output frequency [" << nIterations << "]" << std::endl;
	}

	// --iterations
	if (variableMap.count("iterations"))
	{
		try {
			nIterations = variableMap["iterations"].as<int>();
		} catch (...) { nIterations = 1; } 
		std::cout << "Iterations [" << nIterations << "]" << std::endl;
	}

	// --script
	if (variableMap.count("script"))
	{
		strScript = variableMap["script"].as<std::string>();
		std::cout << "Script [" << strScript << "]" << std::endl;
	}

	// --workdir
	boost::filesystem::path cwdPath;

	if (variableMap.count("workdir"))
	{
		cwdPath = boost::filesystem::path(variableMap["workdir"].as<std::string>());
	}
	else
	{
		// Setting working directory
		boost::filesystem::path scriptPath(strScript);
		cwdPath = boost::filesystem::path(scriptPath.parent_path());
	}

	try {
		boost::filesystem::current_path(cwdPath);
		std::cout << "Working directory [" << cwdPath.string() << "]" << std::endl;;
	} catch (...) { std::cerr << "Error : Unable to set working directory to " << cwdPath.string() << std::endl; }

	// --threads
	if (variableMap.count("threads"))
	{
		try {
			nThreads = variableMap["threads"].as<int>();
		} catch (...) { nThreads = 1; } 
		std::cout << "Threads [" << nThreads << "]" << std::endl;
	}

	// --width
	if (variableMap.count("tilesize"))
	{
		try {
			nSize = variableMap["tilesize"].as<int>();
		} catch (...) { nSize = 16; } 
		std::cout << "Tile size [" << nSize << " x " << nSize << "]" << std::endl;
	}

	// --threads
	if (variableMap.count("tilejobs"))
	{
		try {
			nJobs = variableMap["tilejobs"].as<int>();
		} catch (...) { nJobs = 0x10000; } 
		std::cout << "Jobs before tile subdivision [" << nJobs << "]" << std::endl;
	}

	// --budget
	if (variableMap.count("fps"))
	{
		try {
			nFPS = variableMap["fps"].as<int>();
		} catch (...) { nFPS = 0; } 
		std::cout << "FPS [" << nFPS << "]" << std::endl;
	}

	// --flags
	if (variableMap.count("flags"))
	{
		try {
			nFlags = variableMap["flags"].as<int>();
		} catch (...) { nFlags = 0x01 | 0x02 | 0x04 | 0x08; } 
		std::cout << "Flags [" << nFlags << "]" << std::endl;
	}

	// --discovery
	if (variableMap.count("discovery"))
	{
		try {
			bDiscovery = variableMap["discovery"].as<bool>();
		} catch (...) { bDiscovery = false; } 
		std::cout << "Peer-discovery [" << bDiscovery << "]" << std::endl;
	}

	// --peer
	if (variableMap.count("remoteaddr"))
	{
		try {
			strRemoteAddress = variableMap["remoteaddr"].as<std::string>();
		} catch (...) { strRemoteAddress = "127.0.0.1"; } 
		std::cout << "Remote Address (Peer) [" << strRemoteAddress << "]" << std::endl;
	}

	// --remoteport
	if (variableMap.count("remoteport"))
	{
		try {
			nRemotePort = variableMap["remoteport"].as<int>();
		} catch (...) { nRemotePort = 6666; } 
		std::cout << "Remote Port (Peer) [" << nRemotePort << "]" << std::endl;
	}

	// --localport
	if (variableMap.count("port"))
	{
		try {
			nPort = variableMap["port"].as<int>();
		} catch (...) { nPort = 6666; } 
		std::cout << "Local Port [" << nPort << "]" << std::endl;
	}

	// Initialise new logger
	Logger logger; logger.SetLoggingFilter(bVerbose ? LL_All : LL_ErrorLevel);

	// -- start rendering
	IlluminaPRT(&logger, nVerboseFrequency, nIterations, nFPS,  nThreads, nJobs, nSize, nFlags, strScript, nPort, bDiscovery, strRemoteAddress, nRemotePort);

	// Exit
	return 1;
}

//----------------------------------------------------------------------------------------------s
//----------------------------------------------------------------------------------------------
//	Client-server compilation mode
//----------------------------------------------------------------------------------------------
//	This mode runs IlluminaPRT in server mode in a distributed system, allowing interactive
//	servicing of multiple clients.
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#elif (defined ILLUMINA_DSM)

#include "ServiceManager.h"

//----------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	std::cout << std::endl
		<< "-----------------------------------------------------------" << std::endl
		<< "-- Illumina PRT (http://www.illuminaprt.codeplex.com)" << std::endl 
		<< "--   Version " << Major << "." << Minor << "." << Build << std::endl
		<< "--   Copyright (C) 2010-2013 Keith Bugeja" << std::endl 
		<< "-----------------------------------------------------------" << std::endl;

	// default options
	int nPort = 6660,
		nAdminPort = 6661;

	std::string strPath;
	bool bVerbose = false;

	// Declare the supported options.
	boost::program_options::options_description description("Allowed Settings");

	description.add_options()
		("help", "show this message")
		("verbose", boost::program_options::value<bool>(), "show extended information")
		("workdir", boost::program_options::value<std::string>(), "working directory")
		("port", boost::program_options::value<int>(), "service port")
		("adminport", boost::program_options::value<int>(), "admin port")
		;

	// Declare variable map
	boost::program_options::variables_map variableMap;

	// Parse command line options
	try 
	{
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), variableMap);
		boost::program_options::notify(variableMap);
	} 
	catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::unknown_option> > &exception) 
	{
		std::cout << "Unknown option [" << exception.get_option_name() << "] : Please use --help to display help message." << std::endl;
		return 1;
	}
	catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::invalid_option_value> > &exception) 
	{
		std::cout << "Error parsing input for [" << exception.get_option_name() << "] : Invalid argument value." << std::endl;
		return 1;
	}

	// --help
	if (variableMap.count("help"))
	{
		std::cout << description << std::endl;
		return 1;
	}

	// --verbose
	if (variableMap.count("verbose"))
	{
		bVerbose = variableMap["verbose"].as<bool>();
		std::cout << "Verbose mode [" << (bVerbose ? "ON]" : "OFF]") << std::endl;
	}

	// --port
	if (variableMap.count("port"))
	{
		try {
			nPort = variableMap["port"].as<int>();
		} catch (...) { nPort = 6660; } 
		std::cout << "Port [" << nPort << "]" << std::endl;
	}

	// --adminport
	if (variableMap.count("adminport"))
	{
		try {
			nAdminPort = variableMap["adminport"].as<int>();
		} catch (...) { nAdminPort = 6661; } 
		std::cout << "Admin Port [" << nAdminPort << "]" << std::endl;
	}

	// --workdir
	if (variableMap.count("workdir"))
	{
		strPath = variableMap["workdir"].as<std::string>();
	}
	
	std::cout << "-----------------------------------------------------------" << std::endl << std::endl;

	// -- start service
	ServiceManager *pServiceManager = ServiceManager::GetInstance();
	
	pServiceManager->Initialise(nPort, nAdminPort, strPath, bVerbose);
	pServiceManager->Start();
	pServiceManager->Shutdown();
		
	return 0;
}
#endif