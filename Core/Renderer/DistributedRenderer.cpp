//----------------------------------------------------------------------------------------------
//	Filename:	BasicRenderer.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <vector>

#include "boost/progress.hpp"
#include "boost/mpi.hpp"

#include "Renderer/DistributedRenderer.h"
#include "Integrator/Integrator.h"
#include "Geometry/Intersection.h"
#include "Spectrum/Spectrum.h"
#include "Camera/Camera.h"
#include "Device/Device.h"
#include "Staging/Scene.h"

#include "Sampler/Sampler.h"
#include "Filter/Filter.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
DistributedRenderer::DistributedRenderer(Scene *p_pScene, ICamera *p_pCamera, IIntegrator *p_pIntegrator, 
					IDevice *p_pDevice, IFilter *p_pFilter, int p_nSampleCount, int p_nTileWidth, int p_nTileHeight)
	: m_pScene(p_pScene)
	, m_pCamera(p_pCamera)
	, m_pIntegrator(p_pIntegrator)
	, m_pDevice(p_pDevice)
	, m_pFilter(p_pFilter)
	, m_nSampleCount(p_nSampleCount)
	, m_nTileWidth(p_nTileWidth)
	, m_nTileHeight(p_nTileHeight)
{ 
}
//----------------------------------------------------------------------------------------------
bool DistributedRenderer::Initialise(void)
{
	m_pMPIEnvironment = new mpi::environment();
	m_pMPICommunicator = new mpi::communicator();

	std::cout << "Initialised peer " << m_pMPICommunicator->rank() << "..." << std::endl;

	m_pMPICommunicator->barrier();

	return true;
}
//----------------------------------------------------------------------------------------------
bool DistributedRenderer::Shutdown(void)
{
	m_pMPICommunicator->barrier();

	delete m_pMPICommunicator;
	delete m_pMPIEnvironment;

	return true;
}
//----------------------------------------------------------------------------------------------
void DistributedRenderer::RenderDebug(void)
{
	int deviceWidth = m_pDevice->GetWidth(),
		deviceHeight = m_pDevice->GetHeight();

	int tilesPerRow = deviceWidth / m_nTileWidth,
		tilesPerColumn = deviceHeight / m_nTileHeight,
		tilesPerScreen = tilesPerRow * tilesPerColumn;

	int tileId = -1;

	// Create tile for use within communicator
	Tile tile(0, m_nTileWidth, m_nTileHeight);

	//--------------------------------------------------
	// Prepare task queue
	//--------------------------------------------------
	std::vector<int> m_taskQueue;

	// generate tiles for rendering
	for (int taskId = 0; taskId < tilesPerScreen; ++taskId)
	{
		m_taskQueue.push_back(taskId);
	}

	//--------------------------------------------------
	// Prepare structures for use in rendering
	//--------------------------------------------------
	Intersection intersection;
	Vector2 *pSampleBuffer = new Vector2[m_nSampleCount];

	while(true)
	{
		// Send a new task, if available
		if (m_taskQueue.size() > 0)
		{
			tileId = m_taskQueue.back(); m_taskQueue.pop_back();

			//--------------------------------------------------
			// We have task id - render
			//--------------------------------------------------
			int startTileX = tileId % tilesPerRow,
				startTileY = tileId / tilesPerRow,
				startPixelX = startTileX * m_nTileWidth,
				startPixelY = startTileY * m_nTileHeight;
		
			std::cout << "Tile region : [" << startPixelX << ", " << startPixelY << "] - [" << startPixelX + m_nTileWidth << ", " << startPixelY + m_nTileHeight << "]" << std::endl;

			for (int y = 0; y < m_nTileHeight; y++)
			{
				for (int x = 0; x < m_nTileWidth; x++)
				{
					// Prepare ray samples
					m_pScene->GetSampler()->Get2DSamples(pSampleBuffer, m_nSampleCount);
					(*m_pFilter)(pSampleBuffer, m_nSampleCount);

					// Radiance
					Spectrum Li = 0;

					for (int sample = 0; sample < m_nSampleCount; sample++)
					{
						Ray ray = m_pCamera->GetRay(
							((float)(startPixelX + x)) / deviceWidth, 
							((float)(startPixelY + y)) / deviceHeight, 
							pSampleBuffer[sample].U, pSampleBuffer[sample].V);

						Li += m_pIntegrator->Radiance(m_pScene, ray, intersection);
					}

					Li = Li / m_nSampleCount;
			
					tile.GetImageData()->Set(x, y, RGBPixel(Li[0], Li[1], Li[2]));
				}
			} 

			//--------------------------------------------------
			// Send back result
			//--------------------------------------------------
			tile.SetId(tileId);
		}
		else
			tileId = -1;

		// Aggregate result to buffer
		int startTileX = tile.GetId() % tilesPerRow,
			startTileY = tile.GetId() / tilesPerRow,
			startPixelX = startTileX * m_nTileWidth,
			startPixelY = startTileY * m_nTileHeight;

		for (int y = 0; y < m_nTileHeight; y++)
		{
			for (int x = 0; x < m_nTileWidth; x++)
			{
				const RGBPixel &pixel = tile.GetImageData()->Get(x,y);
				Spectrum L(pixel.R, pixel.G, pixel.B);

				m_pDevice->Set(deviceWidth - startPixelX - x - 1, deviceHeight - startPixelY - y - 1, L);
			}
		}

		if (tileId == -1) break;
	}

	//--------------------------------------------------
	// Frame completed
	//--------------------------------------------------
	m_pDevice->EndFrame();

	delete[] pSampleBuffer;
}
//----------------------------------------------------------------------------------------------
void DistributedRenderer::Render(void)
{
	const int WI_RESULT	= 0x0001;
	const int WI_REQUEST = 0x0002;
	const int WI_TASKID = 0x0003;

	int deviceWidth = m_pDevice->GetWidth(),
		deviceHeight = m_pDevice->GetHeight();

	int tilesPerRow = deviceWidth / m_nTileWidth,
		tilesPerColumn = deviceHeight / m_nTileHeight,
		tilesPerScreen = tilesPerRow * tilesPerColumn;

	int tileId = -1;

	// Create tile for use within communicator
	Tile tile(0, m_nTileWidth, m_nTileHeight);

	//--------------------------------------------------
	// Synchronise all processes
	//--------------------------------------------------
	//std::cout << "Process " << m_pMPICommunicator->rank() << " at start barrier..." << std::endl;
	m_pMPICommunicator->barrier();
	//std::cout << "Process " << m_pMPICommunicator->rank() << " past start barrier..." << std::endl;

	//--------------------------------------------------
	// Master processor should create task queue
	//--------------------------------------------------
	if (m_pMPICommunicator->rank() == 0)
	{
		boost::progress_display renderProgress(tilesPerScreen);
		boost::timer renderTimer;
		renderTimer.restart();

		//--------------------------------------------------
		// Prepare device for rendering
		//--------------------------------------------------
		m_pDevice->BeginFrame();

		//--------------------------------------------------
		// Prepare task queue
		//--------------------------------------------------
		std::vector<int> m_taskQueue;

		// generate tiles for rendering
		for (int taskId = 0; taskId < tilesPerScreen; ++taskId)
		{
			m_taskQueue.push_back(taskId);
		}

		//--------------------------------------------------
		// Distribute initial workload to workers
		//--------------------------------------------------
		// std::cout << "Distributing initial workload..." << std::endl;

		for (int rank = 1; rank < m_pMPICommunicator->size(); rank++)
		{
			// Send a new task, if available
			if (m_taskQueue.size() > 0)
			{
				tileId = m_taskQueue.back(); m_taskQueue.pop_back();
				m_pMPICommunicator->send(rank, WI_TASKID, tileId);
			}
		}

		//--------------------------------------------------
		// Start request-response communication with 
		// worker processors
		//--------------------------------------------------
		int waiting = m_pMPICommunicator->size() - 1;

		while(waiting > 0)
		{
			//--------------------------------------------------
			// Receive request
			//--------------------------------------------------
			mpi::status status = m_pMPICommunicator->recv(mpi::any_source, WI_RESULT, tile.GetSerializationBuffer(), tile.GetSerializationBufferSize());

			//--------------------------------------------------
			// Worker is sending in result
			//--------------------------------------------------
			//std::cout << "Results from " << status.source() << "..." << std::endl;

			// Send a new task, if available
			if (m_taskQueue.size() > 0)
			{
				tileId = m_taskQueue.back(); m_taskQueue.pop_back();
				m_pMPICommunicator->send(status.source(), WI_TASKID, tileId);
			}
			else
			{
				tileId = -1; waiting--;
				m_pMPICommunicator->send(status.source(), WI_TASKID, tileId);

				//std::cout << "Workers waiting : " << waiting << std::endl;
			}

			// Aggregate result to buffer
			int startTileX = tile.GetId() % tilesPerRow,
				startTileY = tile.GetId() / tilesPerRow,
				startPixelX = startTileX * m_nTileWidth,
				startPixelY = startTileY * m_nTileHeight;

			for (int y = 0; y < m_nTileHeight; y++)
			{
				for (int x = 0; x < m_nTileWidth; x++)
				{
					const RGBPixel &pixel = tile.GetImageData()->Get(x,y);
					Spectrum L(pixel.R, pixel.G, pixel.B);

					m_pDevice->Set(deviceWidth - startPixelX - x - 1, deviceHeight - startPixelY - y - 1, L);
				}
			}

			++renderProgress;
		}

		//--------------------------------------------------
		// Frame completed
		//--------------------------------------------------
		m_pDevice->EndFrame();

		std::cout << "Total Render Time : " << renderTimer.elapsed() << " seconds" << std::endl;
	}
	else
	{
		//--------------------------------------------------
		// Prepare structures for use in rendering
		//--------------------------------------------------
		Intersection intersection;
		Vector2 *pSampleBuffer = new Vector2[m_nSampleCount];

		while (true)
		{
			//--------------------------------------------------
			// Receive tile
			//--------------------------------------------------
			mpi::status status = m_pMPICommunicator->recv(0, WI_TASKID, tileId);

			//--------------------------------------------------
			// If termination signal, stop
			//--------------------------------------------------
			if (tileId == -1) 
			{
				//std::cout << "Worker [" << m_pMPICommunicator->rank() << "] terminating ... " << std::endl;
				break;
			}
			else
			{
				//std::cout << "Worker [" << m_pMPICommunicator->rank() << "] has received tile [" << tileId << "] ... " << std::endl;

				//--------------------------------------------------
				// We have task id - render
				//--------------------------------------------------
				int startTileX = tileId % tilesPerRow,
					startTileY = tileId / tilesPerRow,
					startPixelX = startTileX * m_nTileWidth,
					startPixelY = startTileY * m_nTileHeight;
		
				//std::cout << "Tile region : [" << startPixelX << ", " << startPixelY << "] - [" << startPixelX + m_nTileWidth << ", " << startPixelY + m_nTileHeight << "]" << std::endl;

				for (int y = 0; y < m_nTileHeight; y++)
				{
					for (int x = 0; x < m_nTileWidth; x++)
					{
						// Prepare ray samples
						m_pScene->GetSampler()->Get2DSamples(pSampleBuffer, m_nSampleCount);
						(*m_pFilter)(pSampleBuffer, m_nSampleCount);

						// Radiance
						Spectrum Li = 0;

						for (int sample = 0; sample < m_nSampleCount; sample++)
						{
							Ray ray = m_pCamera->GetRay(
								((float)(startPixelX + x)) / deviceWidth, 
								((float)(startPixelY + y)) / deviceHeight, 
								pSampleBuffer[sample].U, pSampleBuffer[sample].V);

							Li += m_pIntegrator->Radiance(m_pScene, ray, intersection);
						}

						Li = Li / m_nSampleCount;
			
						tile.GetImageData()->Set(x, y, RGBPixel(Li[0], Li[1], Li[2]));
					}
				} 

				//--------------------------------------------------
				// Send back result
				//--------------------------------------------------
				tile.SetId(tileId);
				m_pMPICommunicator->send(0, WI_RESULT, tile.GetSerializationBuffer(), tile.GetSerializationBufferSize());
			}
		}

		delete[] pSampleBuffer;
	}

	//--------------------------------------------------
	// Render complete
	//--------------------------------------------------
	//std::cout << "Process " << m_pMPICommunicator->rank() << " at end barrier..." << std::endl;
	m_pMPICommunicator->barrier();
	//std::cout << "Process " << m_pMPICommunicator->rank() << " past end barrier..." << std::endl;
}
//----------------------------------------------------------------------------------------------
/*
void DistributedRenderer::Render(void)
{
	const int WI_RESULT	= 0x0001;
	const int WI_REQUEST = 0x0002;
	const int WI_TASKID = 0x0003;
	const int WI_TERMINATE = 0x0004;

	int deviceWidth = m_pDevice->GetWidth(),
		deviceHeight = m_pDevice->GetHeight();

	int tilesPerRow = deviceWidth / m_nTileWidth,
		tilesPerColumn = deviceHeight / m_nTileHeight,
		tilesPerScreen = tilesPerRow * tilesPerColumn;

	int tasksRemaining = tilesPerScreen,
		tileId = -1;

	// Create tile for use within communicator
	Tile tile(0, m_nTileWidth, m_nTileHeight);

	//--------------------------------------------------
	// Synchronise all processes
	//--------------------------------------------------
	m_pMPICommunicator->barrier();

	//--------------------------------------------------
	// Master processor should create task queue
	//--------------------------------------------------
	if (m_pMPICommunicator->rank() == 0)
	{
		boost::progress_display renderProgress(tilesPerScreen);

		//--------------------------------------------------
		// Prepare task queue
		//--------------------------------------------------
		std::vector<int> m_taskQueue;

		// generate tiles for rendering
		for (int taskId = 0; taskId < tilesPerScreen; ++taskId)
		{
			m_taskQueue.push_back(taskId);
		}

		//--------------------------------------------------
		// Prepare device for rendering
		//--------------------------------------------------
		m_pDevice->BeginFrame();

		//--------------------------------------------------
		// Start request-response communication with 
		// worker processors
		//--------------------------------------------------
		while(tasksRemaining > 0)
		{
			//--------------------------------------------------
			// Receive request
			//--------------------------------------------------
			mpi::status status = m_pMPICommunicator->recv(mpi::any_source, mpi::any_tag, tile.GetSerializationBuffer(), tile.GetSerializationBufferSize());

			//--------------------------------------------------
			// Identify request type
			//--------------------------------------------------
			switch (status.tag())
			{
				//--------------------------------------------------
				// Worker is requesting a new task
				//--------------------------------------------------
				case WI_REQUEST:
				{
					// Send a new task, if available
					if (m_taskQueue.size() > 0)
					{
						tileId = m_taskQueue.back(); m_taskQueue.pop_back();
						m_pMPICommunicator->send(status.source(), WI_TASKID, tileId);
					}
					
					break;
				}

				//--------------------------------------------------
				// Worker is sending in result
				//--------------------------------------------------
				case WI_RESULT:
				{
					int startTileX = tile.GetId() % tilesPerRow,
						startTileY = tile.GetId() / tilesPerRow,
						startPixelX = startTileX * m_nTileWidth,
						startPixelY = startTileY * m_nTileHeight;

					for (int y = 0; y < m_nTileHeight; y++)
					{
						for (int x = 0; x < m_nTileWidth; x++)
						{
							const RGBPixel &pixel = tile.GetImageData()->Get(x,y);
							Spectrum L(pixel.R, pixel.G, pixel.B);

							m_pDevice->Set(deviceWidth - startPixelX - x - 1, deviceHeight - startPixelY - y - 1, L);
						}
					}

					tasksRemaining--;
					++renderProgress;

					break;
				}
			}
		}

		//--------------------------------------------------
		// No more tasks remaining - signal termination
		//--------------------------------------------------
		for (int rank = 1; rank < m_pMPICommunicator->size(); rank++)
			m_pMPICommunicator->send(rank, WI_TERMINATE);

		//--------------------------------------------------
		// Frame completed
		//--------------------------------------------------
		m_pDevice->EndFrame();
	}
	else
	{
		//--------------------------------------------------
		// Prepare structures for use in rendering
		//--------------------------------------------------
		Intersection intersection;
		Vector2 *pSampleBuffer = new Vector2[m_nSampleCount];

		while (true)
		{
			//--------------------------------------------------
			// Request tile
			//--------------------------------------------------
			m_pMPICommunicator->send(0, WI_REQUEST);

			//--------------------------------------------------
			// Receive tile
			//--------------------------------------------------
			mpi::status status = m_pMPICommunicator->recv(0, mpi::any_tag, tileId);

			//--------------------------------------------------
			// If termination, stop
			//--------------------------------------------------
			if (status.tag() == WI_TERMINATE)
				break;

			//--------------------------------------------------
			// We have task id - render
			//--------------------------------------------------
			int startTileX = tileId % tilesPerRow,
				startTileY = tileId / tilesPerRow,
				startPixelX = startTileX * m_nTileWidth,
				startPixelY = startTileY * m_nTileHeight;
		
			for (int y = 0; y < m_nTileHeight; y++)
			{
				for (int x = 0; x < m_nTileWidth; x++)
				{
					// Prepare ray samples
					m_pScene->GetSampler()->Get2DSamples(pSampleBuffer, m_nSampleCount);
					(*m_pFilter)(pSampleBuffer, m_nSampleCount);

					// Radiance
					Spectrum Li = 0;

					for (int sample = 0; sample < m_nSampleCount; sample++)
					{
						Ray ray = m_pCamera->GetRay(
							((float)(startPixelX + x)) / deviceWidth, 
							((float)(startPixelY + y)) / deviceHeight, 
							pSampleBuffer[sample].U, pSampleBuffer[sample].V);

						Li += m_pIntegrator->Radiance(m_pScene, ray, intersection);
					}

					Li = Li / m_nSampleCount;
			
					tile.GetImageData()->Set(x, y, RGBPixel(Li[0], Li[1], Li[2]));
				}
			}

			//--------------------------------------------------
			// Send back result
			//--------------------------------------------------
			tile.SetId(tileId);
			m_pMPICommunicator->send(0, WI_RESULT, tile.GetSerializationBuffer(), tile.GetSerializationBufferSize());
		}

		delete[] pSampleBuffer;
	}

	//--------------------------------------------------
	// Render complete
	//--------------------------------------------------
	if (m_pMPICommunicator->rank() == 0)
		std::cout << "Complete ... " << std::endl;
}
//----------------------------------------------------------------------------------------------
*/