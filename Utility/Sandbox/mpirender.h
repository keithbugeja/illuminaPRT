//----------------------------------------------------------------------------------------------
//	Filename:	DistributedRenderer.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <time.h>
#include <vector>
#include <map>

#include "boost/progress.hpp"
#include "boost/mpi.hpp"
namespace mpi = boost::mpi;

#include "../../Core/External/Compression/Compression.h"

#include "../../Core/Scene/Environment.h"
#include "../../Core/Renderer/Renderer.h"
#include "../../Core/Geometry/Vector2.h"
#include "../../Core/Image/Image.h"

#include "../../Core/Integrator/Integrator.h"
#include "../../Core/Geometry/Intersection.h"
#include "../../Core/Spectrum/Spectrum.h"
#include "../../Core/Camera/Camera.h"
#include "../../Core/Device/Device.h"
#include "../../Core/Scene/Scene.h"

#include "../../Core/Sampler/Sampler.h"
#include "../../Core/Filter/Filter.h"

#include "taskpipeline.h"

//----------------------------------------------------------------------------------------------
#define SCHEDULER_DATA_RENDER 1
// #define SCHEDULER_DATA_COMPRESSION 1
#define SCHEDULER_DATA_TRANSFER 1
#define SCHEDULER_DATA_IO 1
#define SCHEDULER_DATA_IO_FREQUENCY 10

//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core 
	{
		#define WI_TASKID (MM_ChannelUserBase + 0x0001)
		#define WI_RESULT (MM_ChannelUserBase + 0x0002)

		class MPITile
		{
		protected:
			char  *m_pSerializationBuffer;
			int    m_nSerializationBufferSize;

			RadianceBuffer *m_pImageData;

		public:
			MPITile(int p_nId, int p_nWidth, int p_nHeight)
			{ 
				m_nSerializationBufferSize = p_nWidth * p_nHeight * sizeof(RadianceContext) + sizeof(int);
				m_pSerializationBuffer = new char[m_nSerializationBufferSize + 1024];
				m_pImageData = new RadianceBuffer(p_nWidth, p_nHeight, (RadianceContext*)(m_pSerializationBuffer + sizeof(int)));
			}

			~MPITile(void)
			{
				delete m_pImageData;
				delete[] m_pSerializationBuffer;
			}

			inline int GetId(void) { return *(int*)m_pSerializationBuffer; }
			inline void SetId(int p_nId) { *((int*)m_pSerializationBuffer) = p_nId; }

			inline char* GetSerializationBuffer(void) const { return m_pSerializationBuffer; }
			inline int GetSerializationBufferSize(void) const { return m_nSerializationBufferSize; }

			inline RadianceBuffer* GetImageData(void) { return m_pImageData; }
		};

		/*
		class MPITile
		{
		protected:
			char  *m_pSerializationBuffer;
			int    m_nSerializationBufferSize;

			Image *m_pImageData;

		public:
			MPITile(int p_nId, int p_nWidth, int p_nHeight)
			{ 
				m_nSerializationBufferSize = p_nWidth * p_nHeight * sizeof(RGBPixel) + sizeof(int);
				m_pSerializationBuffer = new char[m_nSerializationBufferSize + 1024];
				m_pImageData = new Image(p_nWidth, p_nHeight, (RGBPixel*)(m_pSerializationBuffer + sizeof(int)));
			}

			~MPITile(void)
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
		*/

		class MPIRender
		{
		protected:
			bool m_bStreamEnabled;
			int m_nStreamPort;
			int m_nPixelBufferSize;
			RGBBytePixel *m_pPixelBuffer;

			int m_nPostProcessing;

			int m_nTileWidth,
				m_nTileHeight;

			int m_nSampleCount;

			RadianceBuffer *m_pRadianceBuffer;

			Environment *m_environment;
			IIntegrator *m_pIntegrator;
			IRenderer *m_pRenderer;
			IDevice *m_pDevice;
			IFilter *m_pFilter;
			Scene *m_pScene;

		public:
			MPIRender(Environment *p_environment, int p_nPostProcessing = 0, int p_nSampleCount = 1, int p_nTileWidth = 16, int p_nTileHeight = 16)
				: m_environment(p_environment)
				, m_nTileWidth(p_nTileWidth)
				, m_nTileHeight(p_nTileHeight)
				, m_nSampleCount(p_nSampleCount)
				, m_nPostProcessing(p_nPostProcessing)
			{ }

			bool Initialise(void)
			{
				m_pIntegrator = m_environment->GetIntegrator();
				m_pRenderer = m_environment->GetRenderer();
				m_pDevice = m_environment->GetDevice();
				m_pScene = m_environment->GetScene();
				m_pFilter = m_environment->GetFilter();

				m_bStreamEnabled = false;

				if (m_nPostProcessing)
					m_pRadianceBuffer = new RadianceBuffer(m_pDevice->GetWidth(), m_pDevice->GetHeight());

				return true;
			}

			bool Shutdown(void)
			{
				if (m_nPostProcessing)
					delete m_pRadianceBuffer;

				return true;
			}

			void EnableStream(int p_nPort)
			{
				m_bStreamEnabled = true;
				m_nStreamPort = p_nPort;

				m_nPixelBufferSize = m_pDevice->GetWidth() * m_pDevice->GetHeight();
				m_pPixelBuffer = new RGBBytePixel[m_nPixelBufferSize];
			}

			bool RenderCoordinator(ITaskPipeline::CoordinatorTask *p_coordinator)
			{
				static int iofrequency = SCHEDULER_DATA_IO_FREQUENCY;

				std::string filename = boost::str(boost::format("Output/result_%d.ppm") % p_coordinator->task->GetRank());
				((ImageDevice*)m_pDevice)->SetFilename(filename);

				int deviceWidth = m_pDevice->GetWidth(),
					deviceHeight = m_pDevice->GetHeight();

				int tilesPerRow = deviceWidth / m_nTileWidth,
					tilesPerColumn = deviceHeight / m_nTileHeight,
					tilesPerScreen = tilesPerRow * tilesPerColumn;

				int tileId = -1;

				// Create tile for use within communication
				MPITile tile(0, m_nTileWidth, m_nTileHeight),
					compressedTile(0, m_nTileWidth, m_nTileHeight);

				// Coordinator
				/*
				std::map<int, time_t> lastJobSent;
				std::map<int, time_t> jobTime;
				std::map<int, int> jobsCompleted;
				*/

				//--------------------------------------------------
				// Prepare device for rendering
				//--------------------------------------------------
				#if (defined(SCHEDULER_DATA_IO))
					if (iofrequency == SCHEDULER_DATA_IO_FREQUENCY)
						m_pDevice->BeginFrame();
				#endif

				//--------------------------------------------------
				// Prepare integrator for rending frame
				//--------------------------------------------------
				// m_pIntegrator->Prepare(m_pScene);

				//--------------------------------------------------
				// Prepare task queue
				//--------------------------------------------------
				std::vector<int> m_taskQueue;

				// generate tiles for rendering
				for (int taskId = 0; taskId < tilesPerScreen; ++taskId) {
					m_taskQueue.push_back(taskId);
				}

				//--------------------------------------------------
				// Distribute initial workload to workers
				//--------------------------------------------------
				// std::cout << "[" << p_coordinator->task->GetRank() << "] Distributing initial workload to " << p_coordinator->ready.Size() << " workers..." << std::endl;
				int waiting = 0;

				if (p_coordinator->ready.Size() > 0)
				{
					for (std::vector<Task*>::iterator taskIterator = p_coordinator->ready.TaskList.begin();
						 taskIterator != p_coordinator->ready.TaskList.end(); ++taskIterator)
					{
						if (m_taskQueue.size() == 0)
							break;

						int rank = (*taskIterator)->GetRank();

						tileId = m_taskQueue.back(); m_taskQueue.pop_back();
						TaskCommunicator::Send(&tileId, sizeof(int), rank, WI_TASKID); 
						
						waiting++;

						/*
						lastJobSent[rank] = time(NULL);
						jobsCompleted[rank] = 0;
						jobTime[rank] = 0;
						*/
					}

					//--------------------------------------------------
					// Start request-response communication with 
					// worker processors
					//--------------------------------------------------
					while(waiting > 0)
					{
						//--------------------------------------------------
						// Receive request
						//--------------------------------------------------
						MPI_Status status;

						#if (defined(SCHEDULER_DATA_TRANSFER))
							#if (defined(SCHEDULER_DATA_COMPRESSION))
								// Receive variable sized data
								TaskCommunicator::Probe(MPI_ANY_SOURCE, WI_RESULT, &status);
								
								if (TaskCommunicator::GetSize(&status) < tile.GetSerializationBufferSize())
								{
									TaskCommunicator::Receive(compressedTile.GetSerializationBuffer(), TaskCommunicator::GetSize(&status), status.MPI_SOURCE, WI_RESULT, &status);

									// Decompress data
									Compressor::Decompress(compressedTile.GetSerializationBuffer(), tile.GetSerializationBufferSize(), tile.GetSerializationBuffer());								
								}
								else
									TaskCommunicator::Receive(tile.GetSerializationBuffer(), tile.GetSerializationBufferSize(), MPI_ANY_SOURCE, WI_RESULT, &status);
								
							#else
								TaskCommunicator::Receive(tile.GetSerializationBuffer(), tile.GetSerializationBufferSize(), MPI_ANY_SOURCE, WI_RESULT, &status);
							#endif
						#else
							TaskCommunicator::Receive(tile.GetSerializationBuffer(), sizeof(int), MPI_ANY_SOURCE, WI_RESULT, &status);
						#endif

						//--------------------------------------------------
						// Worker is sending in result
						//--------------------------------------------------
						// std::cout << "Results from " << status.MPI_SOURCE << "..." << std::endl;

						// Send a new task, if available
						if (m_taskQueue.size() > 0)
						{
							/*
							jobTime[status.MPI_SOURCE] = jobTime[status.MPI_SOURCE] + (time(NULL) - lastJobSent[status.MPI_SOURCE]);
							jobsCompleted[status.MPI_SOURCE] = jobsCompleted[status.MPI_SOURCE] + 1;
							lastJobSent[status.MPI_SOURCE] = time(NULL);
							*/

							tileId = m_taskQueue.back(); m_taskQueue.pop_back();
							TaskCommunicator::Send(&tileId, sizeof(int), status.MPI_SOURCE, WI_TASKID);
						}
						else
						{
							tileId = -1; waiting--;
							TaskCommunicator::Send(&tileId, sizeof(int), status.MPI_SOURCE, WI_TASKID);
						}

						// Aggregate result to buffer
						int startTileX = tile.GetId() % tilesPerRow,
							startTileY = tile.GetId() / tilesPerRow,
							startPixelX = startTileX * m_nTileWidth,
							startPixelY = startTileY * m_nTileHeight;

						if (m_nPostProcessing)
						{
							for (int y = 0; y < m_nTileHeight; y++) {
								for (int x = 0; x < m_nTileWidth; x++) {
									m_pRadianceBuffer->Set(startPixelX + x, startPixelY + y, 
										tile.GetImageData()->Get(x, y));
								}
							}
						}
						else
						{
							for (int y = 0; y < m_nTileHeight; y++) {
								for (int x = 0; x < m_nTileWidth; x++) {
									m_pDevice->Set(deviceWidth - startPixelX - x - 1, deviceHeight - startPixelY - y - 1, 
										tile.GetImageData()->Get(x, y).Final);
								}
							}
						}
					}
				}

				//--------------------------------------------------
				// Frame completed
				//--------------------------------------------------
				if (m_nPostProcessing) {
					double start = Platform::GetTime();
					m_pRenderer->PostProcess(m_pRadianceBuffer);
					std::cout << "Post proc : " << Platform::ToSeconds(Platform::GetTime() - start) << "s" << std::endl;
				}

				#if (defined(SCHEDULER_DATA_IO))
					if (iofrequency == SCHEDULER_DATA_IO_FREQUENCY)
						m_pDevice->EndFrame();

					if (++iofrequency > SCHEDULER_DATA_IO_FREQUENCY)
						iofrequency = 0;
				#endif

				/*
				std::cout << "-------------------------------------------------------------------------" << std::endl;
				std::cout << "[" << p_coordinator->task->GetRank() << "] Total Render Time : " << renderTimer.elapsed() << " seconds" << std::endl;
				time_t endTime = time(NULL); std::cout << "[" << p_coordinator->task->GetRank() << "] Total Render Time (system) : " << endTime - startTime << " seconds " << std::endl;
				
				for (std::vector<Task*>::iterator taskIterator = p_coordinator->ready.TaskList.begin();
					 taskIterator != p_coordinator->ready.TaskList.end(); ++taskIterator)
				{
					int rank = (*taskIterator)->GetRank();

					std::cout << "Stats for rank [" << rank << "]" << std::endl;
					std::cout << "-- Jobs completed : " << jobsCompleted[rank] << std::endl;
					std::cout << "-- Total job time : " << jobTime[rank] << std::endl;
				}

				std::cout << "-------------------------------------------------------------------------" << std::endl;
				*/

				if (m_bStreamEnabled)
				{
					std::cout << "Sending image data..." << std::endl;
					ImageDevice *pImageDevice = (ImageDevice*)m_pDevice;
					pImageDevice->WriteToBuffer(m_pPixelBuffer);
					TaskCommunicator::Send(m_pPixelBuffer, 640 * 400 * 3, 0, m_nStreamPort);
					std::cout << "Sent image data..." << std::endl;
				}

				return true;
			}

			bool RenderWorker(Task *p_worker)
			{
				int deviceWidth = m_pDevice->GetWidth(),
					deviceHeight = m_pDevice->GetHeight();

				int tilesPerRow = deviceWidth / m_nTileWidth,
					tilesPerColumn = deviceHeight / m_nTileHeight,
					tilesPerScreen = tilesPerRow * tilesPerColumn;

				int tileId = -1;

				// Create tile for use within communicator
				MPITile tile(0, m_nTileWidth, m_nTileHeight),
					compressedTile(0, m_nTileWidth, m_nTileHeight);

				//--------------------------------------------------
				// Prepare structures for use in rendering
				//--------------------------------------------------
				while (true)
				{
					//--------------------------------------------------
					// Receive tile
					//--------------------------------------------------
					// std::cout << "Worker [" << p_worker->GetRank() << "] waiting for task... " << std::endl;
					TaskCommunicator::Receive(&tileId, sizeof(int), p_worker->GetCoordinatorRank(), WI_TASKID);
					// std::cout << "Worker [" << p_worker->GetRank() << "] has received tile [" << tileId << "] ... " << std::endl;

					//--------------------------------------------------
					// If termination signal, stop
					//--------------------------------------------------
					if (tileId == -1) break;

					#if (defined(SCHEDULER_DATA_RENDER))
						//--------------------------------------------------
						// We have task id - render
						//--------------------------------------------------
						int startTileX = tileId % tilesPerRow,
							startTileY = tileId / tilesPerRow,
							startPixelX = startTileX * m_nTileWidth,
							startPixelY = startTileY * m_nTileHeight;
		
						m_pRenderer->RenderRegion(startPixelX, startPixelY, m_nTileWidth, m_nTileHeight, tile.GetImageData(), 0, 0);

						if (m_nPostProcessing)
							m_pRenderer->PostProcessRegion(tile.GetImageData());

					#endif

					//--------------------------------------------------
					// Send back result
					//--------------------------------------------------
					// std::cout << "Worker [" << p_worker->GetRank() << "] sending tile [" << tileId << "] ... " << std::endl;

					tile.SetId(tileId);
					
					#if (defined(SCHEDULER_DATA_TRANSFER))
						#if (defined(SCHEDULER_DATA_COMPRESSION))
							int compressedSize = Compressor::Compress(tile.GetSerializationBuffer(), tile.GetSerializationBufferSize(), compressedTile.GetSerializationBuffer());

							if (compressedSize < tile.GetSerializationBufferSize())
								TaskCommunicator::Send(compressedTile.GetSerializationBuffer(), compressedSize, p_worker->GetCoordinatorRank(), WI_RESULT);
							else
								TaskCommunicator::Send(tile.GetSerializationBuffer(), tile.GetSerializationBufferSize(), p_worker->GetCoordinatorRank(), WI_RESULT);

						#else
							TaskCommunicator::Send(tile.GetSerializationBuffer(), tile.GetSerializationBufferSize(), p_worker->GetCoordinatorRank(), WI_RESULT);
						#endif
					#else
						TaskCommunicator::Send(tile.GetSerializationBuffer(), sizeof(int), p_worker->GetCoordinatorRank(), WI_RESULT);
					#endif
				}

				return true;
			}
		};
	}
}