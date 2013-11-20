//----------------------------------------------------------------------------------------------
//	Filename:	Environment.h
//	Author:		Keith Bugeja
//	Date:		18/07/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

// Illumina Environment
#include <System/EngineKernel.h>
#include <Scene/Environment.h>
#include <Scene/Scene.h>

#include <External/Compression/Compression.h>
#include <Scene/GeometricPrimitive.h>

// Factories
#include <Camera/CameraFactories.h>
#include <Device/DeviceFactories.h>
#include <Light/LightFactories.h>
#include <Space/SpaceFactories.h>
#include <Shape/ShapeFactories.h>
#include <Filter/FilterFactories.h>
#include <Sampler/SamplerFactories.h>
#include <Texture/TextureFactories.h>
#include <Material/MaterialFactories.h>
#include <Renderer/RendererFactories.h>
#include <Postproc/PostProcessFactories.h>
#include <Integrator/IntegratorFactories.h>
		 
#include <Staging/Acceleration.h>
#include <Sampler/SamplerDiagnostics.h>

//----------------------------------------------------------------------------------------------
#include "ServiceManager.h"
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------

class SandboxEnvironment
{
protected:
	Environment *m_environment;
	EngineKernel *m_engineKernel;

public:
	//----------------------------------------------------------------------------------------------
	SandboxEnvironment(void)
		: m_environment(NULL)
		, m_engineKernel(NULL)
	{ }

	//----------------------------------------------------------------------------------------------
	~SandboxEnvironment(void)
	{ 
	}

	Environment* GetEnvironment(void) { return m_environment; }
	EngineKernel* GetEngineKernel(void) { return m_engineKernel; }

	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	bool Initialise(void)
	{
		Logger *logger = ServiceManager::GetInstance()->GetLogger();

		//----------------------------------------------------------------------------------------------
		// Engine Kernel
		//----------------------------------------------------------------------------------------------
		logger->Write("Environment :: Initialise EngineKernel...", LL_Info);
		m_engineKernel = new EngineKernel();

		//----------------------------------------------------------------------------------------------
		// Sampler
		//----------------------------------------------------------------------------------------------
		logger->Write("Environment :: Registering Samplers...", LL_Info);
		m_engineKernel->GetSamplerManager()->RegisterFactory("Random", new RandomSamplerFactory());
		m_engineKernel->GetSamplerManager()->RegisterFactory("Jitter", new JitterSamplerFactory());
		m_engineKernel->GetSamplerManager()->RegisterFactory("Multijitter", new MultijitterSamplerFactory());
		m_engineKernel->GetSamplerManager()->RegisterFactory("LowDiscrepancy", new LowDiscrepancySamplerFactory());
		m_engineKernel->GetSamplerManager()->RegisterFactory("PrecomputedRandom", new PrecomputedRandomSamplerFactory());
		m_engineKernel->GetSamplerManager()->RegisterFactory("PrecomputedHalton", new PrecomputedHaltonSamplerFactory());
		m_engineKernel->GetSamplerManager()->RegisterFactory("PrecomputedSobol", new PrecomputedSobolSamplerFactory());

		//----------------------------------------------------------------------------------------------
		// Filter
		//----------------------------------------------------------------------------------------------
		logger->Write("Environment :: Registering Filters...", LL_Info);
		m_engineKernel->GetFilterManager()->RegisterFactory("Box", new BoxFilterFactory());
		m_engineKernel->GetFilterManager()->RegisterFactory("Tent", new TentFilterFactory());

		//----------------------------------------------------------------------------------------------
		// Space
		//----------------------------------------------------------------------------------------------
		logger->Write("Environment :: Registering Spaces...", LL_Info);
		m_engineKernel->GetSpaceManager()->RegisterFactory("Basic", new BasicSpaceFactory());

		//----------------------------------------------------------------------------------------------
		// Integrator
		//----------------------------------------------------------------------------------------------
		logger->Write("Environment :: Registering Integrators...", LL_Info);
		m_engineKernel->GetIntegratorManager()->RegisterFactory("PathTracing", new PathIntegratorFactory());
		m_engineKernel->GetIntegratorManager()->RegisterFactory("MLIC", new MLICIntegratorFactory());
		m_engineKernel->GetIntegratorManager()->RegisterFactory("IC", new ICIntegratorFactory());
		m_engineKernel->GetIntegratorManager()->RegisterFactory("IGI", new IGIIntegratorFactory());
		m_engineKernel->GetIntegratorManager()->RegisterFactory("Whitted", new WhittedIntegratorFactory());
		//engineKernel.GetIntegratorManager()->RegisterFactory("Photon", new PhotonIntegratorFactory());
		//engineKernel.GetIntegratorManager()->RegisterFactory("Test", new TestIntegratorFactory());
		
		//----------------------------------------------------------------------------------------------
		// Post processing
		//----------------------------------------------------------------------------------------------
		logger->Write("Environment :: Registering Post Processes...", LL_Info);
		m_engineKernel->GetPostProcessManager()->RegisterFactory("AutoTone", new AutoToneFactory());
		m_engineKernel->GetPostProcessManager()->RegisterFactory("DragoTone", new DragoToneFactory());
		m_engineKernel->GetPostProcessManager()->RegisterFactory("GlobalTone", new GlobalToneFactory());
		m_engineKernel->GetPostProcessManager()->RegisterFactory("Accumulation", new AccumulationBufferFactory());
		m_engineKernel->GetPostProcessManager()->RegisterFactory("Discontinuity", new DiscontinuityBufferFactory());
		m_engineKernel->GetPostProcessManager()->RegisterFactory("Reconstruction", new ReconstructionBufferFactory());
		m_engineKernel->GetPostProcessManager()->RegisterFactory("BilateralFilter", new BilateralFilterFactory());
		m_engineKernel->GetPostProcessManager()->RegisterFactory("MotionBlur", new MotionBlurFactory());
		m_engineKernel->GetPostProcessManager()->RegisterFactory("History", new HistoryBufferFactory());

		//----------------------------------------------------------------------------------------------
		// Renderer
		//----------------------------------------------------------------------------------------------
		logger->Write("Environment :: Registering Renderers...", LL_Info);
		m_engineKernel->GetRendererManager()->RegisterFactory("Basic", new BasicRendererFactory());
		m_engineKernel->GetRendererManager()->RegisterFactory("Disparity", new DisparityRendererFactory());
		m_engineKernel->GetRendererManager()->RegisterFactory("TimeConstrained", new TimeConstrainedRendererFactory());

		//----------------------------------------------------------------------------------------------
		// Device
		//----------------------------------------------------------------------------------------------
		logger->Write("Environment :: Registering Devices...", LL_Info);
		m_engineKernel->GetDeviceManager()->RegisterFactory("SharedMemory", new SharedMemoryDeviceFactory());
		m_engineKernel->GetDeviceManager()->RegisterFactory("GLDisplay", new GLDisplayDeviceFactory());
		m_engineKernel->GetDeviceManager()->RegisterFactory("Display", new DisplayDeviceFactory());
		m_engineKernel->GetDeviceManager()->RegisterFactory("BufferedImage", new BufferedImageDeviceFactory());
		m_engineKernel->GetDeviceManager()->RegisterFactory("Image", new ImageDeviceFactory());
		m_engineKernel->GetDeviceManager()->RegisterFactory("Video", new VideoDeviceFactory());
		m_engineKernel->GetDeviceManager()->RegisterFactory("RTP", new RTPDeviceFactory());

		//----------------------------------------------------------------------------------------------
		// Cameras
		//----------------------------------------------------------------------------------------------
		logger->Write("Environment :: Registering Cameras...", LL_Info);
		m_engineKernel->GetCameraManager()->RegisterFactory("Perspective", new PerspectiveCameraFactory());
		m_engineKernel->GetCameraManager()->RegisterFactory("ThinLens", new ThinLensCameraFactory());

		//----------------------------------------------------------------------------------------------
		// Lights
		//----------------------------------------------------------------------------------------------
		logger->Write("Environment :: Registering Lights...", LL_Info);
		m_engineKernel->GetLightManager()->RegisterFactory("Point", new PointLightFactory());
		m_engineKernel->GetLightManager()->RegisterFactory("DiffuseArea", new DiffuseAreaLightFactory());
		m_engineKernel->GetLightManager()->RegisterFactory("InfiniteArea", new InfiniteAreaLightFactory());

		//----------------------------------------------------------------------------------------------
		// Shapes
		//----------------------------------------------------------------------------------------------
		logger->Write("Environment :: Registering Shapes...", LL_Info);
		m_engineKernel->GetShapeManager()->RegisterFactory("PersistentMesh", new PersistentMeshShapeFactory());
		m_engineKernel->GetShapeManager()->RegisterFactory("KDTreeMeshEx", new KDTreeMeshExShapeFactory());
		m_engineKernel->GetShapeManager()->RegisterFactory("KDTreeMesh", new KDTreeMeshShapeFactory());
		m_engineKernel->GetShapeManager()->RegisterFactory("BVHMesh", new BVHMeshShapeFactory());
		m_engineKernel->GetShapeManager()->RegisterFactory("Quad", new QuadMeshShapeFactory());
		m_engineKernel->GetShapeManager()->RegisterFactory("Triangle", new TriangleShapeFactory());
		m_engineKernel->GetShapeManager()->RegisterFactory("Sphere", new SphereShapeFactory());

		//----------------------------------------------------------------------------------------------
		// Textures
		//----------------------------------------------------------------------------------------------
		logger->Write("Environment :: Registering Textures...", LL_Info);
		m_engineKernel->GetTextureManager()->RegisterFactory("MappedFile", new MemoryMappedTextureFactory());
		m_engineKernel->GetTextureManager()->RegisterFactory("Image", new ImageTextureFactory());
		m_engineKernel->GetTextureManager()->RegisterFactory("Noise", new NoiseTextureFactory());
		m_engineKernel->GetTextureManager()->RegisterFactory("Marble", new MarbleTextureFactory());

		//----------------------------------------------------------------------------------------------
		// Materials
		//----------------------------------------------------------------------------------------------
		logger->Write("Environment :: Registering Materials...", LL_Info);
		m_engineKernel->GetMaterialManager()->RegisterFactory("Matte", new MatteMaterialFactory());
		m_engineKernel->GetMaterialManager()->RegisterFactory("Mirror", new MirrorMaterialFactory());
		m_engineKernel->GetMaterialManager()->RegisterFactory("Glass", new GlassMaterialFactory());
		m_engineKernel->GetMaterialManager()->RegisterFactory("Group", new MaterialGroupFactory());
	
		//----------------------------------------------------------------------------------------------
		// Environment
		//----------------------------------------------------------------------------------------------
		logger->Write("Environment :: Initialising Environment...", LL_Info);
		m_environment = new Environment(m_engineKernel);

		return true;
	}

	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	bool Shutdown(void)
	{
		//----------------------------------------------------------------------------------------------
		// Environment
		//----------------------------------------------------------------------------------------------
		Logger *logger = ServiceManager::GetInstance()->GetLogger();
		logger->Write("Environment :: Shutting down Environment...", LL_Info);
		Safe_Delete(m_environment);

		//----------------------------------------------------------------------------------------------
		// EngineKernel
		//----------------------------------------------------------------------------------------------
		if (m_engineKernel) 
		{
			//----------------------------------------------------------------------------------------------
			// Sampler
			//----------------------------------------------------------------------------------------------
			logger->Write("Environment :: Freeing and unregistering Samplers...", LL_Info);

			delete m_engineKernel->GetSamplerManager()->RequestFactory("Random");
			delete m_engineKernel->GetSamplerManager()->RequestFactory("Jitter");
			delete m_engineKernel->GetSamplerManager()->RequestFactory("Multijitter");
			delete m_engineKernel->GetSamplerManager()->RequestFactory("LowDiscrepancy");
			delete m_engineKernel->GetSamplerManager()->RequestFactory("PrecomputedRandom");
			delete m_engineKernel->GetSamplerManager()->RequestFactory("PrecomputedHalton");
			delete m_engineKernel->GetSamplerManager()->RequestFactory("PrecomputedSobol");

			m_engineKernel->GetSamplerManager()->UnregisterFactory("Random");
			m_engineKernel->GetSamplerManager()->UnregisterFactory("Jitter");
			m_engineKernel->GetSamplerManager()->UnregisterFactory("Multijitter");
			m_engineKernel->GetSamplerManager()->UnregisterFactory("LowDiscrepancy");
			m_engineKernel->GetSamplerManager()->UnregisterFactory("PrecomputedRandom");
			m_engineKernel->GetSamplerManager()->UnregisterFactory("PrecomputedHalton");
			m_engineKernel->GetSamplerManager()->UnregisterFactory("PrecomputedSobol");

			//----------------------------------------------------------------------------------------------
			// Filter
			//----------------------------------------------------------------------------------------------
			logger->Write("Environment :: Freeing and unregistering Filters...", LL_Info);

			delete m_engineKernel->GetFilterManager()->RequestFactory("Box");
			delete m_engineKernel->GetFilterManager()->RequestFactory("Tent");

			m_engineKernel->GetFilterManager()->UnregisterFactory("Box");
			m_engineKernel->GetFilterManager()->UnregisterFactory("Tent");

			//----------------------------------------------------------------------------------------------
			// Space
			//----------------------------------------------------------------------------------------------
			logger->Write("Environment :: Freeing and unregistering Spaces...", LL_Info);
			//Logger::Message("Freeing and unregistering Spaces...", p_bVerbose);
			delete m_engineKernel->GetSpaceManager()->RequestFactory("Basic");
			m_engineKernel->GetSpaceManager()->UnregisterFactory("Basic");

			//----------------------------------------------------------------------------------------------
			// Integrator
			//----------------------------------------------------------------------------------------------
			logger->Write("Environment :: Freeing and unregistering Integrators...", LL_Info);

			delete m_engineKernel->GetIntegratorManager()->RequestFactory("PathTracing");
			delete m_engineKernel->GetIntegratorManager()->RequestFactory("MLIC");
			delete m_engineKernel->GetIntegratorManager()->RequestFactory("IC");
			delete m_engineKernel->GetIntegratorManager()->RequestFactory("IGI");
			delete m_engineKernel->GetIntegratorManager()->RequestFactory("Whitted");
			//delete m_engineKernel->GetIntegratorManager()->RequestFactory("Photon");
			//delete m_engineKernel->GetIntegratorManager()->RequestFactory("Test");

			m_engineKernel->GetIntegratorManager()->UnregisterFactory("PathTracing");
			m_engineKernel->GetIntegratorManager()->UnregisterFactory("MLIC");
			m_engineKernel->GetIntegratorManager()->UnregisterFactory("IC");
			m_engineKernel->GetIntegratorManager()->UnregisterFactory("IGI");
			m_engineKernel->GetIntegratorManager()->UnregisterFactory("Whitted");
			//m_engineKernel->GetIntegratorManager()->UnregisterFactory("Photon");
			//m_engineKernel->GetIntegratorManager()->UnregisterFactory("Test");

			//----------------------------------------------------------------------------------------------
			// Renderer
			//----------------------------------------------------------------------------------------------
			logger->Write("Environment :: Freeing and unregistering Renderers...", LL_Info);

			delete m_engineKernel->GetRendererManager()->RequestFactory("Basic");
			delete m_engineKernel->GetRendererManager()->RequestFactory("Disparity");
			delete m_engineKernel->GetRendererManager()->RequestFactory("TimeConstrained");

			m_engineKernel->GetRendererManager()->UnregisterFactory("Basic");
			m_engineKernel->GetRendererManager()->UnregisterFactory("Disparity");
			m_engineKernel->GetRendererManager()->UnregisterFactory("TimeConstrained");

			//----------------------------------------------------------------------------------------------
			// Device
			//----------------------------------------------------------------------------------------------
			logger->Write("Environment :: Freeing and unregistering Devices...", LL_Info);

			delete m_engineKernel->GetDeviceManager()->RequestFactory("SharedMemory");
			delete m_engineKernel->GetDeviceManager()->RequestFactory("GLDisplay");
			delete m_engineKernel->GetDeviceManager()->RequestFactory("Display");
			delete m_engineKernel->GetDeviceManager()->RequestFactory("BufferedImage");
			delete m_engineKernel->GetDeviceManager()->RequestFactory("Image");
			delete m_engineKernel->GetDeviceManager()->RequestFactory("Video");
			delete m_engineKernel->GetDeviceManager()->RequestFactory("RTP");

			m_engineKernel->GetDeviceManager()->UnregisterFactory("SharedMemory");
			m_engineKernel->GetDeviceManager()->UnregisterFactory("GLDisplay");
			m_engineKernel->GetDeviceManager()->UnregisterFactory("Display");
			m_engineKernel->GetDeviceManager()->UnregisterFactory("BufferedImage");
			m_engineKernel->GetDeviceManager()->UnregisterFactory("Image");
			m_engineKernel->GetDeviceManager()->UnregisterFactory("Video");
			m_engineKernel->GetDeviceManager()->UnregisterFactory("RTP");

			//----------------------------------------------------------------------------------------------
			// Cameras
			//----------------------------------------------------------------------------------------------
			logger->Write("Environment :: Freeing and unregistering Cameras...", LL_Info);

			delete m_engineKernel->GetCameraManager()->RequestFactory("Perspective");
			delete m_engineKernel->GetCameraManager()->RequestFactory("ThinLens");

			m_engineKernel->GetCameraManager()->UnregisterFactory("Perspective");
			m_engineKernel->GetCameraManager()->UnregisterFactory("ThinLens");

			//----------------------------------------------------------------------------------------------
			// Lights
			//----------------------------------------------------------------------------------------------
			logger->Write("Environment :: Freeing and unregistering Lights...", LL_Info);

			delete m_engineKernel->GetLightManager()->RequestFactory("Point");
			delete m_engineKernel->GetLightManager()->RequestFactory("DiffuseArea");
			delete m_engineKernel->GetLightManager()->RequestFactory("InfiniteArea");

			m_engineKernel->GetLightManager()->UnregisterFactory("Point");
			m_engineKernel->GetLightManager()->UnregisterFactory("DiffuseArea");
			m_engineKernel->GetLightManager()->UnregisterFactory("InfiniteArea");

			//----------------------------------------------------------------------------------------------
			// Shapes
			//----------------------------------------------------------------------------------------------
			logger->Write("Environment :: Freeing and unregistering Shapes...", LL_Info);

			delete m_engineKernel->GetShapeManager()->RequestFactory("PersistentMesh");
			delete m_engineKernel->GetShapeManager()->RequestFactory("KDTreeMeshEx");
			delete m_engineKernel->GetShapeManager()->RequestFactory("KDTreeMesh");
			delete m_engineKernel->GetShapeManager()->RequestFactory("BVHMesh");
			delete m_engineKernel->GetShapeManager()->RequestFactory("Quad");
			delete m_engineKernel->GetShapeManager()->RequestFactory("Triangle");
			delete m_engineKernel->GetShapeManager()->RequestFactory("Sphere");

			m_engineKernel->GetShapeManager()->UnregisterFactory("PersistentMesh");
			m_engineKernel->GetShapeManager()->UnregisterFactory("KDTreeMeshEx");
			m_engineKernel->GetShapeManager()->UnregisterFactory("KDTreeMesh");
			m_engineKernel->GetShapeManager()->UnregisterFactory("BVHMesh");
			m_engineKernel->GetShapeManager()->UnregisterFactory("Quad");
			m_engineKernel->GetShapeManager()->UnregisterFactory("Triangle");
			m_engineKernel->GetShapeManager()->UnregisterFactory("Sphere");

			//----------------------------------------------------------------------------------------------
			// Textures
			//----------------------------------------------------------------------------------------------
			logger->Write("Environment :: Freeing and unregistering Textures...", LL_Info);

			delete m_engineKernel->GetTextureManager()->RequestFactory("MappedFile");
			delete m_engineKernel->GetTextureManager()->RequestFactory("Image");
			delete m_engineKernel->GetTextureManager()->RequestFactory("Noise");
			delete m_engineKernel->GetTextureManager()->RequestFactory("Marble");

			m_engineKernel->GetTextureManager()->UnregisterFactory("MappedFile");
			m_engineKernel->GetTextureManager()->UnregisterFactory("Image");
			m_engineKernel->GetTextureManager()->UnregisterFactory("Noise");
			m_engineKernel->GetTextureManager()->UnregisterFactory("Marble");

			//----------------------------------------------------------------------------------------------
			// Materials
			//----------------------------------------------------------------------------------------------
			logger->Write("Environment :: Freeing and unregistering Materials...", LL_Info);

			delete m_engineKernel->GetMaterialManager()->RequestFactory("Matte");
			delete m_engineKernel->GetMaterialManager()->RequestFactory("Mirror");
			delete m_engineKernel->GetMaterialManager()->RequestFactory("Glass");
			delete m_engineKernel->GetMaterialManager()->RequestFactory("Group");

			m_engineKernel->GetMaterialManager()->UnregisterFactory("Matte");
			m_engineKernel->GetMaterialManager()->UnregisterFactory("Mirror");
			m_engineKernel->GetMaterialManager()->UnregisterFactory("Glass");
			m_engineKernel->GetMaterialManager()->UnregisterFactory("Group");
			
			//----------------------------------------------------------------------------------------------
			// Post processing
			//----------------------------------------------------------------------------------------------
			logger->Write("Environment :: Freeing and unregistering Post Processes...", LL_Info);

			delete m_engineKernel->GetPostProcessManager()->RequestFactory("History");
			delete m_engineKernel->GetPostProcessManager()->RequestFactory("MotionBlur");
			delete m_engineKernel->GetPostProcessManager()->RequestFactory("AutoTone");
			delete m_engineKernel->GetPostProcessManager()->RequestFactory("DragoTone");
			delete m_engineKernel->GetPostProcessManager()->RequestFactory("GlobalTone");
			delete m_engineKernel->GetPostProcessManager()->RequestFactory("Accumulation");
			delete m_engineKernel->GetPostProcessManager()->RequestFactory("Discontinuity");
			delete m_engineKernel->GetPostProcessManager()->RequestFactory("Reconstruction");
			delete m_engineKernel->GetPostProcessManager()->RequestFactory("BilateralFilter");

			m_engineKernel->GetPostProcessManager()->UnregisterFactory("History");
			m_engineKernel->GetPostProcessManager()->UnregisterFactory("MotionBlur");
			m_engineKernel->GetPostProcessManager()->UnregisterFactory("AutoTone");
			m_engineKernel->GetPostProcessManager()->UnregisterFactory("DragoTone");
			m_engineKernel->GetPostProcessManager()->UnregisterFactory("GlobalTone");
			m_engineKernel->GetPostProcessManager()->UnregisterFactory("Accumulation");
			m_engineKernel->GetPostProcessManager()->UnregisterFactory("Discontinuity");
			m_engineKernel->GetPostProcessManager()->UnregisterFactory("Reconstruction");
			m_engineKernel->GetPostProcessManager()->UnregisterFactory("BilateralFilter");

			//----------------------------------------------------------------------------------------------
			// Engine Kernel
			//----------------------------------------------------------------------------------------------
			logger->Write("Environment :: Shutting down EngineKernel...", LL_Info);
			Safe_Delete(m_engineKernel);
		}
				
		return true;
	}

	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	bool LoadScene(const std::string &p_strScript)
	{
		// Logger instance
		Logger *logger = ServiceManager::GetInstance()->GetLogger();

		if (m_environment == NULL)
			return false;

		// Load environment script
		logger->Write("Environment :: Loading Environment script...", LL_Info);
	
		if (!m_environment->Load(p_strScript))
		{
			logger->Write("Environment :: Unable to load Environment script!", LL_Critical);
			return false;
		}

		// Initialise integrator and renderer
		m_environment->GetIntegrator()->Initialise(m_environment->GetScene(), m_environment->GetCamera());
		m_environment->GetRenderer()->Initialise();

		return true;
	}
};
