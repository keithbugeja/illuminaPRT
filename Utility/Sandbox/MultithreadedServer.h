//----------------------------------------------------------------------------------------------
//	Filename:	MultithreadedServer.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
// Todo: Generalise structures used... there is way too much repetition (copy-pasting and 
//	     tweaking) of functionality available elsewhere in the core.
//----------------------------------------------------------------------------------------------
#pragma once

#include "PointSet.h"
#include "DualPointGrid.h"

#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include <omp.h>

using namespace Illumina::Core;
using boost::asio::ip::tcp;

class IrradianceConnection
	: public boost::enable_shared_from_this<IrradianceConnection>
{
	struct GridHeader
	{
		float x, y, z, w;
		int sudivs, indices, elements, samples, positions; 
	};

public:
	typedef boost::shared_ptr<IrradianceConnection> pointer;

	static pointer create(boost::asio::io_service &io_service, IIlluminaMT *p_pIllumina, PointSet<Dart> *p_pPointSet, PointShader<Dart> *p_pPointShader, DualPointGrid<Dart> *p_pDualPointGrid)
	{
		return pointer(new IrradianceConnection(io_service, p_pIllumina, p_pPointSet, p_pPointShader, p_pDualPointGrid));
	}

	tcp::socket &socket()
	{
		return socket_;
	}

	void start()
	{
		omp_set_num_threads(20);

		// Create new filter for grid and filter by current camera view
		DualPointGridFilter<Dart> filter;
		m_pDualPointGrid->FilterByView(m_pIllumina->GetEnvironment()->GetCamera(), &filter);

		// Get list of points to shade in view
		std::vector<std::vector<Dart*>*> shadingLists;
		filter.GetFilteredPoints(shadingLists);

		// Initialise shader
		m_pPointShader->Initialise(m_pIllumina->GetEnvironment()->GetScene(), 0.01f, 6, 1);
		m_pPointShader->SetHemisphereDivisions(24, 48);
		m_pPointShader->SetVirtualPointSources(32, 8192); // 256
		m_pPointShader->SetGeometryTerm(0.25f);
		m_pPointShader->Prepare(PointShader<Dart>::PointLit);

		// Keep buffers for transfer
		std::vector<int> indexList;
		std::vector<int> colourList;
		std::vector<float> elementList;
		std::vector<int> sampleIndexList;
		std::vector<int> samplePositionList;
		std::vector<Dart*> qmcPointList;

		// Serialise grid
		m_pDualPointGrid->Serialize(&elementList, &indexList, &sampleIndexList, &samplePositionList);

		// Compose grid header
		Vector3 origin = m_pDualPointGrid->GetOrigin();

		GridHeader header;
		header.x = origin.X; header.y = origin.Y; header.z = origin.Z;
		header.w = m_pDualPointGrid->GetCellSize();
		header.sudivs = m_pDualPointGrid->GetCellSubdivisions();
		header.indices = indexList.size();
		header.elements = elementList.size();
		header.samples = sampleIndexList.size();
		header.positions = samplePositionList.size();

		std::cout << "Irradiance Server :: [Index count : " << header.indices << ", Element count : " << header.elements
			<< ", Samples count : " << header.samples << ", Positions count : " << header.positions << "]" << std::endl;

		boost::system::error_code error;

		boost::asio::write(socket_, boost::asio::buffer(&header, sizeof(GridHeader)), error);
		boost::asio::write(socket_, boost::asio::buffer(indexList.data(), sizeof(int) * indexList.size()), error);
		boost::asio::write(socket_, boost::asio::buffer(elementList.data(), sizeof(float) * elementList.size()), error);
		boost::asio::write(socket_, boost::asio::buffer(sampleIndexList.data(), sizeof(int) * sampleIndexList.size()), error);
		boost::asio::write(socket_, boost::asio::buffer(samplePositionList.data(), sizeof(int) * samplePositionList.size()), error);

		// Grid sent to client
		std::cout << "Irradiance Server :: Grid packed" << std::endl;
		std::cout << "Irradiance Server :: Scene data uploaded..." << std::endl;

		// Initial grid sent
		ICamera* pCamera = m_pIllumina->GetEnvironment()->GetCamera();
		float camera[18];
		int bufferSize[2];

		float moveHash = 0, 
			lastMoveHash = 0;

		SpotLight *pLight = (SpotLight*)m_pIllumina->GetEnvironment()->GetScene()->LightList[0];
		Vector3 lightPosition = pLight->GetPosition(),
			lightDirection = pLight->GetDirection(),
			originalPosition = lightPosition;

		LowDiscrepancySampler selectorSampler;		

		std::cout << "Irradiance Server :: Starting irradiance feedback loop..." << std::endl;

		// Next -> on-demand computation
		while (true)
		{
			// Reset samplers
			//std::cout << "Irradiance Server :: Resetting sampler seeds..." << std::endl;
			// m_pIllumina->GetEnvironment()->GetSampler()->Reset(11371137);
			// selectorSampler.Reset(300131137);

			// Prepare shaders
			//std::cout << "Irradiance Server :: Preparing shader..." << std::endl;
			m_pPointShader->Prepare(PointShader<Dart>::PointLit);

			// Invalidate all points
			//std::cout << "Irradiance Server :: Invalidating points..." << std::endl;
			for (auto point : m_pPointSet->GetContainerInstance().Get())
			{
				point->Irradiance.Set(10, 0, 0);
				point->Invalid = true;
			}

			//std::cout << "Irradiance Server :: Try receive..." << std::endl;

			// Receive observer
			if (boost::asio::read(socket_, boost::asio::buffer(camera, sizeof(float) * 18), error) == sizeof(float) * 18)
			{
				Vector3 observer(camera[0], camera[1], camera[2]),
					forward(camera[3], camera[4], camera[5]),
					right(camera[6], camera[7], camera[8]),
					up(camera[9], camera[10], camera[11]),
					lightPosition(camera[12], camera[13], camera[14]),
					lightDirection(camera[15], camera[16], camera[17]);

				// std::cout << "Camera : " << observer.ToString() << ", " << forward.ToString() << ", " << lightPosition.ToString() << ", " << originalPosition.ToString() << std::endl;

				// Compute change hash
				// moveHash = observer.X + observer.Y + observer.Z + forward.X + forward.Y + forward.Z + lightPosition.X + lightPosition.Y + lightPosition.Z;

				// Move camera and set light position
				pCamera->MoveTo(observer);
				pCamera->LookAt(observer + forward);
				pLight->SetPosition(lightPosition);
				pLight->SetDirection(lightDirection);

				// Clear shading and transfer buffers
				shadingLists.clear(); indexList.clear(); elementList.clear();
				
				// Unfiltering to keep initial camera view
				m_pDualPointGrid->FilterByView(pCamera, &filter);
				filter.GetFilteredPoints(shadingLists);
				
				// Shade points
				int totsamples = 0, partsamples = 0;

				double startTotal = Platform::GetTime();
				#pragma omp parallel for
				for (int index = 0; index < shadingLists.size(); index++)
				{
					auto pointList = shadingLists[index];
					m_pPointShader->Shade(*pointList, PointShader<Dart>::PointLit);
					totsamples += pointList->size();

				}

				//for (auto pointList : shadingLists) {
				//	m_pPointShader->Shade(*pointList, PointShader<Dart>::PointLit);
				//	// m_pPointShader->Shade(*pointList,PointShader<Dart>::FalseColour);
				//	totsamples += pointList->size();
				//}
				double endTotal = Platform::GetTime();

				//std::cout << "Irradiance Server :: Shading time [" << Platform::ToSeconds(endTotal - startTotal) << "]" << std::endl;
				//std::cout << "Irradiance Server :: Shading totals [" << partsamples << " : " << totsamples << "]" << std::endl;

				
				// Serialise points
				m_pDualPointGrid->SerializeUniqueByFilter(&filter, &indexList, &colourList);
				
				// Send points
				bufferSize[0] = indexList.size(); bufferSize[1] = colourList.size();
				//std::cout << "Irradiance Server :: Sending [" << bufferSize[0] << "] , [" << bufferSize[1] << "]" << std::endl;

				boost::asio::write(socket_, boost::asio::buffer(bufferSize, sizeof(int) * 2), error);
				boost::asio::write(socket_, boost::asio::buffer(indexList.data(), sizeof(int) * indexList.size()), error);
				boost::asio::write(socket_, boost::asio::buffer(colourList.data(), sizeof(int) * colourList.size()), error);

				std::cout << "Irradiance Server :: Irradiance sent..." << std::endl;
			}
		}
	}

private:
	IrradianceConnection(boost::asio::io_service& io_service, IIlluminaMT* p_pIllumina, PointSet<Dart> *p_pPointSet, PointShader<Dart> *p_pPointShader, DualPointGrid<Dart> *p_pDualPointGrid)
		: socket_(io_service)
		, m_pIllumina(p_pIllumina)
		, m_pPointSet(p_pPointSet)
		, m_pPointShader(p_pPointShader)
		, m_pDualPointGrid(p_pDualPointGrid)
	{
	}

	void handle_write(const boost::system::error_code& /*error*/,
		size_t /*bytes_transferred*/)
	{
	}

	IIlluminaMT *m_pIllumina; 
	PointSet<Dart> *m_pPointSet;
	PointShader<Dart> *m_pPointShader;
	DualPointGrid<Dart> *m_pDualPointGrid;

	tcp::socket socket_;
	std::string message_;
};

class IrradianceServer
{
	IrradianceServer(boost::asio::io_service& io_service, IIlluminaMT *p_pIllumina, PointSet<Dart> *p_pPointSet, PointShader<Dart> *p_pPointShader, DualPointGrid<Dart> *p_pDualPointGrid)
		: acceptor_(io_service, tcp::endpoint(tcp::v4(), 6666))
		, m_pIllumina(p_pIllumina)
		, m_pPointSet(p_pPointSet)
		, m_pPointShader(p_pPointShader)
		, m_pDualPointGrid(p_pDualPointGrid)
	{
		start_accept();
	}

private:
	tcp::acceptor acceptor_;
	IIlluminaMT *m_pIllumina;
	PointSet<Dart> *m_pPointSet;
	PointShader<Dart> *m_pPointShader;
	DualPointGrid<Dart> *m_pDualPointGrid;

	void start_accept(void)
	{
		IrradianceConnection::pointer new_connection = 
			IrradianceConnection::create(acceptor_ .get_io_service(), m_pIllumina, m_pPointSet, m_pPointShader, m_pDualPointGrid);

		acceptor_.async_accept(new_connection->socket(),
			boost::bind(&IrradianceServer::handle_accept, this, new_connection,
			boost::asio::placeholders::error));
	}

	void handle_accept(IrradianceConnection::pointer new_connection,
		const boost::system::error_code &error)
	{
		if (!error)
		{
			new_connection->start();
			start_accept();
		}
	}

public:
	static void Boot(IIlluminaMT *p_pIllumina, PointSet<Dart> *p_pPointSet, PointShader<Dart> *p_pPointShader, DualPointGrid<Dart> *p_pDualPointGrid)
	{
		try
		{
			boost::asio::io_service io_service;
			IrradianceServer server(io_service, p_pIllumina, p_pPointSet, p_pPointShader, p_pDualPointGrid);
			io_service.run();
		}
		
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
};