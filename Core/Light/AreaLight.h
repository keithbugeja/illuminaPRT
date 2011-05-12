//----------------------------------------------------------------------------------------------
//	Filename:	AreaLight.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "Light/Light.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class IAreaLight : 
			public ILight
		{
		protected:
			IShape *m_pShape;
			Transformation *m_pWorldTransform;

		protected:
			IAreaLight(void);
			IAreaLight(const std::string &p_strName);
			IAreaLight(Transformation *p_pWorldTransform, IShape* p_pShape);
			IAreaLight(const std::string &p_strName, Transformation *p_pWorldTransform, IShape* p_pShape);

		public:			
			virtual IShape* GetShape(void) const;
			virtual void SetShape(IShape* p_pShape);

			virtual Transformation* GetWorldTransform(void) const;
			virtual void SetWorldTransform(Transformation *p_pWorldTransform);
		};
	}
}