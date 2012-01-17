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
		
		public:
			RenderPipeline(Environment *p_environment, bool p_verbose = true)
				: ITaskPipeline(p_verbose)
				, m_environment(p_environment)				
				, m_mpirender(new MPIRender(p_environment))
			{ }

			~RenderPipeline(void)
			{ 
				delete m_mpirender;
			}

			bool LoadScene(const std::string &p_strScript, bool p_bVerbose)
			{
				// Load environment script
				MessageOut("Loading Environment script...", p_bVerbose);

				if (!m_environment->Load(p_strScript))
				{
					std::cerr << "Error : Unable to load environment script." << std::endl;
					//exit(-1);
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

				pIntegrator->Prepare(m_environment->GetScene());

				// Initialisation complete
				MessageOut("Initialisation complete. Rendering in progress...", p_bVerbose);

				return true;
			}

			bool OnInitialiseCoordinator(ArgumentMap &p_argumentMap) 
			{
				std::string script;

				if (p_argumentMap.GetArgument("script", script))
				{
					return LoadScene(script, IsVerbose());
				}

				return false; 
			}
			
			bool OnShutdownCoordinator(void) 
			{ 
				return true; 
			}

			bool OnInitialiseWorker(ArgumentMap &p_argumentMap) 
			{ 
				std::string script;

				if (p_argumentMap.GetArgument("script", script))
				{
					return LoadScene(script, IsVerbose());
				}

				return false; 
			}
			
			bool OnShutdownWorker(void) 
			{ 
				return true; 
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