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
#include "Light/DirectionalLight.h"
#include "Light/DiffuseAreaLight.h"
#include "Light/InfiniteAreaLight.h"

namespace Illumina
{
	namespace Core
	{	
		class DirectionalLightFactory : public Illumina::Core::Factory<Illumina::Core::ILight>
		{
		public:
			Illumina::Core::ILight *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			/*
			 * Arguments
			 * -- Id {String}
			 * -- Distance  {float}
			 * -- Direction {Vector3}
			 * -- Intensity {Spectrum}
			 */
			Illumina::Core::ILight *CreateInstance(ArgumentMap &p_argumentMap)
			{
				float distance = 100.0f;
				Vector3 direction(0);
				Spectrum intensity(0);
				std::string strId;

				p_argumentMap.GetArgument("Distance", distance);
				p_argumentMap.GetArgument("Intensity", intensity);
				p_argumentMap.GetArgument("Direction", direction); direction.Normalize();

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, distance, direction, intensity);

				return CreateInstance(distance, direction, intensity);
			}

			Illumina::Core::ILight *CreateInstance(const std::string &p_strId, float p_fDistance, const Vector3 &p_direction, const Spectrum &p_intensity)
			{
				return new DirectionalLight(p_strId, p_fDistance, p_direction, p_intensity);
			}

			Illumina::Core::ILight *CreateInstance(float p_fDistance, const Vector3 &p_direction, const Spectrum &p_intensity)
			{
				return new DirectionalLight(p_fDistance, p_direction, p_intensity);
			}
		};

		class PointLightFactory : public Illumina::Core::Factory<Illumina::Core::ILight>
		{
		public:
			Illumina::Core::ILight *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			/*
			 * Arguments
			 * -- Id {String}
			 * -- Position {Vector3}
			 * -- Intensity {Spectrum}
			 */
			Illumina::Core::ILight *CreateInstance(ArgumentMap &p_argumentMap)
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

			Illumina::Core::ILight *CreateInstance(const std::string &p_strId, const Vector3 &p_position, const Spectrum &p_intensity)
			{
				return new PointLight(p_strId, p_position, p_intensity);
			}

			Illumina::Core::ILight *CreateInstance(const Vector3 &p_position, const Spectrum &p_intensity)
			{
				return new PointLight(p_position, p_intensity);
			}
		};

		class InfiniteAreaLightFactory : public Illumina::Core::Factory<Illumina::Core::ILight>
		{
		public:
			Illumina::Core::ILight *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			/*
			 * Arguments
			 * -- Id {String}
			 * -- Intensity {Spectrum}
			 */
			Illumina::Core::ILight *CreateInstance(ArgumentMap &p_argumentMap)
			{
				Spectrum intensity(0);
				std::string strId;

				p_argumentMap.GetArgument("Intensity", intensity);

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, intensity);

				return CreateInstance(intensity);
			}

			Illumina::Core::ILight *CreateInstance(const std::string &p_strId, const Spectrum &p_intensity)
			{
				return new InfiniteAreaLight(p_strId, p_intensity);
			}

			Illumina::Core::ILight *CreateInstance(const Spectrum &p_intensity)
			{
				return new InfiniteAreaLight(p_intensity);
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
			 * -- Power {Integer}
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