#pragma once

//----------------------------------------------------------------------------------------------
#include "Path.h"
#include "Logger.h"
#include "Export.h"
#include "Environment.h"

class IIlluminaMT;

class IlluminaMTListener
{
public:
	virtual void OnBeginRender(IIlluminaMT *p_pIlluminaMT) { };
	virtual void OnEndRender(IIlluminaMT *p_pIlluminaMT) { };
	virtual void OnBeginFrame(IIlluminaMT *p_pIlluminaMT) { };
	virtual void OnEndFrame(IIlluminaMT *p_pIlluminaMT) { };
};

class IIlluminaMT
{
	struct Flags
	{
	protected:
		union 
		{
			struct 
			{
				unsigned int	ToneMapping : 1,
								BilateralFilter: 1,
								DiscontinuityBuffer : 1,
								FrameReconstruction : 1,
								AccumulationBuffer : 1,
								OutputToNullDevice : 1;
			};

			int m_nFlags;
		};

	public:
		Flags(void) : m_nFlags(0) { }
		Flags(int p_nFlags) : m_nFlags(p_nFlags) { }

		bool IsToneMappingEnabled(void) const { return ToneMapping != 0; }
		bool IsBilateralFilterEnabled(void) const { return BilateralFilter != 0; }
		bool IsDiscontinuityBufferEnabled(void) const { return DiscontinuityBuffer != 0; }
		bool IsFrameReconstructionEnabled(void) const { return FrameReconstruction != 0; }
		bool IsAccumulationBufferEnabled(void) const { return AccumulationBuffer != 0; }
		bool IsOutputToNullDevice(void) const { return OutputToNullDevice != 0; }

		void EnableToneMapping(bool p_bEnable) { ToneMapping = p_bEnable ? 1 : 0; }
		void EnableBilateralFilter(bool p_bEnable) { BilateralFilter = p_bEnable ? 1 : 0; }
		void EnableDiscontinuityBuffer(bool p_bEnable) { DiscontinuityBuffer = p_bEnable ? 1 : 0; }
		void EnableFrameReconstruction(bool p_bEnable) { FrameReconstruction = p_bEnable ? 1 : 0; }
		void EnableAccumulationBuffer(bool p_bEnable) { AccumulationBuffer = p_bEnable ? 1 : 0; }
		void EnableOutputToNullDevice(bool p_bEnable) { OutputToNullDevice = p_bEnable ? 1 : 0; }

		void FromInt(int p_nFlags) { m_nFlags = p_nFlags; }
		int ToInt(void) const { return m_nFlags; }
	};

protected:
	IlluminaMTListener *m_pListener;
	SandboxEnvironment	m_sandbox;
	Logger	*m_pLogger;

	Environment *m_pEnvironment;
	EngineKernel *m_pEngineKernel;

	IIntegrator *m_pIntegrator;
	IRenderer *m_pRenderer;
	ICamera *m_pCamera;
	ISpace *m_pSpace;

	int m_nLoggerUpdate,
		m_nThreadCount,
		m_nIterations,
		m_nJobs,
		m_nSize,
		m_nFramesPerSecond;

	float m_fFrameBudget;

	std::string m_strScript;

	Flags m_flags;

public:
	Environment* GetEnvironment(void) const { return m_pEnvironment; }
	virtual RadianceBuffer* GetCommitBuffer(void) { return NULL; }

protected:
	IIlluminaMT(void) 
		: m_pLogger(NULL)
		, m_pListener(NULL)
	{ }

	virtual ~IIlluminaMT(void) { }

public:

	void SetScript(const std::string &p_strScript) { m_strScript = p_strScript; }
	
	void SetLogger(Logger *p_pLogger) { m_pLogger = p_pLogger; }
	void SetLoggerUpdate(int p_nFrameCount) { m_nLoggerUpdate = p_nFrameCount; }
	
	void SetThreadCount(int p_nThreadCount) { m_nThreadCount = p_nThreadCount; }

	void SetIterations(int p_nIterations) { m_nIterations = p_nIterations; }
	void SetFlags(int p_nFlags) { m_flags.FromInt(p_nFlags); }

	void SetFrameBudget(float p_fBudget) {m_fFrameBudget = p_fBudget; }

	void AttachListener(IlluminaMTListener *p_pListener) { m_pListener = p_pListener; }

	void Initialise(void) 
	{
		//----------------------------------------------------------------------------------------------
		// Show flags
		//----------------------------------------------------------------------------------------------
		std::stringstream message;

		message << "-- Tone Mapping [" << m_flags.IsToneMappingEnabled() << "]" << std::endl
			<< "-- Bilateral Filter [" << m_flags.IsBilateralFilterEnabled() << "]" << std::endl
			<< "-- Discontinuity Buffer [" << m_flags.IsDiscontinuityBufferEnabled() << "]" << std::endl
			<< "-- Frame Reconstruction [" << m_flags.IsFrameReconstructionEnabled() << "]" << std::endl
			<< "-- Accumulation Buffer [" << m_flags.IsAccumulationBufferEnabled() << "]" << std::endl
			<< "-- Null Device Output [" << m_flags.IsOutputToNullDevice() << "]" << std::endl;

		m_pLogger->Write(message.str());

		//----------------------------------------------------------------------------------------------
		// Illumina sandbox environment 
		//----------------------------------------------------------------------------------------------
		m_sandbox.Initialise();

		if (!m_sandbox.LoadScene(m_strScript))
		{
			m_sandbox.Shutdown();
			return;
		}

		//----------------------------------------------------------------------------------------------
		// Open render device
		//----------------------------------------------------------------------------------------------
		m_pEnvironment = m_sandbox.GetEnvironment();
		m_pEngineKernel = m_sandbox.GetEngineKernel();

		m_pIntegrator = m_pEnvironment->GetIntegrator();
		m_pRenderer = m_pEnvironment->GetRenderer();
		m_pCamera = m_pEnvironment->GetCamera();
		m_pSpace = m_pEnvironment->GetSpace();

		// Open output device
		if (m_pRenderer != NULL)
			m_pRenderer->GetDevice()->Open();
		else
			m_pLogger->Write("Initialisation failed. No renderer defined!");

		//----------------------------------------------------------------------------------------------
		// Perform type-specific initialisation
		//----------------------------------------------------------------------------------------------
		if (!OnInitialise()) 
			m_pLogger->Write("Initialisation failed. OnInitialised event returned with error!");

		// Initialisation complete
		m_pLogger->Write("Initialisation complete. Rendering in progress...");
	}

	void Shutdown(void)
	{
		OnShutdown();

		//----------------------------------------------------------------------------------------------
		// Shutdown system
		//----------------------------------------------------------------------------------------------
	
		// Close output device
		m_pRenderer->GetDevice()->Close();

		// Shutdown renderer and integrator
		m_pRenderer->Shutdown();
		m_pIntegrator->Shutdown();

		// Shutdown sandbox
		m_sandbox.Shutdown();

		//----------------------------------------------------------------------------------------------
		m_pLogger->Write("Complete :: Press enter to continue"); std::getchar();
	}

	virtual void Render(void) = 0;

	virtual bool OnInitialise(void) = 0;
	virtual bool OnShutdown(void) = 0;
};