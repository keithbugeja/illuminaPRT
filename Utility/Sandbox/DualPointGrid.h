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
		std::cout << "DualPointGrid::Build : Generating flat grid structure..." << std::endl;

		
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
						currentIndex += m_context.m_pointMap[key].size();

						for (auto point : m_context.m_pointMap[key])
							point->Pack(&(m_context.m_gridElementList));
					}
				}
			}
		}

		m_context.m_gridIndexList.push_back(currentIndex);


		std::cout << "DualPointGrid::Build : Indexed [" << currentIndex << "] samples in [" << m_context.m_gridIndexList.size() << "] cells." << std::endl;
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
	void PackByFilter(DualPointGridFilter<T> *p_pFilter) 
	{
		for (auto key : p_pFilter->m_keyList)
		{
			//std::cout << "Key : " << std::hex << key << std::dec
			//	<< ", Index : " << m_context.m_gridIndexMap[key] << std::endl;

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
	void Pack(void) 
	{
		for (int key = 0; key < 32 * 32 * 32; key++)
		{
			//std::cout << "Key : " << std::hex << key << std::dec
				//<< ", Index : " << m_context.m_gridIndexMap[key] << std::endl;

			float *data = m_context.m_gridElementList.data() + m_context.m_gridIndexMap[key] * T::GetPackedSize();

			for (auto point : m_context.m_pointMap[key])
			{
				data = point->PackUpdate(data);
			}
		}
	}

	/*
	 * Serialise whole grid
	 */
	void Serialize(std::vector<float> *m_pGridElements, std::vector<int> *m_pGridIndices)
	{
		m_pGridElements->resize(m_context.m_gridElementList.size());
		std::copy(m_context.m_gridElementList.begin(), m_context.m_gridElementList.end(), m_pGridElements->begin());

		m_pGridIndices->resize(m_context.m_gridIndexList.size());
		std::copy(m_context.m_gridIndexList.begin(), m_context.m_gridIndexList.end(), m_pGridIndices->begin());
	}
};

/*
class GPUGrid
{
protected:
	Vector3 m_minExtents,
		m_maxExtents,
		m_size;

	float m_edgeSize,
		m_cellSize;

	int m_subdivisions;

	std::vector<Dart*> m_sampleList;
	std::map<int, std::vector<Dart*>> m_grid;
	std::map<int, int> m_gridIndices;

public:
	Vector3 GetOrigin(void) const {
		return m_minExtents;
	}

	float GetCellSize(void) const {
		return m_cellSize;
	}

	int GetSubdivisions(void) const {
		return m_subdivisions;
	}

protected:
	int MakeKey(const Vector3 &p_position)
	{
		Vector3 offset = (p_position - m_minExtents) / m_cellSize;
		
		int x = (int)offset.X, 
			y = (int)offset.Y,
			z = (int)offset.Z;

		return (int)( (x & 0x1F) | ((y & 0x1F) << 5) | ((z & 0x1F) << 10) ); 
	}

	int MakeKey(int x, int y, int z)
	{
		return (int)( (x & 0x1F) | ((y & 0x1F) << 5) | ((z & 0x1F) << 10) ); 
	}

	void Add(int p_key, Dart *p_pSample)
	{
		if (m_grid.find(p_key) == m_grid.end())
			m_grid[p_key] = std::vector<Dart*>();

		m_grid[p_key].push_back(p_pSample);
	}

	// Optimisation : Only put points in cells that intersect geometry!
	void AddRange(const Vector3 &p_position, float p_fRadius, Dart *p_pSample)
	{
		Vector3 extent(p_fRadius);

		Vector3 minExtent = p_position - extent,
			maxExtent = p_position + extent,
			posIter = Vector3::Zero;

		for (posIter.Z = minExtent.Z; posIter.Z <= maxExtent.Z; posIter.Z += m_cellSize)
		{
			for (posIter.Y = minExtent.Y; posIter.Y <= maxExtent.Y; posIter.Y += m_cellSize)
			{
				for (posIter.X = minExtent.X; posIter.X <= maxExtent.X; posIter.X += m_cellSize)
				{
					Add(MakeKey(posIter), p_pSample);
				}
			}
		}
	}

public:
	void Build(std::vector<Dart*> &p_sampleList, int p_subdivisions, float p_searchRadius)
	{
		std::cout << "GPUGrid :: Estimating dimensions..." << std::endl;

		// Clear grid with sample lists
		m_grid.clear();

		// Set the grid extents
		m_minExtents = Vector3(Maths::Maximum);
		m_maxExtents = -Maths::Maximum;
	
		for(auto sample : p_sampleList)
		{
			m_maxExtents = Vector3::Max(m_maxExtents, sample->Position);
			m_minExtents = Vector3::Min(m_minExtents, sample->Position);

			m_sampleList.push_back(sample);
		}

		std::cout << "GPUGrid :: Extents [" << m_minExtents.ToString() << " - " << m_maxExtents.ToString() << "]" << std::endl;
		std::cout << "GPUGrid :: Inserting irradiance sample placeholders..." << std::endl;

		// Compute the grid parameters
		m_subdivisions = p_subdivisions;
		m_size = m_maxExtents - m_minExtents;
		m_edgeSize = m_size.MaxAbsComponent();
		m_cellSize = m_edgeSize / m_subdivisions;

		// Modify grid to fit cube
		m_size.Set(m_edgeSize, m_edgeSize, m_edgeSize);
		m_maxExtents = m_minExtents + m_size;

		// Fill grid
		for (auto sample : m_sampleList)
		{
			AddRange(sample->Position, p_searchRadius + m_cellSize * 0.5f, sample);
		}

		// By now, all samples exist in their respective cells, so we index
		// the starting element in each cell.
		std::cout << "GPUGrid :: Computing flat sample indices..." << std::endl;

		int currentIndex = 0, key;

		m_gridIndices.clear();

		for (int z = 0; z < 0x1f; z++)
		{
			for (int y = 0; y < 0x1f; y++)
			{
				for (int x = 0; x < 0x1f; x++)
				{
					key = MakeKey(x, y, z);

					m_gridIndices[key] = currentIndex;

					if (m_grid.find(key) != m_grid.end()) 
						currentIndex += m_grid[key].size();
				}
			}
		}

		std::cout << "GPUGrid :: Indexed [" << currentIndex << "] samples in [" << m_gridIndices.size() << "] cells" << std::endl;
	}

	void FilterByView(const ICamera *p_pCamera, FilteredGPUGrid *p_pFilteredGrid)
	{
		// First we get 4 corner rays from which to build frustum planes.
		Ray topLeft = p_pCamera->GetRay(0,0,0,0),
			topRight = p_pCamera->GetRay(1,0,1,0),
			bottomLeft = p_pCamera->GetRay(0,1,0,1),
			bottomRight = p_pCamera->GetRay(1,1,1,1),
			centre(p_pCamera->GetObserver(), p_pCamera->GetFrame().W);

		float in[5], out[5];
		AxisAlignedBoundingBox aabb(m_minExtents, m_maxExtents);

		aabb.Intersects(topLeft, in[0], out[0]);
		aabb.Intersects(topRight, in[1], out[1]);
		aabb.Intersects(bottomLeft, in[2], out[2]);
		aabb.Intersects(bottomRight, in[3], out[3]);
		aabb.Intersects(centre, in[4], out[4]);

		Vector3 frustumVerts[6] = {	
			topLeft.PointAlongRay(out[0]),
			topRight.PointAlongRay(out[1]),
			bottomLeft.PointAlongRay(out[2]),
			bottomRight.PointAlongRay(out[3]),
			centre.PointAlongRay(out[4]),
			p_pCamera->GetObserver() 
		};

		// Build frustum planes
		//Plane left(frustumVerts[0], frustumVerts[2], frustumVerts[4]),
		//	right(frustumVerts[1], frustumVerts[3], frustumVerts[4]),
		//	top(frustumVerts[0], frustumVerts[1], frustumVerts[4]),
		//	bottom(frustumVerts[2], frustumVerts[3], frustumVerts[4]);

		Plane left(frustumVerts[0], frustumVerts[2], frustumVerts[4]),
			right(frustumVerts[4], frustumVerts[3], frustumVerts[1]),
			top(frustumVerts[0], frustumVerts[1], frustumVerts[4]),
			bottom(frustumVerts[2], frustumVerts[3], frustumVerts[4]);

		// Compute aabb for frustum
		AxisAlignedBoundingBox frustum;
		frustum.ComputeFromPoints((Vector3*)frustumVerts, 6);

		Vector3 frustumMinExtent = frustum.GetMinExtent(),
			frustumMaxExtent = frustum.GetMaxExtent(),
			frustumIter = Vector3::Zero;

		std::cout << "Frustum bounds : " << frustumMinExtent.ToString() << " - " << frustumMaxExtent.ToString() << std::endl;

		// Sync inner frustum to grid cells
		Vector3 alignedCellStart = (frustumMinExtent - m_minExtents) / m_cellSize,
			alignedCellSize(m_cellSize);

		alignedCellStart.Set(((int)alignedCellStart.X) * m_cellSize, 
			((int)alignedCellStart.Y) * m_cellSize,
			((int)alignedCellStart.Z) * m_cellSize);
		
		alignedCellStart += m_minExtents;

		AxisAlignedBoundingBox cell;
		int cellcount = 0;

		p_pFilteredGrid->SetGlobalSampleList(&m_sampleList);
		p_pFilteredGrid->FilteredSampleSet.clear();

		for (frustumIter.Z = frustumMinExtent.Z; frustumIter.Z < frustumMaxExtent.Z + m_cellSize * 0.5f; frustumIter.Z += m_cellSize)
		{
			Vector3 alignedCellStart4Y = alignedCellStart;
			for (frustumIter.Y = frustumMinExtent.Y; frustumIter.Y < frustumMaxExtent.Y + m_cellSize * 0.5f; frustumIter.Y += m_cellSize)
			{
				Vector3 alignedCellStart4X = alignedCellStart4Y;
				for (frustumIter.X = frustumMinExtent.X; frustumIter.X < frustumMaxExtent.X + m_cellSize * 0.5f; frustumIter.X += m_cellSize)
				{
					int key = MakeKey(frustumIter);

					cell.SetExtents(alignedCellStart4X, alignedCellStart4X + alignedCellSize);
					alignedCellStart.X += m_cellSize;
					
					if (m_grid.find(key) != m_grid.end())
					{
						p_pFilteredGrid->MarkCell(key);
						
						for (auto sample : m_grid[MakeKey(frustumIter)])
							p_pFilteredGrid->FilteredSampleSet.insert(sample);
					}

					cellcount++;
				}

				alignedCellStart4Y.Y += m_cellSize;
			}

			alignedCellStart.Z += m_cellSize;
		}

		// TODO: Use an LDS to selectively sample points
		// Should we not provide a list of affected cells instead?

		std::cout << "Considered [" << cellcount << " of " << 32 * 32 * 32 << "]" << std::endl;	
		std::cout << "Filtered Samples [" << p_pFilteredGrid->FilteredSampleSet.size() << "]" << std::endl;
	}

	void Serialize(FilteredGPUGrid *p_pFilteredGrid, std::vector<Spectrum> &p_irradianceList, std::vector<int> &p_indexList)
	{
		std::vector<int> *pMarkedCells = p_pFilteredGrid->GetMarkedCells();

		for (auto cell : *pMarkedCells)
		{
			p_indexList.push_back(m_gridIndices[cell]);
			p_indexList.push_back(m_grid[cell].size());

			for (auto e : m_grid[cell])
			{
				p_irradianceList.push_back(e->Irradiance);
			}
		}

		std::cout << "Cells :: [" << p_indexList.size() << "]" << std::endl;
		std::cout << "Irradiance Records :: [" << p_irradianceList.size() << "]" << std::endl;
	}
};
*/
