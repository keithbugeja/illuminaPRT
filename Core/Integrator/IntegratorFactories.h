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