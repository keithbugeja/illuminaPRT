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
#include <omp.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/timer.hpp>
#include <boost/asio.hpp>

// Illumina Environment
#include "System/EngineKernel.h"
#include "Scene/Environment.h"

// Factories
#include "Camera/CameraFactories.h"
#include "Device/DeviceFactories.h"
#include "Light/LightFactories.h"
#include "Space/SpaceFactories.h"
#include "Shape/ShapeFactories.h"
#include "Filter/FilterFactories.h"
#include "Sampler/SamplerFactories.h"
#include "Texture/TextureFactories.h"
#include "Material/MaterialFactories.h"
#include "Renderer/RendererFactories.h"
#include "Integrator/IntegratorFactories.h"

#include "Staging/Acceleration.h"

using namespace Illumina::Core;

//#define TEST_SCHEDULER

#if (!defined(TEST_SCHEDULER))

#include <omp.h>

/*
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/timer.hpp>

// Illumina Environment
#include "System/EngineKernel.h"
#include "Scene/Environment.h"

// Factories
#include "Camera/CameraFactories.h"
#include "Device/DeviceFactories.h"
#include "Light/LightFactories.h"
#include "Space/SpaceFactories.h"
#include "Shape/ShapeFactories.h"
#include "Filter/FilterFactories.h"
#include "Sampler/SamplerFactories.h"
#include "Texture/TextureFactories.h"
#include "Material/MaterialFactories.h"
#include "Renderer/RendererFactories.h"
#include "Integrator/IntegratorFactories.h"

#include "Staging/Acceleration.h"
*/

using namespace Illumina::Core;

void Message(const std::string& p_strMessage, bool p_bVerbose)
{
	if (p_bVerbose) std::cout << p_strMessage << std::endl;
}

//----------------------------------------------------------------------------------------------
void IlluminaPRT(bool p_bVerbose, int p_nIterations, std::string p_strScript)
{
	//----------------------------------------------------------------------------------------------
	// Set number of OMP Threads
	//----------------------------------------------------------------------------------------------
	//std::cout << "Initialising OMP thread count : [Threads = " << p_nOMPThreads << "]" << std::endl;
	//omp_set_num_threads(p_nOMPThreads);

	//----------------------------------------------------------------------------------------------
	// Engine Kernel
	//----------------------------------------------------------------------------------------------
	Message("\nInitialising EngineKernel...", p_bVerbose);
	EngineKernel engineKernel;
	// Initialise factories -- note, factories should be moved to plug-ins a dynamically loaded

	//----------------------------------------------------------------------------------------------
	// Sampler
	//----------------------------------------------------------------------------------------------
	Message("Registering Samplers...", p_bVerbose);
	engineKernel.GetSamplerManager()->RegisterFactory("Random", new RandomSamplerFactory());
	engineKernel.GetSamplerManager()->RegisterFactory("Jitter", new JitterSamplerFactory());
	engineKernel.GetSamplerManager()->RegisterFactory("Multijitter", new MultijitterSamplerFactory());
	engineKernel.GetSamplerManager()->RegisterFactory("Precomputation", new MultijitterSamplerFactory());

	//----------------------------------------------------------------------------------------------
	// Filter
	//----------------------------------------------------------------------------------------------
	Message("Registering Filters...", p_bVerbose);
	engineKernel.GetFilterManager()->RegisterFactory("Box", new BoxFilterFactory());
	engineKernel.GetFilterManager()->RegisterFactory("Tent", new TentFilterFactory());

	//----------------------------------------------------------------------------------------------
	// Space
	//----------------------------------------------------------------------------------------------
	Message("Registering Spaces...", p_bVerbose);
	engineKernel.GetSpaceManager()->RegisterFactory("Basic", new BasicSpaceFactory());

	//----------------------------------------------------------------------------------------------
	// Integrator
	//----------------------------------------------------------------------------------------------
	Message("Registering Integrators...", p_bVerbose);
	engineKernel.GetIntegratorManager()->RegisterFactory("PathTracing", new PathIntegratorFactory());
	engineKernel.GetIntegratorManager()->RegisterFactory("IGI", new IGIIntegratorFactory());
	//engineKernel.GetIntegratorManager()->RegisterFactory("Photon", new PhotonIntegratorFactory());
	engineKernel.GetIntegratorManager()->RegisterFactory("Whitted", new WhittedIntegratorFactory());
	//engineKernel.GetIntegratorManager()->RegisterFactory("Test", new TestIntegratorFactory());

	//----------------------------------------------------------------------------------------------
	// Renderer
	//----------------------------------------------------------------------------------------------
	Message("Registering Renderers...", p_bVerbose);
	engineKernel.GetRendererManager()->RegisterFactory("Basic", new BasicRendererFactory());
	engineKernel.GetRendererManager()->RegisterFactory("Multithreaded", new MultithreadedRendererFactory());
	engineKernel.GetRendererManager()->RegisterFactory("Distributed", new DistributedRendererFactory());

	//----------------------------------------------------------------------------------------------
	// Device
	//----------------------------------------------------------------------------------------------
	Message("Registering Devices...", p_bVerbose);
	engineKernel.GetDeviceManager()->RegisterFactory("Image", new ImageDeviceFactory());

	//----------------------------------------------------------------------------------------------
	// Cameras
	//----------------------------------------------------------------------------------------------
	Message("Registering Cameras...", p_bVerbose);
	engineKernel.GetCameraManager()->RegisterFactory("Perspective", new PerspectiveCameraFactory());
	engineKernel.GetCameraManager()->RegisterFactory("ThinLens", new ThinLensCameraFactory());

	//----------------------------------------------------------------------------------------------
	// Lights
	//----------------------------------------------------------------------------------------------
	Message("Registering Lights...", p_bVerbose);
	engineKernel.GetLightManager()->RegisterFactory("Point", new PointLightFactory());
	engineKernel.GetLightManager()->RegisterFactory("DiffuseArea", new DiffuseAreaLightFactory());
	engineKernel.GetLightManager()->RegisterFactory("InfiniteArea", new InfiniteAreaLightFactory());

	//----------------------------------------------------------------------------------------------
	// Shapes
	//----------------------------------------------------------------------------------------------
	Message("Registering Shapes...", p_bVerbose);
	engineKernel.GetShapeManager()->RegisterFactory("KDTreeMesh", new KDTreeMeshShapeFactory());
	engineKernel.GetShapeManager()->RegisterFactory("Quad", new QuadMeshShapeFactory());
	engineKernel.GetShapeManager()->RegisterFactory("Triangle", new TriangleShapeFactory());
	engineKernel.GetShapeManager()->RegisterFactory("Sphere", new SphereShapeFactory());

	//----------------------------------------------------------------------------------------------
	// Textures
	//----------------------------------------------------------------------------------------------
	Message("Registering Textures...", p_bVerbose);
	engineKernel.GetTextureManager()->RegisterFactory("Image", new ImageTextureFactory());
	engineKernel.GetTextureManager()->RegisterFactory("Noise", new NoiseTextureFactory());
	engineKernel.GetTextureManager()->RegisterFactory("Marble", new MarbleTextureFactory());

	//----------------------------------------------------------------------------------------------
	// Materials
	//----------------------------------------------------------------------------------------------
	Message("Registering Materials...", p_bVerbose);
	engineKernel.GetMaterialManager()->RegisterFactory("Matte", new MatteMaterialFactory());
	engineKernel.GetMaterialManager()->RegisterFactory("Mirror", new MirrorMaterialFactory());
	engineKernel.GetMaterialManager()->RegisterFactory("Glass", new GlassMaterialFactory());
	engineKernel.GetMaterialManager()->RegisterFactory("Group", new MaterialGroupFactory());
	
	//----------------------------------------------------------------------------------------------
	// Environment
	//----------------------------------------------------------------------------------------------
	Message("Initialising Environment...", p_bVerbose);
	Environment environment(&engineKernel);

	// Load environment script
	Message("Loading Environment script...", p_bVerbose);
	if (!environment.Load(p_strScript))
	{
		std::cerr << "Error : Unable to load environment script." << std::endl;
		exit(-1);
	}

	// Alias required components
	IIntegrator *pIntegrator = environment.GetIntegrator();
	IRenderer *pRenderer = environment.GetRenderer();
	ISpace *pSpace = environment.GetSpace();

	// Initialise integrator and renderer
	pIntegrator->Initialise(environment.GetScene(), environment.GetCamera());
	pRenderer->Initialise();

	// Initialisation complete
	Message("Initialisation complete. Rendering in progress...", p_bVerbose);

	// Initialise timing
	boost::timer frameTimer;
	float fTotalFramesPerSecond = 0.f;

	ICamera *pCamera = environment.GetCamera();
	float alpha = Maths::Pi;

	// Cornell
	//Vector3 lookFrom(70, 0, 70),
	//	lookAt(0, 0, 0);
	
	// Kiti
	//Vector3 lookFrom(-19, 1, -19),
	//	lookAt(0, 8, 0);
	
	// Sponza
	//Vector3 lookFrom(800, 100, 200),
	//	lookAt(0, 200, 100);
	for (int nFrame = 0; nFrame < p_nIterations; ++nFrame)
	{
		//alpha += Maths::PiTwo / 256;

		frameTimer.restart();
		
		//pCamera->MoveTo(lookFrom);
		//pCamera->MoveTo(Vector3(Maths::Cos(alpha) * lookFrom.X, lookFrom.Y, Maths::Sin(alpha) * lookFrom.Z));
		//pCamera->LookAt(lookAt);
	 
		// Update space
		pSpace->Update();
	 
		// Render frame
		pRenderer->Render();
	 
		// Compute frames per second
		fTotalFramesPerSecond += (float)(1.0f / frameTimer.elapsed());
		
		if (p_bVerbose)
		{
			std::cout << std::endl;
			std::cout << "-- Frame Render Time : [" << frameTimer.elapsed() << "s]" << std::endl;
			std::cout << "-- Frames per second : [" << fTotalFramesPerSecond / nFrame << "]" << std::endl;
		}
	}

	pRenderer->Shutdown();
	pIntegrator->Shutdown();
}

#define Major 0
#define Minor 5
#define Build 0

int main(int argc, char** argv)
{
	std::cout << "Illumina Renderer : Version " << Major << "." << Minor << "." << Build << " http://www.illuminaprt.codeplex.com " << std::endl;
	std::cout << "Copyright (C) 2010-2011 Keith Bugeja" << std::endl << std::endl;

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

	// -- start rendering
	IlluminaPRT(bVerbose, nIterations, strScript);

	// Exit
	return 1;
}

#else
void MessageOut(const std::string& p_strMessage, bool p_bVerbose)
{
	if (p_bVerbose) std::cout << p_strMessage << std::endl;
}
#include "scheduling.h"

#define Major 0
#define Minor 5
#define Build 0

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