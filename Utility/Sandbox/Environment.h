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
#include "Logger.h"
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
	{  }

	//----------------------------------------------------------------------------------------------
	~SandboxEnvironment(void)
	{ 
	}

	Environment* GetEnvironment(void) { return m_environment; }
	EngineKernel* GetEngineKernel(void) { return m_engineKernel; }

	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	bool Initialise(bool p_bVerbose)
	{
		//----------------------------------------------------------------------------------------------
		// Engine Kernel
		//----------------------------------------------------------------------------------------------
		Logger::Message("\nInitialising EngineKernel...", p_bVerbose);
		m_engineKernel = new EngineKernel();

		//----------------------------------------------------------------------------------------------
		// Sampler
		//----------------------------------------------------------------------------------------------
		Logger::Message("Registering Samplers...", p_bVerbose);
		m_engineKernel->GetSamplerManager()->RegisterFactory("Random", new RandomSamplerFactory());
		m_engineKernel->GetSamplerManager()->RegisterFactory("Jitter", new JitterSamplerFactory());
		m_engineKernel->GetSamplerManager()->RegisterFactory("Multijitter", new MultijitterSamplerFactory());
		m_engineKernel->GetSamplerManager()->RegisterFactory("PrecomputedRandom", new PrecomputedRandomSamplerFactory());
		m_engineKernel->GetSamplerManager()->RegisterFactory("PrecomputedHalton", new PrecomputedHaltonSamplerFactory());
		m_engineKernel->GetSamplerManager()->RegisterFactory("PrecomputedSobol", new PrecomputedSobolSamplerFactory());

		//----------------------------------------------------------------------------------------------
		// Filter
		//----------------------------------------------------------------------------------------------
		Logger::Message("Registering Filters...", p_bVerbose);
		m_engineKernel->GetFilterManager()->RegisterFactory("Box", new BoxFilterFactory());
		m_engineKernel->GetFilterManager()->RegisterFactory("Tent", new TentFilterFactory());

		//----------------------------------------------------------------------------------------------
		// Space
		//----------------------------------------------------------------------------------------------
		Logger::Message("Registering Spaces...", p_bVerbose);
		m_engineKernel->GetSpaceManager()->RegisterFactory("Basic", new BasicSpaceFactory());

		//----------------------------------------------------------------------------------------------
		// Integrator
		//----------------------------------------------------------------------------------------------
		Logger::Message("Registering Integrators...", p_bVerbose);
		m_engineKernel->GetIntegratorManager()->RegisterFactory("PathTracing", new PathIntegratorFactory());
		m_engineKernel->GetIntegratorManager()->RegisterFactory("IGI", new IGIIntegratorFactory());
		m_engineKernel->GetIntegratorManager()->RegisterFactory("Whitted", new WhittedIntegratorFactory());
		//engineKernel.GetIntegratorManager()->RegisterFactory("Photon", new PhotonIntegratorFactory());
		//engineKernel.GetIntegratorManager()->RegisterFactory("Test", new TestIntegratorFactory());
		
		//----------------------------------------------------------------------------------------------
		// Materials
		//----------------------------------------------------------------------------------------------
		Logger::Message("Registering Post Processes...", p_bVerbose);
		m_engineKernel->GetPostProcessManager()->RegisterFactory("AutoTone", new AutoToneFactory());
		m_engineKernel->GetPostProcessManager()->RegisterFactory("DragoTone", new DragoToneFactory());
		m_engineKernel->GetPostProcessManager()->RegisterFactory("Accumulation", new AccumulationBufferFactory());
		m_engineKernel->GetPostProcessManager()->RegisterFactory("Discontinuity", new DiscontinuityBufferFactory());
		m_engineKernel->GetPostProcessManager()->RegisterFactory("Reconstruction", new ReconstructionBufferFactory());

		//----------------------------------------------------------------------------------------------
		// Renderer
		//----------------------------------------------------------------------------------------------
		Logger::Message("Registering Renderers...", p_bVerbose);
		m_engineKernel->GetRendererManager()->RegisterFactory("Basic", new BasicRendererFactory());
		m_engineKernel->GetRendererManager()->RegisterFactory("Distributed", new DistributedRendererFactory());
		m_engineKernel->GetRendererManager()->RegisterFactory("TimeConstrained", new TimeConstrainedRendererFactory());

		//----------------------------------------------------------------------------------------------
		// Device
		//----------------------------------------------------------------------------------------------
		Logger::Message("Registering Devices...", p_bVerbose);
		m_engineKernel->GetDeviceManager()->RegisterFactory("Display", new DisplayDeviceFactory());
		m_engineKernel->GetDeviceManager()->RegisterFactory("Image", new ImageDeviceFactory());
		m_engineKernel->GetDeviceManager()->RegisterFactory("Video", new VideoDeviceFactory());
		m_engineKernel->GetDeviceManager()->RegisterFactory("RTP", new RTPDeviceFactory());

		//----------------------------------------------------------------------------------------------
		// Cameras
		//----------------------------------------------------------------------------------------------
		Logger::Message("Registering Cameras...", p_bVerbose);
		m_engineKernel->GetCameraManager()->RegisterFactory("Perspective", new PerspectiveCameraFactory());
		m_engineKernel->GetCameraManager()->RegisterFactory("ThinLens", new ThinLensCameraFactory());

		//----------------------------------------------------------------------------------------------
		// Lights
		//----------------------------------------------------------------------------------------------
		Logger::Message("Registering Lights...", p_bVerbose);
		m_engineKernel->GetLightManager()->RegisterFactory("Point", new PointLightFactory());
		m_engineKernel->GetLightManager()->RegisterFactory("DiffuseArea", new DiffuseAreaLightFactory());
		m_engineKernel->GetLightManager()->RegisterFactory("InfiniteArea", new InfiniteAreaLightFactory());

		//----------------------------------------------------------------------------------------------
		// Shapes
		//----------------------------------------------------------------------------------------------
		Logger::Message("Registering Shapes...", p_bVerbose);
		m_engineKernel->GetShapeManager()->RegisterFactory("PersistentMesh", new PersistentMeshShapeFactory());
		m_engineKernel->GetShapeManager()->RegisterFactory("KDTreeMesh", new KDTreeMeshShapeFactory());
		m_engineKernel->GetShapeManager()->RegisterFactory("BVHMesh", new BVHMeshShapeFactory());
		m_engineKernel->GetShapeManager()->RegisterFactory("Quad", new QuadMeshShapeFactory());
		m_engineKernel->GetShapeManager()->RegisterFactory("Triangle", new TriangleShapeFactory());
		m_engineKernel->GetShapeManager()->RegisterFactory("Sphere", new SphereShapeFactory());

		//----------------------------------------------------------------------------------------------
		// Textures
		//----------------------------------------------------------------------------------------------
		Logger::Message("Registering Textures...", p_bVerbose);
		m_engineKernel->GetTextureManager()->RegisterFactory("MappedFile", new MemoryMappedTextureFactory());
		m_engineKernel->GetTextureManager()->RegisterFactory("Image", new ImageTextureFactory());
		m_engineKernel->GetTextureManager()->RegisterFactory("Noise", new NoiseTextureFactory());
		m_engineKernel->GetTextureManager()->RegisterFactory("Marble", new MarbleTextureFactory());

		//----------------------------------------------------------------------------------------------
		// Materials
		//----------------------------------------------------------------------------------------------
		Logger::Message("Registering Materials...", p_bVerbose);
		m_engineKernel->GetMaterialManager()->RegisterFactory("Matte", new MatteMaterialFactory());
		m_engineKernel->GetMaterialManager()->RegisterFactory("Mirror", new MirrorMaterialFactory());
		m_engineKernel->GetMaterialManager()->RegisterFactory("Glass", new GlassMaterialFactory());
		m_engineKernel->GetMaterialManager()->RegisterFactory("Group", new MaterialGroupFactory());
	
		//----------------------------------------------------------------------------------------------
		// Environment
		//----------------------------------------------------------------------------------------------
		Logger::Message("Initialising Environment...", p_bVerbose);
		m_environment = new Environment(m_engineKernel);

		return true;
	}

	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	bool Shutdown(bool p_bVerbose)
	{
		//----------------------------------------------------------------------------------------------
		// Environment
		//----------------------------------------------------------------------------------------------
		Logger::Message("Shutting down Environment...", p_bVerbose);
		Safe_Delete(m_environment);

		//----------------------------------------------------------------------------------------------
		// EngineKernel
		//----------------------------------------------------------------------------------------------
		if (m_engineKernel) 
		{
			//----------------------------------------------------------------------------------------------
			// Sampler
			//----------------------------------------------------------------------------------------------
			Logger::Message("Freeing and unregistering Samplers...", p_bVerbose);
			delete m_engineKernel->GetSamplerManager()->RequestFactory("Random");
			delete m_engineKernel->GetSamplerManager()->RequestFactory("Jitter");
			delete m_engineKernel->GetSamplerManager()->RequestFactory("Multijitter");
			delete m_engineKernel->GetSamplerManager()->RequestFactory("PrecomputedRandom");
			delete m_engineKernel->GetSamplerManager()->RequestFactory("PrecomputedHalton");
			delete m_engineKernel->GetSamplerManager()->RequestFactory("PrecomputedSobol");

			m_engineKernel->GetSamplerManager()->UnregisterFactory("Random");
			m_engineKernel->GetSamplerManager()->UnregisterFactory("Jitter");
			m_engineKernel->GetSamplerManager()->UnregisterFactory("Multijitter");
			m_engineKernel->GetSamplerManager()->UnregisterFactory("PrecomputedRandom");
			m_engineKernel->GetSamplerManager()->UnregisterFactory("PrecomputedHalton");
			m_engineKernel->GetSamplerManager()->UnregisterFactory("PrecomputedSobol");

			//----------------------------------------------------------------------------------------------
			// Filter
			//----------------------------------------------------------------------------------------------
			Logger::Message("Freeing and unregistering Filters...", p_bVerbose);
			delete m_engineKernel->GetFilterManager()->RequestFactory("Box");
			delete m_engineKernel->GetFilterManager()->RequestFactory("Tent");

			m_engineKernel->GetFilterManager()->UnregisterFactory("Box");
			m_engineKernel->GetFilterManager()->UnregisterFactory("Tent");

			//----------------------------------------------------------------------------------------------
			// Space
			//----------------------------------------------------------------------------------------------
			Logger::Message("Freeing and unregistering Spaces...", p_bVerbose);
			delete m_engineKernel->GetSpaceManager()->RequestFactory("Basic");
			m_engineKernel->GetSpaceManager()->UnregisterFactory("Basic");

			//----------------------------------------------------------------------------------------------
			// Integrator
			//----------------------------------------------------------------------------------------------
			Logger::Message("Freeing and unregistering Integrators...", p_bVerbose);
			delete m_engineKernel->GetIntegratorManager()->RequestFactory("PathTracing");
			delete m_engineKernel->GetIntegratorManager()->RequestFactory("IGI");
			delete m_engineKernel->GetIntegratorManager()->RequestFactory("Whitted");
			//delete m_engineKernel->GetIntegratorManager()->RequestFactory("Photon");
			//delete m_engineKernel->GetIntegratorManager()->RequestFactory("Test");

			m_engineKernel->GetIntegratorManager()->UnregisterFactory("PathTracing");
			m_engineKernel->GetIntegratorManager()->UnregisterFactory("IGI");
			m_engineKernel->GetIntegratorManager()->UnregisterFactory("Whitted");
			//m_engineKernel->GetIntegratorManager()->UnregisterFactory("Photon");
			//m_engineKernel->GetIntegratorManager()->UnregisterFactory("Test");

			//----------------------------------------------------------------------------------------------
			// Renderer
			//----------------------------------------------------------------------------------------------
			Logger::Message("Freeing and unregistering Renderers...", p_bVerbose);
			delete m_engineKernel->GetRendererManager()->RequestFactory("Basic");
			delete m_engineKernel->GetRendererManager()->RequestFactory("Distributed");
			delete m_engineKernel->GetRendererManager()->RequestFactory("TimeConstrained");

			m_engineKernel->GetRendererManager()->UnregisterFactory("Basic");
			m_engineKernel->GetRendererManager()->UnregisterFactory("Distributed");
			m_engineKernel->GetRendererManager()->UnregisterFactory("TimeConstrained");

			//----------------------------------------------------------------------------------------------
			// Device
			//----------------------------------------------------------------------------------------------
			Logger::Message("Freeing and unregistering Devices...", p_bVerbose);
			delete m_engineKernel->GetDeviceManager()->RequestFactory("Display");
			delete m_engineKernel->GetDeviceManager()->RequestFactory("Image");
			delete m_engineKernel->GetDeviceManager()->RequestFactory("Video");
			delete m_engineKernel->GetDeviceManager()->RequestFactory("RTP");

			m_engineKernel->GetDeviceManager()->UnregisterFactory("Display");
			m_engineKernel->GetDeviceManager()->UnregisterFactory("Image");
			m_engineKernel->GetDeviceManager()->UnregisterFactory("Video");
			m_engineKernel->GetDeviceManager()->UnregisterFactory("RTP");

			//----------------------------------------------------------------------------------------------
			// Cameras
			//----------------------------------------------------------------------------------------------
			Logger::Message("Freeing and unregistering Cameras...", p_bVerbose);
			delete m_engineKernel->GetCameraManager()->RequestFactory("Perspective");
			delete m_engineKernel->GetCameraManager()->RequestFactory("ThinLens");

			m_engineKernel->GetCameraManager()->UnregisterFactory("Perspective");
			m_engineKernel->GetCameraManager()->UnregisterFactory("ThinLens");

			//----------------------------------------------------------------------------------------------
			// Lights
			//----------------------------------------------------------------------------------------------
			Logger::Message("Freeing and unregistering Lights...", p_bVerbose);
			delete m_engineKernel->GetLightManager()->RequestFactory("Point");
			delete m_engineKernel->GetLightManager()->RequestFactory("DiffuseArea");
			delete m_engineKernel->GetLightManager()->RequestFactory("InfiniteArea");

			m_engineKernel->GetLightManager()->UnregisterFactory("Point");
			m_engineKernel->GetLightManager()->UnregisterFactory("DiffuseArea");
			m_engineKernel->GetLightManager()->UnregisterFactory("InfiniteArea");

			//----------------------------------------------------------------------------------------------
			// Shapes
			//----------------------------------------------------------------------------------------------
			Logger::Message("Freeing and unregistering Shapes...", p_bVerbose);
			delete m_engineKernel->GetShapeManager()->RequestFactory("PersistentMesh");
			delete m_engineKernel->GetShapeManager()->RequestFactory("KDTreeMesh");
			delete m_engineKernel->GetShapeManager()->RequestFactory("BVHMesh");
			delete m_engineKernel->GetShapeManager()->RequestFactory("Quad");
			delete m_engineKernel->GetShapeManager()->RequestFactory("Triangle");
			delete m_engineKernel->GetShapeManager()->RequestFactory("Sphere");

			m_engineKernel->GetShapeManager()->UnregisterFactory("PersistentMesh");
			m_engineKernel->GetShapeManager()->UnregisterFactory("KDTreeMesh");
			m_engineKernel->GetShapeManager()->UnregisterFactory("BVHMesh");
			m_engineKernel->GetShapeManager()->UnregisterFactory("Quad");
			m_engineKernel->GetShapeManager()->UnregisterFactory("Triangle");
			m_engineKernel->GetShapeManager()->UnregisterFactory("Sphere");

			//----------------------------------------------------------------------------------------------
			// Textures
			//----------------------------------------------------------------------------------------------
			Logger::Message("Freeing and unregistering Textures...", p_bVerbose);
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
			Logger::Message("Freeing and unregistering Materials...", p_bVerbose);
			delete m_engineKernel->GetMaterialManager()->RequestFactory("Matte");
			delete m_engineKernel->GetMaterialManager()->RequestFactory("Mirror");
			delete m_engineKernel->GetMaterialManager()->RequestFactory("Glass");
			delete m_engineKernel->GetMaterialManager()->RequestFactory("Group");

			m_engineKernel->GetMaterialManager()->UnregisterFactory("Matte");
			m_engineKernel->GetMaterialManager()->UnregisterFactory("Mirror");
			m_engineKernel->GetMaterialManager()->UnregisterFactory("Glass");
			m_engineKernel->GetMaterialManager()->UnregisterFactory("Group");
					
			//----------------------------------------------------------------------------------------------
			// Engine Kernel
			//----------------------------------------------------------------------------------------------
			Logger::Message("Shutting down Engine Kernel...", p_bVerbose);
			Safe_Delete(m_engineKernel);
		}
				
		return true;
	}

	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	bool LoadScene(const std::string &p_strScript, bool p_bVerbose)
	{
		if (m_environment == NULL)
			return false;

		// Load environment script
		Logger::Message("Loading Environment script...", p_bVerbose);
	
		if (!m_environment->Load(p_strScript))
		{
			Logger::Message("Unable to load Environment script.", true, Logger::Critical);
			return false;
		}

		// Initialise integrator and renderer
		m_environment->GetIntegrator()->Initialise(m_environment->GetScene(), m_environment->GetCamera());
		m_environment->GetRenderer()->Initialise();

		return true;
	}
};
