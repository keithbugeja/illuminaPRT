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
			//----------------------------------------------------------------------------------------------
			/* 
			 */
			virtual float Pdf(const Vector3 &p_point, const Vector3 &p_wOut) = 0;

			//----------------------------------------------------------------------------------------------
			/* Returns an approximation of the total emitted power by the luminaire.
			 */
			virtual Spectrum Power(void) = 0;

			//----------------------------------------------------------------------------------------------
			/* Returns the Radiance emitted by the luminaire along the specified ray.
			 */
			virtual Spectrum Radiance(const Ray &p_ray) { return 0.0f; }

			//----------------------------------------------------------------------------------------------
			/* Returns the Radiance emitted at the specified point on the surface of the luminaire along
			 * the direction vector wIn.
			 * 
			 * This method is mainly employed by area lights that have a position and volume in space.
			 */
			virtual Spectrum Radiance(const Vector3 &p_lightSurfacePoint, const Vector3 &p_lightSurfaceNormal, const Vector3 &p_wIn) = 0;
			
			//----------------------------------------------------------------------------------------------
			/* Samples a point on the luminaire.
			 */
			virtual Vector3 SamplePoint(float p_u, float p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf) = 0;

			virtual Vector3 SamplePoint(const Vector3 &p_viewPoint, float p_u, float p_v, Vector3 &p_lightSurfaceNormal, float &p_pdf)
			{
				return SamplePoint(p_u, p_v, p_lightSurfaceNormal, p_pdf);
			}

			//----------------------------------------------------------------------------------------------
			/* Computes emitted radiance towards the specified point. The vector wIn is given the direction
			 * from the sampled point on the luminaire towards the specified surface point:
			 *
			 *    wIn = surfacePoint - sampledPoint
			 * 
			 * The pdf gives specifies the contribution of the estimated radiance, while the visibility 
			 * query structure is enabled to occlusion testing between the two points.			 
			 */
			virtual Spectrum SampleRadiance(const Vector3 &p_surfacePoint, float p_u, float p_v, Vector3 &p_wIn, float &p_pdf, VisibilityQuery &p_visibilityQuery) = 0;

			virtual Spectrum SampleRadiance(const Vector3 &p_surfacePoint, const Vector3 &p_surfaceNormal, float p_u, float p_v, Vector3 &p_wIn, float &p_pdf, VisibilityQuery &p_visibilityQuery) 
			{
				return SampleRadiance(p_surfacePoint, p_u, p_v, p_wIn, p_pdf, p_visibilityQuery);
			}

			virtual Spectrum SampleRadiance(const Scene* p_pScene, float p_u, float p_v, float p_w, float p_x, Ray &p_ray, float &p_pdf) = 0;
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