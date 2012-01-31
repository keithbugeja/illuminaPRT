#include "taskpipeline.h"
#include "mpirender.h"

#pragma once

namespace Illumina
{
	namespace Core
	{
		class RenderPipeline
			: public ITaskPipeline
		{
		protected:
			MPIRender *m_mpirender;
			Environment *m_environment;
			EngineKernel *m_engineKernel;
		
		public:
			RenderPipeline(const std::string &p_arguments, bool p_verbose = true)
				: ITaskPipeline(p_arguments, p_verbose)
				, m_mpirender(NULL)
				, m_environment(NULL)				
				, m_engineKernel(NULL)
			{  }

			~RenderPipeline(void)
			{ 
			}

			bool Initialise(ArgumentMap &p_argumentMap)
			{
				//----------------------------------------------------------------------------------------------
				// Engine Kernel
				//----------------------------------------------------------------------------------------------
				m_engineKernel = new EngineKernel();

				//----------------------------------------------------------------------------------------------
				// Sampler
				//----------------------------------------------------------------------------------------------
				m_engineKernel->GetSamplerManager()->RegisterFactory("Random", new RandomSamplerFactory());
				m_engineKernel->GetSamplerManager()->RegisterFactory("Jitter", new JitterSamplerFactory());
				m_engineKernel->GetSamplerManager()->RegisterFactory("Multijitter", new MultijitterSamplerFactory());

				//----------------------------------------------------------------------------------------------
				// Filter
				//----------------------------------------------------------------------------------------------
				m_engineKernel->GetFilterManager()->RegisterFactory("Box", new BoxFilterFactory());
				m_engineKernel->GetFilterManager()->RegisterFactory("Tent", new TentFilterFactory());

				//----------------------------------------------------------------------------------------------
				// Space
				//----------------------------------------------------------------------------------------------
				m_engineKernel->GetSpaceManager()->RegisterFactory("Basic", new BasicSpaceFactory());

				//----------------------------------------------------------------------------------------------
				// Integrator
				//----------------------------------------------------------------------------------------------
				m_engineKernel->GetIntegratorManager()->RegisterFactory("PathTracing", new PathIntegratorFactory());
				m_engineKernel->GetIntegratorManager()->RegisterFactory("IGI", new IGIIntegratorFactory());
				//m_engineKernel->GetIntegratorManager()->RegisterFactory("Photon", new PhotonIntegratorFactory());
				m_engineKernel->GetIntegratorManager()->RegisterFactory("Whitted", new WhittedIntegratorFactory());
				//m_engineKernel->GetIntegratorManager()->RegisterFactory("Test", new TestIntegratorFactory());

				//----------------------------------------------------------------------------------------------
				// Renderer
				//----------------------------------------------------------------------------------------------
				m_engineKernel->GetRendererManager()->RegisterFactory("Basic", new BasicRendererFactory());
				m_engineKernel->GetRendererManager()->RegisterFactory("Multithreaded", new MultithreadedRendererFactory());
				m_engineKernel->GetRendererManager()->RegisterFactory("Distributed", new DistributedRendererFactory());

				//----------------------------------------------------------------------------------------------
				// Device
				//----------------------------------------------------------------------------------------------
				m_engineKernel->GetDeviceManager()->RegisterFactory("Image", new ImageDeviceFactory());

				//----------------------------------------------------------------------------------------------
				// Cameras
				//----------------------------------------------------------------------------------------------
				m_engineKernel->GetCameraManager()->RegisterFactory("Perspective", new PerspectiveCameraFactory());
				m_engineKernel->GetCameraManager()->RegisterFactory("ThinLens", new ThinLensCameraFactory());

				//----------------------------------------------------------------------------------------------
				// Lights
				//----------------------------------------------------------------------------------------------
				m_engineKernel->GetLightManager()->RegisterFactory("Point", new PointLightFactory());
				m_engineKernel->GetLightManager()->RegisterFactory("DiffuseArea", new DiffuseAreaLightFactory());
				m_engineKernel->GetLightManager()->RegisterFactory("InfiniteArea", new InfiniteAreaLightFactory());

				//----------------------------------------------------------------------------------------------
				// Shapes
				//----------------------------------------------------------------------------------------------
				m_engineKernel->GetShapeManager()->RegisterFactory("KDTreeMesh", new KDTreeMeshShapeFactory());
				m_engineKernel->GetShapeManager()->RegisterFactory("Quad", new QuadMeshShapeFactory());
				m_engineKernel->GetShapeManager()->RegisterFactory("Triangle", new TriangleShapeFactory());
				m_engineKernel->GetShapeManager()->RegisterFactory("Sphere", new SphereShapeFactory());

				//----------------------------------------------------------------------------------------------
				// Textures
				//----------------------------------------------------------------------------------------------
				m_engineKernel->GetTextureManager()->RegisterFactory("Image", new ImageTextureFactory());
				m_engineKernel->GetTextureManager()->RegisterFactory("Noise", new NoiseTextureFactory());
				m_engineKernel->GetTextureManager()->RegisterFactory("Marble", new MarbleTextureFactory());

				//----------------------------------------------------------------------------------------------
				// Materials
				//----------------------------------------------------------------------------------------------
				m_engineKernel->GetMaterialManager()->RegisterFactory("Matte", new MatteMaterialFactory());
				m_engineKernel->GetMaterialManager()->RegisterFactory("Mirror", new MirrorMaterialFactory());
				m_engineKernel->GetMaterialManager()->RegisterFactory("Glass", new GlassMaterialFactory());
				m_engineKernel->GetMaterialManager()->RegisterFactory("Group", new MaterialGroupFactory());

				//----------------------------------------------------------------------------------------------
				// Environment
				//----------------------------------------------------------------------------------------------
				m_environment = new Environment(m_engineKernel);

				//----------------------------------------------------------------------------------------------
				// MPIRender
				//----------------------------------------------------------------------------------------------
				int spp = 1, 
					tileWidth = 8, 
					tileHeight = 8;

				if (p_argumentMap.ContainsArgument("spp"))
					p_argumentMap.GetArgument("spp", spp);

				if (p_argumentMap.ContainsArgument("tw"))
					p_argumentMap.GetArgument("tw", tileWidth);

				if (p_argumentMap.ContainsArgument("th"))
					p_argumentMap.GetArgument("th", tileHeight);

				m_mpirender = new MPIRender(m_environment, spp, tileWidth, tileHeight);

				return true;
			}

			bool Shutdown(void)
			{
				//----------------------------------------------------------------------------------------------
				// MPIRender
				//----------------------------------------------------------------------------------------------
				if (m_mpirender) 
					delete m_mpirender;

				//----------------------------------------------------------------------------------------------
				// Environment
				//----------------------------------------------------------------------------------------------
				if (m_environment) 
					delete m_environment;

				//----------------------------------------------------------------------------------------------
				// EngineKernel
				//----------------------------------------------------------------------------------------------
				if (m_engineKernel) 
				{
					//----------------------------------------------------------------------------------------------
					// Sampler
					//----------------------------------------------------------------------------------------------
					delete m_engineKernel->GetSamplerManager()->RequestFactory("Random");
					delete m_engineKernel->GetSamplerManager()->RequestFactory("Jitter");
					delete m_engineKernel->GetSamplerManager()->RequestFactory("Multijitter");

					m_engineKernel->GetSamplerManager()->UnregisterFactory("Random");
					m_engineKernel->GetSamplerManager()->UnregisterFactory("Jitter");
					m_engineKernel->GetSamplerManager()->UnregisterFactory("Multijitter");

					//----------------------------------------------------------------------------------------------
					// Filter
					//----------------------------------------------------------------------------------------------
					delete m_engineKernel->GetFilterManager()->RequestFactory("Box");
					delete m_engineKernel->GetFilterManager()->RequestFactory("Tent");

					m_engineKernel->GetFilterManager()->UnregisterFactory("Box");
					m_engineKernel->GetFilterManager()->UnregisterFactory("Tent");

					//----------------------------------------------------------------------------------------------
					// Space
					//----------------------------------------------------------------------------------------------
					delete m_engineKernel->GetSpaceManager()->RequestFactory("Basic");
					m_engineKernel->GetSpaceManager()->UnregisterFactory("Basic");

					//----------------------------------------------------------------------------------------------
					// Integrator
					//----------------------------------------------------------------------------------------------
					delete m_engineKernel->GetIntegratorManager()->RequestFactory("PathTracing");
					delete m_engineKernel->GetIntegratorManager()->RequestFactory("IGI");
					//delete m_engineKernel->GetIntegratorManager()->RequestFactory("Photon");
					delete m_engineKernel->GetIntegratorManager()->RequestFactory("Whitted");
					//delete m_engineKernel->GetIntegratorManager()->RequestFactory("Test");

					m_engineKernel->GetIntegratorManager()->UnregisterFactory("PathTracing");
					m_engineKernel->GetIntegratorManager()->UnregisterFactory("IGI");
					//m_engineKernel->GetIntegratorManager()->UnregisterFactory("Photon");
					m_engineKernel->GetIntegratorManager()->UnregisterFactory("Whitted");
					//m_engineKernel->GetIntegratorManager()->UnregisterFactory("Test");

					//----------------------------------------------------------------------------------------------
					// Renderer
					//----------------------------------------------------------------------------------------------
					delete m_engineKernel->GetRendererManager()->RequestFactory("Basic");
					delete m_engineKernel->GetRendererManager()->RequestFactory("Multithreaded");
					delete m_engineKernel->GetRendererManager()->RequestFactory("Distributed");

					m_engineKernel->GetRendererManager()->UnregisterFactory("Basic");
					m_engineKernel->GetRendererManager()->UnregisterFactory("Multithreaded");
					m_engineKernel->GetRendererManager()->UnregisterFactory("Distributed");

					//----------------------------------------------------------------------------------------------
					// Device
					//----------------------------------------------------------------------------------------------
					delete m_engineKernel->GetDeviceManager()->RequestFactory("Image");
					m_engineKernel->GetDeviceManager()->UnregisterFactory("Image");

					//----------------------------------------------------------------------------------------------
					// Cameras
					//----------------------------------------------------------------------------------------------
					delete m_engineKernel->GetCameraManager()->RequestFactory("Perspective");
					delete m_engineKernel->GetCameraManager()->RequestFactory("ThinLens");

					m_engineKernel->GetCameraManager()->UnregisterFactory("Perspective");
					m_engineKernel->GetCameraManager()->UnregisterFactory("ThinLens");

					//----------------------------------------------------------------------------------------------
					// Lights
					//----------------------------------------------------------------------------------------------
					delete m_engineKernel->GetLightManager()->RequestFactory("Point");
					delete m_engineKernel->GetLightManager()->RequestFactory("DiffuseArea");
					delete m_engineKernel->GetLightManager()->RequestFactory("InfiniteArea");

					m_engineKernel->GetLightManager()->UnregisterFactory("Point");
					m_engineKernel->GetLightManager()->UnregisterFactory("DiffuseArea");
					m_engineKernel->GetLightManager()->UnregisterFactory("InfiniteArea");

					//----------------------------------------------------------------------------------------------
					// Shapes
					//----------------------------------------------------------------------------------------------
					delete m_engineKernel->GetShapeManager()->RequestFactory("KDTreeMesh");
					delete m_engineKernel->GetShapeManager()->RequestFactory("Quad");
					delete m_engineKernel->GetShapeManager()->RequestFactory("Triangle");
					delete m_engineKernel->GetShapeManager()->RequestFactory("Sphere");

					m_engineKernel->GetShapeManager()->UnregisterFactory("KDTreeMesh");
					m_engineKernel->GetShapeManager()->UnregisterFactory("Quad");
					m_engineKernel->GetShapeManager()->UnregisterFactory("Triangle");
					m_engineKernel->GetShapeManager()->UnregisterFactory("Sphere");

					//----------------------------------------------------------------------------------------------
					// Textures
					//----------------------------------------------------------------------------------------------
					delete m_engineKernel->GetTextureManager()->RequestFactory("Image");
					delete m_engineKernel->GetTextureManager()->RequestFactory("Noise");
					delete m_engineKernel->GetTextureManager()->RequestFactory("Marble");

					m_engineKernel->GetTextureManager()->UnregisterFactory("Image");
					m_engineKernel->GetTextureManager()->UnregisterFactory("Noise");
					m_engineKernel->GetTextureManager()->UnregisterFactory("Marble");

					//----------------------------------------------------------------------------------------------
					// Materials
					//----------------------------------------------------------------------------------------------
					delete m_engineKernel->GetMaterialManager()->RequestFactory("Matte");
					delete m_engineKernel->GetMaterialManager()->RequestFactory("Mirror");
					delete m_engineKernel->GetMaterialManager()->RequestFactory("Glass");
					delete m_engineKernel->GetMaterialManager()->RequestFactory("Group");

					m_engineKernel->GetMaterialManager()->UnregisterFactory("Matte");
					m_engineKernel->GetMaterialManager()->UnregisterFactory("Mirror");
					m_engineKernel->GetMaterialManager()->UnregisterFactory("Glass");
					m_engineKernel->GetMaterialManager()->UnregisterFactory("Group");

					delete m_engineKernel;
				}
				
				return true;
			}

			bool LoadScene(const std::string &p_strScript, bool p_bVerbose)
			{
				if (!m_environment->Load(p_strScript))
				{
					std::cerr << "Error : Unable to load environment script." << std::endl;
					return false;
				}

				// Alias required components
				IIntegrator *pIntegrator = m_environment->GetIntegrator();
				IRenderer *pRenderer = m_environment->GetRenderer();
				ISpace *pSpace = m_environment->GetSpace();

				// Initialise integrator and renderer
				pIntegrator->Initialise(m_environment->GetScene(), m_environment->GetCamera());
				pRenderer->Initialise();
				m_mpirender->Initialise();

				// Prepare integrator with current scene
				pIntegrator->Prepare(m_environment->GetScene());

				return true;
			}

			bool OnInitialiseCoordinator(ArgumentMap &p_argumentMap) 
			{
				std::string script;

				if (p_argumentMap.GetArgument("script", script))
					return (Initialise(p_argumentMap) && LoadScene(script, IsVerbose()));

				return false; 
			}
			
			bool OnShutdownCoordinator(void) 
			{ 
				return Shutdown();
			}

			bool OnInitialiseWorker(ArgumentMap &p_argumentMap) 
			{ 
				std::string script;

				if (p_argumentMap.GetArgument("script", script))
					return (Initialise(p_argumentMap) && LoadScene(script, IsVerbose()));

				return false; 
			}
			
			bool OnShutdownWorker(void) 
			{ 
				return Shutdown();
			}

			void OnCoordinatorReceiveMasterMessage(CoordinatorTask &p_coordinator, Message &p_message, MPI_Status *p_status, MPI_Request *p_request)
			{
				switch(p_message.Id)
				{
					// Init task termination
					case MT_Direction:
					{	
						std::cout << "[" << p_coordinator.group.GetCoordinatorRank() << "] :: Coordinator received [DIRECTION]." << std::endl;

						DirectionMessage *msg = (DirectionMessage*)&p_message;
						ParseDirectionMessage(msg);

						Vector3 observer = m_environment->GetCamera()->GetObserver();
						PositionMessage posMsg(observer);
						p_coordinator.ready.Broadcast(p_coordinator.task, &posMsg, MM_ChannelWorkerStatic);

						break;
					}
				}
			}

			void OnCoordinatorReceiveWorkerMessage(CoordinatorTask &p_coordinator, Message &p_message, MPI_Status *p_status, MPI_Request *p_request)
			{ }

			void OnWorkerReceiveCoordinatorMessage(Task *p_worker, Message &p_message)
			{
				switch(p_message.Id)
				{
					case MT_Position:
					{
						std::cout << "[" << p_worker->GetRank() << "] :: Worker received [POSITION]." << std::endl;
						PositionMessage *msg = (PositionMessage*)&p_message;
						m_environment->GetCamera()->MoveTo(msg->GetPosition());
					}
				}
			}

			void ParseDirectionMessage(DirectionMessage *p_message)
			{
				switch(p_message->GetDirection())
				{
					case CIT_Left:
						m_environment->GetCamera()->Move(Vector3::UnitXNeg * 5);
						break;
					case CIT_Right:
						m_environment->GetCamera()->Move(Vector3::UnitXPos * 5);
						break;
					case CIT_Forwards:
						m_environment->GetCamera()->Move(Vector3::UnitZNeg * 5);
						break;
					case CIT_Backwards:
						m_environment->GetCamera()->Move(Vector3::UnitZPos * 5);
						break;
					case CIT_Up:
						m_environment->GetCamera()->Move(Vector3::UnitYPos * 5);
						break;
					case CIT_Down:
						m_environment->GetCamera()->Move(Vector3::UnitYNeg * 5);
						break;
				}
			}

			bool ExecuteCoordinator(CoordinatorTask &p_coordinator)
			{
				ICamera *pCamera = m_environment->GetCamera();
				ISpace *pSpace = m_environment->GetSpace();

				// Update space
				pSpace->Update();
	 
				m_mpirender->RenderCoordinator(&p_coordinator);

				return true;
			}

			bool ExecuteWorker(Task *p_worker)
			{
				ICamera *pCamera = m_environment->GetCamera();
				ISpace *pSpace = m_environment->GetSpace();

				// Update space
				pSpace->Update();

				m_mpirender->RenderWorker(p_worker);

				return true;
			}
		};
	}
}