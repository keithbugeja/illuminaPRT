//----------------------------------------------------------------------------------------------
//	Filename:	main.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
// TODO:
// Double check ILight-derived classes ... some methods have not been tested properly.
// ?? DistributedRenderer should not instantiate MPI - change it to have it passed to the object
// Polish object factories
// Move factories to CorePlugins.dll
// Finish scene loaders
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		const int Major = 0;
		const int Minor = 5;
		const int Build = 0;
	}
}
//----------------------------------------------------------------------------------------------
#include <omp.h>
#include <iostream>
#include <sstream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>

//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
// Required in both scheduler and renderer mode
#include "Environment.h"
#include "Logger.h"
//----------------------------------------------------------------------------------------------
#define ILLUMINA_SHAREDMEMORY
//#define ILLUMINA_DISTRIBUTED
//----------------------------------------------------------------------------------------------
#if (defined(ILLUMINA_SHAREDMEMORY))
//----------------------------------------------------------------------------------------------
void IlluminaPRT(bool p_bVerbose, int p_nIterations, std::string p_strScript)
{
	//----------------------------------------------------------------------------------------------
	// Illumina sandbox environment 
	//----------------------------------------------------------------------------------------------
	SandboxEnvironment sandbox;

	sandbox.Initialise(p_bVerbose);
	sandbox.LoadScene(p_strScript, p_bVerbose);

	//----------------------------------------------------------------------------------------------
	// Alias required components
	//----------------------------------------------------------------------------------------------
	IIntegrator *pIntegrator = sandbox.GetEnvironment()->GetIntegrator();
	IRenderer *pRenderer = sandbox.GetEnvironment()->GetRenderer();
	ICamera *pCamera = sandbox.GetEnvironment()->GetCamera();
	ISpace *pSpace = sandbox.GetEnvironment()->GetSpace();

	Environment *pEnvironment = sandbox.GetEnvironment();
	EngineKernel *pEngineKernel = sandbox.GetEngineKernel();

	// Open output device
	pRenderer->GetDevice()->Open();

	// Initialisation complete
	Logger::Message("Initialisation complete. Rendering in progress...", p_bVerbose);

	//----------------------------------------------------------------------------------------------
	// Initialise timing
	//----------------------------------------------------------------------------------------------
	float fTotalFramesPerSecond = 0.f;
	double start, elapsed = 0, eventStart, eventComplete;

	//----------------------------------------------------------------------------------------------
	// Render loop
	//----------------------------------------------------------------------------------------------
	RadianceBuffer *pRadianceBuffer = new RadianceBuffer(
		pRenderer->GetDevice()->GetWidth(), pRenderer->GetDevice()->GetHeight()),
		*pRadianceAccumulationBuffer = new RadianceBuffer(
		pRenderer->GetDevice()->GetWidth(), pRenderer->GetDevice()->GetHeight());

	IPostProcess *pDiscontinuityBuffer = pEngineKernel->GetPostProcessManager()->CreateInstance("Discontinuity", "DiscontinuityBuffer", "");
	IPostProcess *pAutoTone = pEngineKernel->GetPostProcessManager()->CreateInstance("AutoTone", "AutoTone", "");
	IPostProcess *pDragoTone = pEngineKernel->GetPostProcessManager()->CreateInstance("DragoTone", "DragoTone", "");
	IPostProcess *pReconstructionBuffer = pEngineKernel->GetPostProcessManager()->CreateInstance("Reconstruction", "ReconstructionBuffer", "");
	
	AccumulationBuffer *pAccumulationBuffer = (AccumulationBuffer*)pEngineKernel->GetPostProcessManager()->CreateInstance("Accumulation", "AccumulationBuffer", "");
	pAccumulationBuffer->SetAccumulationBuffer(pRadianceAccumulationBuffer);
	pAccumulationBuffer->Reset();

	float alpha = Maths::Pi;
	Matrix3x3 rotation;
	
	struct regioninfo_t 
	{
		double lastActual;
		double lastPredicted;
		double nextTime;
		double frameBudget;
	};
	
	const int regionWidth = 32;
	const int regionHeight = 32;

	const int regionX = pRenderer->GetDevice()->GetWidth() / regionWidth;
	const int regionY = pRenderer->GetDevice()->GetHeight() / regionHeight;

	const int regions = regionX * regionY;

	float totalBudget = 0.5f;
	float requiredBudget = 0.f;
	std::vector<regioninfo_t> reg(regions);
	
	for (int j = 0; j < regions; j++)
	{
		reg[j].lastActual = 0;
		reg[j].lastPredicted = 0;
		reg[j].nextTime = 0;
		reg[j].frameBudget = 0;
	}

	//pRenderer->SetRenderBudget(1e+20f);
	pRenderer->SetRenderBudget(0.05f / ((float)regions * 0.33f));
	//pRenderer->SetRenderBudget(10.f);
	Vector3 observer = pCamera->GetObserver();

	for (int nFrame = 0; nFrame < p_nIterations; ++nFrame)
	{
		#if (defined(TEST_TILERENDER))
			
			// Animate scene 
			alpha += Maths::PiTwo / 180.f;
		
			rotation.MakeRotation(Vector3::UnitYPos, alpha);

			////((GeometricPrimitive*)pSpace->PrimitiveList[0])->WorldTransform.SetScaling(Vector3::Ones * 20.0f);
			//// ((GeometricPrimitive*)pSpace->PrimitiveList[0])->WorldTransform.SetRotation(rotation);

			////pCamera->MoveTo(lookFrom);
			////pCamera->MoveTo(Vector3(Maths::Cos(alpha) * lookFrom.X, lookFrom.Y, Maths::Sin(alpha) * lookFrom.Z));
			////pCamera->LookAt(lookAt);
			Vector3 observer_ = observer;
			observer_.Z += Maths::Cos(alpha) * 4.f;
			observer_.X += Maths::Sin(alpha) * 2.f;
			pCamera->MoveTo(observer_);

			// Start timer
			start = Platform::GetTime();

			// Prepare integrator
			pIntegrator->Prepare(pEnvironment->GetScene());

			if (p_bVerbose) 
			{
				eventComplete = Platform::GetTime();
				elapsed = Platform::ToSeconds(eventComplete - start); 
				std::cout << std::endl << "-- Frame " << nFrame;
				std::cout << std::endl << "-- Integrator Preparation Time : [" << elapsed << "s]" << std::endl;

				eventStart = Platform::GetTime();
			}

			// Update space
			pSpace->Update();

			if (p_bVerbose) 
			{
				eventComplete = Platform::GetTime();
				elapsed = Platform::ToSeconds(eventComplete - eventStart); 
				std::cout << "-- Space Update Time : [" << elapsed << "s]" << std::endl;
				
				eventStart = Platform::GetTime();
			}
	 
			// Render frame
			#pragma omp parallel for schedule(static, 8) num_threads(4)
			for (int y = 0; y < regionY; y++)
			{
				for (int x = 0; x < regionX; x++)
				{
					double regionStart = Platform::GetTime();
					pRenderer->RenderRegion(pRadianceBuffer, x * regionWidth, y * regionHeight, regionWidth, regionHeight, x * regionWidth, y * regionHeight);
					double regionEnd = Platform::GetTime();

					double lastActual = Platform::ToSeconds(regionEnd - regionStart);
					reg[x + y * regionX].lastActual = lastActual;
					reg[x + y * regionX].nextTime = reg[x + y * regionX].lastPredicted * 0.5 + lastActual * 0.5;
					reg[x + y * regionX].lastPredicted = reg[x + y * regionX].nextTime;
				}
			}

			requiredBudget = 0.f;
			
			for (int j = 0; j < regions; j++)
			{
				requiredBudget += reg[j].nextTime;
			}

			if (p_bVerbose) 
			{
				eventComplete = Platform::GetTime();
				elapsed = Platform::ToSeconds(eventComplete - eventStart); 
				std::cout << "-- Radiance Computation Time : [" << elapsed << "s]" << std::endl;

				eventStart = Platform::GetTime();
			}

			// Post-process frame
			pReconstructionBuffer->Apply(pRadianceBuffer, pRadianceBuffer);
			pDiscontinuityBuffer->Apply(pRadianceBuffer, pRadianceBuffer);

			pEnvironment->GetScene()->GetSampler()->Reset();
			//pAccumulationBuffer->Reset();
			//pAccumulationBuffer->Apply(pRadianceBuffer, pRadianceBuffer);

			pDragoTone->Apply(pRadianceBuffer, pRadianceBuffer);
			// pAutoTone->Apply(pRadianceBuffer, pRadianceBuffer);

			if (p_bVerbose) 
			{
				eventComplete = Platform::GetTime();
				elapsed = Platform::ToSeconds(eventComplete - eventStart); 
				std::cout << "-- Post-processing Time : [" << elapsed << "s]" << std::endl;

				eventStart = Platform::GetTime();
			}

			// Commit frame
			static int frameId = 4;
			// if (frameId++ % 5 == 0)
				pRenderer->Commit(pRadianceBuffer);

			// Compute frames per second
			elapsed = Platform::ToSeconds(Platform::GetTime() - start);
			fTotalFramesPerSecond += (float)(1.f/elapsed);
		
			if (p_bVerbose)
			{
				std::cout << "-- Frame Render Time : [" << elapsed << "s]" << std::endl;
				std::cout << "-- Frames per second : [" << fTotalFramesPerSecond / (nFrame + 1)<< "]" << std::endl;

				for (int j = 0; j < regions; j++)
				{
					reg[j].frameBudget = reg[j].nextTime / requiredBudget;
					// std::cout << "[Region " << j << "] : B:[" << reg[j].frameBudget << "], T:[" << reg[j].lastActual << "s], P:[" << reg[j].lastPredicted << "s], N:[" << reg[j].nextTime << "]" << std::endl;  
				}
			}
		#else
			// Update space acceleration structure
			pSpace->Update();

			// Render image
			pRenderer->Render();
		#endif
	}

	//----------------------------------------------------------------------------------------------
	// Shutdown system
	//----------------------------------------------------------------------------------------------
	// Close output device
	pRenderer->GetDevice()->Close();

	//----------------------------------------------------------------------------------------------
	// Shutdown renderer and integrator
	pRenderer->Shutdown();
	pIntegrator->Shutdown();

	//----------------------------------------------------------------------------------------------
	// Shutdown snadbox
	sandbox.Shutdown(p_bVerbose);

	//----------------------------------------------------------------------------------------------
	if (p_bVerbose)
	{
		Logger::Message("Complete :: Press enter to continue", true);
		int v = std::getchar();
	}
}
//----------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	std::cout << "Illumina Renderer : Version " << Illumina::Core::Major << "." << Illumina::Core::Minor << "." << Illumina::Core::Build << " http://www.illuminaprt.codeplex.com " << std::endl;
	std::cout << "Copyright (C) 2010-2012 Keith Bugeja" << std::endl << std::endl;

	// default options
	int nIterations = 1;
	bool bVerbose = false;
	std::string strScript("default.ilm");

	// Declare the supported options.
	boost::program_options::options_description description("Allowed Settings");

	description.add_options()
		("help", "show this message")
		("verbose", boost::program_options::value<bool>(), "show extended information")
		("script", boost::program_options::value<std::string>(), "script file to render")
		("workdir", boost::program_options::value<std::string>(), "working directory")
		("iterations", boost::program_options::value<int>(), "iterations to execute")
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

	// -- start rendering
	IlluminaPRT(bVerbose, nIterations, strScript);

	// Exit
	return 1;
}
//----------------------------------------------------------------------------------------------
#elif (defined ILLUMINA_DISTRIBUTED)
//----------------------------------------------------------------------------------------------
#include "scheduling.h"
//----------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	std::cout << "Illumina Renderer Service : Version " << Major << "." << Minor << "." << Build << " http://www.illuminaprt.codeplex.com " << std::endl;
	std::cout << "Copyright (C) 2010-2012 Keith Bugeja" << std::endl << std::endl;

	// default options
	int nIterations = 1;
	bool bVerbose = false;
	std::string strScript("default.ilm");

	// Declare the supported options.
	boost::program_options::options_description description("Allowed Settings");

	description.add_options()
		("help", "show this message")
		("verbose", boost::program_options::value<bool>(), "show extended information")
		("script", boost::program_options::value<std::string>(), "script file to render")
		("workdir", boost::program_options::value<std::string>(), "working directory")
		("iterations", boost::program_options::value<int>(), "interations to execute")
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

	// -- start service
	RunAsServer(argc, argv, bVerbose);
	
	return 0;
}
#endif