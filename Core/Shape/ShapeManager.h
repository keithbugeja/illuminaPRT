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
#include "Shape/KDTreeMesh.h"
#include "Shape/IndexedTriangle.h"
#include "Shape/VertexFormats.h"

namespace Illumina
{
	namespace Core
	{
		typedef FactoryManager<IShape> ShapeManager;
		
		//----------------------------------------------------------------------------------------------
		// Sphere shape factory
		//----------------------------------------------------------------------------------------------
		class SphereShapeFactory : public Illumina::Core::Factory<Illumina::Core::IShape>
		{
		public:
			Illumina::Core::IShape *CreateInstance(void)
			{
				throw new Exception("SphereShapeFactory cannot create a Sphere without required parameters!");
			}

			Illumina::Core::IShape *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strName;
				Vector3 centre;
				float fRadius;

				if (p_argumentMap.GetArgument("Centre", centre) && 
					p_argumentMap.GetArgument("Radius", fRadius))
				{
					if (p_argumentMap.GetArgument("Name", strName))
						return CreateInstance(strName, centre, fRadius);
					else
						return CreateInstance(centre, fRadius);
				}

				throw new Exception("Invalid arguments to SphereShapeFactory!");
			}

			Illumina::Core::IShape *CreateInstance(const Vector3 &p_centre, float p_fRadius)
			{
				return new Sphere(p_centre, p_fRadius);
			}

			Illumina::Core::IShape *CreateInstance(const std::string &p_strName, const Vector3 &p_centre, float p_fRadius)
			{
				return new Sphere(p_strName, p_centre, p_fRadius);
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
				throw new Exception("TriangleShapeFactory cannot create a Sphere without required parameters!");
			}

			Illumina::Core::IShape *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strName;
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
						if (p_argumentMap.GetArgument("Name", strName))
							return CreateInstance(strName, p_position[0], p_position[1], p_position[2], p_uv[0], p_uv[1], p_uv[2]);

						return CreateInstance(p_position[0], p_position[1], p_position[2], p_uv[0], p_uv[1], p_uv[2]);
					}

					if (p_argumentMap.GetArgument("Name", strName))
						return CreateInstance(strName, p_position[0], p_position[1], p_position[2]);

					return CreateInstance(p_position[0], p_position[1], p_position[2]);
				}

				throw new Exception("Invalid arguments to TriangleShapeFactory!");
			}

			Illumina::Core::IShape *CreateInstance(const std::string &p_strName, const Vector3 &p_p0, const Vector3 &p_p1, const Vector3 &p_p2)
			{
				return new Triangle(p_strName, p_p0, p_p1, p_p2);
			}

			Illumina::Core::IShape *CreateInstance(const std::string &p_strName, 
				const Vector3 &p_p0, const Vector3 &p_p1, const Vector3 &p_p2,
				const Vector2 &p_uv0, const Vector2 &p_uv1, const Vector2 &p_uv2)
			{
				return new Triangle(p_strName, p_p0, p_p1, p_p2, p_uv0, p_uv1, p_uv2);
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
				std::string strName;

				if (p_argumentMap.GetArgument("Name", strName))
					return CreateInstance(strName);
				
				return CreateInstance();
			}

			Illumina::Core::IShape *CreateInstance(const std::string &p_strName)
			{
				return new BasicMesh(p_strName);
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
				std::string strName;
				
				int maxDepth, 
					maxObjects;

				if (p_argumentMap.GetArgument("MaximumTreeDepth", maxDepth) && 
					p_argumentMap.GetArgument("MaximumLeafObjects", maxObjects))
				{
					if (p_argumentMap.GetArgument("Name", strName))
						return CreateInstance(strName, maxDepth, maxObjects);
					
					return CreateInstance(maxDepth, maxObjects);
				}

				throw new Exception("Invalid arguments to KDTreeMeshShapeFactory!");
			}

			Illumina::Core::IShape *CreateInstance(int p_nMaxDepth, int p_nMaxLeafObjects)
			{
				return new KDTreeMesh(p_nMaxDepth, p_nMaxLeafObjects);
			}

			Illumina::Core::IShape *CreateInstance(const std::string &p_strName, int p_nMaxDepth, int p_nMaxLeafObjects)
			{
				return new KDTreeMesh(p_strName, p_nMaxDepth, p_nMaxLeafObjects);
			}
		};
	}
}