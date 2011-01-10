//----------------------------------------------------------------------------------------------
//	Filename:	ShapeFactory.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <fstream>

#include <boost/shared_ptr.hpp>

#include "Shape/Shape.h"
#include "Shape/VertexFormats.h"
#include "Threading/List.h"
#include "Staging/WavefrontLoader.h"

namespace Illumina
{
	namespace Core
	{
		class ShapeFactory
		{
		public:
			template<class TMesh, class TVertex>
			static void SaveMesh(const std::string& p_strMeshFile, boost::shared_ptr<TMesh> p_pMesh)
			{
				int nVertices = p_pMesh->VertexList.Size();
				int nTriangles = p_pMesh->TriangleList.Size();

				std::ofstream meshFile;

				meshFile.open(p_strMeshFile.c_str(), std::ios::binary | std::ios::trunc);

				if (!meshFile.is_open())
				{
					std::cerr << "ERROR -- Couldn't open file \'" << p_strMeshFile << "\'" << std::endl;
				}

				for (int n = 0; n < nVertices; n++)
				{
					meshFile << "v " << p_pMesh->VertexList[n].Position.X << " " <<
						p_pMesh->VertexList[n].Position.Y << " " <<
						p_pMesh->VertexList[n].Position.Z << std::endl;
				}

				for (int n = 0; n < nTriangles; n++)
				{
					meshFile << "f " << (p_pMesh->TriangleList[n].GetVertexIndex(0) + 1) << " " <<
						(p_pMesh->TriangleList[n].GetVertexIndex(1) + 1) << " " <<
						(p_pMesh->TriangleList[n].GetVertexIndex(2) + 1) << std::endl;
				}

				meshFile.close();
			}

			// Quick and dirty obj loader :
			// TODO: Make a ModelIO interface and provide an obj implementation
			template<class TMesh, class TVertex>
			static boost::shared_ptr<TMesh> LoadMesh(const std::string& p_strMeshFile, EngineKernel *p_pEngineKernel, MaterialGroup **p_pMaterialGroup)
			{
				return WavefrontLoader::LoadMesh<TMesh, TVertex>(p_strMeshFile, p_pEngineKernel, p_pMaterialGroup, false);
			}

			// Quick and dirty obj loader :
			// TODO: Make a ModelIO interface and provide an obj implementation
			template<class TMesh, class TVertex>
			static boost::shared_ptr<TMesh> LoadMesh2(const std::string& p_strMeshFile)
			{
				boost::shared_ptr<TMesh> mesh(new TMesh);

				std::ifstream meshFile;

				// Open image file
				meshFile.open(p_strMeshFile.c_str());

				// If file couldn't be opened, report error and quit
				if (!meshFile.is_open())
				{
					std::cerr << "ERROR -- Couldn't open file \'" << p_strMeshFile << "\'" << std::endl;
					exit(-1);
				}

				float x, y, z;
				int v1, v2, v3, iDummy;
				char strLine[256], cDummy;
				TVertex vertex;

				while(meshFile.getline(strLine, 80))
				{
					switch(strLine[0])
					{
						case 'v':
							if (strLine[1] != ' ') break;

							sscanf(strLine, "%c %f %f %f", &cDummy, &x, &y, &z);
							vertex.Position.Set(x, y, z);
							vertex.UV.Set(0, 0);

							mesh->AddVertex(vertex);
							break;

						case 'f':
							for (char* p = strLine; *p != '\0'; p++) if (*p == '/') *p = 32;

							sscanf(strLine, "%c %d %d %d %d %d %d %d %d %d",
								&cDummy,
								&v1, &iDummy, &iDummy,
								&v2, &iDummy, &iDummy,
								&v3, &iDummy, &iDummy);

							mesh->AddIndexedTriangle(v1 - 1, v2 - 1, v3 - 1);
							break;

						default:
							break;
					}
				}

				return mesh;
			}

			template<class TMesh, class TVertex>
			static boost::shared_ptr<TMesh> CreateQuad(const Vector3 &p_v0, const Vector3 &p_v1, 
				const Vector3 &p_v2, const Vector3 &p_v3)
			{
				boost::shared_ptr<TMesh> mesh(new TMesh);

				TVertex vertex[4];

				// If vertex has position, initialise
				if (TVertex::GetDescriptor() & VertexFormat::Position)
				{
					vertex[0].Position = p_v0;
					vertex[1].Position = p_v1;
					vertex[2].Position = p_v2;
					vertex[3].Position = p_v3;
				}

				// If vertex has UVs, initialise
				if (TVertex::GetDescriptor() & VertexFormat::UV)
				{
					vertex[0].UV.Set(0,0);
					vertex[1].UV.Set(1,0);
					vertex[2].UV.Set(0,1);
					vertex[3].UV.Set(1,1);
				}

				// Add vertices and faces to mesh
				mesh->AddVertex(vertex[0]);
				mesh->AddVertex(vertex[1]);
				mesh->AddVertex(vertex[2]);
				mesh->AddVertex(vertex[3]);

				mesh->AddIndexedTriangle(0, 2, 1);
				mesh->AddIndexedTriangle(1, 2, 3);

				return mesh;
			}

			template<class TMesh, class TVertex>
			static boost::shared_ptr<TMesh> CreateBox(const Vector3 &p_minExtent, const Vector3 &p_maxExtent)
			{
				boost::shared_ptr<TMesh> mesh(new TMesh);

				TVertex vertex[24];

				// If vertex has position, initialise
				if (TVertex::GetDescriptor() & VertexFormat::Position)
				{
					vertex[0].Position.Set(p_minExtent.X, p_maxExtent.Y, p_maxExtent.Z);
					vertex[1].Position.Set(p_maxExtent.X, p_maxExtent.Y, p_maxExtent.Z);
					vertex[2].Position.Set(p_minExtent.X, p_minExtent.Y, p_maxExtent.Z);
					vertex[3].Position.Set(p_maxExtent.X, p_minExtent.Y, p_maxExtent.Z);

					vertex[4].Position.Set(p_minExtent.X, p_maxExtent.Y, p_minExtent.Z);
					vertex[5].Position.Set(p_maxExtent.X, p_maxExtent.Y, p_minExtent.Z);
					vertex[6].Position.Set(p_minExtent.X, p_minExtent.Y, p_minExtent.Z);
					vertex[7].Position.Set(p_maxExtent.X, p_minExtent.Y, p_minExtent.Z);

					vertex[8].Position.Set(p_minExtent.X, p_maxExtent.Y, p_minExtent.Z);
					vertex[9].Position.Set(p_maxExtent.X, p_maxExtent.Y, p_minExtent.Z);
					vertex[10].Position.Set(p_minExtent.X, p_maxExtent.Y, p_maxExtent.Z);
					vertex[11].Position.Set(p_maxExtent.X, p_maxExtent.Y, p_maxExtent.Z);

					vertex[12].Position.Set(p_maxExtent.X, p_minExtent.Y, p_minExtent.Z);
					vertex[13].Position.Set(p_minExtent.X, p_minExtent.Y, p_minExtent.Z);
					vertex[14].Position.Set(p_maxExtent.X, p_minExtent.Y, p_maxExtent.Z);
					vertex[15].Position.Set(p_minExtent.X, p_minExtent.Y, p_maxExtent.Z);

					vertex[16].Position.Set(p_minExtent.X, p_minExtent.Y, p_minExtent.Z);
					vertex[17].Position.Set(p_minExtent.X, p_minExtent.Y, p_maxExtent.Z);
					vertex[18].Position.Set(p_minExtent.X, p_maxExtent.Y, p_minExtent.Z);
					vertex[19].Position.Set(p_minExtent.X, p_maxExtent.Y, p_maxExtent.Z);

					vertex[20].Position.Set(p_maxExtent.X, p_maxExtent.Y, p_maxExtent.Z);
					vertex[21].Position.Set(p_maxExtent.X, p_maxExtent.Y, p_minExtent.Z);
					vertex[22].Position.Set(p_maxExtent.X, p_minExtent.Y, p_maxExtent.Z);
					vertex[23].Position.Set(p_maxExtent.X, p_minExtent.Y, p_minExtent.Z);
				}

				// If vertex has UVs, initialise
				if (TVertex::GetDescriptor() & VertexFormat::UV)
				{
					for (int nQuadGroup=0; nQuadGroup < 23; nQuadGroup+=4)
					{
						vertex[nQuadGroup + 0].UV.Set(0,0);
						vertex[nQuadGroup + 1].UV.Set(1,0);
						vertex[nQuadGroup + 2].UV.Set(0,1);
						vertex[nQuadGroup + 3].UV.Set(1,1);
					}
				}

				// Add vertices and faces to mesh
				for (int nQuadGroup=0; nQuadGroup < 23; nQuadGroup+=4)
				{
					mesh->AddVertex(vertex[nQuadGroup + 0]);
					mesh->AddVertex(vertex[nQuadGroup + 1]);
					mesh->AddVertex(vertex[nQuadGroup + 2]);
					mesh->AddVertex(vertex[nQuadGroup + 3]);

					mesh->AddIndexedTriangle(nQuadGroup + 0, nQuadGroup + 2, nQuadGroup + 1);
					mesh->AddIndexedTriangle(nQuadGroup + 1, nQuadGroup + 2, nQuadGroup + 3);
				}

				return mesh;
			}
		};
	}
}
