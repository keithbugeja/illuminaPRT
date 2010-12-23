//----------------------------------------------------------------------------------------------
//	Filename:	Device.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "Shape/DifferentialSurface.h"
#include "Geometry/Transform.h"
#include "Spectrum/Spectrum.h"
#include "Texture/Texture.h"

namespace Illumina
{
	namespace Core
	{
		class BSDF
		{
		public:
			//static LocalToSurface(const DifferentialSurface p_surface, const Vector3 &p_vector, Vector3 &p_out);
			//static WorldToSurface(const Transformation p_worldTransform, const DifferentialSurface, p_surface, const Vector3 &p_vector, Vector3 &p_out);
			//static SurfaceToLocal(const DifferentialSurface p_surface, const Vector3 &p_vector, Vector3 &p_out);
			//static SurfaceToWorld(const Transformation p_worldTransform, const DifferentialSurface, p_surface, const Vector3 &p_vector, Vector3 &p_out);

			virtual double F(const Vector3 &wIn, const Vector3 &wOut) { return 0.0f; }
			virtual double SampleF(const Vector3 &wIn, Vector3 &wOut) { return 0.0f; }
			virtual double Pdf(const Vector3 &wIn, const Vector3 &wOut) { return 1.0f; }
			virtual double Rho(Vector3 &wOut) const { return 0.0f; } // Reflectance function
		};

		/*
		class BSDFDiffuse : public BSDF
		{
		}
		*/

		class IMaterial
		{
		public:
			virtual const BSDF& GetBSDF(void) const;
		};

		/*
		class BasicMaterial : public IMaterial
		{
		protected:
			BSDFDiffuse m_bsdf;
			Spectrum m_spectrum;

		public:
			BasicMaterial(const Spectrum &p_spectrum)
				: m_spectrum(p_spectrum)
			{ }

			const BSDF& GetBSDF(void) const
			{
				return m_bsdf;
			}
		};
		*/
	}
}