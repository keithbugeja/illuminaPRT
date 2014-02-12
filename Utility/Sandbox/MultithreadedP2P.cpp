#include "Defs.h"

#if (defined(ILLUMINA_P2P))

#include "MultithreadedP2P.h"

//----------------------------------------------------------------------------------------------
P2PListener2Way::P2PListener2Way(void) { }
//----------------------------------------------------------------------------------------------
void P2PListener2Way::NewsCastThreadHandler(P2PListener2Way *p_pListener)
{
	// int m_newscastCycle = 0;

	while(p_pListener->IsRunning())
	{
		// std::cout << "---->> NewsCast Cycle : [" << m_newscastCycle++ << "]" << std::endl;

		if (p_pListener->m_pWFICIntegrator != NULL)
			p_pListener->NewsCast();
	
		// p_pListener->Dump_TransactionCache(m_newscastCycle);

		boost::this_thread::sleep(boost::posix_time::millisec(p_pListener->m_newscastPush));
	}
}
//----------------------------------------------------------------------------------------------
void P2PListener2Way::NewsUpdateThreadHandler(P2PListener2Way *p_pListener)
{
	// int m_newsupdateCycle = 0;

	while(p_pListener->IsRunning())
	{
		// std::cout << "---->> NewsUpdate Cycle : [" << m_newsupdateHandler++ << "]" << std::endl;

		if (p_pListener->m_pWFICIntegrator != NULL)
			p_pListener->NewsUpdate();
	
		boost::this_thread::sleep(boost::posix_time::millisec(p_pListener->m_newscastPull));
	}
}
//----------------------------------------------------------------------------------------------
void P2PListener2Way::BackgroundThreadHandler(P2PListener2Way *p_pListener)
{
	while(p_pListener->IsRunning())
	{
		if (p_pListener->m_pWFICIntegrator != NULL)
		{
			p_pListener->NewsCast();
			p_pListener->NewsUpdate();
		}
	
		boost::this_thread::sleep(boost::posix_time::millisec(500));
	}
}
//----------------------------------------------------------------------------------------------
bool P2PListener2Way::State_InitiateConnection(void)
{
	std::vector<HostId> hostList,
		connectedHostList;

	// Can't newscast if empty
	if (m_hostDirectory.IsEmpty()) return false;

	// Get current host list
	m_hostDirectory.GetDirectory(hostList);

	// Choose next host
	int nextHost = m_random.Next(65535) % hostList.size();
	m_exchangeHostId = hostList[nextHost];

	// Connect to chosen host
	std::cout << "---> [P2P Subsystem] :: Initiating connection to [" << m_exchangeHostId.ToIPv4String() << ":" << m_exchangeHostId.GetPort() << "]" << std::endl;
	// m_pPeer->Connect(m_exchangeHostId, 0);
	m_pPeer->Connect(m_exchangeHostId, m_newscastPush * 2);
				
	// New state / set deadline (2.5 x push + pull)
	m_newscastState = NCConnect;
	m_newscastDeadline = RakNet::GetTimeMS() + (m_newscastPush * 10);

	return true;
}
//----------------------------------------------------------------------------------------------
bool P2PListener2Way::State_Connect(HostId p_hostId)
{
	RakNet::ConnectionState cs = m_pPeer->GetConnectionState(p_hostId);
	switch(cs)
	{
		case RakNet::ConnectionState::IS_CONNECTING:
		{
			std::cout << "---> [P2P Subsystem] :: Still connecting ..." << std::endl;
			return true;
		}

		case RakNet::ConnectionState::IS_CONNECTED:
		{
			std::cout << "---> [P2P Subsystem] :: Connected" << std::endl;
			m_newscastState = NCPeerSend;
			return true;
		}

		case RakNet::ConnectionState::IS_SILENTLY_DISCONNECTING:
		case RakNet::ConnectionState::IS_DISCONNECTING:
		case RakNet::ConnectionState::IS_DISCONNECTED:
		case RakNet::ConnectionState::IS_NOT_CONNECTED:
		{
			std::cout << "---> [P2P Subsystem] :: Connection failed with error code [" << (int)cs << "]" << std::endl;
			m_newscastState = NCTerminateConnection;
			return false;
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool P2PListener2Way::State_PeerSend(HostId p_hostId, bool p_bResponse)
{
	// Get peer list
	std::vector<HostId> hostList;
	m_hostDirectory.GetDirectory(hostList);

	// Set transaction
	HostDirectoryTransaction peerSendTR(m_pPeer->GetHostId());
	peerSendTR.SetData(hostList);

	// Serialise to bitstream
	RakNet::BitStream bitStream;
	peerSendTR.WriteToBitStream(bitStream);

	std::cout << "---> [P2P Subsystem] :: Sending Peer directory to [" << p_hostId.ToIPv4String() << " : " << p_hostId.GetPort() << "]" << std::endl;
	m_pPeer->SendIddStream(p_hostId, TTPeerList, bitStream, p_bResponse ? 1 : 0);

	return true;
}
//----------------------------------------------------------------------------------------------
bool P2PListener2Way::State_PeerReceive(RakNet::BitStream &p_bitStream, HostId p_hostId)
{
	std::vector<HostId> hostList;
	HostDirectoryTransaction received;
	received.ReadFromBitStream(p_bitStream);
	received.GetData(hostList);

	std::cout << "---> [P2P Subsystem] :: Received peers [size = " << hostList.size() << "] from [" << p_hostId.ToIPv4String() << " : " << p_hostId.GetPort() << "]" << std::endl;
	for (auto host : hostList) std::cout << "------->" << host.ToIPv4String() << ":" << host.GetPort() << std::endl;
		
	// Remove entry of this host
	auto me = std::find(hostList.begin(), hostList.end(), m_pPeer->GetHostId());
		if (me != hostList.end()) hostList.erase(me);

	// Modify local host directory to include remote list
	m_hostDirectory.Add(p_hostId);
	m_hostDirectory.AddToDirectory(hostList);
	m_hostDirectory.Sort();
	m_hostDirectory.Truncate();

	// Show updated directory
	hostList.clear();
	m_hostDirectory.GetDirectory(hostList);

	std::cout << "---> [P2P Subsystem] :: Updated local directory ..." << std::endl;
	for (auto host : hostList) std::cout << "------->" << host.ToIPv4String() << ":" << host.GetPort() << std::endl;

	return true;
}
//----------------------------------------------------------------------------------------------
bool P2PListener2Way::State_TransactionSend(HostId p_hostId, bool p_bResponse)
{
	std::vector<boost::uuids::uuid> transactionList;
	for (auto transaction : m_transactionMap)
		transactionList.push_back(transaction.first);

	// If we have an empty transaction, then push a dummy UUID to trigger response
	if (transactionList.empty()) transactionList.push_back(boost::uuids::uuid());

	RakNet::BitStream bitStream;
	TransactionListTransaction transactionExchangeTR(m_pPeer->GetHostId());
	transactionExchangeTR.SetData(transactionList);
	transactionExchangeTR.WriteToBitStream(bitStream);

	std::cout << "---> [P2P Subsystem] :: Send catalogue to [" << p_hostId.ToIPv4String() << " : " << p_hostId.GetPort() << "]" << std::endl;
	m_pPeer->SendIddStream(p_hostId, TTTransactionList, bitStream, p_bResponse ? 1 : 0);
		
	return true;
}
//----------------------------------------------------------------------------------------------
bool P2PListener2Way::State_TransactionReceive(RakNet::BitStream &p_bitStream, HostId p_hostId, std::vector<boost::uuids::uuid> &p_outRequestList)
{
	std::vector<boost::uuids::uuid> transactionList;
	p_outRequestList.clear();

	TransactionListTransaction received;
	received.ReadFromBitStream(p_bitStream);
	received.GetData(transactionList);

	// Request transactions from the list that we don't have
	std::cout << "---> [P2P Subsystem] :: Received catalogue [size = " << transactionList.size() << "]" << std::endl;
		
	std::set<boost::uuids::uuid> transactionSet;
	for (auto uuid : transactionList)
		transactionSet.insert(uuid);

	for (auto transaction : m_transactionMap)
	{
		if (transactionSet.find(transaction.first) == transactionSet.end())
			p_outRequestList.push_back(transaction.first);
	}

	for (auto uuid : p_outRequestList)
		std::cout << "------->" << ITransaction::GetIdString(uuid) << "[-]" << std::endl;

	// Increase deadline
	m_newscastDeadline += (m_newscastPull + m_newscastPush) * p_outRequestList.size();

	return p_outRequestList.size() > 0;
}
//----------------------------------------------------------------------------------------------
bool P2PListener2Way::State_IrradianceSend(HostId p_hostId, std::vector<boost::uuids::uuid> &p_requestList, bool p_bResponse)
{
	// Prepare irradiance exchange transaction
	RakNet::BitStream bitStream;
	IrradianceRecordTransaction irradianceExchangeTR(m_pPeer->GetHostId());
	int epoch; std::vector<MLIrradianceCacheRecord*> recordList;

	// Go through each transaction and send
	for (auto uuid : p_requestList)
	{
		std::cout << "---> [P2P Subsystem] :: Responding to transaction " << ITransaction::GetIdString(uuid) << " ..." << std::endl;

		epoch = m_transactionMap[uuid]; recordList.clear();
		m_pWFICIntegrator->GetByEpoch(epoch, recordList);
		irradianceExchangeTR.SetId(uuid);

		std::cout << "---> [P2P Subsystem] :: Fetched [" << recordList.size() << "] for epoch [" << epoch << "]" << std::endl;
		if (!recordList.empty())
		{
			std::vector<MLIrradianceCacheRecord> irradianceList;
			for (auto record : recordList)
				irradianceList.push_back(*record);

			bitStream.Reset();
			irradianceExchangeTR.SetData(irradianceList);
			irradianceExchangeTR.WriteToBitStream(bitStream);

			std::cout << "---> [P2P Subsystem] :: Sent transaction " << ITransaction::GetIdString(uuid) << "..." << std::endl;
			m_pPeer->SendIddStream(p_hostId, TTIrradianceSamples, bitStream, p_bResponse ? 1 : 0);
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------
bool P2PListener2Way::State_IrradianceReceive(RakNet::BitStream &p_bitStream, HostId p_hostId)
{
	std::vector<MLIrradianceCacheRecord> irradianceList;
	IrradianceRecordTransaction received;
	received.ReadFromBitStream(p_bitStream);
	received.GetData(irradianceList);

	MLIrradianceCache *pIrradianceCache = m_pWFICIntegrator->GetIrradianceCache();
	m_transactionMap[received.GetId()] = m_newscastEpoch;
	m_transactionRecordMap[received.GetId()] = TransactionRecord(received.GetId(), received.GetType(), received.GetHostId());

	for (auto irradiance : irradianceList)
	{
		pIrradianceCache->Insert(m_pWFICIntegrator->RequestRecord(&irradiance, m_newscastEpoch));
	}

	std::cout << "---> [P2P Subsystem] :: Transaction " << received.GetIdString() << " bound to epoch [" << m_newscastEpoch << "]" << std::endl;
	m_newscastEpoch++;

	return true;
}
//----------------------------------------------------------------------------------------------
void P2PListener2Way::Dump_TransactionCache(int p_nCycle, bool p_bFilePerCycle)
{
	std::ofstream transactionCache;

	if (p_bFilePerCycle)
	{
		std::stringstream filename; filename << "transactionCache_" << p_nCycle << ".txt";
		transactionCache.open(filename.str(), std::ios_base::binary);
	}
	else
		transactionCache.open("transactionCache.txt", std::ios_base::binary);

	transactionCache << "Cycle [" << p_nCycle << "]" << std::endl;
	transactionCache << "Event List:" << std::endl;
	for (auto transactionRecord : m_transactionRecordMap)
	{
		transactionCache << transactionRecord.second.ToString() << std::endl;
	}

	std::vector<HostId> hostDirectory;
	m_hostDirectory.GetDirectory(hostDirectory);
	transactionCache << "Host List:" << std::endl;

	for (auto host : hostDirectory)
	{
		transactionCache << host.ToIPv4String() << " : " << host.GetPort() << std::endl;
	}

	transactionCache << "Integrator Properties:" << std::endl;
	transactionCache << m_pWFICIntegrator->ToString() << std::endl;

	transactionCache.close();
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// UpdateLocalCatalogue
//----------------------------------------------------------------------------------------------
void P2PListener2Way::UpdateLocalCatalogue(void)
{
	// We need a quota of at least 100 samples before promoting to a transaction!
	if (m_pWFICIntegrator->HasEpochQuota(P2PLISTENER_TX_QUOTA))
	{
		boost::uuids::uuid transactionId = ITransaction::GenerateId();
		int lastEpoch = m_pWFICIntegrator->NextEpoch();
		m_transactionMap[transactionId] = lastEpoch;		
		m_transactionRecordMap[transactionId] = TransactionRecord(transactionId, TransactionType::TTOriginator, m_pPeer->GetHostId());

		std::cout << "---> [P2P Subsystem] :: Created transaction " << ITransaction::GetIdString(transactionId) 
			<< " bound to epoch [" << lastEpoch << "]" << std::endl;
	}
}
//----------------------------------------------------------------------------------------------
// Newsupdate method
//----------------------------------------------------------------------------------------------
void P2PListener2Way::NewsUpdate(void)
{
	unsigned char streamId; HostId hostId; RakNet::BitStream bitStream;

	// Check if we have any pending packets (not response packets)
	while (m_pPeer->ReceiveIddStream(bitStream, streamId, hostId))
	{
		std::vector<boost::uuids::uuid> requestList;

		std::cout << "---> [P2P Subsystem] :: Received data from [" << hostId.ToIPv4String() << " : " << hostId.GetPort() << "]" << std::endl;
		switch(streamId)
		{
			case TTPeerList:
				std::cout << "---> [P2P Subsystem] :: Responding to peer list exchange ..." << std::endl;
				State_PeerReceive(bitStream, hostId);
				State_PeerSend(hostId, true);
				break;

			case TTTransactionList:
				std::cout << "---> [P2P Subsystem] :: Responding to catalogue exchange ..." << std::endl;
				State_TransactionReceive(bitStream, hostId, requestList);
				State_TransactionSend(hostId, true);

				std::cout << "---> [P2P Subsystem] :: Sending irradiance samples ..." << std::endl;
				State_IrradianceSend(hostId, requestList);
				break;

			case TTIrradianceSamples:
				std::cout << "---> [P2P Subsystem] :: Receiving irradiance samples ..." << std::endl;
				State_IrradianceReceive(bitStream, hostId);
				break;
		}
	}
}

//----------------------------------------------------------------------------------------------
// Newscast method
//----------------------------------------------------------------------------------------------
void P2PListener2Way::NewsCast(void)
{
	// Has the deadline run out?
	if (RakNet::GetTimeMS() > m_newscastDeadline && m_newscastState!= NCInitiateConnection)
	{
		std::cout << "---> [P2P Subsystem :: Connection deadline expired!" << std::endl;
		m_newscastState = NCTerminateConnection;
	}

	switch(m_newscastState)
	{
		case NCInitiateConnection:
		{
			std::cout << "---> [P2P Subsystem] :: State = [Initiate Connection]" << std::endl;
				
			// Couldn't initiate connection -> switch to quiescent mode
			if (!State_InitiateConnection()) 
			{
				std::cout << "---> [P2P Subsystem] :: Initiation Failed!" << std::endl;
				// m_newscastState = NCQuiescent;
				m_newscastState = NCTerminateConnection;
			}
					
			break;
		}

		case NCConnect:
		{
			// --- >> Increased deadline
			m_newscastDeadline += m_newscastTimeout;

			// Connected
			std::cout << "---> [P2P Subsystem] :: State = [Connect]" << std::endl;
				
			if (!State_Connect(m_exchangeHostId))
			{
				std::cout << "---> [P2P Subsystem] :: Connection Failed!" << std::endl;
				// m_newscastState = NCQuiescent;
				m_newscastState = NCTerminateConnection;
			}

			break;
		}

		case NCPeerSend:
		{
			// --- >> Increased deadline
			m_newscastDeadline += m_newscastTimeout;

			std::cout << "---> [P2P Subsystem] :: State = [Peer Send]" << std::endl;
			State_PeerSend(m_exchangeHostId);
			m_newscastState = NCQuiescent;
			break;
		}

		case NCTransactionSend:
		{
			std::cout << "---> [P2P Subsystem] :: State = [Transaction Send]" << std::endl;
			State_TransactionSend(m_exchangeHostId);
			m_newscastState = NCQuiescent;
			break;
		}

		case NCIrradianceSend:
		{
			std::cout << "---> [P2P Subsystem] :: State = [Irradiance Send]" << std::endl;
			State_IrradianceSend(m_exchangeHostId, m_exchangeRequestList);
			m_newscastState = NCQuiescent;
			break;
		}

		case NCQuiescent:
		{
			// std::cout << "---> [P2P Subsystem] :: State = [Quiescent]" << std::endl;
			unsigned char streamId; HostId hostId; RakNet::BitStream bitStream;

			// Check if we have any pending packets
			while (m_pPeer->ReceiveIddStream(bitStream, streamId, hostId, 0x01))
			{
				std::cout << "---> [P2P Subsystem] :: Received response data from [" << hostId.ToIPv4String() << " : " << hostId.GetPort() << "]" << std::endl;
				switch(streamId)
				{
				case TTPeerList:
					std::cout << "---> [P2P Subsystem] :: State = [Quiescent :: Peer Receive]" << std::endl;
					State_PeerReceive(bitStream, hostId);
					m_newscastState = NCTransactionSend;
					break;

				case TTTransactionList:
					std::cout << "---> [P2P Subsystem] :: State = [Quiescent :: Transaction Receive]" << std::endl;
					State_TransactionReceive(bitStream, hostId, m_exchangeRequestList);
					m_newscastState = NCIrradianceSend;
					break;

				case TTIrradianceSamples:
					std::cout << "---> [P2P Subsystem] :: State = [Quiescent :: Irradiance Receive]" << std::endl;
					State_IrradianceReceive(bitStream, hostId);
					m_newscastState = NCQuiescent;
					break;
				}
			}

			break;
		}

		case NCTerminateConnection:
		{
			std::cout << "---> [P2P Subsystem] :: State = [Terminate Connection]" << std::endl;
			m_pPeer->Disconnect(m_exchangeHostId);

			m_newscastState = NCInitiateConnection;

			// Newscast cycle
			Dump_TransactionCache(m_newscastCycle++);
			std::cout << "---> [P2P Subsystem] :: Newscast Cycle = [" << m_newscastCycle << "]" << std::endl;

			break;
		}
	}
}
//----------------------------------------------------------------------------------------------
HostDirectory& P2PListener2Way::GetHostDirectory(void) { 
	return m_hostDirectory; 
}
//----------------------------------------------------------------------------------------------
bool P2PListener2Way::IsRunning(void) { return m_bIsRunning; }
//----------------------------------------------------------------------------------------------
void P2PListener2Way::SetPeer(Peer *p_pPeer, Role p_eRole) 
{
	m_pPeer = p_pPeer;
	m_eRole = p_eRole;
}
//----------------------------------------------------------------------------------------------
void P2PListener2Way::SetEventPush(int p_nPush)
{
	m_newscastPush = p_nPush;
}
//----------------------------------------------------------------------------------------------
void P2PListener2Way::SetEventPull(int p_nPull)
{
	m_newscastPull = p_nPull;
}
//----------------------------------------------------------------------------------------------
void P2PListener2Way::SetTimeout(int p_nTimeout)
{
	m_newscastTimeout = p_nTimeout;
}
//----------------------------------------------------------------------------------------------
void P2PListener2Way::OnBeginRender(IIlluminaMT *p_pIlluminaMT)
{
	IIntegrator *pIntegrator = p_pIlluminaMT->GetEnvironment()->GetIntegrator();
	if (pIntegrator->GetType() == "WFIC")
	{
		m_pWFICIntegrator = (MLICIntegrator*)pIntegrator;
		if (m_eRole == P2PReceive) { m_pWFICIntegrator->DisableSampleGeneration(true); std::cout << "Peer :: Disable Sample Generation = [true]" << std::endl; }

		// Allocate record buffer (maximum of 100K samples)
		m_pRecordBuffer = new MLIrradianceCacheRecord[102400];
	} 
	else
	{
		m_pWFICIntegrator = NULL;
		m_pRecordBuffer = NULL;
	}
	
	m_newscastEpoch = P2PLISTENER_GIC_EPOCH;
	m_newscastState = NCInitiateConnection;
	m_newscastDeadline = 0;
	m_newscastCycle = 0;

	// Initialise path
	LoadCameraScript(p_pIlluminaMT->GetCameraScript());

	// Start background thread
	m_bIsRunning = true;

	//boost::thread inputThreadHandler = 
	//	boost::thread(boost::bind(P2PListener2Way::BackgroundThreadHandler, this));

	boost::thread newscastThreadHandler = 
		boost::thread(boost::bind(P2PListener2Way::NewsCastThreadHandler, this));
	boost::thread newsupdateThreadHandler = 
		boost::thread(boost::bind(P2PListener2Way::NewsUpdateThreadHandler, this));
}
//----------------------------------------------------------------------------------------------
void P2PListener2Way::OnEndRender(IIlluminaMT *p_pIlluminaMT)
{
}
//----------------------------------------------------------------------------------------------
void P2PListener2Way::OnBeginFrame(IIlluminaMT *p_pIlluminaMT) 
{ 
	BeginPath(p_pIlluminaMT);
	//ICamera* pCamera = p_pIlluminaMT->GetEnvironment()->GetCamera();
	//pCamera->MoveTo(pCamera->GetObserver() + pCamera->GetFrame().W * 1.0f);
};
//----------------------------------------------------------------------------------------------
void P2PListener2Way::OnEndFrame(IIlluminaMT *p_pIlluminaMT)
{
	if (m_pWFICIntegrator != NULL)
	{
		UpdateLocalCatalogue();
		
		/* 
			NewsCast();
			NewsUpdate();
		*/
	
		// std::cout << m_pWFICIntegrator->ToString() << std::endl;
	}
}
//----------------------------------------------------------------------------------------------
#endif