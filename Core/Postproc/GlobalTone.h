//----------------------------------------------------------------------------------------------
//	Filename:	GlobalTone.h
//	Author:		Keith Bugeja
//	Date:		27/03/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Postproc/PostProcess.h"

//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------
		class GlobalTone
			: public IPostProcess
		{
		protected:
			Spectrum m_scaleFactor;
			float m_fBias;

		public:
			GlobalTone(void)
				: IPostProcess()
			{ }

			GlobalTone(const std::string &p_strName)
				: IPostProcess(p_strName)
			{ }

			void Reset(void) 
			{ }

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput, int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight)
			{
				RadianceContext *pInputContext,
					*pOutputContext;

				for (int y = p_nRegionY; y < p_nRegionHeight; ++y)
				{
					for (int x = p_nRegionX; x < p_nRegionWidth; ++x)
					{
						pInputContext = p_pInput->GetP(x, y);
						pOutputContext = p_pOutput->GetP(x, y);

						pOutputContext->Final = pInputContext->Final / (pInputContext->Final + 1);
						pOutputContext->Flag = 1;
					}
				}

				return true;
			}

			bool Apply(RadianceBuffer *p_pInput, RadianceBuffer *p_pOutput)
			{
				return Apply(p_pInput, p_pOutput, 0, 0, p_pInput->GetWidth(), p_pInput->GetHeight());
			}

			std::string ToString(void) const { return "[AutoTone]"; }
		};
		//----------------------------------------------------------------------------------------------
	}
}