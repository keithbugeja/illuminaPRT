//----------------------------------------------------------------------------------------------
//	Filename:	DualPointGrid.h
//	Author:		Keith Bugeja
//	Date:		6/12/2013
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <Maths/Montecarlo.h>
#include <Scene/Visibility.h>
#include "PointGrid.h"
#include "PointShader.h"
#include "Environment.h"
#include "MultithreadedCommon.h"

#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
// DualPointGridContext
// Context for sharing grid properties with filters
//----------------------------------------------------------------------------------------------
template <class T>
class DualPointGridContext
{
public:
	// Store points
	std::vector<T*> m_pointList;
	
	// Store hashed grid
	std::map<int, std::vector<T*>> m_pointMap;

	// Store point to flat grid map (point id -> flat grid positions)
	std::map<int, std::vector<int>> m_pointGridElementMap;

	// Store flat grid
	std::vector<float> m_gridElementList;
	
	// Store cell indices for flat grid
	std::vector<int> m_gridIndexList;
	std::map<int, int> m_gridIndexMap;

	AxisAlignedBoundingBox m_gridBoundingBox;

	Vector3 m_gridOrigin,
		m_gridExtents,
		m_gridSize;

	float m_fGridCellSize,
		m_fGridEdgeLength;
};

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

template<class T>
class DualPointGridFilter
{
	template <class T> friend class DualPointGrid;

protected:
	DualPointGridContext<T> *m_pContext;
	std::vector<int> m_keyList;

public:
	void AddToFilter(int p_nKey) {
		m_keyList.push_back(p_nKey);
	}

	std::vector<T*>* GetGlobalPointList(void) { 
		return &(m_pContext->m_pointList);
	}

	void GetFilteredPoints(std::vector<std::vector<T*>*> &p_pointLists)
	{
		p_pointLists.clear();

		for (auto key : m_keyList) {
			p_pointLists.push_back(&(m_pContext->m_pointMap[key]));
		}
	}
};

/*
 * Uses a two-way point grid representation to ease the mixture of CPU-GPU operations
 */
template <class T>
class DualPointGrid
{
	template <class T> friend class DualPointGridFilter;

protected:

	// Grid subdivisions / key generation parameters are constants
	static const int CellShift = 5;
	static const int CellMask = 0x1F;
	static const int CellSubdivisions = 32;

	DualPointGridContext<T> m_context;

	int m_nPointReferenceCount;

protected:
	/*
	 * Bind filter by sharing context
	 */
	void BindFilter(DualPointGridFilter<T> *p_pFilter)
	{
		p_pFilter->m_keyList.clear();
		p_pFilter->m_pContext = &m_context;
	}

	inline int MakeKey(const Vector3 &p_position)
	{
		Vector3 offset = (p_position - m_context.m_gridOrigin) / m_context.m_fGridCellSize;
		
		int x = (int)offset.X, 
			y = (int)offset.Y,
			z = (int)offset.Z;

		return (int)( (x & CellMask) | ((y & CellMask) << (CellShift)) | ((z & CellMask) << (CellShift << 1)) ); 
	}

	inline int MakeKey(int x, int y, int z)
	{
		return (int)( (x & CellMask) | ((y & CellMask) << (CellShift)) | ((z & CellMask) << (CellShift << 1)) ); 
	}

	void Add(int p_key, T *p_pPoint)
	{
		if (m_context.m_pointMap.find(p_key) == m_context.m_pointMap.end())
			m_context.m_pointMap[p_key] = std::vector<T*>();

		m_context.m_pointMap[p_key].push_back(p_pPoint);
	}

	// Optimisation : Only put points in cells that intersect geometry!
	void AddRange(const Vector3 &p_position, float p_fRadius, T *p_pPoint)
	{
		Vector3 extent(p_fRadius);

		Vector3 minExtent = p_position - extent,
			maxExtent = p_position + extent,
			posIter = Vector3::Zero;

		for (posIter.Z = minExtent.Z; posIter.Z <= maxExtent.Z; posIter.Z += m_context.m_fGridCellSize)
		{
			for (posIter.Y = minExtent.Y; posIter.Y <= maxExtent.Y; posIter.Y += m_context.m_fGridCellSize)
			{
				for (posIter.X = minExtent.X; posIter.X <= maxExtent.X; posIter.X += m_context.m_fGridCellSize)
				{
					Add(MakeKey(posIter), p_pPoint);
					m_nPointReferenceCount++;
				}
			}
		}
	}

public:
	Vector3 GetOrigin(void) const {
		return m_context.m_gridOrigin;
	}

	float GetCellSize(void) const {
		return m_context.m_fGridCellSize;
	}

	int GetCellSubdivisions(void) const {
		return CellSubdivisions;
	}

public:
	DualPointGrid(void)
		: m_nPointReferenceCount(0)
	{ }

	/*
	 * Build the dual grid using the provided point list and search radius.
	 * Note that the construction uses multiple references per sample if the 
	 * area of influence of the latter covers a number of cells.
	 */ 
	void Build(std::vector<T*> &p_pointList, float p_fSearchRadius)
	{
		std::cout << "DualPointGrid::Build : Estimating grid dimensions..." << std::endl;

		// Invalidate AABB
		m_context.m_gridBoundingBox.Invalidate();

		// Invalidate extents
		m_context.m_gridOrigin = Maths::Maximum;
		m_context.m_gridExtents = -Maths::Maximum;

		// Invalidate internal point list
		m_context.m_pointList.clear();

		// We prefer the following method instead of the AABB's Union method 
		// to avoid internal state update on each call.
		for (auto point : p_pointList)
		{
			m_context.m_gridOrigin = Vector3::Min(m_context.m_gridOrigin, point->Position);
			m_context.m_gridExtents = Vector3::Max(m_context.m_gridExtents, point->Position);
		
			// Push sample into internal list
			m_context.m_pointList.push_back(point);
		}
		

		std::cout << "DualPointGrid::Build : Extents [" << m_context.m_gridOrigin.ToString() << " - " << m_context.m_gridExtents.ToString() << "]" << std::endl;
		std::cout << "DualPointGrid::Build : Normalising grid..." << std::endl;


		// Compute grid size
		m_context.m_gridSize = m_context.m_gridExtents - m_context.m_gridOrigin;
		m_context.m_fGridEdgeLength = m_context.m_gridSize.MaxAbsComponent();
		m_context.m_fGridCellSize = m_context.m_fGridEdgeLength / CellSubdivisions;

		// Modify grid to fit cube
		m_context.m_gridSize = m_context.m_fGridEdgeLength;
		m_context.m_gridExtents = m_context.m_gridOrigin + m_context.m_gridSize;

		// Update grid AABB extents
		m_context.m_gridBoundingBox.SetExtents(m_context.m_gridOrigin, m_context.m_gridExtents);

		
		std::cout << "DualPointGrid::Build : Grid normalised with extents [" << m_context.m_gridOrigin.ToString() << " - " << m_context.m_gridExtents.ToString() << "]" << std::endl;
		std::cout << "DualPointGrid::Build : Populating normalised grid..." << std::endl;


		// Invalidate point map
		m_context.m_pointMap.clear();

		// Fill normalised grid (note : can improve by adding points only in cells containing geometry)
		for (auto point : m_context.m_pointList)
		{
			AddRange(point->Position, p_fSearchRadius + m_context.m_fGridCellSize * 0.5f, point);
		}


		std::cout << "DualPointGrid::Build : Normalised grid populated with [" << m_context.m_pointList.size() << "] unique points and [" << m_nPointReferenceCount << "] references." << std::endl;
		std::cout << "DualPointGrid::Build : Packing grid..." << std::endl;

		Pack();

		std::cout << "DualPointGrid::Build : Build complete." << std::endl;
	}

	/* 
	 * Filter the point set by view frustum.
	 */
	void FilterByView(const ICamera *p_pCamera, DualPointGridFilter<T> *p_pFilter)
	{
		// First we get 4 corner rays from which to build frustum planes.
		Ray topLeft = p_pCamera->GetRay(0,0,0,0),
			topRight = p_pCamera->GetRay(1,0,1,0),
			bottomLeft = p_pCamera->GetRay(0,1,0,1),
			bottomRight = p_pCamera->GetRay(1,1,1,1),
			centre(p_pCamera->GetObserver(), p_pCamera->GetFrame().W);

		// Intersect with bounding box
		float in[5], out[5];

		m_context.m_gridBoundingBox.Intersects(topLeft, in[0], out[0]);
		m_context.m_gridBoundingBox.Intersects(topRight, in[1], out[1]);
		m_context.m_gridBoundingBox.Intersects(bottomLeft, in[2], out[2]);
		m_context.m_gridBoundingBox.Intersects(bottomRight, in[3], out[3]);
		m_context.m_gridBoundingBox.Intersects(centre, in[4], out[4]);

		// Use intersection points to generate frustum
		Vector3 frustumVerts[6] = 
		{	
			topLeft.PointAlongRay(out[0]),
			topRight.PointAlongRay(out[1]),
			bottomLeft.PointAlongRay(out[2]),
			bottomRight.PointAlongRay(out[3]),
			centre.PointAlongRay(out[4]),
			p_pCamera->GetObserver() 
		};

		// Compute aabb for frustum
		AxisAlignedBoundingBox frustum;
		frustum.ComputeFromPoints((Vector3*)frustumVerts, 6);

		Vector3 frustumOrigin = frustum.GetMinExtent(), 
			frustumExtents = frustum.GetMaxExtent(), 
			frustumIter = Vector3::Zero;

		std::cout << "Frustum bounds : " << frustumOrigin.ToString() << " - " << frustumExtents.ToString() << std::endl;

		/*
		// Sync inner frustum to grid cells
		Vector3 alignedCellStart = (frustumOrigin - m_gridOrigin) / m_fGridCellSize,
			alignedCellSize(m_fGridCellSize);

		alignedCellStart.Set(((int)alignedCellStart.X) * m_fGridCellSize, 
			((int)alignedCellStart.Y) * m_fGridCellSize,
			((int)alignedCellStart.Z) * m_fGridCellSize);
		
		alignedCellStart += m_gridOrigin;
		*/

		// Populate filtered grid
		// AxisAlignedBoundingBox cell; 
		int cellcount = 0;

		// Bind filter
		BindFilter(p_pFilter);

		// Iterate through grid
		for (frustumIter.Z = frustumOrigin.Z; frustumIter.Z < frustumExtents.Z + m_context.m_fGridCellSize * 0.5f; frustumIter.Z += m_context.m_fGridCellSize)
		{
			for (frustumIter.Y = frustumOrigin.Y; frustumIter.Y < frustumExtents.Y + m_context.m_fGridCellSize * 0.5f; frustumIter.Y += m_context.m_fGridCellSize)
			{
				for (frustumIter.X = frustumOrigin.X; frustumIter.X < frustumExtents.X + m_context.m_fGridCellSize * 0.5f; frustumIter.X += m_context.m_fGridCellSize)
				{
					int key = MakeKey(frustumIter);
				
					// If grid cell exists, we mark it
					if (m_context.m_pointMap.find(key) != m_context.m_pointMap.end())
						p_pFilter->AddToFilter(key);

					cellcount++;
				}
			}
		}


		std::cout << "Considered [" << cellcount << " of " << CellSubdivisions * CellSubdivisions * CellSubdivisions << "]" << std::endl;	
		// std::cout << "Filtered Samples [" << p_pFilteredGrid->FilteredSampleSet.size() << "]" << std::endl;
	}

	/*
	 *
	 */
	void Pack(void)
	{
		// Initialise index
		int currentIndex = 0, key;

		// Clear flat grid
		m_context.m_gridIndexMap.clear();
		m_context.m_gridIndexList.clear();
		m_context.m_gridElementList.clear();

		for (int z = 0; z < CellSubdivisions; z++)
		{
			for (int y = 0; y < CellSubdivisions; y++)
			{
				for (int x = 0; x < CellSubdivisions; x++)
				{
					// Make key for current cell
					key = MakeKey(x, y, z);

					// Add current point to cell array position
					m_context.m_gridIndexMap[key] = currentIndex;
					m_context.m_gridIndexList.push_back(currentIndex);

					// If cell is not empty...
					if (m_context.m_pointMap.find(key) != m_context.m_pointMap.end()) 
					{
						// currentIndex += m_context.m_pointMap[key].size();

						for (auto point : m_context.m_pointMap[key])
						{
							m_context.m_pointGridElementMap[point->UniqueId].push_back(currentIndex++);
							
							point->PackAdd(&(m_context.m_gridElementList));
						}
					}
				}
			}
		}

		m_context.m_gridIndexList.push_back(currentIndex);

		std::cout << "DualPointGrid::Pack : Indexed [" << currentIndex << "] samples in [" << m_context.m_gridIndexList.size() << "] cells." << std::endl;
	}

	/*
	 * Packs samples into pre-existing GPU grid
	 */
	void PackUpdate(void) 
	{
		for (int key = 0; key < 32 * 32 * 32; key++)
		{
			float *data = (float*)(m_context.m_gridElementList.data()) + m_context.m_gridIndexMap[key] * T::GetPackedSize();

			for (auto point : m_context.m_pointMap[key])
			{
				// point->Irradiance.Set(key >> 10, (key >> 5) & 31, key & 31);
				data = point->PackUpdate(data);
			}
		}
	}

	/*
	 * 
	 */
	void PackByFilter(DualPointGridFilter<T> *p_pFilter) 
	{
		for (auto key : p_pFilter->m_keyList)
		{
			float *data = m_context.m_gridElementList.data() + m_context.m_gridIndexMap[key] * T::GetPackedSize();

			for (auto point : m_context.m_pointMap[key])
			{
				data = point->PackUpdate(data);
			}
		}
	}

	/*
	 *
	 */ 
	void SerializeUniqueByFilter(DualPointGridFilter<T> *p_pFilter, std::vector<int> *p_pIndices, std::vector<int> *p_pColours)
	{
		p_pIndices->clear();
		p_pColours->clear();

		std::map<int, int> colourMap;

		int r, g, b, colour;

		for (auto key : p_pFilter->m_keyList)
		{
			for (auto point : m_context.m_pointMap[key])
			{
				r = (int)(point->Irradiance[0] / (point->Irradiance[0] + 1) * 255.f);
				g = (int)(point->Irradiance[1] / (point->Irradiance[1] + 1) * 255.f);
				b = (int)(point->Irradiance[2] / (point->Irradiance[2] + 1) * 255.f);

				colour = r << 16 | g << 8 | b;

				// point->Irradiance.Set(0, 0, 1);

				/*
				colourMap[point->UniqueId] = 
					((int)(point->Irradiance[0] / (point->Irradiance[0] + 1) * 255.0f)) << 16 +
					((int)(point->Irradiance[1] / (point->Irradiance[1] + 1) * 255.0f)) << 8 +
					((int)(point->Irradiance[2] / (point->Irradiance[2] + 1) * 255.0f));
					*/

				colourMap[point->UniqueId] = colour; //0xFFFFFF;
			}
		}

		for (auto pointPair : colourMap)
		{
			p_pIndices->push_back(pointPair.first);
			p_pColours->push_back(pointPair.second);
		}
	}


	/*
	 *
	 */ 
	void SerializeByFilter(DualPointGridFilter<T> *p_pFilter, std::vector<float> *p_pElements, std::vector<int> *p_pIndices)
	{
		p_pElements->clear();
		p_pIndices->clear();

		for (auto key : p_pFilter->m_keyList)
		{
			int offset = m_context.m_gridIndexMap[key] * T::GetPackedSize(),
				count = 0;

			p_pIndices->push_back(offset);

			for (auto point : m_context.m_pointMap[key])
			{
				point->PackUpdateAdd(p_pElements);
				count++;
			}

			p_pIndices->push_back(count);
		}
	}

	/*
	 * Serialise whole grid
	 */
	void Serialize(std::vector<float> *p_pGridElements, std::vector<int> *p_pGridIndices)
	{
		p_pGridElements->resize(m_context.m_gridElementList.size());
		std::copy(m_context.m_gridElementList.begin(), m_context.m_gridElementList.end(), p_pGridElements->begin());

		p_pGridIndices->resize(m_context.m_gridIndexList.size());
		std::copy(m_context.m_gridIndexList.begin(), m_context.m_gridIndexList.end(), p_pGridIndices->begin());
	}

	/*
	 * Serialise whole grid
	 */
	void Serialize(std::vector<float> *p_pGridElements, std::vector<int> *p_pGridIndices, std::vector<int> *p_pSampleIndices, std::vector<int> *p_pSamplePositions)
	{
		p_pGridElements->resize(m_context.m_gridElementList.size());
		std::copy(m_context.m_gridElementList.begin(), m_context.m_gridElementList.end(), p_pGridElements->begin());

		p_pGridIndices->resize(m_context.m_gridIndexList.size());
		std::copy(m_context.m_gridIndexList.begin(), m_context.m_gridIndexList.end(), p_pGridIndices->begin());

		for (auto entry : m_context.m_pointGridElementMap)
		{
			// Index sample and number of entries
			p_pSampleIndices->push_back(entry.first);
			p_pSampleIndices->push_back(entry.second.size());

			for (auto position : entry.second)
				p_pSamplePositions->push_back(position);
		}
	}
};