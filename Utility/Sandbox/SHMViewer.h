#pragma once

#include "Environment.h"

#if (defined __PLATFORM_WINDOWS__)
#include <boost/interprocess/windows_shared_memory.hpp>
#endif

#include <boost/interprocess/mapped_region.hpp>

class SHMViewer
{
protected:
	Illumina::Core::DisplayDevice m_displayDevice;

	#if (defined __PLATFORM_WINDOWS__)
	boost::interprocess::windows_shared_memory
		*m_pSharedMemorySink;
	#else
	boost::interprocess::shared_memory_object
		*m_pSharedMemorySink;
	#endif

	boost::interprocess::mapped_region 
		*m_pSharedMemoryRegion;

	bool m_bIsOpen;

	int m_nWidth,
		m_nHeight,
		m_nSegmentSize;

public:
	SHMViewer(int p_nWidth, int p_nHeight)
		: m_nWidth(p_nWidth)
		, m_nHeight(p_nHeight)
		, m_nSegmentSize(p_nWidth * p_nHeight * 3)
		, m_bIsOpen(false)
		, m_displayDevice(p_nWidth, p_nHeight)
	{ }

	bool Open(void)
	{
		if (m_bIsOpen) return false;

		if (!m_displayDevice.Open())
			return false;

		#if (defined __PLATFORM_WINDOWS__)
		try
		{
		// Create a native windows shared memory object.
		m_pSharedMemorySink = new boost::interprocess::windows_shared_memory(
			boost::interprocess::open_only, "Global\\IlluminaPRT_OutputSink", 
			boost::interprocess::read_only);
		}
		catch(boost::interprocess::interprocess_exception &ex)
		{
			std::cout << "Unexpected exception: " << ex.what() << std::endl;
		}
#else
		// Create shared memory object for POSIX compliant systems
		// ...
		#endif
	
		// Map the whole shared memory in this process
		m_pSharedMemoryRegion = new boost::interprocess::mapped_region(
			*m_pSharedMemorySink, boost::interprocess::read_only);

		m_bIsOpen = true;

		return true;
	}

	IDevice* SetDummyOutput(void)
	{
		SharedMemoryDevice *shm = new SharedMemoryDevice(512, 512, "IlluminaPRT_OutputSink");
		shm->Open();
		return shm;
	}

	void Update(void)
	{
		m_displayDevice.BeginFrame();

		Spectrum s;
		unsigned char* channel = (unsigned char*)m_pSharedMemoryRegion->get_address();

		for (int y = 0; y < m_nHeight; y++)
		{
			for (int x = 0; x < m_nWidth; x++)
			{
				s.Set(*channel++, *channel++, *channel++);
				m_displayDevice.Set(x, y, s / 255.0f);
			}
		}

		m_displayDevice.EndFrame();
	}

	void Close(void)
	{
		if (m_bIsOpen)
		{
			// Free allocations
			delete m_pSharedMemoryRegion;
			delete m_pSharedMemorySink;

			m_displayDevice.Close();

			// Close viewer channel to shm
			m_bIsOpen = false;
		}
	}
};