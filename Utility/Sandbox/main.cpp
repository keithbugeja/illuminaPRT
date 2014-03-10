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
#include "Defs.h"

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
#include "GBuffer.h"

#include "Multithreaded.h"
#include "MultithreadedFrameless.h"

//----------------------------------------------------------------------------------------------
// Should follow Core/System/Platform.h (due to windows.h conflicts)
//----------------------------------------------------------------------------------------------
class SimpleListener 
	: public IlluminaMTListener
{
protected:
	PathEx m_path;
	float m_fDeltaTime;
	int m_nFrameNumber;

public:
	SimpleListener(void)
		: m_nFrameNumber(0)
		, m_fDeltaTime(0)
	{ 
		m_path.Clear();
		m_path.FromString("path={{-16, -14, -5.75},{90,0,0},{-8, -14, -5.75},{90,0,0},{-2.36, -14, -5.75},{90,0,0}}");
	}

	void OnBeginFrame(IIlluminaMT *p_pIlluminaMT) 
	{ 
		/*
		ICamera* pCamera = p_pIlluminaMT->GetEnvironment()->GetCamera();
		Vector3 position, lookAt;

		// m_fDeltaTime += 0.02f; if (m_fDeltaTime > 1.f) m_fDeltaTime = 1.f;

		m_path.Get(m_fDeltaTime, position, lookAt);

		pCamera->MoveTo(position);
		pCamera->LookAt(lookAt);

		std::cout << "Position : " << position.ToString() << ", LookAt : " << lookAt.ToString() << std::endl;
		pCamera->MoveTo(pCamera->GetObserver() + pCamera->GetFrame().W * 1.0f);
		 */
	};

	void OnEndFrame(IIlluminaMT *p_pIlluminaMT)
	{
		// GBuffer::Persist("Output\\gbuffer.gbf", m_nFrameNumber++, p_pIlluminaMT->GetEnvironment()->GetCamera(), p_pIlluminaMT->GetCommitBuffer());
	}
};

//----------------------------------------------------------------------------------------------
void IlluminaPRT(Logger *p_pLogger, int p_nVerboseFrequency, 
	int p_nIterations, int p_nThreads, int p_nFPS, 
	int p_nJobs, int p_nSize, int p_nFlags, 
	std::string p_strScript)
{
	//IlluminaMTFrameless illumina;
	IlluminaMT illumina;

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
#include "PointSet.h"
#include "Export.h"
#include "Peer.h"

#include "Multithreaded.h"
#include "MultithreadedFrameless.h"

#include "MultithreadedP2P.h"
#include "MultithreadedServer.h"

//----------------------------------------------------------------------------------------------
void IlluminaPRT_IrradianceCompute(IIlluminaMT *p_pIllumina)
{
	Environment *pEnvironment = p_pIllumina->GetEnvironment();

	std::cout << "Loading point cloud..." << std::endl;
	
	PointSet<Dart> pointSet;
	//pointSet.Load("Output//vertices_kitchen.asc");
	pointSet.Load("Output//vertices_barber.asc");

	PointShader<Dart> shader;
	shader.Initialise(pEnvironment->GetScene(), 0.01f, 6, 1);
	shader.SetHemisphereDivisions(24, 64);
	shader.SetVirtualPointSources(2048, 8192); 
	shader.SetGeometryTerm(0.01f);
	shader.Prepare(PointShader<Dart>::PointLit);
	//shader.Prepare(PointShader<Dart>::PathTraced);

	std::cout << "Shading points..." << std::endl;

	shader.Shade(pointSet.GetContainerInstance().Get(), PointShader<Dart>::PointLit, true);
	//shader.Shade(pointSet.GetContainerInstance().Get(), PointShader<Dart>::PathTraced);

	std::cout << "Shading ready..." << std::endl;
	std::cout << "Saving point cloud..." << std::endl;

	//pointSet.Save("Output//vertices_kitchen_shaded.asc");
	pointSet.Save("Output//vertices_barber_shaded.asc");

	std::cout << "Irradiance computation ready!"<< std::endl;
	std::getchar();
	return;
}

//----------------------------------------------------------------------------------------------
void IlluminaPRT_IrradianceServer(IIlluminaMT *p_pIllumina)
{
	Environment *pEnv = p_pIllumina->GetEnvironment();

	/* */
	PointSet<Dart> pointSet;
	//pointSet.Initialise(pEnv->GetScene(), 0.0025f, 0.01f, 0.75f, 1024, 64, 48, 24, 0.01f, Vector3(32));
	//pointSet.Initialise(pEnv->GetScene(), 0.0025f, 0.1f, 0.75f, 1024, 64, 48, 24, 0.01f, Vector3(32));
	pointSet.Initialise(pEnv->GetScene(), 0.0025f, 0.1f, 0.75f, 512, 64, 48, 24, 0.01f, Vector3(32));

	//pointSet.Load("Output//pointcloud_full.asc");
	//pointSet.Load("Output//vertices.asc");
	
	pointSet.Generate();
	pointSet.Save("Output//pointcloud_full.asc");
	std::cout << "Generated point set. Elements in grid [" << pointSet.GetContainerInstance().Get().size() << "]" << std::endl;


	PointShader<Dart> shader;
	shader.Initialise(pEnv->GetScene(), 0.01f, 6, 1);
	shader.SetHemisphereDivisions(24, 64);
	shader.SetVirtualPointSources(1024, 8192); // 256
	shader.SetGeometryTerm(0.01f);
	//shader.Prepare(PointShader<Dart>::PointLit);
	shader.Prepare(PointShader<Dart>::PathTraced);

	std::cout << "Starting shading..." << std::endl;

	//shader.Shade(pointSet.GetContainerInstance().Get(), PointShader<Dart>::PointLit);
	shader.Shade(pointSet.GetContainerInstance().Get(), PointShader<Dart>::PathTraced);

	pointSet.Save("Output//vertices_shaded.asc");

	std::cout << "Shading READY!"<< std::endl;
	std::getchar();
	return;

	//std::cout << "Shading points..." << std::endl;

	//std::vector<Dart*> shadingList;
	//FilteredGPUGrid filteredGrid;

	//GPUGrid grid;
	//grid.Build(pointSet.GetContainerInstance().Get(), 32, 1.f);

	DualPointGrid<Dart> grid;
	//grid.Build(pointSet.GetContainerInstance().Get(), 1.0f);
	
	/*
	grid.FilterByView(illumina.GetEnvironment()->GetCamera(), &filteredGrid);
	//grid.FilterByView(illumina.GetEnvironment()->GetCamera(), cellShadingList);
	//grid.FilterByView(illumina.GetEnvironment()->GetCamera(), shadingList);

	filteredGrid.GetFilteredSampleList(shadingList);
	std::vector<Dart*> &v = pointSet.Get().Get();
	for (auto a : v)
		a->Irradiance.Set(10, 0, 10);

	// shader.Shade(pointSet.Get().Get());
	// shader.Shade(pointSet.Get().Get(), emitterList, 0.25f);

	std::cout << "Points to shade : [" << shadingList.size() << "]" << std::endl;

	shader.Shade(shadingList, emitterList, 0.25f);

	std::vector<int> indexList;
	std::vector<Spectrum> irradianceList;

	// 1. use indexing to send stuff 
	grid.Serialize(&filteredGrid, irradianceList, indexList);
	*/

	IrradianceServer::Boot(p_pIllumina, &pointSet, &shader, &grid);
	std::cout << "Server started" << std::endl;

	// shader.Shade(shadingList);
	
	pointSet.Save("Output//pointcloud_full.asc");
	std::cout << "Point cloud saved." << std::endl;
}

void IlluminaPRT(
	Logger *p_pLogger, int p_nVerboseFrequency, int p_nIterations, int p_nFPS,
	int p_nRenderThreads, int p_nJobsPerFrame, int p_nTileSize, int p_nFlags, 
	std::string p_strScript, std::string p_strCameraScript,
	int p_nPort, bool p_bAutomaticDiscovery, std::string p_strPeerIP, int p_nPeerPort,
	int p_nPush, int p_nPull, int p_nTimeout)
{
	/*
	 * Diagnostics and quick tests
	 */
	//SamplerDiagnostics diagnostics;

	//diagnostics.DistributionTest(new LowDiscrepancySampler(), 512*512, "Output\\ld.ppm");
	//diagnostics.DistributionTest(new RandomSampler(), 512*512, "Output\\rng.ppm");
	//diagnostics.DistributionTest(new MultijitterSampler(), 512*512, "Output\\mj.ppm");
	//diagnostics.DistributionTest(new JitterSampler(), 512*512, "Output\\j.ppm");

	/*HostId hidA = HostId::MakeHostId(0x01010101, 0xFFFF),
		hidB = HostId::MakeHostId(0x20202020, 0xF0F0);

	SparseVectorClock a(hidA),
		b(hidB);

	a.Tick();
	b.Tick();

	std::cout << "A: " << a.ToString() << std::endl;
	std::cout << "B: " << b.ToString() << std::endl;

	b.Send();
	a.Receive(b);
	a.Send();
	b.Receive(a);

	std::cout << "A: " << a.ToString() << std::endl;
	std::cout << "B: " << b.ToString() << std::endl;

	std::cout << "OK" << std::endl;
	std::getchar();

	return;*/

	/*
	 * Diagnostics and quick tests
	 */

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
	illumina.SetCameraScript(p_strCameraScript);
	illumina.SetThreadCount(p_nRenderThreads);
	illumina.SetIterations(p_nIterations);
	illumina.SetJobs(p_nJobsPerFrame, p_nTileSize);
	illumina.SetFrameBudget(0);

	P2PListener2Way listener;
	listener.SetEventPush(p_nPush);
	listener.SetEventPull(p_nPull);
	listener.SetTimeout(p_nTimeout);
	listener.SetPeer(&localHost, p_nPort > 7100 ? P2PListener2Way::P2PReceive : P2PListener2Way::P2PSendReceive);	

	HostDirectory &hostDir = listener.GetHostDirectory();
	hostDir.Add(remoteHost);

	illumina.AttachListener(&listener);

	illumina.Initialise();

	IlluminaPRT_IrradianceCompute(&illumina);

	// If irradiance server is enabled
	// IlluminaPRT_IrradianceServer(&illumina);

	// If p2p
	// illumina.Render();	
	
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
		nFlags = 0xFF,
		nSync = 0, 
		nPush = 500, 
		nPull = 500,
		nTimeout = 30000;

	bool bVerbose = false,
		bDiscovery = false;

	int nRemotePort = 6666,
		nPort = 6666;

	std::string strScript("default.ilm"),
		strRemoteAddress("127.0.0.1"),
		strCameraScript;

	// Declare the supported options.
	boost::program_options::options_description description("Allowed Settings");

	description.add_options()
		("help", "show this message")
		("verbose", boost::program_options::value<bool>(), "show extended information")
		("statfreq", boost::program_options::value<int>(), "show frame statistics every nth frame (requires verbose)")
		("script", boost::program_options::value<std::string>(), "script file to render")
		("camerascript", boost::program_options::value<std::string>(), "camera fly-by script")
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
		("sync", boost::program_options::value<int>(), "delay until system clock is a multiple of value (minutes)")
		("eventpush", boost::program_options::value<int>(), "event push (msec)")
		("eventpull", boost::program_options::value<int>(), "event pull (msec)")
		("eventtimeout", boost::program_options::value<int>(), "event timeout (msec)")
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

	// --camerapath
	if (variableMap.count("camerascript"))
	{
		strCameraScript = variableMap["camerascript"].as<std::string>();
		std::cout << "Camera Path [" << strCameraScript << "]" << std::endl;
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

	// --eventpush
	if (variableMap.count("eventpush"))
	{
		try 
		{
			nPush = variableMap["eventpush"].as<int>();
		} catch (...) { nPush = 500; }
		std::cout << "Event Push [" << nPush << "]" << std::endl;
	}

	// --eventpull
	if (variableMap.count("eventpull"))
	{
		try 
		{
			nPull = variableMap["eventpull"].as<int>();
		} catch (...) { nPull = 500; }
		std::cout << "Event Pull [" << nPull << "]" << std::endl;
	}
	
	// --eventtimeout
	if (variableMap.count("eventtimeout"))
	{
		try 
		{
			nTimeout = variableMap["eventtimeout"].as<int>();
		} catch (...) { nTimeout = 30000; }
		std::cout << "Event Timeout [" << nTimeout << "]" << std::endl;
	}

	// --sync
	if (variableMap.count("sync"))
	{
		try 
		{
			nSync = variableMap["sync"].as<int>();
		} catch (...) { nSync = 0; }
		std::cout << "Synchronised Start [" << nSync << "]" << std::endl;
	}

	// Handle sync start here
	if (nSync > 0)
	{
		int nSyncSec = nSync * 60;
		long lTime = (long)Platform::ToSeconds(Platform::GetTime()),
			lDelay = nSyncSec - (lTime % nSyncSec);

		std::cout << "Sleeping for [" << lDelay << " of " << nSyncSec << "] seconds" << std::endl;
		boost::this_thread::sleep(boost::posix_time::seconds(lDelay));
	}

	// Initialise new logger
	Logger logger; logger.SetLoggingFilter(bVerbose ? LL_All : LL_ErrorLevel);

	// -- start rendering
	IlluminaPRT(&logger, nVerboseFrequency, nIterations, nFPS,  nThreads, nJobs, nSize, nFlags, strScript, strCameraScript, nPort, bDiscovery, strRemoteAddress, nRemotePort, nPush, nPull, nTimeout);
	
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