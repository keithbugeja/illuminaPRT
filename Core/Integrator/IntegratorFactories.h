//----------------------------------------------------------------------------------------------
//	Filename:	LightFactories.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <iostream>

#include "Integrator/Integrator.h"
#include "Integrator/PathIntegrator.h"
#include "Integrator/IGIIntegrator.h"
#include "Integrator/PhotonIntegrator.h"
#include "Integrator/WhittedIntegrator.h"
#include "Integrator/TestIntegrator.h"

namespace Illumina
{
	namespace Core
	{		
		class PathIntegratorFactory : public Illumina::Core::Factory<Illumina::Core::IIntegrator>
		{
		public:
			Illumina::Core::IIntegrator *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			/*
			 * Arguments
			 * -- Id {String}
			 * -- RayDepth {Integer}
			 * -- ShadowRays {Integer}
			 * -- Epsilon {Integer}
			 */
			Illumina::Core::IIntegrator *CreateInstance(ArgumentMap &p_argumentMap)
			{
				int raydepth = 6,
					shadowrays = 1;

				float reflectEpsilon = 1e-4f;

				std::string strId;

				p_argumentMap.GetArgument("RayDepth", raydepth);
				p_argumentMap.GetArgument("ShadowRays", shadowrays);
				p_argumentMap.GetArgument("Epsilon", reflectEpsilon);

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, raydepth, shadowrays, reflectEpsilon);

				return CreateInstance(raydepth, shadowrays, reflectEpsilon);
			}

			Illumina::Core::IIntegrator *CreateInstance(const std::string &p_strId, int p_nRayDepth, int p_nShadowRays, float p_fReflectEpsilon)
			{
				return new PathIntegrator(p_strId, p_nRayDepth, p_nShadowRays, p_fReflectEpsilon);
			}

			Illumina::Core::IIntegrator *CreateInstance(int p_nRayDepth, int p_nShadowRays, float p_fReflectEpsilon)
			{
				return new PathIntegrator(p_nRayDepth, p_nShadowRays, p_fReflectEpsilon);
			}
		};

		class IGIIntegratorFactory : public Illumina::Core::Factory<Illumina::Core::IIntegrator>
		{
		public:
			Illumina::Core::IIntegrator *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			/*
			 * Arguments
			 * -- Id {String}
			 * -- MaxVPLs {Integer}
			 * -- RayDepth {Integer}
			 * -- ShadowRays {Integer}
			 * -- Epsilon {Integer}
			 */
			Illumina::Core::IIntegrator *CreateInstance(ArgumentMap &p_argumentMap)
			{
				int maxVPLs, 
					raydepth,
					shadowrays;

				float reflectEpsilon = 1e-4f;

				std::string strId;

				p_argumentMap.GetArgument("MaxVPL", maxVPLs);
				p_argumentMap.GetArgument("RayDepth", raydepth);
				p_argumentMap.GetArgument("ShadowRays", shadowrays);
				p_argumentMap.GetArgument("Epsilon", reflectEpsilon);

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, maxVPLs, raydepth, shadowrays, reflectEpsilon);

				return CreateInstance(maxVPLs, raydepth, shadowrays, reflectEpsilon);
			}

			Illumina::Core::IIntegrator *CreateInstance(const std::string &p_strId, int p_nMaxVPLs, int p_nRayDepth, int p_nShadowRays, float p_fReflectEpsilon)
			{
				return new IGIIntegrator(p_strId, p_nMaxVPLs, p_nRayDepth, p_nShadowRays, p_fReflectEpsilon);
			}

			Illumina::Core::IIntegrator *CreateInstance(int p_nMaxVPLs, int p_nRayDepth, int p_nShadowRays, float p_fReflectEpsilon)
			{
				return new IGIIntegrator(p_nMaxVPLs, p_nRayDepth, p_nShadowRays, p_fReflectEpsilon);
			}
		};

		class PhotonIntegratorFactory : public Illumina::Core::Factory<Illumina::Core::IIntegrator>
		{
		public:
			Illumina::Core::IIntegrator *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			/*
			 * Arguments
			 * -- Id {String}
			 * -- RayDepth {Integer}
			 * -- ShadowRays {Integer}
			 * -- Photons {Integer}
			 * -- Epsilon {Integer}
			 */
			Illumina::Core::IIntegrator *CreateInstance(ArgumentMap &p_argumentMap)
			{
				int raydepth = 6,
					shadowrays = 1,
					photons = 1e+5;

				float reflectEpsilon = 1e-4f;

				std::string strId;

				p_argumentMap.GetArgument("RayDepth", raydepth);
				p_argumentMap.GetArgument("ShadowRays", shadowrays);
				p_argumentMap.GetArgument("Photons", photons);
				p_argumentMap.GetArgument("Epsilon", reflectEpsilon);

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, photons, raydepth, shadowrays, reflectEpsilon);

				return CreateInstance(photons, raydepth, shadowrays, reflectEpsilon);
			}

			Illumina::Core::IIntegrator *CreateInstance(const std::string &p_strId, int p_nPhotons, int p_nRayDepth, int p_nShadowRays, float p_fReflectEpsilon)
			{
				return new PhotonIntegrator(p_strId, p_nPhotons, p_nRayDepth, p_nShadowRays, p_fReflectEpsilon);
			}

			Illumina::Core::IIntegrator *CreateInstance(int p_nPhotons, int p_nRayDepth, int p_nShadowRays, float p_fReflectEpsilon)
			{
				return new PhotonIntegrator(p_nPhotons, p_nRayDepth, p_nShadowRays, p_fReflectEpsilon);
			}
		};

		class WhittedIntegratorFactory : public Illumina::Core::Factory<Illumina::Core::IIntegrator>
		{
		public:
			Illumina::Core::IIntegrator *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			/*
			 * Arguments
			 * -- Id {String}
			 * -- RayDepth {Integer}
			 * -- ShadowRays {Integer}
			 */
			Illumina::Core::IIntegrator *CreateInstance(ArgumentMap &p_argumentMap)
			{
				int raydepth = 6,
					shadowrays = 1;

				std::string strId;

				p_argumentMap.GetArgument("RayDepth", raydepth);
				p_argumentMap.GetArgument("ShadowRays", shadowrays);

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, raydepth, shadowrays);

				return CreateInstance(raydepth, shadowrays);
			}

			Illumina::Core::IIntegrator *CreateInstance(const std::string &p_strId, int p_nRayDepth, int p_nShadowRays)
			{
				return new WhittedIntegrator(p_strId, p_nRayDepth, p_nShadowRays, (p_nShadowRays > 0));
			}

			Illumina::Core::IIntegrator *CreateInstance(int p_nRayDepth = 6, int p_nShadowRays = 1)
			{
				return new WhittedIntegrator(p_nRayDepth, p_nShadowRays, (p_nShadowRays > 0));
			}
		};

		class TestIntegratorFactory : public Illumina::Core::Factory<Illumina::Core::IIntegrator>
		{
		public:
			Illumina::Core::IIntegrator *CreateInstance(void)
			{
				return new TestIntegrator();
			}

			/*
			 * Arguments
			 * -- Id {String}
			 */
			Illumina::Core::IIntegrator *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId);

				return CreateInstance();
			}

			Illumina::Core::IIntegrator *CreateInstance(const std::string &p_strId)
			{
				return new TestIntegrator(p_strId);
			}
		};
	}
}