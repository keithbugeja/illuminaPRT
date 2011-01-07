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
#include "Material/Matte.h"
#include "Material/Mirror.h"

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

		class MatteMaterialFactory : public Illumina::Core::Factory<Illumina::Core::IMaterial>
		{
		public:
			Illumina::Core::IMaterial *CreateInstance(void)
			{
				return new MatteMaterial(Spectrum(0.5));
			}

			Illumina::Core::IMaterial *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strName;
				Spectrum reflectivity;

				if (p_argumentMap.GetArgument("Name", strName) && 
					p_argumentMap.GetArgument("Reflectivity", reflectivity))
				{
					return new MatteMaterial(strName, reflectivity);
				}

				throw new Exception("Invalid arguments to DiffuseMaterialFactory!");
			}

			Illumina::Core::IMaterial *CreateInstance(const std::string &p_strName, const Spectrum &p_reflectivity)
			{
				return new MatteMaterial(p_strName, p_reflectivity);
			}
		};

		class MirrorMaterialFactory : public Illumina::Core::Factory<Illumina::Core::IMaterial>
		{
		public:
			Illumina::Core::IMaterial *CreateInstance(void)
			{
				return new MirrorMaterial(Spectrum(0.5));
			}

			Illumina::Core::IMaterial *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strName;
				Spectrum reflectivity;

				if (p_argumentMap.GetArgument("Name", strName) && 
					p_argumentMap.GetArgument("Reflectivity", reflectivity))
				{
					return new MirrorMaterial(strName, reflectivity);
				}

				throw new Exception("Invalid arguments to DiffuseMaterialFactory!");
			}

			Illumina::Core::IMaterial *CreateInstance(const std::string &p_strName, const Spectrum &p_reflectivity)
			{
				return new MirrorMaterial(p_strName, p_reflectivity);
			}
		};
	}
}