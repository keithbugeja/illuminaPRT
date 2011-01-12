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
#include "Material/Glass.h"

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
					return CreateInstance(strName);
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
					return CreateInstance(strName, reflectivity);
				}

				throw new Exception("Invalid arguments to MatteMaterialFactory!");
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
					return CreateInstance(strName, reflectivity);
				}

				throw new Exception("Invalid arguments to MirrorMaterialFactory!");
			}

			Illumina::Core::IMaterial *CreateInstance(const std::string &p_strName, const Spectrum &p_reflectivity)
			{
				return new MirrorMaterial(p_strName, p_reflectivity);
			}
		};

		class GlassMaterialFactory : public Illumina::Core::Factory<Illumina::Core::IMaterial>
		{
		public:
			Illumina::Core::IMaterial *CreateInstance(void)
			{
				return new GlassMaterial(Spectrum(0.5));
			}

			Illumina::Core::IMaterial *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strName;
				Spectrum reflectivity;
				float absorption,
					etaI, etaT;

				if (p_argumentMap.GetArgument("Name", strName) && 
					p_argumentMap.GetArgument("Reflectivity", reflectivity) &&
					p_argumentMap.GetArgument("Absorption", absorption) &&
					p_argumentMap.GetArgument("EtaI", etaI) &&
					p_argumentMap.GetArgument("EtaT", etaT))
				{
					return CreateInstance(strName, reflectivity, absorption, etaI, etaT);
				}

				throw new Exception("Invalid arguments to GlassMaterialFactory!");
			}

			Illumina::Core::IMaterial *CreateInstance(const std::string &p_strName, const Spectrum &p_reflectivity, float p_fAbsorption, float p_fEtaI, float p_fEtaT, ITexture* p_pTexture = NULL)
			{
				return new GlassMaterial(p_strName, p_reflectivity, p_fAbsorption, p_fEtaI, p_fEtaT, p_pTexture);
			}
		};
	}
}