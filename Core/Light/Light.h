//----------------------------------------------------------------------------------------------
//	Filename:	Light.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "System/FactoryManager.h"

#include "Object/Object.h"

#include "Spectrum/Spectrum.h"
#include "Threading/List.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// ILight : Abstract base class for luminaires. 
		//----------------------------------------------------------------------------------------------
		class ILight 
			: public Object
		{
		protected:
			ILight(const std::string &p_strName) : Object(p_strName) { }
			ILight(void) { }

		public:
			virtual float Pdf(const Vector3 &p_point, const Vector3 &p_wOut) = 0;

			virtual Spectrum Power(void) = 0;
			virtual Spectrum Radiance(const Vector3 &p_point, const Vector3 &p_normal, const Vector3 &p_wIn) = 0;
			virtual Spectrum Radiance(const Ray &p_ray) { return 0.0f; }

			//----------------------------------------------------------------------------------------------
			/* 
			 * Sample radiance methods should return surface radiance on the luminaire compounded with the geometry term:
			 * Le(x', x - x') * G(x, x'), where G(x, x') = ((x'/|x'|) . n') / (p(x') * |x - x'|^2)
			 * Note that the foreshortening term (x/|x| . n) missing from G() should be supplied.
			 *
			 * The method also populates a VisibilityQuery structure which allows the test for visibility between
			 * points, v(x, x') to be carried out.
			 *
			 * Note that the direction of wIn is from the sampled point on the luminaire towards p_point.
			 */
			virtual Spectrum SampleRadiance(const Vector3 &p_point, Vector3 &p_wIn, VisibilityQuery &p_visibilityQuery) = 0;
			virtual Spectrum SampleRadiance(const Vector3 &p_point, double p_u, double p_v, Vector3& p_wIn, VisibilityQuery &p_visibilityQuery) = 0;
			//----------------------------------------------------------------------------------------------
			std::string ToString(void) const { return "ILight"; }
		};

		typedef List<ILight*> LightList;
		typedef boost::shared_ptr<ILight> LightPtr;

		//----------------------------------------------------------------------------------------------
		// LightManager : All Light factories must register with object.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<ILight> LightManager;
	}
}