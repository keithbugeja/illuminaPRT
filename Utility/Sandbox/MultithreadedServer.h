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

using namespace Illumina::Core;
using boost::asio::ip::tcp;

class IrradianceConnection
	: public boost::enable_shared_from_this<IrradianceConnection>
{
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

		DualPointGridFilter<Dart> filter;
		m_pDualPointGrid->FilterByView(m_pIllumina->GetEnvironment()->GetCamera(), &filter);

		std::vector<std::vector<Dart*>*> shadingLists;
		filter.GetFilteredPoints(shadingLists);

		m_pPointShader->Initialise(m_pIllumina->GetEnvironment()->GetScene(), 0.01f, 6, 1);
		m_pPointShader->SetHemisphereDivisions(24, 48);
		m_pPointShader->SetVirtualPointSources(256, 8192);
		m_pPointShader->SetGeometryTerm(0.25f);
		m_pPointShader->Prepare(PointShader<Dart>::PointLit);

		for (auto pointList : shadingLists)
			m_pPointShader->Shade(*pointList, PointShader<Dart>::PointLit);
		
		// m_pPointShader->Shade(*(filter.GetGlobalPointList()), PointShader<Dart>::PointLit);

		std::vector<int> indexList;
		std::vector<float> elementList;

		m_pDualPointGrid->PackByFilter(&filter);
		// m_pDualPointGrid->Pack();
		m_pDualPointGrid->Serialize(&elementList, &indexList);

		std::cout << "Compute :: Ready" << std::endl;

		int bufferSize[2] = { indexList.size(), elementList.size() };
		
		std::cout << "Index count : " << bufferSize[0] << ", Element count : " << bufferSize[1] << std::endl;

		boost::system::error_code error;

		boost::asio::write(socket_, boost::asio::buffer(bufferSize, sizeof(int) * 2), error);
		boost::asio::write(socket_, boost::asio::buffer(indexList.data(), sizeof(int) * indexList.size()), error);
		boost::asio::write(socket_, boost::asio::buffer(elementList.data(), sizeof(float) * elementList.size()), error);

		/*
		std::vector<int> indexList;
		std::vector<Spectrum> irradianceList;

		// 1. use indexing to send stuff 
		m_pGPUGrid->Serialize(&filteredGrid, irradianceList, indexList);

		std::cout << "Compute :: Ready" << std::endl;

		int bufferSize[2] = { indexList.size(), irradianceList.size() };
		std::cout << "Index count : " << bufferSize[0] << ", Irradiance count : " << bufferSize[1] << std::endl;

		std::cout << "Send indices" << std::endl;

		boost::system::error_code error;

		boost::asio::write(socket_, boost::asio::buffer(bufferSize, sizeof(int) * 2), error);
		boost::asio::write(socket_, boost::asio::buffer(indexList.data(), sizeof(int) * indexList.size()), error);
		boost::asio::write(socket_, boost::asio::buffer(irradianceList.data(), sizeof(Spectrum) * irradianceList.size()), error);
		*/

		/*
		boost::asio::async_write(socket_, boost::asio::buffer(bufferSize, sizeof(int) * 2),
			boost::bind(&IrradianceConnection::handle_write, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));

		boost::asio::async_write(socket_, boost::asio::buffer(indexList.data(), sizeof(int) * indexList.size()),
			boost::bind(&IrradianceConnection::handle_write, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));

		std::cout << "Send spectra" << std::endl;
		boost::asio::async_write(socket_, boost::asio::buffer(irradianceList.data(), sizeof(Spectrum) * irradianceList.size()),
			boost::bind(&IrradianceConnection::handle_write, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
		*/
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