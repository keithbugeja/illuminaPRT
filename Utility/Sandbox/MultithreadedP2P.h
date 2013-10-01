#pragma once

#include "MultithreadedCommon.h"
#include "Peer.h"

class P2PListener 
	: public IlluminaMTListener
{
public:
	enum Role 
	{
		P2PSend,
		P2PReceive,
		P2PSendReceive
	};

protected:
	// List of currently active neighbours
	std::vector<Neighbour> m_neighbourList;

	// P2P message hub for current peer
	Peer2 *m_pPeer;

	// Role of current peer
	Role m_eRole;

	// WFIC Integrator
	MLICIntegrator *m_pWFICIntegrator;

	// Record buffer for sends/receives
	MLIrradianceCacheRecord *m_pRecordBuffer;

public:
	void SetPeer(Peer2 *p_pPeer, Role p_eRole = P2PSendReceive) 
	{
		m_pPeer = p_pPeer;
		m_eRole = p_eRole;
	}

	void OnBeginRender(IIlluminaMT *p_pIlluminaMT)
	{
		IIntegrator *pIntegrator = p_pIlluminaMT->GetEnvironment()->GetIntegrator();
		if (pIntegrator->GetType() == "WFIC")
		{
			m_pWFICIntegrator = (MLICIntegrator*)pIntegrator;
			if (m_eRole == P2PReceive) m_pWFICIntegrator->DisableSampleGeneration(true);

			// Allocate record buffer (maximum of 100K samples)
			m_pRecordBuffer = new MLIrradianceCacheRecord[102400];
		} 
		else
		{
			m_pWFICIntegrator = NULL;
			m_pRecordBuffer = NULL;
		}

		std::cout << "P2PListener :: Populating neighbour list..." << std::endl;
		m_pPeer->GetNeighbours(m_neighbourList);

		if (!m_neighbourList.empty())
		{
			std::cout << "P2PListener :: Found " << m_neighbourList.size() << " neighbours." << std::endl;
			
			for (auto neighbour : m_neighbourList)
			{
				std::cout << "P2PListener :: Connecting to " << neighbour.Address << ":" << neighbour.Port << "..." << std::endl;
				m_pPeer->Connect(neighbour, 500);
			}
		}
	}

	void OnEndRender(IIlluminaMT *p_pIlluminaMT)
	{
			for (auto neighbour : m_neighbourList)
				m_pPeer->Disconnect(neighbour);
	}

	void OnBeginFrame(IIlluminaMT *p_pIlluminaMT) 
	{ 
		ICamera* pCamera = p_pIlluminaMT->GetEnvironment()->GetCamera();
		// pCamera->MoveTo(pCamera->GetObserver() + pCamera->GetFrame().W * 1.0f);
	};

	void OnEndFrame(IIlluminaMT *p_pIlluminaMT)
	{
		// Share irradiance?
		if (m_pWFICIntegrator != NULL)
		{
			// Do we have any neighbours with whom to share?
			if (m_neighbourList.size() > 0)
			{
				if (m_eRole == P2PSend)
				{
					int epoch = m_pWFICIntegrator->NextEpoch();
					std::vector<MLIrradianceCacheRecord*> recordList;
					m_pWFICIntegrator->GetByEpoch(epoch, recordList);

					std::cout << "P2PListener :: [Send Role] :: Fetched [" << recordList.size() << "] for epoch [" << epoch << "]" << std::endl;
					
					MLIrradianceCacheRecord *pRecordBuffer = m_pRecordBuffer;
					for (auto record : recordList)
						*pRecordBuffer++ = *record;

					// For each connected neighbour, share
					for (auto neighbour : m_neighbourList)
					{
						m_pPeer->RawSend(neighbour, (char*)m_pRecordBuffer, 
							recordList.size() * sizeof(MLIrradianceCacheRecord));
					}
				} 
				else if (m_eRole == P2PReceive)
				{
					MLIrradianceCache *pIrradianceCache = 
						m_pWFICIntegrator->GetIrradianceCache();

					Neighbour neighbour;

					// While any messages are waiting in the queue
					for(std::vector<unsigned char> receiveBuffer;
						m_pPeer->RawReceive(receiveBuffer, neighbour);
						receiveBuffer.clear())
					{
						MLIrradianceCacheRecord *pRecord = 
							(MLIrradianceCacheRecord*)receiveBuffer.data();
						int recordCount = receiveBuffer.size() / sizeof(MLIrradianceCacheRecord);

						std::cout << "P2PListener :: [Receive Role] :: Received [" << recordCount << "] samples from [" << neighbour.Address << ":" << neighbour.Port << "]" << std::endl;

						for(;recordCount > 0; --recordCount, pRecord++)
							pIrradianceCache->Insert(m_pWFICIntegrator->RequestRecord(pRecord));
					}
				}
			}
		}
	}
};
