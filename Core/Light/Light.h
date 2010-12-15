//----------------------------------------------------------------------------------------------
//	Filename:	Light.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "Threading/List.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class ILight
		{
		public:
			virtual Spectrum Power(void) = 0;
			virtual Spectrum Radiance(const Vector3 &p_point, Vector3 &p_wOut, VisibilityQuery &p_visibilityQuery) = 0;
			virtual Spectrum Radiance(const Vector3 &p_point, double p_u, double p_v, Vector3& p_wOut, VisibilityQuery &p_visibilityQuery) = 0;
		};

		typedef List<ILight*> LightList;
		typedef boost::shared_ptr<ILight> LightPtr;
	}
}