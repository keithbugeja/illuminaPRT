//----------------------------------------------------------------------------------------------
//	Filename:	BSDF.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "Object/Object.h"
#include "Spectrum/Spectrum.h"
#include "Geometry/Transform.h"
#include "Shape/DifferentialSurface.h"

namespace Illumina
{
	namespace Core
	{
		class BSDF 
			: public Object
		{
		public:
			static void LocalToSurface(const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out);
			static void WorldToSurface(const Transformation &p_worldTransform, const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out);
			static void SurfaceToLocal(const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out);
			static void SurfaceToWorld(const Transformation &p_worldTransform, const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out);

			static void GenerateVectorInHemisphere(float p_u, float p_v, Vector3 &p_out);

			BSDF(void) { }
			BSDF(const std::string &p_strName) : Object(p_strName) { }

			virtual Spectrum Rho(Vector3 &p_wOut);
			virtual Spectrum SampleF(const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v, float *p_pdf);
			virtual Spectrum F(const Vector3 &p_wOut, const Vector3 &p_wIn);
			virtual float Pdf(const Vector3 &p_wOut, const Vector3 &p_wIn);
		};
	}
}