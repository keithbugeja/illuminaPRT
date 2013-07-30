//----------------------------------------------------------------------------------------------
//	Filename:	PolygonSceneLoader.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
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

#include "Scene/PolygonSceneLoader.h"

#include "Scene/Environment.h"
#include "Shape/VertexFormats.h"
#include "Shape/TriangleMesh.h"
#include "Shape/KDTreeMesh.h"
#include "Shape/KDTreeMeshEx.h"
#include "Shape/BVHMesh.h"
#include "Shape/PersistentMesh.h"
#include "Shape/BasicMesh.h"

#include "Material/MaterialGroup.h"
#include "Material/Mirror.h"
#include "Material/Matte.h"
#include "Material/Glass.h"

#include "Spectrum/Spectrum.h"

using namespace Illumina::Core;

/*
	name        type        number of bytes
	---------------------------------------
	char       character                 1
	uchar      unsigned character        1
	short      short integer             2
	ushort     unsigned short integer    2
	int        integer                   4
	uint       unsigned integer          4
	float      single-precision float    4
	double     double-precision float    8
*/

/*
	comment TexutureFile xxx

	vertex property names
	x, y, z, nx, ny, nz, red, green, blue, alpha

	face property names
	vertex_indices, texcoord, red, green, blue, alpha
 */
//----------------------------------------------------------------------------------------------
/*
struct WavefrontVertex
{
private:
	struct Hash 
	{
		Int64 Position : 21;
		Int64 Texture : 21;
		Int64 Normal : 21;
	};

public:
	int Position,
		Texture,
		Normal;	

	Int64 GetVertexHash(void)
	{
		Hash hash;

		hash.Position = Position;
		hash.Normal = Normal;
		hash.Texture = Texture;

		return *(Int64*)&hash;
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
	std::map<Int64, int> VertexMap;

	std::vector<Vector3> PositionList;
	std::vector<Vector3> NormalList;
	std::vector<Vector2> UVList;

	std::vector<WavefrontMaterial> MaterialList;

	ITriangleMesh *Mesh;
	MaterialGroup *Materials;

	std::string ObjectName;
	int CurrentMaterialId;

	WavefrontContext(void)
		: Mesh(NULL)
		, Materials(NULL)
		, CurrentMaterialId(-1)
	{ }
};
*/
//----------------------------------------------------------------------------------------------
struct PolygonContext
{
	std::string ObjectName;

	ITriangleMesh *Mesh;
	MaterialGroup *Materials;

	std::vector<Vector3> PositionList;
	std::vector<Vector3> NormalList;
};
//----------------------------------------------------------------------------------------------
PolygonSceneLoader::PolygonSceneLoader(Environment *p_pEnvironment)
	: ISceneLoader(p_pEnvironment)
{ 
	BOOST_ASSERT(p_pEnvironment != NULL);
	m_pEngineKernel = p_pEnvironment->GetEngineKernel();
}
//----------------------------------------------------------------------------------------------
bool PolygonSceneLoader::LoadMaterial(const std::string &p_strFilename, PolygonContext &p_context)
{
	ITexture *pTexture; IMaterial *pMaterial;
	std::stringstream textureArgumentStream;

	boost::filesystem::path materialPath(p_strFilename);
	std::string materialName = materialPath.filename().string();

	textureArgumentStream << "Id=" << materialName << ";" 
						<< "Filename=" << p_strFilename << ";";

	// Query extension of image file
	std::string extension = boost::to_upper_copy(materialName.substr((materialName.find_last_of(".") + 1)));

	// Are we loading a memory-mapped texture?
	if (extension.find("MMF") != std::string::npos)
	{
		pTexture = m_pEngineKernel->GetTextureManager()->CreateInstance("MappedFile", 
			materialName, textureArgumentStream.str());
	}
	else
	{
		textureArgumentStream << "Filetype=" << extension << ";";

		pTexture = m_pEngineKernel->GetTextureManager()->CreateInstance("Image", 
			materialName, textureArgumentStream.str());
	}

	std::stringstream argumentStream;

	argumentStream << "Id=" << materialName << ";Reflectivity={1,1,1};";
	pMaterial = m_pEngineKernel->GetMaterialManager()->CreateInstance("Matte", materialName, argumentStream.str());
	((MatteMaterial*)pMaterial)->SetTexture(pTexture);

	p_context.Materials->Add(pMaterial, p_context.Materials->Size());

	return true;
}
//----------------------------------------------------------------------------------------------
bool PolygonSceneLoader::Load(const std::string &p_strFilename, PolygonContext &p_context)
{
	// Use filename as the default object name
	boost::filesystem::path geometryPath(p_strFilename);
	p_context.ObjectName = geometryPath.filename().string();

	p_context.Mesh = 
		new KDTreeMeshEx(p_context.ObjectName);
		//new PersistentMesh(context.ObjectName, "Z:\\Object");
		//new KDTreeMesh(context.ObjectName);
		//new BVHMesh(context.ObjectName);
		//new BasicMesh(context.ObjectName);

	p_context.Materials = 
		new MaterialGroup(p_context.ObjectName);

	// Open wavefront file
	std::ifstream plyFile;
	plyFile.open(p_strFilename.c_str());
	
	if (!plyFile.is_open())
	{
		std::cerr << "Error : Couldn't open file '" << p_strFilename << "'" << std::endl;
		exit(-1);
	}

	std::string currentLine;

	std::vector<std::string> tokenList,
		faceTokenList;

	bool magic, header,
		binary, littleEndian;

	header = !(magic = 
		binary = 
		littleEndian = false);

	int vertexcount,
		facecount;

	// Parse header
	while(std::getline(plyFile, currentLine))
	{
		tokenList.clear();

		// Tokenise line
		if (Tokenise(currentLine, " \n\r", tokenList) == 0)
			continue;

		// Trim whitespace at edges
		for (size_t tokenIndex = 0; tokenIndex < tokenList.size(); ++tokenIndex)
			boost::trim(tokenList[tokenIndex]);

		if (header)
		{
			// Magic number
			if (tokenList[0] == "ply")
			{
				magic = true;
				std::cout << "PLY :: magic number" << std::endl;
			} 
			else if (tokenList[0] == "format")
			{
				if (tokenList.size() != 3)
					continue;

				if (tokenList[1] == "binary_little_endian")
				{
					binary = littleEndian = true;
					std::cout << "PLY :: body format :: binary | little endian" << std::endl;
				} 
				else
				{
					binary = littleEndian = false;
					std::cout << "PLY :: body format :: ascii" << std::endl;
				}
			} 
			else if (tokenList[0] == "comment")
			{
				if (tokenList.size() != 3)
					continue;

				if (tokenList[1] == "TextureFile")
				{
					std::cout << "PLY :: texture :: " << tokenList[2] << std::endl;
					LoadMaterial((geometryPath.parent_path() / tokenList[2]).string(), p_context);
				}
				else
				{
					std::cout << "PLY :: comment :: " << tokenList[2] << std::endl; 
				}
			} 
			else if (tokenList[0] == "element")
			{
				if (tokenList.size() != 3)
					continue;
			
				if (tokenList[1] == "vertex")
				{
					vertexcount = boost::lexical_cast<int>(tokenList[2]);
					std::cout << "PLY :: vertex element, count = " << vertexcount << std::endl;
				}
				else if (tokenList[1] == "face")
				{
					facecount = boost::lexical_cast<int>(tokenList[2]);
					std::cout << "PLY :: face element, count = " << facecount << std::endl;
				}
			} 
			else if (tokenList[0] == "property")
			{
				if (tokenList.size() < 2)
					continue;

				// We expect to find a set number of properties, 
				// so they should be validated
				
				// [vertices]
				// x y z nx ny nz |-> float

				// [faces]
				// vertex_indices |-> list uchar int
				// texture_coords |-> list uchar float
				// texture_number |-> int

				std::cout << "PLY :: property :: " << tokenList[1] << " :: " << tokenList[2] << std::endl;
			} 
			else if (tokenList[0] == "end_header")
			{
				header = false;
				std::cout << "PLY :: header complete" << std::endl;	
			}
		}
		else
		{
			if (vertexcount > 0)
			{
				// read x, y, z, nx, ny, nz
			}
			else if (facecount > 0)
			{
				// read count, f0, f1, f2, count, t0, t1, t2, t3, t4, t5, t6, texture_idx
			}
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool PolygonSceneLoader::Import(const std::string &p_strFilename, unsigned int p_uiGeneralFlags, ArgumentMap* p_pArgumentMap)
{
	PolygonContext context;

	if (Load(p_strFilename, context))
	{
	}

	//// Provide wavefront scene context for loader
	//WavefrontContext context;
	//
	//// Load geometry
	//if (!LoadGeometry(p_strFilename, context))
	//	return false;

	//if (context.Mesh != NULL)
	//{
	//	std::string meshName = context.Mesh->GetName();
	//	
	//	if (p_pArgumentMap != NULL) 
	//		p_pArgumentMap->GetArgument("Id", meshName);
	//	
	//	//IShape* pShape = new PersistentMesh(meshName, "Z:\\Object");
	//	//m_pEngineKernel->GetShapeManager()->RegisterInstance(meshName, pShape);
	//	m_pEngineKernel->GetShapeManager()->RegisterInstance(meshName, context.Mesh);

	//	//context.Mesh->UpdateNormals();
	//}

	//if (context.Materials != NULL)
	//{
	//	std::string materialGroupName = context.Materials->GetName();
	//	
	//	if (p_pArgumentMap != NULL) 
	//		p_pArgumentMap->GetArgument("MaterialGroupId", materialGroupName);
	//	
	//	m_pEngineKernel->GetMaterialManager()->RegisterInstance(materialGroupName, context.Materials);
	//}
	return true;
}
//----------------------------------------------------------------------------------------------
bool PolygonSceneLoader::Export(const std::string &p_strFilename, unsigned int p_uiGeneralFlags, ArgumentMap* p_pArgumentMap)
{
	return true;
}
////----------------------------------------------------------------------------------------------
//bool WavefrontSceneLoader::LoadMaterials(const std::string &p_strFilename, WavefrontContext &p_context)
//{
//	// Get material library filename as a path
//	boost::filesystem::path materialPath(p_strFilename);
//
//	// Open wavefront file
//	std::ifstream materialFile;
//	materialFile.open(p_strFilename.c_str());
//	
//	if (!materialFile.is_open())
//	{
//		std::cerr << "Error : Couldn't open file '" << p_strFilename << "'" << std::endl;
//		exit(-1);
//	}
//
//	// define some temporary containers
//	Vector2 vector2;
//	Vector3 vector3;
//	// int value;
//
//	std::string currentLine;
//	std::vector<std::string> tokenList;
//
//	WavefrontMaterial material;
//
//	while(std::getline(materialFile, currentLine))
//	{
//		tokenList.clear();
//
//		// Tokenise line
//		if (Tokenise(currentLine, " \n\r", tokenList) == 0)
//			continue;
//
//		// Trim whitespace at edges
//		for (size_t tokenIndex = 0; tokenIndex < tokenList.size(); ++tokenIndex)
//			boost::trim(tokenList[tokenIndex]);
//
//		if (tokenList[0] == "newmtl") // New material
//		{
//			if (tokenList.size() != 2)
//				continue;
//			
//			material.Name = tokenList[1];
//			material.Type = WavefrontMaterial::Matte;
//
//			p_context.MaterialList.push_back(material);
//		}
//		else if (tokenList[0] == "illum") // Illumination model
//		{
//			if (tokenList.size() != 2)
//				continue;
//
//			switch (boost::lexical_cast<int>(tokenList[1]))
//			{
//				case 4:
//					p_context.MaterialList.back().Type = WavefrontMaterial::Glass;
//					break;
//
//				case 5:
//					p_context.MaterialList.back().Type = WavefrontMaterial::Mirror;
//					break;
//
//				default:
//					p_context.MaterialList.back().Type = WavefrontMaterial::Matte;
//			}
//		}
//		else if (tokenList[0] == "map_Ka") // Ambient map
//		{
//			if (tokenList.size() != 2)
//				continue;
//
//			p_context.MaterialList.back().AmbientMap = tokenList[1];
//		}
//		else if (tokenList[0] == "map_Kd") // Normal map
//		{
//			if (tokenList.size() != 2)
//				continue;
//
//			p_context.MaterialList.back().DiffuseMap = tokenList[1];
//		}
//		else if (tokenList[0] == "map_Ks") // Specular map
//		{
//			if (tokenList.size() != 2)
//				continue;
//
//			p_context.MaterialList.back().SpecularMap = tokenList[1];
//		}
//		else if (tokenList[0] == "Ka") // Ambient values
//		{
//			if (tokenList.size() != 4)
//				continue;
//
//			p_context.MaterialList.back().Ambient.Set(
//				boost::lexical_cast<float>(tokenList[1]),
//				boost::lexical_cast<float>(tokenList[2]),
//				boost::lexical_cast<float>(tokenList[3]));
//		}
//		else if (tokenList[0] == "Kd") // Diffuse values
//		{
//			if (tokenList.size() != 4)
//				continue;
//
//			p_context.MaterialList.back().Diffuse.Set(
//				boost::lexical_cast<float>(tokenList[1]),
//				boost::lexical_cast<float>(tokenList[2]),
//				boost::lexical_cast<float>(tokenList[3]));
//		}
//		else if (tokenList[0] == "Ks") // Specular values
//		{
//			if (tokenList.size() != 4)
//				continue;
//
//			p_context.MaterialList.back().Specular.Set(
//				boost::lexical_cast<float>(tokenList[1]),
//				boost::lexical_cast<float>(tokenList[2]),
//				boost::lexical_cast<float>(tokenList[3]));
//		}
//		else if (tokenList[0] == "Ke") // Emissive values
//		{
//			if (tokenList.size() != 4)
//				continue;
//
//			p_context.MaterialList.back().Emissive.Set(
//				boost::lexical_cast<float>(tokenList[1]),
//				boost::lexical_cast<float>(tokenList[2]),
//				boost::lexical_cast<float>(tokenList[3]));
//		}
//		else if (tokenList[0] == "Ns") // Emissive values
//		{
//			if (tokenList.size() != 2)
//				continue;
//
//			p_context.MaterialList.back().Shininess =
//				boost::lexical_cast<float>(tokenList[1]);
//		}
//		else if (tokenList[0] == "Ni") // Emissive values
//		{
//			if (tokenList.size() != 2)
//				continue;
//
//			p_context.MaterialList.back().RefractiveIndex =
//				boost::lexical_cast<float>(tokenList[1]);
//		}
//	}
//
//	materialFile.close();
//
//	// Add Wavefront materials to context material group
//	if (p_context.Materials == NULL)
//		p_context.Materials = new MaterialGroup(materialPath.filename().string());
//
//	int baseGroupId = p_context.Materials->Size();
//
//	for (int groupId = 0; groupId < p_context.MaterialList.size(); ++groupId)
//	{
//		IMaterial *pMaterial = NULL;
//		ITexture *pTexture = NULL;
//
//		const WavefrontMaterial& material = p_context.MaterialList.at(groupId); 
//
//		std::stringstream argumentStream;
//		argumentStream << "Id=" << material.Name << ";Shininess=" << material.Shininess << ";Absorption=" << 1.0f 
//			<< ";Eta={" << 1.0f << "," << material.RefractiveIndex << "};";
//
//		if (!material.DiffuseMap.empty())
//		{
//			if (!m_pEngineKernel->GetTextureManager()->QueryInstance(material.DiffuseMap))
//			{
//				// Set texture factory generic arguments
//				std::stringstream textureArgumentStream;
//
//				textureArgumentStream << "Id=" << material.DiffuseMap << ";" 
//					<< "Filename=" << (materialPath.parent_path() / material.DiffuseMap).string() << ";";
//
//				// Query extension of image file
//				std::string extension = boost::to_upper_copy(material.DiffuseMap.substr((material.DiffuseMap.find_last_of(".") + 1)));
//
//				// Are we loading a memory-mapped texture?
//				if (extension.find("MMF") != std::string::npos)
//				{
//					pTexture = m_pEngineKernel->GetTextureManager()->CreateInstance("MappedFile", 
//						material.DiffuseMap, textureArgumentStream.str());
//				}
//				else
//				{
//					textureArgumentStream << "Filetype=" << extension << ";";
//
//					std::cout << "Filetype = " << extension << std::endl;
//
//					pTexture = m_pEngineKernel->GetTextureManager()->CreateInstance("Image", 
//						material.DiffuseMap, textureArgumentStream.str());
//				}				 
//			}
//			else
//			{
//				pTexture = m_pEngineKernel->GetTextureManager()->RequestInstance(material.DiffuseMap);
//			}
//		}
//
//		switch(material.Type)
//		{
//			case WavefrontMaterial::Matte:
//			{
//				argumentStream << "Reflectivity={" << material.Diffuse[0] << "," << material.Diffuse[1] << "," << material.Diffuse[2] << "};";
//				pMaterial = m_pEngineKernel->GetMaterialManager()->CreateInstance("Matte", material.Name, argumentStream.str());
//				((MatteMaterial*)pMaterial)->SetTexture(pTexture);
//				break;
//			}
//
//			case WavefrontMaterial::Mirror:
//			{
//				argumentStream << "Reflectivity={" << material.Diffuse[0] << "," << material.Diffuse[1] << "," << material.Diffuse[2] << "};";
//				pMaterial = m_pEngineKernel->GetMaterialManager()->CreateInstance("Mirror", material.Name, argumentStream.str());
//				((MirrorMaterial*)pMaterial)->SetTexture(pTexture);
//				break;
//			}
//
//			case WavefrontMaterial::Glass:
//			{
//				argumentStream << "Reflectivity={{" << material.Diffuse[0] << "," << material.Diffuse[1] << "," << material.Diffuse[2] << "},"
//					<< "{" << material.Diffuse[0] << "," << material.Diffuse[1] << "," << material.Diffuse[2] << "}};";
//				pMaterial = m_pEngineKernel->GetMaterialManager()->CreateInstance("Glass", material.Name, argumentStream.str());
//				((GlassMaterial*)pMaterial)->SetTexture(pTexture);
//				break;
//			}
//		}
//
//		p_context.Materials->Add(pMaterial, baseGroupId + groupId);
//	}
//
//	return true;
//}
////----------------------------------------------------------------------------------------------
//bool WavefrontSceneLoader::LoadGeometry(const std::string &p_strFilename, WavefrontContext &p_context)
//{
//	// Use filename as the default object name
//	boost::filesystem::path geometryPath(p_strFilename);
//	p_context.ObjectName = geometryPath.filename().string();
//	
//	//p_context.Mesh = new PersistentMesh(p_context.ObjectName, "Z:\\Object");
//	//p_context.Mesh = new KDTreeMesh(p_context.ObjectName);
//	p_context.Mesh = new KDTreeMeshEx(p_context.ObjectName);
//	//p_context.Mesh = new BVHMesh(p_context.ObjectName);
//	//p_context.Mesh = new BasicMesh(p_context.ObjectName);
//
//	// Open wavefront file
//	std::ifstream wavefrontFile;
//	wavefrontFile.open(p_strFilename.c_str());
//	
//	if (!wavefrontFile.is_open())
//	{
//		std::cerr << "Error : Couldn't open file '" << p_strFilename << "'" << std::endl;
//		exit(-1);
//	}
//	
//	// define some temporary containers
//	Vector2 vector2;
//	Vector3 vector3;
//
//	std::string currentLine;
//
//	std::vector<std::string> tokenList,
//		faceTokenList;
//
//	while(std::getline(wavefrontFile, currentLine))
//	{
//		tokenList.clear();
//
//		// Tokenise line
//		if (Tokenise(currentLine, " \n\r", tokenList) == 0)
//			continue;
//
//		// Trim whitespace at edges
//		for (size_t tokenIndex = 0; tokenIndex < tokenList.size(); ++tokenIndex)
//			boost::trim(tokenList[tokenIndex]);
//
//		if (tokenList[0] == "o") // Object - set geometry to friendly object name
//		{
//			if (tokenList.size() != 2)
//				continue;
//
//			p_context.ObjectName = tokenList[1];
//		}
//		else if (tokenList[0] == "v") // Position
//		{
//			if (tokenList.size() != 4)
//				continue;
//
//			vector3.Set(boost::lexical_cast<float>(tokenList[1]),
//						boost::lexical_cast<float>(tokenList[2]),
//						boost::lexical_cast<float>(tokenList[3]));
//
//				p_context.PositionList.push_back(vector3);
//		}
//		else if (tokenList[0] == "vn") // Normal
//		{
//			if (tokenList.size() != 4)
//				continue;
//
//			vector3.Set(boost::lexical_cast<float>(tokenList[1]),
//						boost::lexical_cast<float>(tokenList[2]),
//						boost::lexical_cast<float>(tokenList[3]));
//
//			p_context.NormalList.push_back(vector3);
//		}
//		else if (tokenList[0] == "vt") // Texture coordinates
//		{
//			if (tokenList.size() < 3)
//				continue;
//
//			vector2.Set(boost::lexical_cast<float>(tokenList[1]),
//						boost::lexical_cast<float>(tokenList[2]));
//
//			p_context.UVList.push_back(vector2);
//		}
//		else if (tokenList[0] == "f") // Face
//		{
//			// Ignore if there aren't enough vertices to form a surface.
//			// We are interested only in tri/quad faces, so ignore higher
//			// order polygons too.
//			if (tokenList.size() < 4 || tokenList.size() > 5)
//				continue;
//
//			WavefrontVertex vertex;
//			int vertexIndex[4] = {0, 0, 0, 0};
//
//			for (size_t index = 1; index < tokenList.size(); index++)
//			{
//				Tokenise(tokenList[index], "/", faceTokenList);
//
//				vertex.Position = boost::lexical_cast<int>(faceTokenList[0]);
//
//				if (faceTokenList.size() == 2) {
//					vertex.Normal = boost::lexical_cast<int>(faceTokenList[1]);
//				} else if (faceTokenList.size() == 3) {
//					vertex.Texture = boost::lexical_cast<int>(faceTokenList[1]);
//					vertex.Normal = boost::lexical_cast<int>(faceTokenList[2]);
//				} 
//
//				// Search for vertex in map
//				Int64 hash = vertex.GetVertexHash();
//
//				if (p_context.VertexMap.find(hash) == p_context.VertexMap.end())
//				{
//					Vertex meshVertex;
//
//					meshVertex.Position = p_context.PositionList[vertex.Position - 1];
//					meshVertex.UV = (vertex.Texture == 0) ? Vector2::Zero : p_context.UVList[vertex.Texture - 1];
//					meshVertex.Normal = (vertex.Normal == 0) ? Vector3::Zero : p_context.NormalList[vertex.Normal - 1];
//					
//					p_context.VertexMap[hash] = vertexIndex[index - 1] = 
//						p_context.Mesh->VertexList.Size();
//
//					// Add vertex to mesh
//					p_context.Mesh->AddVertex(meshVertex);
//				}
//				else
//				{
//					vertexIndex[index - 1] = p_context.VertexMap[hash];
//				}
//			}
//			
//			// Add faces to mesh
//			p_context.Mesh->AddIndexedTriangle(vertexIndex[0], vertexIndex[1], vertexIndex[2], p_context.CurrentMaterialId);
//			
//			if (tokenList.size() == 5) 
//				p_context.Mesh->AddIndexedTriangle(vertexIndex[0], vertexIndex[2], vertexIndex[3], p_context.CurrentMaterialId);
//		}
//		else if (tokenList[0] == "mtllib")
//		{
//			if (tokenList.size() != 2)
//				continue;
//
//			std::string materialLibraryFilename;
//			materialLibraryFilename = (geometryPath.parent_path() / tokenList[1]).string();
//
//			LoadMaterials(materialLibraryFilename, p_context);
//		}
//		else if (tokenList[0] == "usemtl")
//		{
//			if (tokenList.size() != 2)
//				continue;
//
//			p_context.CurrentMaterialId = p_context.Materials->GetGroupId(tokenList[1]);
//		}
//	}
//	
//	wavefrontFile.close();
//	return true;
//}
//----------------------------------------------------------------------------------------------
int PolygonSceneLoader::Tokenise(std::string &p_strText, char *p_pSeparators, std::vector<std::string> &p_tokenList)
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