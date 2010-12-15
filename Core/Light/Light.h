#pragma once

#include "Geometry/Vector3.h"
#include "Spectrum/Spectrum.h"
#include "Staging/Visibility.h"

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
	}
}