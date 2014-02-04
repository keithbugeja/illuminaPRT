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
		m_newscastEpoch,
		m_newscastPush,
		m_newscastPull;
	
	HostId m_exchangeHostId;
	std::vector<boost::uuids::uuid> m_exchangeRequestList;

	// Transactions
	std::map<boost::uuids::uuid, int> m_transactionMap;
	std::map<boost::uuids::uuid, TransactionRecord> m_transactionRecordMap;

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

	void Dump_TransactionCache(int p_nCycle, bool p_bFilePerCycle = true);
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
	void SetEventPush(int p_nPush);
	void SetEventPull(int p_nPull);
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

	bool LoadCameraScript(std::string p_strCameraScript)
	{
		if (p_strCameraScript.empty())
			return false;

		std::ifstream cameraFile;
		cameraFile.open(p_strCameraScript.c_str(), std::ios::binary);

		if (!cameraFile.is_open())
			return false;
		
		std::string line, cameraType, 
			deltaTime, generation, range, angle;

		std::stringstream cameraPathString;
		std::vector<std::string> cameraPathVector;

		std::getline(cameraFile, cameraType); boost::trim(cameraType);
		std::getline(cameraFile, deltaTime); boost::trim(deltaTime);
		std::getline(cameraFile, generation); boost::trim(generation);
		std::getline(cameraFile, range); boost::trim(range);
		std::getline(cameraFile, angle); boost::trim(angle);

		while(std::getline(cameraFile, line))
		{
			boost::trim(line);
			cameraPathVector.push_back(line);
			std::cout << ":: " << line << std::endl;
		}

		cameraFile.close();

		// Now create actual path according
		m_path.Clear();
		
		// Strict camera type
		if (cameraType == "strict")
		{
			std::cout << "Camera Path [Strict]" << std::endl;
			
			cameraPathString << "path={";

			for (int index = 0; index < cameraPathVector.size(); index++)
			{
				cameraPathString << cameraPathVector[index];
				if (index < cameraPathVector.size() - 1)
					cameraPathString << ", ";
			}

			cameraPathString << "}";
		}

		// Shuffle
		else if (cameraType == "shuffle")
		{
			Random random(Platform::GetCycles());

			std::cout << "Camera Path [Shuffle]" << std::endl;

			cameraPathString << "path={";
			
			while (!cameraPathVector.empty())
			{
				int index = random.Next(cameraPathVector.size());
				cameraPathString << cameraPathVector[index];
				cameraPathVector.erase(cameraPathVector.begin() + index);

				if (!cameraPathVector.empty())
					cameraPathString << ",";
			}
			
			cameraPathString << "}";
		}

		// Random
		else if (cameraType == "random")
		{
			Random random(Platform::GetCycles());

			std::cout << "Camera Path [Random]" << std::endl;
			
			float fRange = boost::lexical_cast<float>(range),
				fAngle = boost::lexical_cast<float>(angle);

			int nGeneration = boost::lexical_cast<int>(generation),
				index = random.Next(cameraPathVector.size());
			
			std::vector<Vector3> vertexList;
			ArgumentMap argMap("vertex={" + cameraPathVector[index] + "}");
			argMap.GetArgument("vertex", vertexList);

			Vector3 position = vertexList[0],
				heading = vertexList[1];

			cameraPathString << "path={" << cameraPathVector[index] << ", ";

			for (index = 0; index < nGeneration; index++)
			{
				Vector3 displacement = 
					Montecarlo::UniformSampleSphere(random.NextFloat(), random.NextFloat()); 
				
				displacement.Y = 0.0f; displacement.Normalize(); 
				position += displacement * fRange;

				heading.X += ((random.NextFloat() - 0.5f) * 2.0f) * fAngle;

				cameraPathString << "{" << position.X << ", " << position.Y << ", " << position.Z << "}, {" 
					<< heading.X << ", " << heading.Y << ", " << heading.Z << "}";

				if (index < nGeneration - 1) cameraPathString << ", ";
			}
		}

		// Delta time parameter
		m_nFrameNumber = 0;
		m_fDeltaTime = boost::lexical_cast<float>(deltaTime);
		std::cout << "-- Delta Time [" << m_fDeltaTime << "]" << std::endl;
		std::cout << "-- Path [" << cameraPathString.str() << "]" << std::endl;

		// Build path from string;
		m_path.FromString(cameraPathString.str());

		return true;
	}

	void BeginPath(IIlluminaMT *p_pIlluminaMT)
	{
		ICamera* pCamera = p_pIlluminaMT->GetEnvironment()->GetCamera();
		Vector3 position, lookAt, position2;

		m_fDeltaTime += 0.002f; if (m_fDeltaTime > 1.f) m_fDeltaTime = 1.f;

		m_path.Get(m_fDeltaTime, position, lookAt);
		pCamera->MoveTo(position); 
		pCamera->LookAt(lookAt);
	}
};
