#include "Staging/Scene.h"

using namespace Illumina::Core;

Scene::Scene(void)
	: m_pSpace(NULL)
{ }

Scene::Scene(ISpace *p_pSpace)
	: m_pSpace(p_pSpace)
{ }

bool Scene::Intersects(const Ray &p_ray, Intersection &p_intersection)
{
	return m_pSpace->Intersects(p_ray, 0.0f, p_intersection);
}

bool Scene::Intersects(const Ray &p_ray)
{
	return m_pSpace->Intersects(p_ray, 0.0f);
}

ISpace* Scene::GetSpace(void)
{
	return m_pSpace;
}

void Scene::SetSpace(ISpace *p_pSpace)
{
	m_pSpace = p_pSpace;
}