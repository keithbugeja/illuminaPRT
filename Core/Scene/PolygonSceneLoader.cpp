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
	std::string ContainerType;

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

	if (m_pEngineKernel->GetShapeManager()->QueryFactory(p_context.ContainerType))
	{
		ArgumentMap argumentMap("Name=" + p_context.ObjectName + ";");

		p_context.Mesh = (ITriangleMesh*)
			m_pEngineKernel->GetShapeManager()->RequestFactory(p_context.ContainerType)
				->CreateInstance(argumentMap);
	}
	else
		p_context.Mesh = new KDTreeMesh(p_context.ObjectName);

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
				vertexcount--;

				// read x, y, z, nx, ny, nz
				Vertex vertex;

				vertex.Position.Set(
					boost::lexical_cast<float>(tokenList[0]),
					boost::lexical_cast<float>(tokenList[1]),
					boost::lexical_cast<float>(tokenList[2]));

				vertex.Normal.Set(
					boost::lexical_cast<float>(tokenList[3]),
					boost::lexical_cast<float>(tokenList[4]),
					boost::lexical_cast<float>(tokenList[5]));

				p_context.Mesh->AddVertex(vertex);

				// std::cout << vertex.Position.ToString() << ", " << vertex.Normal.ToString() << std::endl;

				if (vertexcount == 0)
					std::cout << "PLY :: vertices parsed" << std::endl;
			}
			else if (facecount > 0)
			{
				facecount--;
				// read count, f0, f1, f2, count, t0, t1, t2, t3, t4, t5, t6, texture_idx
				int vertices = boost::lexical_cast<int>(tokenList[0]);

				// we only consider tesselated geometry
				if (vertices <= 4)
				{
					int vertexIndex[4] = {0, 0, 0, 0};
					for (int idx = 0; idx < vertices; idx++)
					{
						vertexIndex[idx] = boost::lexical_cast<int>(tokenList[idx + 1]);
						// std::cout << vertexIndex[idx] << "::";
					}
					// std::cout << std::endl;

					int texcoord = boost::lexical_cast<int>(tokenList[vertices + 1]);
					int textureIdx = boost::lexical_cast<int>(tokenList[vertices + texcoord + 2]); 

					p_context.Mesh->AddIndexedTriangle(vertexIndex[0], vertexIndex[1], vertexIndex[2], textureIdx);
					if (vertices == 4)
						p_context.Mesh->AddIndexedTriangle(vertexIndex[0], vertexIndex[2], vertexIndex[3], textureIdx); 
				}

				if (facecount == 0)
				{
					std::cout << "PLY :: faces parsed" << std::endl;
				}
			}
		}
	}

	plyFile.close();

	return true;
}
//----------------------------------------------------------------------------------------------
bool PolygonSceneLoader::Import(const std::string &p_strFilename, unsigned int p_uiGeneralFlags, ArgumentMap* p_pArgumentMap)
{
	PolygonContext context;

	if (p_pArgumentMap != NULL) 
		p_pArgumentMap->GetArgument("Container", context.ContainerType);

	if (Load(p_strFilename, context))
	{
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
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool PolygonSceneLoader::Export(const std::string &p_strFilename, unsigned int p_uiGeneralFlags, ArgumentMap* p_pArgumentMap)
{
	return true;
}
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
