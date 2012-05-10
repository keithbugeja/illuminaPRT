//----------------------------------------------------------------------------------------------
//	Filename:	TimeConstrainedRenderer.h
//	Author:		Keith Bugeja
//	Date:		27/04/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Renderer/BaseRenderer.h"
#include "Sampler/PrecomputationSampler.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class TimeConstrainedRenderer 
			: public BaseRenderer
		{
		protected:			
			using IRenderer::m_pRadianceBuffer;
			using IRenderer::m_pIntegrator;
			using IRenderer::m_pDevice;
			using IRenderer::m_pFilter;
			using IRenderer::m_pScene;

			using BaseRenderer::m_nSampleCount;

			float m_fRenderBudget;

			PrecomputedHaltonSampler m_ldSampler;

		public:
			TimeConstrainedRenderer(const std::string &p_strName, Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, 
				IDevice *p_pDevice = NULL, IFilter *p_pFilter = NULL, RadianceBuffer *p_pRadianceBuffer = NULL, int p_nSampleCount = 1);
			
			TimeConstrainedRenderer(Scene *p_pScene = NULL, IIntegrator *p_pIntegrator = NULL, IDevice *p_pDevice = NULL, 
				IFilter *p_pFilter = NULL, RadianceBuffer *p_pRadianceBuffer = NULL, int p_nSampleCount = 1);

			void RenderRegion(RadianceBuffer *p_pRadianceBuffer, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, int p_nBufferX, int p_nBufferY);

			void SetRenderBudget(float p_fRenderBudget) { m_fRenderBudget = p_fRenderBudget; }
		};
	}
}