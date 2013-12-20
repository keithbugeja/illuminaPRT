//----------------------------------------------------------------------------------------------
//	Filename:	PointGrid.h
//	Author:		Keith Bugeja
//	Date:		27/02/2013
//----------------------------------------------------------------------------------------------
#pragma once

#include <Maths/Montecarlo.h>
#include <Scene/Visibility.h>

#include <string>

using namespace Illumina::Core;

// Type T expected to implement
// radius : float
// position : Vector3

template<class T>
class PointGrid
{
protected:
	std::vector<T*> m_pointList;
	std::map<Int64, std::vector<T*>> m_pointGrid;

	Vector3 m_centroid,
		m_size, m_partitions,
		m_origin;

	struct Hash 
	{
		Int64 X : 21;
		Int64 Y : 21;
		Int64 Z : 21;
	};

public:
	Int64 MakeKey(const Vector3 &p_position)
	{
		Vector3 floatKey = (p_position - m_origin);
		floatKey.Set(floatKey.X / m_partitions.X, floatKey.Y / m_partitions.Y, floatKey.Z / m_partitions.Z); 
		
		Hash hash;

		hash.X = (int)Maths::Floor(floatKey.X) & 0xFFFFF;
		hash.Y = (int)Maths::Floor(floatKey.Y) & 0xFFFFF;
		hash.Z = (int)Maths::Floor(floatKey.Z) & 0xFFFFF;

		return *(Int64*)&hash;
	}

protected:
	void AddRange(T *p_pPoint) 
	{
		Vector3 extent(p_pPoint->Radius),
			iterator;

		Vector3 minExtent(p_pPoint->Position - extent),
				maxExtent(p_pPoint->Position + extent);

		Vector3 stepSize(m_size.X / m_partitions.X, m_size.Y / m_partitions.Y, m_size.Z / m_partitions.Y);

		for (iterator.X = minExtent.X; iterator.X < maxExtent.X; iterator.X+=stepSize.X)
		{
			for (iterator.Y = minExtent.Y; iterator.Y < maxExtent.Y; iterator.Y+=stepSize.Y)
			{
				for (iterator.Z = minExtent.Z; iterator.Z < maxExtent.Z; iterator.Z+=stepSize.Z)
				{
					Int64 key = MakeKey(iterator);
					m_pointGrid[key].push_back(p_pPoint);
				}
			}
		}
	}

public:
	PointGrid(void) { }

	PointGrid(Vector3 &p_centroid, Vector3 &p_size, Vector3 &p_partitions)
		: m_centroid(p_centroid)
		, m_size(p_size)
		, m_partitions(p_partitions)
		, m_origin(m_centroid - m_size * 0.5f)
	{ }

	~PointGrid(void) {
		Clear();
	}

	void Initialise(Vector3 &p_centroid, Vector3 &p_size, Vector3 &p_partitions)
	{
		m_centroid = p_centroid;
		m_size = p_size;
		m_partitions = p_partitions;
		m_origin = m_centroid - m_size / 2;

		Clear();
	}

	void Clear(void) 
	{ 		
		for (auto point : m_pointList)
			delete point;

		m_pointList.clear();
		m_pointGrid.clear();
	}

	std::vector<T*>& Get(Int64 p_key) {
		return m_pointGrid[p_key];
	}

	std::vector<T*>& Get(Vector3 p_position) {
		return Get(MakeKey(p_position));
	}

	void Add(Int64 p_key, T& p_point) 
	{
		T* point = new T(p_point);
		m_pointList.push_back(point);

		AddRange(point);
	}

	void Add(Vector3 p_position, T& p_point) {
		Add(MakeKey(p_position), p_point);
	}

	void Get(std::vector<T> &p_pointList)
	{
		p_pointList.clear();

		for (auto point : m_pointList)
			p_pointList.push_back(*point);
	}

	std::vector<T*> Get(void) { 
		return m_pointList; 
	}

	bool Contains(Vector3 p_centre, Vector3 p_normal, float p_fRadius, float p_fAspect)
	{
		Vector3 minExtent(p_centre.X - p_fRadius,
			p_centre.Y - p_fRadius,
			p_centre.Z - p_fRadius),
				maxExtent(p_centre.X + p_fRadius,
			p_centre.Y + p_fRadius,
			p_centre.Z + p_fRadius);

		Vector3 stepSize(m_size.X / m_partitions.X, m_size.Y / m_partitions.Y, m_size.Z / m_partitions.Y);
		Vector3 iterator;

		float aspect = p_fAspect * p_fRadius;

		for (iterator.X = minExtent.X; iterator.X < maxExtent.X; iterator.X+=stepSize.X)
		{
			for (iterator.Y = minExtent.Y; iterator.Y < maxExtent.Y; iterator.Y+=stepSize.Y)
			{
				for (iterator.Z = minExtent.Z; iterator.Z < maxExtent.Z; iterator.Z+=stepSize.Z)
				{
					std::vector<T*> list = Get(iterator);
					for (auto point : list)
					{
						if (Vector3::Distance(point->Position, p_centre) < p_fRadius)
						{
							if (Vector3::AbsDot(p_normal, (point->Position - p_centre)) < aspect)
								return true;
						}
					}
				}
			}
		}

		return false;
	}

	bool Contains(Vector3 p_centre, float p_fRadius)
	{
		Vector3 minExtent(p_centre.X - p_fRadius,
			p_centre.Y - p_fRadius,
			p_centre.Z - p_fRadius),
				maxExtent(p_centre.X + p_fRadius,
			p_centre.Y + p_fRadius,
			p_centre.Z + p_fRadius);

		Vector3 stepSize(m_size.X / m_partitions.X, m_size.Y / m_partitions.Y, m_size.Z / m_partitions.Y);
		Vector3 iterator;

		for (iterator.X = minExtent.X; iterator.X < maxExtent.X; iterator.X+=stepSize.X)
		{
			for (iterator.Y = minExtent.Y; iterator.Y < maxExtent.Y; iterator.Y+=stepSize.Y)
			{
				for (iterator.Z = minExtent.Z; iterator.Z < maxExtent.Z; iterator.Z+=stepSize.Z)
				{
					std::vector<T*> list = Get(iterator);
					for (auto point : list)
					{
						if (Vector3::Distance(point->Position, p_centre) < p_fRadius)
							return true;
					}
				}
			}
		}

		return false;
	}

	size_t Size(void) { 
		return m_pointList.size();		
	}
};