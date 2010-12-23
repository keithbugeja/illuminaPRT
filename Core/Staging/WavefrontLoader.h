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

#include <boost/shared_ptr.hpp>

#include "Shape/Shape.h"
#include "Shape/VertexFormats.h"
#include "Threading/List.h"

namespace Illumina
{
	namespace Core
	{
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

		struct WavefrontFace
		{
			WavefrontVertex Vertex[3];
		};

		class WavefrontLoader
		{
		public:
			template<class TMesh, class TVertex>
			static boost::shared_ptr<TMesh> LoadMesh(const std::string& p_strMeshFile)
			{
				boost::shared_ptr<TMesh> mesh(new TMesh);
				
				std::map<std::string, int> vertexMap; 

				std::vector<Vector2> textureCoordList;
				std::vector<Vector3> positionList,
					normalList;

				std::string strLine,
					strType;

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
								if (vertex.Normal == 0) std::cout << "Invalid normal for index " << mesh->VertexList.Size() << "!" << std::endl;
								if (face.Vertex[i].Texture < textureCoordList.size()) vertex.UV = textureCoordList[face.Vertex[i].Texture];
								
								vertexMap[hash] = vertexIndex[i] = mesh->VertexList.Size();
								mesh->AddVertex(vertex);
							}
							else
							{
								vertexIndex[i] = vertexMap[hash];
							}
						}

						mesh->AddIndexedTriangle(vertexIndex[0], vertexIndex[1], vertexIndex[2]);
					}
				}

				// Explicit closing of the file
				meshFile.close();

				std::cout << "Parsed " << mesh->VertexList.Size() << " vertices, " << mesh->TriangleList.Size() << " faces... " << std::endl;

				return mesh;
			}
		};
	}
}