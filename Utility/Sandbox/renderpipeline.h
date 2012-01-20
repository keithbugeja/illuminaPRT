#include "taskpipeline.h"
#include "mpirender.h"

#pragma once

namespace Illumina
{
	namespace Core
	{
		/*
	void InitialiseIllumina(EngineKernel **p_engineKernel, bool p_bVerbose)
	{
	}

	void ShutdownIllumina(EngineKernel *p_engineKernel, bool p_bVerbose)
	{
		delete p_engineKernel;
	}

	*/
		class RenderPipeline
			: public ITaskPipeline
		{
		protected:
			MPIRender *m_mpirender;
			Environment *m_environment;
			EngineKernel *m_engineKernel;
		
		public:
			RenderPipeline(std::string &p_arguments, bool p_verbose = true)
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
				m_engineKernel->GetIntegratorManager()->RegisterFactory("Photon", new PhotonIntegratorFactory());
				m_engineKernel->GetIntegratorManager()->RegisterFactory("Whitted", new WhittedIntegratorFactory());
				m_engineKernel->GetIntegratorManager()->RegisterFactory("Test", new TestIntegratorFactory());

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
				if (m_mpirender) delete m_mpirender;
				if (m_environment) delete m_environment;
				if (m_engineKernel) delete m_engineKernel;
				
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
				float alpha = Maths::Pi;

				// Cornell
				/*
				Vector3 lookFrom(70, 0, 70),
					lookAt(0, 0, 0);
				*/
				Vector3 lookFrom(800, 100, 200),
					lookAt(0, 200, 100);

				// Update space
				pSpace->Update();
	 
				m_mpirender->RenderCoordinator(&p_coordinator);

				// Render frame
				// pRenderer->Render();

				/*
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
				*/
				return true;
			}

			bool ExecuteWorker(Task *p_worker)
			{
				ICamera *pCamera = m_environment->GetCamera();
				ISpace *pSpace = m_environment->GetSpace();
				float alpha = Maths::Pi;

				// Cornell
				/*
				Vector3 lookFrom(70, 0, 70),
					lookAt(0, 0, 0);
				*/
				Vector3 lookFrom(800, 100, 200),
					lookAt(0, 200, 100);

				// Update space
				pSpace->Update();

				m_mpirender->RenderWorker(p_worker);

				return true;
			}
		};
	}
}