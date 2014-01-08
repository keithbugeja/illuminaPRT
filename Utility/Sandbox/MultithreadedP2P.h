#pragma once

#include "MultithreadedCommon.h"
#include "Transaction.h"
#include "Peer.h"
#include "Path.h"

#define P2PLISTENER_GIC_EPOCH	0x7FFFFF
#define P2PLISTENER_TX_QUOTA	500

/* */
class P2PListener2Way 
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
	enum State
	{
		NCInitiateConnection,
		NCConnect,
		NCPeerSend,
		NCPeerReceive,
		NCTransactionSend,
		NCTransactionReceive,
		NCIrradianceSend,
		NCIrradianceReceive,
		NCQuiescent,
		NCTerminateConnection
	};

protected:
	// P2P message hub for current peer
	Peer *m_pPeer;

	// Role of current peer
	Role m_eRole;

	// WFIC Integrator
	MLICIntegrator *m_pWFICIntegrator;

	// Record buffer for sends/receives
	MLIrradianceCacheRecord *m_pRecordBuffer;

	// Host directory for known peers
	HostDirectory m_hostDirectory;

	// Random number generator
	Illumina::Core::Random m_random;

	// Newscast exchange details
	State m_newscastState;
	int m_newscastDeadline,
		m_newscastEpoch;

	HostId m_exchangeHostId;
	std::vector<boost::uuids::uuid> m_exchangeRequestList;

	// Transactions
	std::map<boost::uuids::uuid, int> m_transactionMap;

	bool m_bIsRunning;

public:
	//----------------------------------------------------------------------------------------------
	HostDirectory& GetHostDirectory(void);
	//----------------------------------------------------------------------------------------------

protected:
	static void BackgroundThreadHandler(P2PListener2Way *p_pListener);
	static void NewsCastThreadHandler(P2PListener2Way *p_pListener);
	static void NewsUpdateThreadHandler(P2PListener2Way *p_pListener);

	//----------------------------------------------------------------------------------------------
	bool State_InitiateConnection(void);
	bool State_Connect(HostId p_hostId);
	bool State_PeerSend(HostId p_hostId, bool p_bResponse = false);
	bool State_PeerReceive(RakNet::BitStream &p_bitStream, HostId p_hostId);
	bool State_TransactionSend(HostId p_hostId, bool p_bResponse = false);
	bool State_TransactionReceive(RakNet::BitStream &p_bitStream, HostId p_hostId, std::vector<boost::uuids::uuid> &p_outRequestList);
	bool State_IrradianceSend(HostId p_hostId, std::vector<boost::uuids::uuid> &p_requestList, bool p_bResponse = false);
	bool State_IrradianceReceive(RakNet::BitStream &p_bitStream, HostId p_hostId);
	//----------------------------------------------------------------------------------------------
public:
	P2PListener2Way(void);

	//----------------------------------------------------------------------------------------------
	void UpdateLocalCatalogue(void);
	void NewsUpdate(void);
	void NewsCast(void);
	//----------------------------------------------------------------------------------------------
	bool IsRunning(void);
	void SetPeer(Peer *p_pPeer, Role p_eRole = P2PSendReceive);
	void SetCameraPath(std::vector<PathEx> &p_cameraPath);
	//----------------------------------------------------------------------------------------------
	void OnBeginRender(IIlluminaMT *p_pIlluminaMT);
	void OnEndRender(IIlluminaMT *p_pIlluminaMT);
	void OnBeginFrame(IIlluminaMT *p_pIlluminaMT);
	void OnEndFrame(IIlluminaMT *p_pIlluminaMT);
	//----------------------------------------------------------------------------------------------

protected:
	//----------------------------------------------------------------------------------------------
	// Crap for testing...
	//----------------------------------------------------------------------------------------------
	PathEx m_path;
	float m_fDeltaTime;
	int m_nFrameNumber;

	void InitPath(void)
	{
		Random r(Platform::GetCycles());

		m_nFrameNumber = 0;
		m_fDeltaTime = 0;

		m_path.Clear();

		// Generate a random path
		PathVertexEx vertex;
		vertex.position.Set(0, -14, 0);
		//vertex.position.Set(-8, 2.5, 0);
		vertex.orientation.Set(Maths::DegToRad(90), 0, 0);
		m_path.AddVertex(vertex);

		for (int n = 0; n < 12; n++)
		{
			Vector3 position = Montecarlo::UniformSampleSphere(r.NextFloat(), r.NextFloat()); 
			position.Y = 0; position.Normalize(); position *= 3.0f;

			vertex.position += position;
			vertex.orientation.X += (r.NextFloat() - 0.5f) * Maths::DegToRad(20);

			m_path.AddVertex(vertex);
		}

		// m_path.FromString("path={{-16, -14, -5.75},{90,0,0},{-8, -14, -5.75},{90,0,0},{-2.36, -14, -5.75},{90,0,0}}");
	}

	void BeginPath(IIlluminaMT *p_pIlluminaMT)
	{
		ICamera* pCamera = p_pIlluminaMT->GetEnvironment()->GetCamera();
		Vector3 position, lookAt, position2;

		m_fDeltaTime += 0.002f; if (m_fDeltaTime > 1.f) m_fDeltaTime = 1.f;

		m_path.Get(m_fDeltaTime, position, lookAt);
		pCamera->MoveTo(position); //pCamera->LookAt(lookAt);
		m_path.Get(m_fDeltaTime + 0.01f, position, lookAt);
		pCamera->LookAt(position);

		// std::cout << "Position : " << position.ToString() << ", LookAt : " << lookAt.ToString() << std::endl;
		// pCamera->MoveTo(pCamera->GetObserver() + pCamera->GetFrame().W * 1.0f);
	}
};
