//----------------------------------------------------------------------------------------------
//	Filename:	Intersection.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"

#include "Geometry/Transform.h"
#include "Shape/DifferentialSurface.h"
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

			void Reset(void);

			bool IsEmissive(void) const;
			bool HasMaterial(void) const;

			IPrimitive* GetPrimitive(void) const;
			void SetPrimitive(IPrimitive* p_pPrimitive);

			IMaterial* GetMaterial(void) const;
			void SetMaterial(IMaterial* p_pMaterial);

			ILight* GetLight(void) const;
			void SetLight(ILight* p_pLight);
		};
	}
}