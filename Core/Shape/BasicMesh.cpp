//----------------------------------------------------------------------------------------------
//	Filename:	BasicMesh.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Shape/BasicMesh.h"
#include "Shape/TriangleMesh.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
boost::shared_ptr<ITriangleMesh> BasicMesh::CreateInstance(void) {
	return boost::shared_ptr<ITriangleMesh>(new BasicMesh());
}
//----------------------------------------------------------------------------------------------
bool BasicMesh::Intersects(const Ray &p_ray, DifferentialSurface &p_surface)
{
	DifferentialSurface surface;

	bool bIntersect = false;
	Ray ray(p_ray);

	for (int idx = 0, count = (int)ITriangleMesh::TriangleList.Size(); idx < count; idx++)
	{
		if (ITriangleMesh::TriangleList[idx].Intersects(ray, surface))
		{
			if (ray.Max > p_surface.Distance)
			{
				ray.Max = surface.Distance;
				p_surface = surface;
				bIntersect = true;
			}
		}
	}

	return bIntersect;
}
//----------------------------------------------------------------------------------------------
bool BasicMesh::Intersects(const Ray &p_ray)
{
	for (int idx = 0, count = (int)ITriangleMesh::TriangleList.Size(); idx < count; idx++)
	{
		if (ITriangleMesh::TriangleList[idx].Intersects(p_ray))
			return true;					
	}

	return false;
}
//----------------------------------------------------------------------------------------------
