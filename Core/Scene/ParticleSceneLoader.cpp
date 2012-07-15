//----------------------------------------------------------------------------------------------
//	Filename:	ParticleSceneLoader.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//  Notes:	The Particle Scene loader is still partly based on the WavefrontSceneLoader... the 
//			material file is identical to .obj support material files (.mtl). Some day I will 
//			have to encode material files too in some binary format and rewrite this loader.
//----------------------------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>

#include "Scene/Environment.h"
#include "Scene/ParticleSceneLoader.h"
#include "Shape/PersistentMesh.h"

#include "Material/MaterialGroup.h"
#include "Material/Mirror.h"
#include "Material/Matte.h"
#include "Material/Glass.h"

#include "Spectrum/Spectrum.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
struct ParticleMaterial
{
	enum MaterialType
	{
		Matte,
		Mirror,
		Glass
	} Type;

	std::string Name;

	float Shininess;
	float RefractiveIndex;

	Spectrum Ambient;
	Spectrum Diffuse;
	Spectrum Specular;
	Spectrum Emissive;

	std::string AmbientMap;
	std::string DiffuseMap;
	std::string SpecularMap;
	std::string BumpMap;
};
//----------------------------------------------------------------------------------------------
struct ParticleContext
{
	std::vector<ParticleMaterial> MaterialList;

	IShape *Mesh;
	MaterialGroup *Materials;

	std::string ObjectName;
	int CurrentMaterialId;

	ParticleContext(void)
		: Mesh(NULL)
		, Materials(NULL)
		, CurrentMaterialId(-1)
	{ }
};
//----------------------------------------------------------------------------------------------
ParticleSceneLoader::ParticleSceneLoader(Environment *p_pEnvironment)
	: ISceneLoader(p_pEnvironment)
{ 
	BOOST_ASSERT(p_pEnvironment != NULL);
	m_pEngineKernel = p_pEnvironment->GetEngineKernel();
}
//----------------------------------------------------------------------------------------------
bool ParticleSceneLoader::Import(const std::string &p_strFilename, unsigned int p_uiGeneralFlags, ArgumentMap* p_pArgumentMap)
{
	// Provide wavefront scene context for loader
	ParticleContext context;
	
	// Load geometry
	if (!LoadGeometry(p_strFilename, context))
		return false;

	if (context.Mesh != NULL)
	{
		std::string meshName = context.Mesh->GetName();
		
		if (p_pArgumentMap != NULL) 
			p_pArgumentMap->GetArgument("Id", meshName);
		
		m_pEngineKernel->GetShapeManager()->RegisterInstance(meshName, context.Mesh);
	}

	if (context.Materials != NULL)
	{
		std::string materialGroupName = context.Materials->GetName();
		
		if (p_pArgumentMap != NULL) 
			p_pArgumentMap->GetArgument("MaterialGroupId", materialGroupName);
		
		m_pEngineKernel->GetMaterialManager()->RegisterInstance(materialGroupName, context.Materials);
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool ParticleSceneLoader::Export(const std::string &p_strFilename, unsigned int p_uiGeneralFlags, ArgumentMap* p_pArgumentMap)
{
	return true;
}
//----------------------------------------------------------------------------------------------
bool ParticleSceneLoader::LoadMaterials(const std::string &p_strFilename, ParticleContext &p_context)
{
	// Get material library filename as a path
	boost::filesystem::path materialPath(p_strFilename);

	// Open wavefront file
	std::ifstream materialFile;
	materialFile.open(p_strFilename.c_str());
	
	if (!materialFile.is_open())
	{
		std::cerr << "Error : Couldn't open file '" << p_strFilename << "'" << std::endl;
		exit(-1);
	}

	// define some temporary containers
	Vector2 vector2;
	Vector3 vector3;
	int value;

	std::string currentLine;
	std::vector<std::string> tokenList;

	ParticleMaterial material;

	while(std::getline(materialFile, currentLine))
	{
		tokenList.clear();

		// Tokenise line
		if (Tokenise(currentLine, " \n\r", tokenList) == 0)
			continue;

		// Trim whitespace at edges
		for (size_t tokenIndex = 0; tokenIndex < tokenList.size(); ++tokenIndex)
			boost::trim(tokenList[tokenIndex]);

		if (tokenList[0] == "newmtl") // New material
		{
			if (tokenList.size() != 2)
				continue;
			
			material.Name = tokenList[1];
			material.Type = ParticleMaterial::Matte;

			p_context.MaterialList.push_back(material);
		}
		else if (tokenList[0] == "illum") // Illumination model
		{
			if (tokenList.size() != 2)
				continue;

			switch (boost::lexical_cast<int>(tokenList[1]))
			{
				case 4:
					p_context.MaterialList.back().Type = ParticleMaterial::Glass;
					break;

				case 5:
					p_context.MaterialList.back().Type = ParticleMaterial::Mirror;
					break;

				default:
					p_context.MaterialList.back().Type = ParticleMaterial::Matte;
			}
		}
		else if (tokenList[0] == "map_Ka") // Ambient map
		{
			if (tokenList.size() != 2)
				continue;

			p_context.MaterialList.back().AmbientMap = tokenList[1];
		}
		else if (tokenList[0] == "map_Kd") // Normal map
		{
			if (tokenList.size() != 2)
				continue;

			p_context.MaterialList.back().DiffuseMap = tokenList[1];
		}
		else if (tokenList[0] == "map_Ks") // Specular map
		{
			if (tokenList.size() != 2)
				continue;

			p_context.MaterialList.back().SpecularMap = tokenList[1];
		}
		else if (tokenList[0] == "Ka") // Ambient values
		{
			if (tokenList.size() != 4)
				continue;

			p_context.MaterialList.back().Ambient.Set(
				boost::lexical_cast<float>(tokenList[1]),
				boost::lexical_cast<float>(tokenList[2]),
				boost::lexical_cast<float>(tokenList[3]));
		}
		else if (tokenList[0] == "Kd") // Diffuse values
		{
			if (tokenList.size() != 4)
				continue;

			p_context.MaterialList.back().Diffuse.Set(
				boost::lexical_cast<float>(tokenList[1]),
				boost::lexical_cast<float>(tokenList[2]),
				boost::lexical_cast<float>(tokenList[3]));
		}
		else if (tokenList[0] == "Ks") // Specular values
		{
			if (tokenList.size() != 4)
				continue;

			p_context.MaterialList.back().Specular.Set(
				boost::lexical_cast<float>(tokenList[1]),
				boost::lexical_cast<float>(tokenList[2]),
				boost::lexical_cast<float>(tokenList[3]));
		}
		else if (tokenList[0] == "Ke") // Emissive values
		{
			if (tokenList.size() != 4)
				continue;

			p_context.MaterialList.back().Emissive.Set(
				boost::lexical_cast<float>(tokenList[1]),
				boost::lexical_cast<float>(tokenList[2]),
				boost::lexical_cast<float>(tokenList[3]));
		}
		else if (tokenList[0] == "Ns") // Emissive values
		{
			if (tokenList.size() != 2)
				continue;

			p_context.MaterialList.back().Shininess =
				boost::lexical_cast<float>(tokenList[1]);
		}
		else if (tokenList[0] == "Ni") // Emissive values
		{
			if (tokenList.size() != 2)
				continue;

			p_context.MaterialList.back().RefractiveIndex =
				boost::lexical_cast<float>(tokenList[1]);
		}
	}

	materialFile.close();

	// Add Wavefront materials to context material group
	if (p_context.Materials == NULL)
		p_context.Materials = new MaterialGroup(materialPath.filename().string());

	int baseGroupId = p_context.Materials->Size();

	for (int groupId = 0; groupId < p_context.MaterialList.size(); ++groupId)
	{
		IMaterial *pMaterial = NULL;
		ITexture *pTexture = NULL;

		const ParticleMaterial& material = p_context.MaterialList.at(groupId); 

		std::stringstream argumentStream;
		argumentStream << "Id=" << material.Name << ";Shininess=" << material.Shininess << ";Absorption=" << 1.0f 
			<< ";Eta={" << 1.0f << "," << material.RefractiveIndex << "};";

		if (!material.DiffuseMap.empty())
		{
			if (!m_pEngineKernel->GetTextureManager()->QueryInstance(material.DiffuseMap))
			{
				// Set texture factory generic arguments
				std::stringstream textureArgumentStream;

				textureArgumentStream << "Id=" << material.DiffuseMap << ";" 
					<< "Filename=" << (materialPath.parent_path() / material.DiffuseMap).string() << ";";

				// Query extension of image file
				std::string extension = boost::to_upper_copy(material.DiffuseMap.substr((material.DiffuseMap.find_last_of(".") + 1)));

				// Are we loading a memory-mapped texture?
				if (extension.find("MMF") != std::string::npos)
				{
					pTexture = m_pEngineKernel->GetTextureManager()->CreateInstance("MappedFile", 
						material.DiffuseMap, textureArgumentStream.str());
				}
				else
				{
					textureArgumentStream << "Filetype=" << extension << ";";

					std::cout << "Filetype = " << extension << std::endl;

					pTexture = m_pEngineKernel->GetTextureManager()->CreateInstance("Image", 
						material.DiffuseMap, textureArgumentStream.str());
				}				 
			}
			else
			{
				pTexture = m_pEngineKernel->GetTextureManager()->RequestInstance(material.DiffuseMap);
			}
		}

		switch(material.Type)
		{
			case ParticleMaterial::Matte:
			{
				argumentStream << "Reflectivity={" << material.Diffuse[0] << "," << material.Diffuse[1] << "," << material.Diffuse[2] << "};";
				pMaterial = m_pEngineKernel->GetMaterialManager()->CreateInstance("Matte", material.Name, argumentStream.str());
				((MatteMaterial*)pMaterial)->SetTexture(pTexture);
				break;
			}

			case ParticleMaterial::Mirror:
			{
				argumentStream << "Reflectivity={" << material.Diffuse[0] << "," << material.Diffuse[1] << "," << material.Diffuse[2] << "};";
				pMaterial = m_pEngineKernel->GetMaterialManager()->CreateInstance("Mirror", material.Name, argumentStream.str());
				((MirrorMaterial*)pMaterial)->SetTexture(pTexture);
				break;
			}

			case ParticleMaterial::Glass:
			{
				argumentStream << "Reflectivity={{" << material.Diffuse[0] << "," << material.Diffuse[1] << "," << material.Diffuse[2] << "},"
					<< "{" << material.Diffuse[0] << "," << material.Diffuse[1] << "," << material.Diffuse[2] << "}};";
				pMaterial = m_pEngineKernel->GetMaterialManager()->CreateInstance("Glass", material.Name, argumentStream.str());
				((GlassMaterial*)pMaterial)->SetTexture(pTexture);
				break;
			}
		}

		p_context.Materials->Add(pMaterial, baseGroupId + groupId);
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool ParticleSceneLoader::LoadGeometry(const std::string &p_strFilename, ParticleContext &p_context)
{
	// Use filename as the default object name
	boost::filesystem::path geometryPath(p_strFilename);
	p_context.ObjectName = geometryPath.filename().string();

	// Create Persistent Mesh
	p_context.Mesh = new PersistentMesh(p_context.ObjectName, p_strFilename);

	// Load materials
	std::string materialLibraryFilename = p_strFilename + ".mtl";
	LoadMaterials(materialLibraryFilename, p_context);
	
	return true;
}
//----------------------------------------------------------------------------------------------
int ParticleSceneLoader::Tokenise(std::string &p_strText, char *p_pSeparators, std::vector<std::string> &p_tokenList)
{
	boost::char_separator<char> separator(p_pSeparators);
	boost::tokenizer<boost::char_separator<char> > tokens(p_strText, separator);

	p_tokenList.clear();

	for (boost::tokenizer<boost::char_separator<char> >::iterator iterator = tokens.begin(); iterator != tokens.end(); ++iterator)
	{
		std::string token = *iterator;
		p_tokenList.push_back(token);
	}

	return p_tokenList.size();
}
//----------------------------------------------------------------------------------------------
