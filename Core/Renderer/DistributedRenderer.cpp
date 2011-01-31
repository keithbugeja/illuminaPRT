//----------------------------------------------------------------------------------------------
//	Filename:	BasicRenderer.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#include <time.h>
#include <vector>
#include <map>

#include "boost/progress.hpp"
#include "boost/mpi.hpp"

#include "Renderer/DistributedRenderer.h"
#include "Integrator/Integrator.h"
#include "Geometry/Intersection.h"
#include "Spectrum/Spectrum.h"
#include "Camera/Camera.h"
#include "Device/Device.h"
#include "Scene/Scene.h"

#include "Sampler/Sampler.h"
#include "Filter/Filter.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
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

			inline int GetId(void) { return *(int*)m_pSerializationBuffer; }
			inline void SetId(int p_nId) { *((int*)m_pSerializationBuffer) = p_nId; }

			inline char* GetSerializationBuffer(void) const { return m_pSerializationBuffer; }
			inline int GetSerializationBufferSize(void) const { return m_nSerializationBufferSize; }

			inline Image* GetImageData(void) { return m_pImageData; }
		};
	}
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
DistributedRenderer::DistributedRenderer(const std::string &p_strName, 
	Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, 
	IFilter *p_pFilter, int p_nSampleCount, int p_nTileWidth, int p_nTileHeight)
	: IRenderer(p_strName, p_pScene, p_pIntegrator, p_pDevice, p_pFilter)
	, m_nTileWidth(p_nTileWidth)
	, m_nTileHeight(p_nTileHeight)
	, m_nSampleCount(p_nSampleCount)
{ }
//----------------------------------------------------------------------------------------------
DistributedRenderer::DistributedRenderer(Scene *p_pScene, IIntegrator *p_pIntegrator, IDevice *p_pDevice, 
	IFilter *p_pFilter, int p_nSampleCount, int p_nTileWidth, int p_nTileHeight)
	: IRenderer(p_pScene, p_pIntegrator, p_pDevice, p_pFilter)
	, m_nTileWidth(p_nTileWidth)
	, m_nTileHeight(p_nTileHeight)
	, m_nSampleCount(p_nSampleCount)
{ }
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

	// Haven't figured out the proper way out of boost MPI
	m_pMPICommunicator->abort(0);
	m_pMPIEnvironment->abort(0);

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
						//Ray ray = m_pCamera->GetRay(
						Ray ray = m_pScene->GetCamera()->GetRay(
							(startPixelX + x + pSampleBuffer[sample].U) / deviceWidth, 
							(startPixelY + y + pSampleBuffer[sample].V) / deviceHeight, 
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
	const int WI_TASKID = 0x0002;

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
		std::map<int, time_t> lastJobSent;
		std::map<int, time_t> jobTime;
		std::map<int, int> jobsCompleted;

		boost::progress_display renderProgress(tilesPerScreen);
		boost::timer renderTimer;
		renderTimer.restart();

		time_t startTime = time(NULL);

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
				lastJobSent[rank] = time(NULL);
				jobsCompleted[rank] = 0;
				jobTime[rank] = 0;

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
				jobTime[status.source()] = jobTime[status.source()] + (time(NULL) - lastJobSent[status.source()]);
				jobsCompleted[status.source()] = jobsCompleted[status.source()] + 1;
				lastJobSent[status.source()] = time(NULL);

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
		
		time_t endTime = time(NULL);

		std::cout << "Total Render Time (system) : " << endTime - startTime << " seconds " << std::endl;

		for (int rank = 1; rank < m_pMPICommunicator->size(); rank++)
		{
			std::cout << "Stats for rank [" << rank << "]" << std::endl;
			std::cout << "-- Jobs completed : " << jobsCompleted[rank] << std::endl;
			std::cout << "-- Total job time : " << jobTime[rank] << std::endl;
		}
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
							Ray ray = m_pScene->GetCamera()->GetRay(
								(startPixelX + x + pSampleBuffer[sample].U) / deviceWidth, 
								(startPixelY + y + pSampleBuffer[sample].V) / deviceHeight, 
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