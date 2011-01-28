//----------------------------------------------------------------------------------------------
//	Filename:	Environment.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class Environment
		{
		protected:
			EngineKernel *m_pEngineKernel;

			IRenderer *m_pRenderer;
			Scene *m_pScene;

			bool m_bIsInitialised;

		public:
			Environment(EngineKernel *p_pEngineKernel, IRenderer *p_pRenderer = NULL);
			~Environment(void) { };

			bool IsInitialised(void) const;
			bool Initialise(void);
			void Shutdown(void);

			EngineKernel* GetEngineKernel(void) const;

			void SetIntegrator(IIntegrator *p_pIntegrator);
			IIntegrator* GetIntegrator(void) const;

			void SetSampler(ISampler *p_pSampler);
			ISampler* GetSampler(void) const;

			void SetFilter(IFilter *p_pFilter);
			IFilter* GetFilter(void) const;

			void SetDevice(IDevice *p_pDevice);
			IDevice* GetDevice(void) const;

			void SetCamera(ICamera *p_pCamera);
			ICamera* GetCamera(void) const;

			void SetSpace(ISpace *p_pSpace);
			ISpace* GetSpace(void) const;

			void SetRenderer(IRenderer *p_pRenderer);
			IRenderer* GetRenderer(void) const;

			void SetScene(Scene *p_pScene);
			Scene* GetScene(void) const;

			bool Load(const std::string &p_strEnvironmentName);
			bool Save(const std::string &p_strEnvironmentName);
		};
	}
}