//----------------------------------------------------------------------------------------------
//	Filename:	Space.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "System/FactoryManager.h"

#include "Object/Object.h"

#include "Geometry/Intersection.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// ISpace : Abstract base class for world-space container structures. 
		//----------------------------------------------------------------------------------------------
		class ISpace 
			: public Object
		{
		protected:
			ISpace(const std::string &p_strName) : Object(p_strName) { }
			ISpace(void) { }

		public:
			List<IPrimitive*> PrimitiveList;

			virtual bool Initialise(void) = 0;
			virtual void Shutdown(void) = 0;
			virtual bool Update(void) = 0;
			virtual bool Build(void) = 0;

			virtual bool Intersects(const Ray &p_ray, float p_fTime, Intersection &p_intersection, IPrimitive *p_pExclude) const = 0;
			virtual bool Intersects(const Ray &p_ray, float p_fTime, Intersection &p_intersection) const = 0;
			virtual bool Intersects(const Ray &p_ray, float p_fTime, IPrimitive *p_pExclude) const = 0;
			virtual bool Intersects(const Ray &p_ray, float p_fTime) const = 0;

			virtual std::string ToString(void) const { return "ISpace"; }
		};

		//----------------------------------------------------------------------------------------------
		// SpaceManager : All Space factories must register with object.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<ISpace> SpaceManager;
	} 
}