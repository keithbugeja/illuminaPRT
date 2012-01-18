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

			Image *m_pImageData;

		public:
			MPITile(int p_nId, int p_nWidth, int p_nHeight)
			{ 
				m_nSerializationBufferSize = p_nWidth * p_nHeight * sizeof(RGBPixel) + sizeof(int);
				m_pSerializationBuffer = new char[m_nSerializationBufferSize];
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

		class MPIRender
		{
		protected:
			int m_nTileWidth,
				m_nTileHeight;

			int m_nSampleCount;

			Environment *m_environment;
			
			IDevice *m_pDevice;
			IIntegrator *m_pIntegrator;
			IFilter *m_pFilter;
			Scene *m_pScene;


		public:
			MPIRender(Environment *p_environment, int p_nSampleCount = 1, int p_nTileWidth = 8, int p_nTileHeight = 8)
				: m_environment(p_environment)
				, m_nTileWidth(p_nTileWidth)
				, m_nTileHeight(p_nTileHeight)
				, m_nSampleCount(p_nSampleCount)
			{ }

			bool Initialise(void)
			{
				m_pDevice = m_environment->GetDevice();
				m_pIntegrator = m_environment->GetIntegrator();
				m_pScene = m_environment->GetScene();
				m_pFilter = m_environment->GetFilter();

				return true;
			}

			bool Shutdown(void)
			{
				return true;
			}

			bool RenderCoordinator(ITaskPipeline::CoordinatorTask *p_coordinator)
			{
				((ImageDevice*)m_pDevice)->SetFilename(boost::str(boost::format("Output/result_%d.ppm") % p_coordinator->task->GetRank()));

				int deviceWidth = m_pDevice->GetWidth(),
					deviceHeight = m_pDevice->GetHeight();

				int tilesPerRow = deviceWidth / m_nTileWidth,
					tilesPerColumn = deviceHeight / m_nTileHeight,
					tilesPerScreen = tilesPerRow * tilesPerColumn;

				int tileId = -1;

				// Create tile for use within communicator
				MPITile tile(0, m_nTileWidth, m_nTileHeight);

				// Coordinator
				std::map<int, time_t> lastJobSent;
				std::map<int, time_t> jobTime;
				std::map<int, int> jobsCompleted;

				// boost::progress_display renderProgress(tilesPerScreen);
				boost::timer renderTimer;
				renderTimer.restart();

				time_t startTime = time(NULL);

				//--------------------------------------------------
				// Prepare device for rendering
				//--------------------------------------------------
				m_pDevice->BeginFrame();

				//--------------------------------------------------
				// Prepare integrator for rending frame
				//--------------------------------------------------
				// m_pIntegrator->Prepare(m_pScene);

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
				std::cout << "Distributing initial workload to " << p_coordinator->ready.Size() << " workers..." << std::endl;

				if (p_coordinator->ready.Size() > 0)
				{

					for (std::vector<Task*>::iterator taskIterator = p_coordinator->ready.TaskList.begin();
						 taskIterator != p_coordinator->ready.TaskList.end(); ++taskIterator)
					{
						int rank = (*taskIterator)->GetRank();

						// Send a new task, if available
						if (m_taskQueue.size() > 0)
						{
							lastJobSent[rank] = time(NULL);
							jobsCompleted[rank] = 0;
							jobTime[rank] = 0;

							// std::cout<<"Send task " << tileId << " to rank " << rank << std::endl;
							tileId = m_taskQueue.back(); m_taskQueue.pop_back();
							TaskCommunicator::Send(&tileId, sizeof(int), rank, WI_TASKID); 
						}
					}

					//--------------------------------------------------
					// Start request-response communication with 
					// worker processors
					//--------------------------------------------------
					int waiting = p_coordinator->ready.Size();
					while(waiting > 0)
					{
						//--------------------------------------------------
						// Receive request
						//--------------------------------------------------
						MPI_Status status; TaskCommunicator::Receive(tile.GetSerializationBuffer(), tile.GetSerializationBufferSize(), MPI_ANY_SOURCE, WI_RESULT, &status);

						//--------------------------------------------------
						// Worker is sending in result
						//--------------------------------------------------
						// std::cout << "Results from " << status.MPI_SOURCE << "..." << std::endl;

						// Send a new task, if available
						if (m_taskQueue.size() > 0)
						{
							jobTime[status.MPI_SOURCE] = jobTime[status.MPI_SOURCE] + (time(NULL) - lastJobSent[status.MPI_SOURCE]);
							jobsCompleted[status.MPI_SOURCE] = jobsCompleted[status.MPI_SOURCE] + 1;
							lastJobSent[status.MPI_SOURCE] = time(NULL);

							tileId = m_taskQueue.back(); m_taskQueue.pop_back();
							TaskCommunicator::Send(&tileId, sizeof(int), status.MPI_SOURCE, WI_TASKID);
						}
						else
						{
							tileId = -1; waiting--;
							TaskCommunicator::Send(&tileId, sizeof(int), status.MPI_SOURCE, WI_TASKID);
							// std::cout << "Workers waiting : " << waiting << std::endl;
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

						// ++renderProgress;
					}
				}
				else
				{
					std::cout << "Rendering empty frame : not enough workers available." << std::endl;
				}

				//--------------------------------------------------
				// Frame completed
				//--------------------------------------------------
				m_pDevice->EndFrame();

				std::cout << "-------------------------------------------------------------------------" << std::endl;
				std::cout << "Total Render Time : " << renderTimer.elapsed() << " seconds" << std::endl;
		
				time_t endTime = time(NULL);

				std::cout << "Total Render Time (system) : " << endTime - startTime << " seconds " << std::endl;

				for (std::vector<Task*>::iterator taskIterator = p_coordinator->ready.TaskList.begin();
					 taskIterator != p_coordinator->ready.TaskList.end(); ++taskIterator)
				{
					int rank = (*taskIterator)->GetRank();

					std::cout << "Stats for rank [" << rank << "]" << std::endl;
					std::cout << "-- Jobs completed : " << jobsCompleted[rank] << std::endl;
					std::cout << "-- Total job time : " << jobTime[rank] << std::endl;
				}

				std::cout << "-------------------------------------------------------------------------" << std::endl;

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
				MPITile tile(0, m_nTileWidth, m_nTileHeight);

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
					// std::cout << "Worker [" << p_worker->GetRank() << "] waiting for task... " << std::endl;
					TaskCommunicator::Receive(&tileId, sizeof(int), p_worker->GetCoordinatorRank(), WI_TASKID);

					//--------------------------------------------------
					// If termination signal, stop
					//--------------------------------------------------
					if (tileId == -1) 
					{
						std::cout << "Worker [" << p_worker->GetRank() << "] terminating ... " << std::endl;
						break;
					}
					else
					{
						// std::cout << "Worker [" << p_worker->GetRank() << "] has received tile [" << tileId << "] ... " << std::endl;

						//--------------------------------------------------
						// We have task id - render
						//--------------------------------------------------
						int startTileX = tileId % tilesPerRow,
							startTileY = tileId / tilesPerRow,
							startPixelX = startTileX * m_nTileWidth,
							startPixelY = startTileY * m_nTileHeight;
		
						IntegratorContext context;

						// std::cout << "Tile region : [" << startPixelX << ", " << startPixelY << "] - [" << startPixelX + m_nTileWidth << ", " << startPixelY + m_nTileHeight << "]" << std::endl;

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
									context.SampleIndex = sample;
									context.SurfacePosition.Set(startPixelX + x + pSampleBuffer[sample].U, startPixelY + y + pSampleBuffer[sample].V);
									context.NormalisedPosition.Set(context.SurfacePosition.X / deviceWidth, context.SurfacePosition.Y / deviceHeight);

									context.SurfacePosition.Set(x, y);

									Ray ray = m_pScene->GetCamera()->GetRay(
										context.NormalisedPosition.X,
										context.NormalisedPosition.Y,
										//(startPixelX + x + pSampleBuffer[sample].U) / deviceWidth, 
										//(startPixelY + y + pSampleBuffer[sample].V) / deviceHeight, 
										pSampleBuffer[sample].U, pSampleBuffer[sample].V);
																		
									Li += m_pIntegrator->Radiance(&context, m_pScene, ray, intersection);
								}

								Li = Li / m_nSampleCount;
								tile.GetImageData()->Set(x, y, RGBPixel(Li[0], Li[1], Li[2]));
							}
						} 

						//--------------------------------------------------
						// Send back result
						//--------------------------------------------------
						// std::cout << "Worker [" << p_worker->GetRank() << "] sending tile [" << tileId << "] ... " << std::endl;

						tile.SetId(tileId);
						TaskCommunicator::Send(tile.GetSerializationBuffer(), tile.GetSerializationBufferSize(), p_worker->GetCoordinatorRank(), WI_RESULT);
					}
				}

				delete[] pSampleBuffer;

				return true;
			}
		};
	}
}