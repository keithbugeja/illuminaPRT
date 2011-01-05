//----------------------------------------------------------------------------------------------
//	Filename:	WavefrontLoader.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <fstream>
#include <map>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

#include "System/EngineKernel.h"
#include "Shape/VertexFormats.h"
#include "Shape/Shape.h"
#include "Spectrum/Spectrum.h"

namespace Illumina
{
	namespace Core
	{
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
			WavefrontVertex Vertex[3];
		};
		//----------------------------------------------------------------------------------------------
		struct WavefrontMaterial
		{
			std::string Name;

			float Shininess;
			float RefractiveIndex;

			Spectrum Ambient;
			Spectrum Diffuse;
			Spectrum Specular;
			Spectrum Emissive;

			std::string AmbientMap;
			std::string DiffuseMap;
			std::string BumpMap;
		};
		//----------------------------------------------------------------------------------------------
		class WavefrontLoader
		{
		protected:
			static bool LoadMaterials(const std::string &p_strMaterialsFile, std::vector<WavefrontMaterial> &p_materialList)
			{
				std::ifstream file;
				file.open(p_strMaterialsFile.c_str());

				// If file couldn't be opened, report error and quit
				if (!file.is_open())
				{
					std::cerr << "ERROR -- Couldn't open file \'" << p_strMaterialsFile << "\'" << std::endl;
					exit(-1);
				}

				std::string strLine, strDummy;
				float value[3];

				while(std::getline(file, strLine))
				{
					std::stringstream line(strLine);
					
					// New material
					if (strLine.find("newmtl") != std::string::npos)
					{
						line>>strDummy>>strDummy;
						WavefrontMaterial material;
						material.Name = strDummy;

						p_materialList.push_back(material);

						//std::cout<<"Newmtl " << strDummy << std::endl;
					}
					else if (strLine.find("map_Kd") != std::string::npos)
					{
						line>>strDummy>>strDummy;
						p_materialList.back().DiffuseMap = strDummy;

						//std::cout<<"map_Kd " << strDummy << std::endl;
					}
					else if (strLine.find("Kd") != std::string::npos)
					{
						line>>strDummy>>value[0]>>value[1]>>value[2];
						p_materialList.back().Diffuse.Set(value);

						//std::cout<<"Kd " << value[0] << " " << value[1] << " " << value[2] << std::endl;
					}
					else if (strLine.find("Ka") != std::string::npos)
					{
						line>>strDummy>>value[0]>>value[1]>>value[2];
						p_materialList.back().Ambient.Set(value);

						//std::cout<<"Ka " << value[0] << " " << value[1] << " " << value[2] << std::endl;
					}
					else if (strLine.find("Ks") != std::string::npos)
					{
						line>>strDummy>>value[0]>>value[1]>>value[2];
						p_materialList.back().Specular.Set(value);

						//std::cout<<"Ks " << value[0] << " " << value[1] << " " << value[2] << std::endl;
					}
					else if (strLine.find("Ke") != std::string::npos)
					{
						line>>strDummy>>value[0]>>value[1]>>value[2];
						p_materialList.back().Emissive.Set(value);

						//std::cout<<"Ke " << value[0] << " " << value[1] << " " << value[2] << std::endl;
					}
					else if (strLine.find("Ns") != std::string::npos)
					{
						line>>strDummy>>value[0];
						p_materialList.back().Shininess = value[0];

						//std::cout<<"Ns " << value[0] << std::endl;
					}
					else if (strLine.find("Ni") != std::string::npos)
					{
						line>>strDummy>>value[0];
						p_materialList.back().RefractiveIndex = value[0];

						//std::cout<<"Ni " << value[0] << std::endl;
					}
				}

				file.close();

				std::cout << "-- Material file loaded " << p_materialList.size() << " entries..." << std::endl;

				return true;
			}

		public:
			template<class TMesh, class TVertex>
			static boost::shared_ptr<TMesh> LoadMesh(const std::string &p_strMeshFile, EngineKernel *p_pEngineKernel, MaterialGroup **p_pMaterialGroup)
			{
				int currentMaterialId = -1;

				std::map<std::string, int> vertexMap; 
				std::vector<Vector2> textureCoordList;
				std::vector<Vector3> positionList, normalList;
				std::vector<WavefrontMaterial> materialList;
				std::string strLine, strType;

				boost::shared_ptr<TMesh> mesh(new TMesh);

				std::ifstream meshFile;
				meshFile.open(p_strMeshFile.c_str());

				// If file couldn't be opened, report error and quit
				if (!meshFile.is_open())
				{
					std::cerr << "ERROR -- Couldn't open file \'" << p_strMeshFile << "\'" << std::endl;
					exit(-1);
				}

				while(std::getline(meshFile, strLine))
				{
					std::stringstream meshLine(strLine);

					meshLine >> strType;

					if(strType == "v")
					{
						Vector3 position;
						meshLine >> position.X >> position.Y >> position.Z;
						positionList.push_back(position);
					}
					else if(strType == "vt")
					{
						Vector2 textureCoords;
						meshLine >> textureCoords.U >> textureCoords.V;
						textureCoordList.push_back(textureCoords);
					}
					else if(strType == "vn")
					{
						Vector3 normal;
						meshLine >> normal.X >> normal.Y >> normal.Z;
						normalList.push_back(normal);
					}
					else if(strType == "f")
					{
						char separator;
						int vertexIndex[3];
						WavefrontFace face;

						for (int i = 0; i < 3; i++)
						{
							meshLine >> face.Vertex[i].Position >> separator
								>> face.Vertex[i].Texture >> separator
								>> face.Vertex[i].Normal;

							face.Vertex[i].Position--;
							if (face.Vertex[i].Normal != 0) face.Vertex[i].Normal--;
							if (face.Vertex[i].Texture != 0) face.Vertex[i].Texture--;

							// Vertex not found in map
							std::string hash = face.Vertex[i].GetVertexHash();
							if (vertexMap.find(hash) == vertexMap.end())
							{
								TVertex vertex;								

								vertex.Position = positionList[face.Vertex[i].Position];
								vertex.Normal = normalList[face.Vertex[i].Normal];
								if (vertex.Normal == 0) std::cout << "-- Invalid normal for index [" << mesh->VertexList.Size() << "]" << std::endl;
								if (face.Vertex[i].Texture < textureCoordList.size()) vertex.UV = textureCoordList[face.Vertex[i].Texture];
								
								vertexMap[hash] = vertexIndex[i] = mesh->VertexList.Size();
								mesh->AddVertex(vertex);
							}
							else
							{
								vertexIndex[i] = vertexMap[hash];
							}
						}

						mesh->AddIndexedTriangle(vertexIndex[0], vertexIndex[1], vertexIndex[2], currentMaterialId);
					}
					else if (strLine.find("mtllib") != string::npos)
					{
						std::string file, filepath;
						meshLine >> file >> file;
						std::cout<< "-- Loading materials file '" << file << "'..." << std::endl;
						
						boost::filesystem::path meshPath(p_strMeshFile);
						filepath = (meshPath.parent_path() / file).string();
						LoadMaterials(filepath, materialList);

						*p_pMaterialGroup = (MaterialGroup*)p_pEngineKernel->GetMaterialManager()->CreateInstance("Group", file);

						for (int matIdx = 0; matIdx < materialList.size(); matIdx++)
						{
							const WavefrontMaterial &material = materialList.at(matIdx);

							std::string indexName = material.Name;

							std::stringstream argumentStream;
							argumentStream<<"Name="<<indexName<<";Reflectivity="<<material.Diffuse[0]<<","<<material.Diffuse[1]<<","<<material.Diffuse[2]<<";";
							IMaterial *pMaterial = p_pEngineKernel->GetMaterialManager()->CreateInstance("Diffuse", indexName, argumentStream.str());
							(*p_pMaterialGroup)->Add(pMaterial, matIdx);

							if (material.DiffuseMap.size() != 0)
							{ 
								std::string diffuseMap = material.DiffuseMap;

								std::stringstream textureArgStream;
								textureArgStream<<"Name="<<diffuseMap<<";Filename="<<(meshPath.parent_path() / diffuseMap).string()<<";Filetype=PPM;";
								ITexture *pTexture = p_pEngineKernel->GetTextureManager()->CreateInstance("Image", diffuseMap, textureArgStream.str());
							
								DiffuseMaterial *pDiffuse = (DiffuseMaterial*)pMaterial;
								pDiffuse->SetDiffuseTexture(pTexture);
							}
						} 
					}
					else if (strLine.find("usemtl") != string::npos)
					{
						std::string materialName;
						meshLine >> materialName >> materialName;

						int currentMaterialId = (*p_pMaterialGroup)->GetGroupId(materialName);
						std::cout<< "-- Using material [" << materialName << ":" << currentMaterialId << "]..." << std::endl;
					}
				}

				// Explicit closing of the file
				meshFile.close();

				std::cout << "-- Parsed " << mesh->VertexList.Size() << " vertices, " << mesh->TriangleList.Size() << " faces... " << std::endl;

				return mesh;
			}
		};
	}
}