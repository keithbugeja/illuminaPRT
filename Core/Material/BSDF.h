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
#include "Material/BxDF.h"

namespace Illumina
{
	namespace Core
	{
		class BSDF 
			: public Object
		{
		protected:
			int GetBxDFCount(BxDF::Type p_bxdfType, bool p_bExactMatch = false);
			int GetBxDF(BxDF::Type p_bxdfType, int p_nBxDFIndex, BxDF **p_pBxDF, bool p_bExactMatch = false);

		public:
			static void LocalToSurface(const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out, bool p_bUseShadingNormals = false);
			static void WorldToSurface(const Transformation &p_worldTransform, const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out, bool p_bUseShadingNormals = false);
			static void SurfaceToLocal(const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out, bool p_bUseShadingNormals = false);
			static void SurfaceToWorld(const Transformation &p_worldTransform, const DifferentialSurface &p_surface, const Vector3 &p_vector, Vector3 &p_out, bool p_bUseShadingNormals = false);

			static void GenerateVectorInHemisphere(float p_u, float p_v, Vector3 &p_out);

			BSDF(void) { }
			BSDF(const std::string &p_strId) : Object(p_strId) { }

			bool HasBxDFType(BxDF::Type p_bxdfType, bool p_bExactMatch = true);

			virtual Spectrum Rho(Vector3 &p_wOut, int p_nSampleCount = 1, float *p_nSampleList = NULL, BxDF::Type p_bxdfType = BxDF::All_Combined);
			virtual Spectrum SampleF(const DifferentialSurface &p_surface, const Vector3 &p_wOut, Vector3 &p_wIn, float p_u, float p_v, 
				float *p_pdf, BxDF::Type p_bxdfType = BxDF::All_Combined, BxDF::Type *p_sampledBxDFType = NULL);
			virtual Spectrum F(const DifferentialSurface &p_surface, const Vector3 &p_wOut, const Vector3 &p_wIn, BxDF::Type p_bxdfType = BxDF::All_Combined);
			virtual float Pdf(const Vector3 &p_wOut, const Vector3 &p_wIn, BxDF::Type p_bxdfType = BxDF::All_Combined);
		
			virtual Spectrum SampleTexture(const DifferentialSurface &p_surface, int p_nBxDFIndex);
		
		protected:
			List<BxDF*> m_bxdfList;
		};
	}
}