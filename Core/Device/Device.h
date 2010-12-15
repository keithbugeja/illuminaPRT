//----------------------------------------------------------------------------------------------
//	Filename:	Device.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"

namespace Illumina
{
	namespace Core
	{
		class IDevice
		{
		public:
			virtual int GetWidth(void) const = 0;
			virtual int GetHeight(void) const = 0;

			virtual void BeginFrame(void) = 0;
			virtual void EndFrame(void) = 0;

			virtual void Set(int p_nX, int p_nY, const Spectrum &p_spectrum) = 0;
			virtual void Set(float p_fX, float p_fY, const Spectrum &p_spectrum) = 0;
		};
	}
}