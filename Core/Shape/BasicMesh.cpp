//----------------------------------------------------------------------------------------------
//	Filename:	BasicMesh.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Shape/BasicMesh.h"
#include "Shape/TriangleMesh.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
boost::shared_ptr<ITriangleMesh> BasicMesh::CreateInstance(void) {
	return boost::shared_ptr<ITriangleMesh>(new BasicMesh());
}
//----------------------------------------------------------------------------------------------
bool BasicMesh::Intersects(const Ray &p_ray, float p_fTime, DifferentialSurface &p_surface)
{
	bool bIntersect = false;
	Ray ray(p_ray);

	for (int idx = 0, count = (int)ITriangleMesh::TriangleList.Size(); idx < count; idx++)
	{
		if (ITriangleMesh::TriangleList[idx].Intersects(ray, p_fTime, p_surface))
		{
			ray.Max = p_surface.Distance;
			bIntersect = true;
		}
	}

	return bIntersect;
}
//----------------------------------------------------------------------------------------------
bool BasicMesh::Intersects(const Ray &p_ray, float p_fTime)
{
	for (int idx = 0, count = (int)ITriangleMesh::TriangleList.Size(); idx < count; idx++)
	{
		if (ITriangleMesh::TriangleList[idx].Intersects(p_ray, p_fTime))
			return true;					
	}

	return false;
}
//----------------------------------------------------------------------------------------------
