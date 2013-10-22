#pragma once
#include <System/Platform.h>
#include <Geometry/Vector3.h>
#include <Geometry/Spline.h>

//----------------------------------------------------------------------------------------------
/* NOTE (TODO:) 
 * Path/PathEx should be refined, generalised and placed in the Geometry section of the core
 * Illumina library.
 */
//----------------------------------------------------------------------------------------------
class Path
{
protected:
	std::vector<Vector3> m_vertexList;
	std::vector<float> m_pivotList;
	float m_fTime;
public:
	bool IsEmpty(void) { return m_vertexList.empty(); }
	void Clear(void) { m_vertexList.clear(); m_pivotList.clear(); Reset(); } 
	void Reset(void) { m_fTime = 0; }
	void Move(float p_fDeltaT) { m_fTime += p_fDeltaT; }

	void AddVertex(const Vector3 &p_pVertex) 
	{ 
		m_vertexList.push_back(p_pVertex); 
	}

	void PreparePath(void)
	{
		Illumina::Core::Interpolator::ComputePivots(m_vertexList, m_pivotList);
	}

	Vector3 GetPosition(float p_fTime)
	{
		if (m_vertexList.size() <= 2)
			return Vector3::Zero;

		return Illumina::Core::Interpolator::Lagrange(m_vertexList, m_pivotList, p_fTime);
	}

	Vector3 GetPosition(void) 
	{
		return GetPosition(m_fTime);
	}
};
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
struct PathVertexEx
{
	Vector3 position;
	Vector3 orientation;
};

class PathEx
{
protected:
	std::vector<Vector3> m_positionList,
		m_orientationList;
	std::vector<float> m_pivotList;
	float m_fTime;
public:
	bool IsEmpty(void) { return m_positionList.empty(); }
	void Clear(void) { 
		m_positionList.clear(); 
		m_orientationList.clear(); 
		m_pivotList.clear(); 
		Reset(); 
	} 
	
	void Reset(void) { m_fTime = 0; }
	void Move(float p_fDeltaT) { m_fTime += p_fDeltaT; }

	void AddVertex(const PathVertexEx &p_pVertex) 
	{ 
		std::cout << "PathEx :: Adding vertex [" << p_pVertex.position.ToString() << ", " << p_pVertex.orientation.ToString() << "]" << std::endl;

		m_positionList.push_back(p_pVertex.position);
		m_orientationList.push_back(p_pVertex.orientation);
	}

	void PreparePath(void) {
		Illumina::Core::Interpolator::ComputePivots(m_orientationList, m_pivotList);
	}

	void Get(float p_fTime, Vector3 &p_position, Vector3 &p_lookat)
	{
		if (m_positionList.size() <= 2)
		{
			p_position = Vector3::Zero;
			p_lookat = Vector3::UnitZPos;
		} 
		else
		{
			p_position = Illumina::Core::Interpolator::Lagrange(m_positionList, m_pivotList, p_fTime);
			p_lookat = Illumina::Core::Interpolator::Lagrange(m_orientationList, m_pivotList, p_fTime);

			std::cout << p_fTime << ":" << p_position.ToString() << ":" << p_lookat.ToString() << std::endl;
		}
	}

	void Get(Vector3 &p_position, Vector3 &p_lookat) {
		Get(m_fTime, p_position, p_lookat);
	}

	void FromString(const std::string &p_strPathString) 
	{
		float fDeltaTime = 5e-3f;
		std::vector<Vector3> vertexList;

		ArgumentMap argumentMap(p_strPathString);
		argumentMap.GetArgument("path", vertexList);

		bool vertexFull = false;
		PathVertexEx vertex;

		for (auto vertexElement : vertexList)
		{
			if (vertexFull)
			{
				float angle = vertexElement.X / 360 * Maths::PiTwo;
				vertex.orientation = vertex.position + Vector3(Maths::Sin(angle),0 , Maths::Cos(angle));
				AddVertex(vertex);
			}
			else
				vertex.position = vertexElement;

			vertexFull=!vertexFull;
		}

		PreparePath();
	}
};
