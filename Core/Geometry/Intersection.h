//----------------------------------------------------------------------------------------------
//	Filename:	Intersection.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"

#include "Spectrum/Spectrum.h"
#include "Geometry/Transform.h"
#include "Shape/DifferentialSurface.h"
#include "System/Buffer.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		class Intersection
		{
		protected:
			IPrimitive *m_pPrimitive;
			IMaterial *m_pMaterial;
			ILight *m_pLight;

		public:
			DifferentialSurface Surface;
			Transformation WorldTransform;

		public:
			Intersection(void);
			Intersection(const Intersection &p_intersection);

			void Reset(void);
			void Prepare(void);

			bool IsValid(void) const;
			bool IsEmissive(void) const;
			bool HasMaterial(void) const;

			IPrimitive* GetPrimitive(void) const;
			void SetPrimitive(IPrimitive* p_pPrimitive);

			IMaterial* GetMaterial(void) const;
			void SetMaterial(IMaterial* p_pMaterial);

			ILight* GetLight(void) const;
			void SetLight(ILight* p_pLight);

			const Intersection& operator=(const Intersection &p_intersection);
		};

		class RadianceContext
		{
		public:
			Spectrum Final;
			Spectrum Direct;
			Spectrum Indirect;
			Spectrum Albedo;

			Vector3 Normal;
			Vector3 Position;

			Ray ViewRay;

			char Flag;

		public:
			inline void SetSpatialContext(Intersection *p_pIntersection)
			{
				ViewRay.Set(p_pIntersection->Surface.RayOriginWS, 
					p_pIntersection->Surface.RayDirectionWS);

				Normal = p_pIntersection->Surface.ShadingBasisWS.W;
				Position = p_pIntersection->Surface.PointWS;
			}
		};

		class RadianceBuffer
			: public Buffer2D<RadianceContext>
		{ 
		public:
			RadianceBuffer(int p_nWidth, int p_nHeight)
				: Buffer2D<RadianceContext>::Buffer2D(p_nWidth, p_nHeight)
			{ }

			RadianceBuffer(int p_nWidth, int p_nHeight, RadianceContext *p_pBuffer)
				: Buffer2D<RadianceContext>::Buffer2D(p_nWidth, p_nHeight, p_pBuffer)
			{ }
		};
	}
}