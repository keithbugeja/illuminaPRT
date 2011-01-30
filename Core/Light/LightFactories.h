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

#include "Light/Light.h"
#include "Light/PointLight.h"
#include "Light/DiffuseAreaLight.h"

namespace Illumina
{
	namespace Core
	{		
		class PointLightFactory : public Illumina::Core::Factory<Illumina::Core::ILight>
		{
		public:
			Illumina::Core::IIntegrator *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			/*
			 * Arguments
			 * -- Id {String}
			 * -- Position {Vector3}
			 * -- Intensity {Spectrum}
			 */
			Illumina::Core::IIntegrator *CreateInstance(ArgumentMap &p_argumentMap)
			{
				Vector3 position(0);
				Spectrum intensity(0);
				std::string strId;

				p_argumentMap.GetArgument("Position", position);
				p_argumentMap.GetArgument("Intensity", intensity);

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, position, intensity);

				return CreateInstance(position, intensity);
			}

			Illumina::Core::IIntegrator *CreateInstance(const std::string &p_strId, const Vector3 &p_position, const Spectrum &p_intensity)
			{
				return new PointLight(p_strId, p_position, p_intensity);
			}

			Illumina::Core::IIntegrator *CreateInstance(const Vector3 &p_position, const Spectrum &p_intensity)
			{
				return new PointLight(p_position, p_intensity);
			}
		};

		class DiffuseAreaLightFactory : public Illumina::Core::Factory<Illumina::Core::ILight>
		{
		public:
			Illumina::Core::ILight *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			/*
			 * Arguments
			 * -- Id {String}
			 * -- RayDepth {Integer}
			 * -- ShadowRays {Integer}
			 */
			Illumina::Core::ILight *CreateInstance(ArgumentMap &p_argumentMap)
			{
				Spectrum power;
				std::string strId;

				p_argumentMap.GetArgument("Power", power);

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, power);

				return CreateInstance(power);
			}

			Illumina::Core::ILight *CreateInstance(const std::string &p_strId, const Spectrum &p_power)
			{
				return new DiffuseAreaLight(p_strId, NULL, NULL, p_power);
			}

			Illumina::Core::ILight *CreateInstance(const Spectrum &p_power)
			{
				return new DiffuseAreaLight(NULL, NULL, p_power);
			}
		};
	}
}