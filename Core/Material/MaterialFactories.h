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

#include "Material/Material.h"
#include "Material/MaterialGroup.h"
#include "Material/Matte.h"
#include "Material/Mirror.h"
#include "Material/Glass.h"

namespace Illumina
{
	namespace Core
	{	
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

				if (p_argumentMap.GetArgument("Id", strName) && 
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

				if (p_argumentMap.GetArgument("Id", strName) && 
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
				return new GlassMaterial(Spectrum(0.5), Spectrum(0.5));
			}

			Illumina::Core::IMaterial *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strName;
				float absorption;
				std::vector<Spectrum> reflectivity;
				Vector2 eta;

				if (p_argumentMap.GetArgument("Id", strName) && 
					p_argumentMap.GetArgument("Reflectivity", reflectivity) &&
					p_argumentMap.GetArgument("Absorption", absorption) &&
					p_argumentMap.GetArgument("Eta", eta))
				{
					return CreateInstance(strName, reflectivity[0], reflectivity[1], absorption, eta.U, eta.V);
				}

				throw new Exception("Invalid arguments to GlassMaterialFactory!");
			}

			Illumina::Core::IMaterial *CreateInstance(const std::string &p_strName, const Spectrum &p_reflectivity, const Spectrum &p_transmittance, 
				float p_fAbsorption, float p_fEtaI, float p_fEtaT, ITexture* p_pTexture = NULL)
			{
				return new GlassMaterial(p_strName, p_reflectivity, p_transmittance, p_fAbsorption, p_fEtaI, p_fEtaT, p_pTexture);
			}
		};
	}
}