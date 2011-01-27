//----------------------------------------------------------------------------------------------
//	Filename:	WavefrontSceneLoader.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <fstream>
#include <map>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "System/EngineKernel.h"
#include "Scene/WavefrontSceneLoader.h"
#include "Spectrum/Spectrum.h"
#include "Shape/VertexFormats.h"
#include "Shape/Shape.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
struct WavefrontVertex
{
	int Position,
		Texture,
		Normal;

	std::string GetVertexHash(void)
	{
		std::stringstream hash;
		hash << std::hex << Position << ':' << Texture << ':' << Normal << std::dec;
		return hash.str();
	}
};
//----------------------------------------------------------------------------------------------
struct WavefrontFace
{
	int MaterialIndex;
	WavefrontVertex Vertex[4];
};
//----------------------------------------------------------------------------------------------
struct WavefrontMaterial
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
struct WavefrontContext
{
	std::map<std::string, int> VertexMap;

	std::vector<Vector3> PositionList;
	std::vector<Vector3> NormalList;
	std::vector<Vector2> UVList;

	std::vector<WavefrontMaterial> MaterialList;

	ITriangleMesh *Mesh;
	MaterialGroup *MaterialGroup;

	std::string ObjectName;
	int CurrentMaterialId;

	WavefrontContext(void)
		: Mesh(NULL)
		, MaterialGroup(NULL)
		, CurrentMaterialId(-1)
	{ }
};
//----------------------------------------------------------------------------------------------
WavefrontSceneLoader::WavefrontSceneLoader(EngineKernel *p_pEngineKernel, Environment *p_pEnvironment)
	: ISceneLoader(p_pEngineKernel, p_pEnvironment)
{ }
//----------------------------------------------------------------------------------------------
bool WavefrontSceneLoader::Import(const std::string &p_strFilename, unsigned int p_uiGeneralFlags, ArgumentMap* p_pArgumentMap)
{
	// Provide wavefront scene context for loader
	WavefrontContext context;
	
	// Load geometry
	LoadGeometry(p_strFilename, context);

	std::string meshName = context.Mesh->GetName(),
		materialGroupName = context.MaterialGroup->GetName();

	if (p_pArgumentMap != NULL)
	{
		p_pArgumentMap->GetArgument("MeshName", meshName);
		p_pArgumentMap->GetArgument("MaterialGroupName", materialGroupName);
	}

	// Register assets
	m_pEngineKernel->GetShapeManager()->RegisterInstance(meshName, context.Mesh);
	m_pEngineKernel->GetMaterialManager()->RegisterInstance(materialGroupName, context.MaterialGroup);

	return true;
}
//----------------------------------------------------------------------------------------------
bool WavefrontSceneLoader::Export(const std::string &p_strFilename, unsigned int p_uiGeneralFlags, ArgumentMap* p_pArgumentMap)
{
	return true;
}
//----------------------------------------------------------------------------------------------
bool WavefrontSceneLoader::LoadMaterials(const std::string &p_strFilename, WavefrontContext &p_context)
{
	// Get material library filename as a path
	boost::filesystem::path materialPath(p_strFilename);

	// Open wavefront file
	std::ifstream materialFile;
	materialFile.open(p_strFilename.c_str());
	
	if (!materialFile.is_open())
	{
		std::cerr << "Error : Couldn't open file \'" << p_strFilename << "\'" << std::endl;
		exit(-1);
	}

	// define some temporary containers
	Vector2 vector2;
	Vector3 vector3;
	int value;

	std::string currentLine;
	std::vector<std::string> tokenList;

	WavefrontMaterial material;

	while(std::getline(materialFile, currentLine))
	{
		tokenList.clear();

		if (Tokenise(currentLine, " ", tokenList) == 0)
			continue;

		if (tokenList[0] == "newmtl") // New material
		{
			if (tokenList.size() != 2)
				continue;
			
			material.Name = tokenList[1];
			material.Type = WavefrontMaterial::Matte;

			p_context.MaterialList.push_back(material);
		}
		else if (tokenList[0] == "illum") // Illumination model
		{
			if (tokenList.size() != 4)
				continue;

			switch (boost::lexical_cast<int>(tokenList[1]))
			{
				case 4:
					p_context.MaterialList.back().Type = WavefrontMaterial::Glass;
					break;

				case 5:
					p_context.MaterialList.back().Type = WavefrontMaterial::Mirror;
					break;

				default:
					p_context.MaterialList.back().Type = WavefrontMaterial::Matte;
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
		else if (tokenList[0] == "ka") // Ambient values
		{
			if (tokenList.size() != 4)
				continue;

			p_context.MaterialList.back().Ambient.Set(
				boost::lexical_cast<float>(tokenList[1]),
				boost::lexical_cast<float>(tokenList[2]),
				boost::lexical_cast<float>(tokenList[3]));
		}
		else if (tokenList[0] == "kd") // Diffuse values
		{
			if (tokenList.size() != 4)
				continue;

			p_context.MaterialList.back().Diffuse.Set(
				boost::lexical_cast<float>(tokenList[1]),
				boost::lexical_cast<float>(tokenList[2]),
				boost::lexical_cast<float>(tokenList[3]));
		}
		else if (tokenList[0] == "ks") // Specular values
		{
			if (tokenList.size() != 4)
				continue;

			p_context.MaterialList.back().Specular.Set(
				boost::lexical_cast<float>(tokenList[1]),
				boost::lexical_cast<float>(tokenList[2]),
				boost::lexical_cast<float>(tokenList[3]));
		}
		else if (tokenList[0] == "ke") // Emissive values
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
	if (p_context.MaterialGroup == NULL)
		p_context.MaterialGroup = new MaterialGroup(materialPath.filename());

	int baseGroupId = p_context.MaterialGroup->Size();

	for (int groupId = 0; groupId < p_context.MaterialList.size(); ++groupId)
	{
		IMaterial *pMaterial = NULL;
		ITexture *pTexture = NULL;

		const WavefrontMaterial& material = p_context.MaterialList.at(groupId); 

		std::stringstream argumentStream;
		argumentStream << "Name=" << material.Name << ";"
			<< "Reflectivity=" << material.Diffuse[0] << "," << material.Diffuse[1] << "," << material.Diffuse[2] << ";"
			<< "Reflectivity=" << material.Diffuse[0] << "," << material.Diffuse[1] << "," << material.Diffuse[2] << ";"
			<< "Shininess=" << material.Shininess << "Absorption=" << 1.0f << ";EtaI=" << 1.0f << ";EtaT=" << material.RefractiveIndex << ";";

		if (!material.DiffuseMap.empty())
		{
			if (!m_pEngineKernel->GetTextureManager()->QueryInstance(material.DiffuseMap))
			{
				std::stringstream textureArgumentStream;
				textureArgumentStream << "Name=" << material.DiffuseMap << ";" 
					<< "Filename=" << (materialPath.parent_path() / material.DiffuseMap).string() << ";"
					<< "Filetype=PPM;";

				pTexture = m_pEngineKernel->GetTextureManager()->CreateInstance("Image", 
					material.DiffuseMap, textureArgumentStream.str());
			}
			else
			{
				pTexture = m_pEngineKernel->GetTextureManager()->RequestInstance(material.DiffuseMap);
			}
		}

		switch(material.Type)
		{
			case WavefrontMaterial::Matte:
			{
				pMaterial = m_pEngineKernel->GetMaterialManager()->CreateInstance("Matte", material.Name, argumentStream.str());
				((MatteMaterial*)pMaterial)->SetTexture(pTexture);
			}

			case WavefrontMaterial::Mirror:
			{
				pMaterial = m_pEngineKernel->GetMaterialManager()->CreateInstance("Mirror", material.Name, argumentStream.str());
				((MirrorMaterial*)pMaterial)->SetTexture(pTexture);
			}

			case WavefrontMaterial::Glass:
			{
				pMaterial = m_pEngineKernel->GetMaterialManager()->CreateInstance("Glass", material.Name, argumentStream.str());
				((GlassMaterial*)pMaterial)->SetTexture(pTexture);
			}
		}

		p_context.MaterialGroup->Add(pMaterial, baseGroupId + groupId);
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool WavefrontSceneLoader::LoadGeometry(const std::string &p_strFilename, WavefrontContext &p_context)
{
	// Use filename as the default object name
	boost::filesystem::path geometryPath(p_strFilename);
	p_context.ObjectName = geometryPath.filename();
	p_context.Mesh = new KDTreeMesh(p_context.ObjectName);

	// Open wavefront file
	std::ifstream wavefrontFile;
	wavefrontFile.open(p_strFilename.c_str());
	
	if (!wavefrontFile.is_open())
	{
		std::cerr << "Error : Couldn't open file \'" << p_strFilename << "\'" << std::endl;
		exit(-1);
	}

	// define some temporary containers
	Vector2 vector2;
	Vector3 vector3;

	std::string currentLine;

	std::vector<std::string> tokenList,
		faceTokenList;

	while(std::getline(wavefrontFile, currentLine))
	{
		tokenList.clear();

		if (Tokenise(currentLine, " ", tokenList) == 0)
			continue;

		if (tokenList[0] == "o") // Object - set geometry to friendly object name
		{
			if (tokenList.size() != 2)
				continue;

			p_context.ObjectName = tokenList[1];
		}
		else if (tokenList[0] == "v") // Position
		{
			if (tokenList.size() != 4)
				continue;

			vector3.Set(boost::lexical_cast<float>(tokenList[1]),
						boost::lexical_cast<float>(tokenList[2]),
						boost::lexical_cast<float>(tokenList[3]));

				p_context.PositionList.push_back(vector3);
		}
		else if (tokenList[0] == "vn") // Normal
		{
			if (tokenList.size() != 4)
				continue;

			vector3.Set(boost::lexical_cast<float>(tokenList[1]),
						boost::lexical_cast<float>(tokenList[2]),
						boost::lexical_cast<float>(tokenList[3]));

			p_context.NormalList.push_back(vector3);
		}
		else if (tokenList[0] == "vt") // Texture coordinates
		{
			if (tokenList.size() != 3)
				continue;

			vector2.Set(boost::lexical_cast<float>(tokenList[1]),
						boost::lexical_cast<float>(tokenList[2]));

			p_context.UVList.push_back(vector2);
		}
		else if (tokenList[0] == "f") // Face
		{
			// Ignore if there aren't enough vertices to form a surface.
			// We are interested only in tri/quad faces, so ignore higher
			// order polygons too.
			if (tokenList.size() < 4 || tokenList.size() > 5)
				continue;

			WavefrontVertex vertex;
			int vertexIndex[4];

			for (size_t index = 1; index < tokenList.size(); index++)
			{
				Tokenise(tokenList[1], "/", faceTokenList);

				vertex.Position = boost::lexical_cast<int>(faceTokenList[0]);
				vertex.Texture = boost::lexical_cast<int>(faceTokenList[1]);
				vertex.Normal = boost::lexical_cast<int>(faceTokenList[2]);

				// Search for vertex in map
				std::string hash = vertex.GetVertexHash();
				if (p_context.VertexMap.find(hash) == p_context.VertexMap.end())
				{
					Vertex meshVertex;

					meshVertex.Position = p_context.PositionList[vertex.Position - 1];
					meshVertex.UV = (vertex.Texture == 0) ? Vector2::Zero : p_context.UVList[vertex.Texture - 1];
					meshVertex.Normal = (vertex.Normal == 0) ? Vector3::Zero : p_context.NormalList[vertex.Normal - 1];
					
					p_context.VertexMap[hash] = vertexIndex[index] = 
						p_context.Mesh->VertexList.Size();

					// Add vertex to mesh
					p_context.Mesh->AddVertex(meshVertex);
				}
				else
				{
					vertexIndex[index] = p_context.VertexMap[hash];
				}
			}

			// Add faces to mesh
			p_context.Mesh->AddIndexedTriangle(vertexIndex[0], vertexIndex[1], vertexIndex[2], p_context.CurrentMaterialId);
			
			if (tokenList.size() == 5) 
				p_context.Mesh->AddIndexedTriangle(vertexIndex[0], vertexIndex[2], vertexIndex[3], p_context.CurrentMaterialId);
		}
		else if (tokenList[0] == "mtlib")
		{
			if (tokenList.size() != 2)
				continue;

			std::string materialLibraryFilename;
			materialLibraryFilename = (geometryPath.parent_path() / tokenList[1]).string();
			LoadMaterials(materialLibraryFilename, p_context);
		}
		else if (tokenList[0] == "usemtl")
		{
			if (tokenList.size() != 2)
				continue;

			p_context.CurrentMaterialId = p_context.MaterialGroup->GetGroupId(tokenList[1]);
		}
	}

	wavefrontFile.close();

	return true;
}
//----------------------------------------------------------------------------------------------