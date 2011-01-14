//----------------------------------------------------------------------------------------------
//	Filename:	DistributedRenderer.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "boost/mpi.hpp"
namespace mpi = boost::mpi;

#include "Renderer/Renderer.h"
#include "Geometry/Vector2.h"
#include "Image/Image.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class DistributedRenderer : public IRenderer
		{
		protected:			
			IIntegrator *m_pIntegrator;
			ICamera *m_pCamera;
			IDevice *m_pDevice;
			IFilter *m_pFilter;
			Scene *m_pScene;

			int m_nTileWidth,
				m_nTileHeight;

			int m_nSampleCount;

			// MPI Context information
			mpi::environment* m_pMPIEnvironment;
			mpi::communicator* m_pMPICommunicator;
		
		public:
			DistributedRenderer(Scene *p_pScene, ICamera *p_pCamera, IIntegrator *p_pIntegrator, 
					IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount = 1, int p_nTileWidth = 8, int p_nTileHeight = 8);

			bool Initialise(void);
			bool Shutdown(void);

			void Render(void);
			void RenderDebug(void);
		};

		class Region
		{
		public:
			Vector2 RegionStart,
				RegionEnd;
		public:
			Region(void) { }
			
			Region(const Vector2 &p_regionStart, const Vector2 &p_regionEnd)
				: RegionStart(p_regionStart)
				, RegionEnd(p_regionEnd)
			{ }
			
			Region(const Region &p_tile) 
				: RegionStart(p_tile.RegionStart)
				, RegionEnd(p_tile.RegionEnd)
			{ }

			float GetWidth(void) const
			{
				return RegionEnd.X - RegionStart.X;
			}

			float GetHeight(void) const
			{
				return RegionEnd.Y - RegionStart.Y;
			}
		};

		class Tile
		{
		protected:
			char  *m_pSerializationBuffer;
			int    m_nSerializationBufferSize;

			Image *m_pImageData;

		public:
			Tile(int p_nId, int p_nWidth, int p_nHeight)
			{ 
				m_nSerializationBufferSize = p_nWidth * p_nHeight * sizeof(RGBPixel) + sizeof(int);
				m_pSerializationBuffer = new char[m_nSerializationBufferSize];
				m_pImageData = new Image(p_nWidth, p_nHeight, (RGBPixel*)(m_pSerializationBuffer + sizeof(int)));
			}

			~Tile(void)
			{
				delete m_pImageData;
				delete[] m_pSerializationBuffer;
			}

			int GetId(void) 
			{
				return *(int*)m_pSerializationBuffer;
			}

			void SetId(int p_nId)
			{
				*((int*)m_pSerializationBuffer) = p_nId;
			}

			Image* GetImageData(void)
			{
				return m_pImageData;
			}

			char* GetSerializationBuffer(void) const
			{
				return m_pSerializationBuffer;
			}

			int GetSerializationBufferSize(void) const
			{
				return m_nSerializationBufferSize;
			}
		};
	}
}