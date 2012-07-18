//----------------------------------------------------------------------------------------------
//	Filename:	ShapeManager.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <iostream>

#include "System/FactoryManager.h"

#include "Shape/Sphere.h"
#include "Shape/Triangle.h"
#include "Shape/BasicMesh.h"
#include "Shape/BVHMesh.h"
#include "Shape/KDTreeMesh.h"
#include "Shape/PersistentMesh.h"
#include "Shape/IndexedTriangle.h"
#include "Shape/VertexFormats.h"
#include "Shape/ShapeForge.h"

namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
		// Sphere shape factory
		//----------------------------------------------------------------------------------------------
		class SphereShapeFactory : public Illumina::Core::Factory<Illumina::Core::IShape>
		{
		public:
			Illumina::Core::IShape *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			Illumina::Core::IShape *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;
				Vector3 centre;
				float fRadius;

				if (p_argumentMap.GetArgument("Centre", centre) && 
					p_argumentMap.GetArgument("Radius", fRadius))
				{
					if (p_argumentMap.GetArgument("Id", strId))
						return CreateInstance(strId, centre, fRadius);

					return CreateInstance(centre, fRadius);
				}

				throw new Exception("Invalid arguments to SphereShapeFactory!");
			}

			Illumina::Core::IShape *CreateInstance(const Vector3 &p_centre, float p_fRadius)
			{
				return new Sphere(p_centre, p_fRadius);
			}

			Illumina::Core::IShape *CreateInstance(const std::string &p_strId, const Vector3 &p_centre, float p_fRadius)
			{
				return new Sphere(p_strId, p_centre, p_fRadius);
			}
		};

		//----------------------------------------------------------------------------------------------
		// Triangle shape factory
		//----------------------------------------------------------------------------------------------
		class TriangleShapeFactory : public Illumina::Core::Factory<Illumina::Core::IShape>
		{
		public:
			Illumina::Core::IShape *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			Illumina::Core::IShape *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;
				Vector3 p_position[3];
				Vector2 p_uv[2];

				if (p_argumentMap.GetArgument("P0", p_position[0]) && 
					p_argumentMap.GetArgument("P1", p_position[1]) && 
					p_argumentMap.GetArgument("P2", p_position[2])) 					
				{
					if (p_argumentMap.GetArgument("UV0", p_uv[0]) &&
						p_argumentMap.GetArgument("UV1", p_uv[1]) &&
						p_argumentMap.GetArgument("UV2", p_uv[2]))
					{
						if (p_argumentMap.GetArgument("Id", strId))
							return CreateInstance(strId, p_position[0], p_position[1], p_position[2], p_uv[0], p_uv[1], p_uv[2]);

						return CreateInstance(p_position[0], p_position[1], p_position[2], p_uv[0], p_uv[1], p_uv[2]);
					}

					if (p_argumentMap.GetArgument("Id", strId))
						return CreateInstance(strId, p_position[0], p_position[1], p_position[2]);

					return CreateInstance(p_position[0], p_position[1], p_position[2]);
				}

				throw new Exception("Invalid arguments to TriangleShapeFactory!");
			}

			Illumina::Core::IShape *CreateInstance(const std::string &p_strId, const Vector3 &p_p0, const Vector3 &p_p1, const Vector3 &p_p2)
			{
				return new Triangle(p_strId, p_p0, p_p1, p_p2);
			}

			Illumina::Core::IShape *CreateInstance(const std::string &p_strId, 
				const Vector3 &p_p0, const Vector3 &p_p1, const Vector3 &p_p2,
				const Vector2 &p_uv0, const Vector2 &p_uv1, const Vector2 &p_uv2)
			{
				return new Triangle(p_strId, p_p0, p_p1, p_p2, p_uv0, p_uv1, p_uv2);
			}

			Illumina::Core::IShape *CreateInstance(const Vector3 &p_p0, const Vector3 &p_p1, const Vector3 &p_p2)
			{
				return new Triangle(p_p0, p_p1, p_p2);
			}

			Illumina::Core::IShape *CreateInstance(const Vector3 &p_p0, const Vector3 &p_p1, const Vector3 &p_p2,
				const Vector2 &p_uv0, const Vector2 &p_uv1, const Vector2 &p_uv2)
			{
				return new Triangle(p_p0, p_p1, p_p2, p_uv0, p_uv1, p_uv2);
			}
		};

		//----------------------------------------------------------------------------------------------
		// Quad shape factory
		// Note	that the factory produces ITriangleMesh objects with vertex format type Vertex and
		// IndexedTriangle type faces.
		//----------------------------------------------------------------------------------------------
		class QuadMeshShapeFactory : public Illumina::Core::Factory<Illumina::Core::IShape>
		{
		public:
			Illumina::Core::IShape *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			Illumina::Core::IShape *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::vector<Vector3> vertices;
				std::string strId;

				p_argumentMap.GetArgument("Vertices", vertices);

				if (vertices.size() != 4)
					throw new Exception("QuadMeshShapeFactory : Incorrect number of vertices.");

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, vertices);
				
				return CreateInstance(vertices);
			}

			Illumina::Core::IShape *CreateInstance(const std::string &p_strId, std::vector<Vector3> &p_vertices)
			{
				//ITriangleMesh *pMesh = new BasicMesh(p_strId);
				ITriangleMesh *pMesh = new KDTreeMesh(p_strId);
				ShapeForge::CreateQuad(p_vertices[0], p_vertices[1], p_vertices[2], p_vertices[3], pMesh);
				return pMesh;
			}

			Illumina::Core::IShape *CreateInstance(std::vector<Vector3> &p_vertices)
			{
				//ITriangleMesh *pMesh = new BasicMesh();
				ITriangleMesh *pMesh = new KDTreeMesh();
				ShapeForge::CreateQuad(p_vertices[0], p_vertices[1], p_vertices[2], p_vertices[3], pMesh);
				return pMesh;
			}
		};

		//----------------------------------------------------------------------------------------------
		// BasicMesh shape factory
		// Note	that the factory produces ITriangleMesh objects with vertex format type Vertex and
		// IndexedTriangle type faces.
		//----------------------------------------------------------------------------------------------
		class BasicMeshShapeFactory : public Illumina::Core::Factory<Illumina::Core::IShape>
		{
		public:
			Illumina::Core::IShape *CreateInstance(void)
			{
				return new BasicMesh();
			}

			Illumina::Core::IShape *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId);
				
				return CreateInstance();
			}

			Illumina::Core::IShape *CreateInstance(const std::string &p_strId)
			{
				return new BasicMesh(p_strId);
			}
		};

		//----------------------------------------------------------------------------------------------
		// KD-Tree shape factory
		// Note	that the factory produces ITriangleMesh objects with vertex format type Vertex and
		// IndexedTriangle type faces.
		//----------------------------------------------------------------------------------------------
		class KDTreeMeshShapeFactory : public Illumina::Core::Factory<Illumina::Core::IShape>
		{
		public:
			Illumina::Core::IShape *CreateInstance(void)
			{
				return new KDTreeMesh();
			}

			Illumina::Core::IShape *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;
				
				int maxDepth, 
					maxObjects;

				if (p_argumentMap.GetArgument("MaximumTreeDepth", maxDepth) && 
					p_argumentMap.GetArgument("MaximumLeafObjects", maxObjects))
				{
					if (p_argumentMap.GetArgument("Name", strId))
						return CreateInstance(strId, maxDepth, maxObjects);
					
					return CreateInstance(maxDepth, maxObjects);
				}

				throw new Exception("Invalid arguments to KDTreeMeshShapeFactory!");
			}

			Illumina::Core::IShape *CreateInstance(int p_nMaxDepth, int p_nMaxLeafObjects)
			{
				return new KDTreeMesh(p_nMaxDepth, p_nMaxLeafObjects);
			}

			Illumina::Core::IShape *CreateInstance(const std::string &p_strId, int p_nMaxDepth, int p_nMaxLeafObjects)
			{
				return new KDTreeMesh(p_strId, p_nMaxDepth, p_nMaxLeafObjects);
			}
		};

		//----------------------------------------------------------------------------------------------
		// BVH shape factory
		// Note	that the factory produces ITriangleMesh objects with vertex format type Vertex and
		// IndexedTriangle type faces.
		//----------------------------------------------------------------------------------------------
		class BVHMeshShapeFactory : public Illumina::Core::Factory<Illumina::Core::IShape>
		{
		public:
			Illumina::Core::IShape *CreateInstance(void)
			{
				return new BVHMesh();
			}

			Illumina::Core::IShape *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;
				
				int maxDepth, 
					maxObjects;

				if (p_argumentMap.GetArgument("MaximumTreeDepth", maxDepth) && 
					p_argumentMap.GetArgument("MaximumLeafObjects", maxObjects))
				{
					if (p_argumentMap.GetArgument("Name", strId))
						return CreateInstance(strId, maxDepth, maxObjects);
					
					return CreateInstance(maxDepth, maxObjects);
				}

				throw new Exception("Invalid arguments to KDTreeMeshShapeFactory!");
			}

			Illumina::Core::IShape *CreateInstance(int p_nMaxDepth, int p_nMaxLeafObjects)
			{
				return new BVHMesh(p_nMaxDepth, p_nMaxLeafObjects);
			}

			Illumina::Core::IShape *CreateInstance(const std::string &p_strId, int p_nMaxDepth, int p_nMaxLeafObjects)
			{
				return new BVHMesh(p_strId, p_nMaxDepth, p_nMaxLeafObjects);
			}
		};

		//----------------------------------------------------------------------------------------------
		// Persistent Mesh factory
		//----------------------------------------------------------------------------------------------
		class PersistentMeshShapeFactory : public Illumina::Core::Factory<Illumina::Core::IShape>
		{
		public:
			Illumina::Core::IShape *CreateInstance(void)
			{
				throw new Exception("Cannot instantiate Persistent Mesh without trunk name!");
			}

			Illumina::Core::IShape *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId,
					strTrunkName;

				if (p_argumentMap.GetArgument("TrunkName", strTrunkName))
				{
					if (p_argumentMap.GetArgument("Name", strId))
						return CreateInstance(strId, strTrunkName);
					
					return CreateInstance(strTrunkName);
				}

				throw new Exception("Invalid arguments to PersistentMeshShapeFactory!");
			}

			Illumina::Core::IShape *CreateInstance(const std::string &p_strTrunkName)
			{
				return new PersistentMesh(p_strTrunkName);
			}

			Illumina::Core::IShape *CreateInstance(const std::string &p_strId, const std::string &p_strTrunkName)
			{
				return new PersistentMesh(p_strId, p_strTrunkName);
			}
		};
	}
}