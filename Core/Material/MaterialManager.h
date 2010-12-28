//----------------------------------------------------------------------------------------------
//	Filename:	MaterialManager.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <iostream>

#include "System/FactoryManager.h"

#include "Material/Material.h"
#include "Material/MaterialGroup.h"
#include "Material/DiffuseMaterial.h"
#include "Material/PhongMaterial.h"

namespace Illumina
{
	namespace Core
	{
		typedef FactoryManager<IMaterial> MaterialManager;
		
		class MaterialGroupFactory : public Illumina::Core::Factory<Illumina::Core::IMaterial>
		{
		public:
			Illumina::Core::IMaterial *CreateInstance(void)
			{
				return new MaterialGroup();
			}

			Illumina::Core::IMaterial *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strName;

				if (p_argumentMap.GetArgument("Name", strName))
				{
					return new MaterialGroup(strName);
				}

				throw new Exception("Invalid arguments to MaterialGroupFactory!");
			}

			Illumina::Core::IMaterial *CreateInstance(const std::string &p_strName)
			{
				return new MaterialGroup(p_strName);
			}
		};

		class DiffuseMaterialFactory : public Illumina::Core::Factory<Illumina::Core::IMaterial>
		{
		public:
			Illumina::Core::IMaterial *CreateInstance(void)
			{
				return new DiffuseMaterial(Spectrum(0.5));
			}

			Illumina::Core::IMaterial *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strName;
				Spectrum reflectivity;

				if (p_argumentMap.GetArgument("Name", strName) && 
					p_argumentMap.GetArgument("Reflectivity", reflectivity))
				{
					return new DiffuseMaterial(strName, reflectivity);
				}

				throw new Exception("Invalid arguments to DiffuseMaterialFactory!");
			}

			Illumina::Core::IMaterial *CreateInstance(const std::string &p_strName, const Spectrum &p_reflectivity)
			{
				return new DiffuseMaterial(p_strName, p_reflectivity);
			}
		};

		class PhongMaterialFactory : public Illumina::Core::Factory<Illumina::Core::IMaterial>
		{
		public:
			Illumina::Core::IMaterial* CreateInstance(void)
			{
				return new PhongMaterial(Spectrum(0.5), 16.0f);
			}

			Illumina::Core::IMaterial *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strName;
				Spectrum reflectivity;
				float exponent;

				if (p_argumentMap.GetArgument("Name", strName) && 
					p_argumentMap.GetArgument("Reflectivity", reflectivity) &&
					p_argumentMap.GetArgument("Exponent", exponent))
				{
					return new PhongMaterial(strName, reflectivity, exponent);
				}

				throw new Exception("Invalid arguments to PhongMaterialFactory!");
			}

			Illumina::Core::IMaterial *CreateInstance(const std::string &p_strName, const Spectrum &p_reflectivity, float p_exponent)
			{
				return new PhongMaterial(p_strName, p_reflectivity, p_exponent);
			}
		};
	}
}