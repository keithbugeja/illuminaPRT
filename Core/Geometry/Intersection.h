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
			enum DetailField
			{
				DF_Final		= 0x0001,
				DF_Direct		= 0x0002,
				DF_Indirect		= 0x0003,
				DF_Albedo		= 0x0004,

				DF_Distance		= 0x0008,
				DF_Normal		= 0x0010,
				DF_Position		= 0x0020,
				DF_ViewRay		= 0x0040,

				DF_Computed			= 0x0080,
				DF_ToneMapped		= 0x0100,
				DF_Accumulated		= 0x0200,
				DF_Processed		= 0x0400,
				DF_MaskEnabled		= 0x0500
			};

		public:
			Spectrum Final;
			Spectrum Direct;
			Spectrum Indirect;
			Spectrum Albedo;

			Vector3 Normal;
			Vector3 Position;

			Ray ViewRay;

			float Distance;

			unsigned short Flags;

		public:
			inline void SetSpatialContext(Intersection *p_pIntersection)
			{
				Flags |= DF_Distance | DF_Normal | DF_Position | DF_ViewRay;
				
				ViewRay.Set(p_pIntersection->Surface.RayOriginWS, 
					p_pIntersection->Surface.RayDirectionWS);

				Normal = p_pIntersection->Surface.ShadingBasisWS.W;
				Position = p_pIntersection->Surface.PointWS;
			
				Distance = p_pIntersection->Surface.Distance;
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