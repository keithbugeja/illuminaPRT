//----------------------------------------------------------------------------------------------
//	Filename:	Scene.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include "Scene/Scene.h"
#include "Geometry/Intersection.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
Scene::Scene(ISpace *p_pSpace, ICamera *p_pCamera, ISampler *p_pSampler)
	: m_pSpace(p_pSpace)
	, m_pCamera(p_pCamera)
	, m_pSampler(p_pSampler)
{ }
//----------------------------------------------------------------------------------------------
bool Scene::Intersects(const Ray &p_ray, Intersection &p_intersection, IPrimitive *p_pExclude)
{
	p_intersection.Prepare();

	return m_pSpace->Intersects(p_ray, p_intersection, p_pExclude);
}
//----------------------------------------------------------------------------------------------
bool Scene::Intersects(const Ray &p_ray, Intersection &p_intersection)
{
	p_intersection.Prepare();

	return m_pSpace->Intersects(p_ray, p_intersection);
}
//----------------------------------------------------------------------------------------------
bool Scene::Intersects(const Ray &p_ray)
{
	return m_pSpace->Intersects(p_ray);
}
//----------------------------------------------------------------------------------------------
bool Scene::Intersects(const Ray &p_ray, IPrimitive *p_pExclude)
{
	return m_pSpace->Intersects(p_ray, p_pExclude);
}
//----------------------------------------------------------------------------------------------
ISpace* Scene::GetSpace(void) const
{
	return m_pSpace;
}
//----------------------------------------------------------------------------------------------
void Scene::SetSpace(ISpace *p_pSpace)
{
	m_pSpace = p_pSpace;
}
//----------------------------------------------------------------------------------------------
ICamera* Scene::GetCamera(void) const
{
	return m_pCamera;
}
//----------------------------------------------------------------------------------------------
void Scene::SetCamera(ICamera *p_pCamera)
{
	m_pCamera = p_pCamera;
}
//----------------------------------------------------------------------------------------------
ISampler* Scene::GetSampler(void) const
{
	return m_pSampler;
}
//----------------------------------------------------------------------------------------------
void Scene::SetSampler(ISampler *p_pSampler)
{
	m_pSampler = p_pSampler;
}
//----------------------------------------------------------------------------------------------
