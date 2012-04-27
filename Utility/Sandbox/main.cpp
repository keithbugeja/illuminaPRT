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
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>

// Illumina Environment
#include "System/EngineKernel.h"
#include "Scene/Environment.h"

#include "External/Compression/Compression.h"
#include "../../Core/Scene/GeometricPrimitive.h"

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
#include "Postproc/PostProcessFactories.h"
#include "Integrator/IntegratorFactories.h"

#include "Staging/Acceleration.h"
#include "Sampler/SamplerDiagnostics.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------

// #define TEST_SCHEDULER
#define TEST_TILERENDER

//----------------------------------------------------------------------------------------------
#if (!defined(TEST_SCHEDULER))
//----------------------------------------------------------------------------------------------
void Message(const std::string& p_strMessage, bool p_bVerbose)
{
	if (p_bVerbose) std::cout << p_strMessage << std::endl;
}
//----------------------------------------------------------------------------------------------
void IlluminaPRT(bool p_bVerbose, int p_nIterations, std::string p_strScript)
{
	//----------------------------------------------------------------------------------------------
	// Engine Kernel
	//----------------------------------------------------------------------------------------------
	Message("\nInitialising EngineKernel...", p_bVerbose);
	EngineKernel engineKernel;

	//----------------------------------------------------------------------------------------------
	// Perform any required tests
	//----------------------------------------------------------------------------------------------
	Message("\nPerforming Tests...", p_bVerbose);
	
	SamplerDiagnostics samplerTest;
	ISampler *pSampler;
	float result;
	const int sampleSize = 1e+4; //1e+6;

	pSampler = new RandomSampler();
	std::cout << "-- Random Sampler --" << std::endl;
	result = samplerTest.FrequencyTest(pSampler, sampleSize);
	std::cout << "p-value (E[x] > 0.01) = " << result << std::endl;
	result = samplerTest.ChiSquareTest(pSampler, sampleSize);
	std::cout << "chi^2, 9 dof (E[x] < 33.1) = " << result << std::endl;
	samplerTest.DistributionTest(pSampler, sampleSize, "Z:\\random.ppm");
	delete pSampler;

	pSampler = new PrecomputedHaltonSampler();
	std::cout << "-- Precomputed Halton --" << std::endl;
	result = samplerTest.FrequencyTest(pSampler, sampleSize);
	std::cout << "p-value (E[x] > 0.01) = " << result << std::endl;
	result = samplerTest.ChiSquareTest(pSampler, sampleSize);
	std::cout << "chi^2, 9 dof (E[x] < 33.1) = " << result << std::endl;
	samplerTest.DistributionTest(pSampler, sampleSize, "Z:\\halton.ppm");
	delete pSampler;

	pSampler = new PrecomputedSobolSampler();
	std::cout << "-- Precomputed Sobol --" << std::endl;
	result = samplerTest.FrequencyTest(pSampler, sampleSize);
	std::cout << "p-value (E[x] > 0.01) = " << result << std::endl;
	result = samplerTest.ChiSquareTest(pSampler, sampleSize);
	std::cout << "chi^2, 9 dof (E[x] < 33.1) = " << result << std::endl;
	samplerTest.DistributionTest(pSampler, sampleSize, "Z:\\sobol.ppm");
	delete pSampler;

	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------
	// Sampler
	//----------------------------------------------------------------------------------------------
	Message("Registering Samplers...", p_bVerbose);
	engineKernel.GetSamplerManager()->RegisterFactory("Random", new RandomSamplerFactory());
	engineKernel.GetSamplerManager()->RegisterFactory("Jitter", new JitterSamplerFactory());
	engineKernel.GetSamplerManager()->RegisterFactory("Multijitter", new MultijitterSamplerFactory());
	engineKernel.GetSamplerManager()->RegisterFactory("PrecomputedRandom", new PrecomputedRandomSamplerFactory());
	engineKernel.GetSamplerManager()->RegisterFactory("PrecomputedHalton", new PrecomputedHaltonSamplerFactory());
	engineKernel.GetSamplerManager()->RegisterFactory("PrecomputedSobol", new PrecomputedSobolSamplerFactory());

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
	// Materials
	//----------------------------------------------------------------------------------------------
	Message("Registering Post Processes...", p_bVerbose);
	engineKernel.GetPostProcessManager()->RegisterFactory("AutoTone", new AutoToneFactory());
	engineKernel.GetPostProcessManager()->RegisterFactory("DragoTone", new DragoToneFactory());
	engineKernel.GetPostProcessManager()->RegisterFactory("Accumulation", new AccumulationBufferFactory());
	engineKernel.GetPostProcessManager()->RegisterFactory("Discontinuity", new DiscontinuityBufferFactory());

	//----------------------------------------------------------------------------------------------
	// Renderer
	//----------------------------------------------------------------------------------------------
	Message("Registering Renderers...", p_bVerbose);
	engineKernel.GetRendererManager()->RegisterFactory("Basic", new BasicRendererFactory());
	engineKernel.GetRendererManager()->RegisterFactory("Multipass", new MultipassRendererFactory());
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
	engineKernel.GetShapeManager()->RegisterFactory("BVHMesh", new BVHMeshShapeFactory());
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

	//----------------------------------------------------------------------------------------------
	// Alias required components
	//----------------------------------------------------------------------------------------------
	IIntegrator *pIntegrator = environment.GetIntegrator();
	IRenderer *pRenderer = environment.GetRenderer();
	ICamera *pCamera = environment.GetCamera();
	ISpace *pSpace = environment.GetSpace();

	// Initialise integrator and renderer
	pIntegrator->Initialise(environment.GetScene(), environment.GetCamera());
	pRenderer->Initialise();

	// Initialisation complete
	Message("Initialisation complete. Rendering in progress...", p_bVerbose);

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

	IPostProcess *pDiscontinuityBuffer = engineKernel.GetPostProcessManager()->CreateInstance("Discontinuity", "DiscontinuityBuffer", "");
	IPostProcess *pAutoTone = engineKernel.GetPostProcessManager()->CreateInstance("AutoTone", "AutoTone", "");
	IPostProcess *pDragoTone = engineKernel.GetPostProcessManager()->CreateInstance("DragoTone", "DragoTone", "");
	
	AccumulationBuffer *pAccumulationBuffer = (AccumulationBuffer*)engineKernel.GetPostProcessManager()->CreateInstance("Accumulation", "AccumulationBuffer", "");
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
	
	const int regionX = pRenderer->GetDevice()->GetWidth() / 40;
	const int regionY = pRenderer->GetDevice()->GetHeight() / 40;
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

	for (int nFrame = 0; nFrame < p_nIterations; ++nFrame)
	{
		#if (defined(TEST_TILERENDER))
			//// Animate scene
			//alpha += Maths::PiTwo / 32.f;
		
			//rotation.MakeRotation(Vector3::UnitYPos, alpha);

			//// ((GeometricPrimitive*)pSpace->PrimitiveList[0])->WorldTransform.SetScaling(Vector3::Ones * 20.0f);
			//// ((GeometricPrimitive*)pSpace->PrimitiveList[0])->WorldTransform.SetRotation(rotation);

			////pCamera->MoveTo(lookFrom);
			////pCamera->MoveTo(Vector3(Maths::Cos(alpha) * lookFrom.X, lookFrom.Y, Maths::Sin(alpha) * lookFrom.Z));
			////pCamera->LookAt(lookAt);

			// Start timer
			start = Platform::GetTime();

			// Prepare integrator
			pIntegrator->Prepare(environment.GetScene());

			if (p_bVerbose) 
			{
				eventComplete = Platform::GetTime();
				elapsed = Platform::ToSeconds(eventComplete - start); 
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
			#pragma omp parallel for num_threads(3)
			for (int y = 0; y < regionY; y++)
			{
				for (int x = 0; x < regionX; x++)
				{
					double regionStart = Platform::GetTime();
					pRenderer->RenderRegion(pRadianceBuffer, x * 40, y * 40, 40, 40, x * 40, y * 40);
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
			pDiscontinuityBuffer->Apply(pRadianceBuffer, pRadianceBuffer);
			pAccumulationBuffer->Apply(pRadianceBuffer, pRadianceBuffer);
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

	char c; std::cin >> c;

	pRenderer->Shutdown();
	pIntegrator->Shutdown();
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
//----------------------------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------------------------
void MessageOut(const std::string& p_strMessage, bool p_bVerbose)
{
	if (p_bVerbose) std::cout << p_strMessage << std::endl;
}
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