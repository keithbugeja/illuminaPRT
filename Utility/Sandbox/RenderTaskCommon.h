//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskCommon.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include <boost/asio.hpp>
#include <External/Compression/Compression.h>
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
class RenderTilePackets
{
public:
	struct Packet
	{
		short XStart, YStart; 
		short XSize, YSize;
	};

public:
	std::vector<RenderTilePackets::Packet> m_packetList;

public:
	Packet& GetPacket(int p_nPacketID) {
		return m_packetList[p_nPacketID];
	}

	size_t GetPacketCount(void) const {
		return m_packetList.size();
	}

	void GeneratePackets(int p_nFrameWidth, int p_nFrameHeight, int p_nTileSize, int p_nStepSize)
	{
		int tileSize = p_nTileSize,
			varTileSize = tileSize;

		int tilesPerRow = p_nFrameWidth / tileSize,
			tilesPerCol = p_nFrameHeight / tileSize,
			tilesPerPage = tilesPerRow * tilesPerCol;

		int tileX, tileY, 
			stepIncrement = 0, 
			currentStep = 1;

		Packet tilePacket;

		m_packetList.clear();

		for (int tileID = 0; tileID < tilesPerPage; tileID++)
		{
			tileX = (tileID % tilesPerRow) * tileSize,
			tileY = (tileID / tilesPerRow) * tileSize;

			tilePacket.XSize = varTileSize;
			tilePacket.YSize = varTileSize;

			for (int subTileY = 0; subTileY < currentStep; subTileY++)
			{
				for (int subTileX = 0; subTileX < currentStep; subTileX++)
				{
					tilePacket.XStart = tileX + subTileX * varTileSize;
					tilePacket.YStart = tileY + subTileY * varTileSize;
							
					m_packetList.push_back(tilePacket);
				}
			}
				
			if (varTileSize <= 16)
				continue;

			if (++stepIncrement == p_nStepSize)
			{
				stepIncrement = 0;
				currentStep <<= 1;
				varTileSize >>= 1;
			}
		}
	}
};
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
struct RenderTaskContext
{
	int TileWidth,
		TileHeight,
		FrameWidth,
		FrameHeight,
		TilesPerRow,
		TilesPerColumn,
		TotalTiles,
		TileBatchSize,
		WorkersRequired;

	bool AdaptiveTiles;

	RenderTilePackets TilePackets;
};
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
class SerialisableRenderTile
{
private:
	enum Uncompressed 
	{
		ucHdr_id = 0,
		ucHdr_width = 1,
		ucHdr_height = 2,
		ucHdr_count = 3
	};

	enum Compressed
	{
		cHdr_size = 0,
		cHdr_count = 1
	};

protected:
	int *m_pBuffer,
		*m_pCompressableBuffer,
		*m_pImageBuffer;
	
	size_t m_nCompressableSize;

	char *m_pCompressedBuffer;
	size_t m_nCompressedSize;

	int *m_pTransferBuffer;
	size_t m_nTransferSize;

	RadianceBuffer *m_pRadianceBuffer;

public:
	SerialisableRenderTile(int p_nID, int p_nMaxWidth, int p_nMaxHeight)
	{
		// Buffer, id, width, height
		m_nCompressableSize = p_nMaxWidth * p_nMaxHeight * sizeof(RadianceContext) +
			sizeof(int) * ucHdr_count;

		// Initialise unknown quantities
		m_nCompressedSize = 
			m_nTransferSize = 0;

		// Buffer allocations
		m_pBuffer = (int*)new char[m_nCompressableSize];
		m_pBuffer[ucHdr_id] = -1;
		m_pBuffer[ucHdr_width] = p_nMaxWidth;
		m_pBuffer[ucHdr_height] = p_nMaxHeight;

		m_pImageBuffer = m_pBuffer + ucHdr_count; 

		m_pTransferBuffer = (int*)new char[m_nCompressableSize * 2 + sizeof(int)];
		m_pCompressedBuffer = (char*)(m_pTransferBuffer + 1);

		m_pRadianceBuffer = new RadianceBuffer(p_nMaxWidth, p_nMaxHeight, (RadianceContext*)m_pImageBuffer);
	}

	~SerialisableRenderTile(void) 
	{
		delete[] m_pBuffer;
		delete[] m_pTransferBuffer;
		delete m_pRadianceBuffer;
	}

	inline void Resize(int p_nWidth, int p_nHeight)
	{
		m_pBuffer[ucHdr_width] = p_nWidth;
		m_pBuffer[ucHdr_height] = p_nHeight;

		Reinterpret();
	}

	inline void Reinterpret(void) 
	{
		int width = m_pBuffer[ucHdr_width],
			height = m_pBuffer[ucHdr_height];

		m_nCompressableSize = width * height * sizeof(RadianceContext) +
			sizeof(int) * ucHdr_count;

		Safe_Delete(m_pRadianceBuffer);

		m_pRadianceBuffer = new RadianceBuffer(width, height, (RadianceContext*)m_pImageBuffer);
	}

	inline int GetID(void) const { return m_pBuffer[ucHdr_id]; }
	inline void SetID(int p_nID) { m_pBuffer[ucHdr_id] = p_nID; }

	inline int GetWidth(void) const { return m_pBuffer[ucHdr_width]; }
	inline int GetHeight(void) const { return m_pBuffer[ucHdr_height]; }

	inline void Package(void) 
	{
		m_nCompressedSize = Compress();
		m_pTransferBuffer[cHdr_size] = m_nCompressableSize;
		m_nTransferSize = m_nCompressedSize + sizeof(int);
	}

	inline void Unpackage(void)
	{
		m_nCompressableSize = m_pTransferBuffer[cHdr_size];
		Decompress();
		Reinterpret();
	}

	inline size_t Compress(void) 
	{
		m_nCompressedSize = Illumina::Core::Compressor::Compress((char*)m_pBuffer, m_nCompressableSize, m_pCompressedBuffer);
		return m_nCompressedSize;
	}

	inline void Decompress(void) 
	{
		Illumina::Core::Compressor::Decompress(m_pCompressedBuffer, m_nCompressableSize, (char*)m_pBuffer);
	}

	inline char* GetTransferBuffer(void) const {
		return (char*)m_pTransferBuffer;
	}

	inline size_t GetTransferBufferSize(void) const {
		return m_nTransferSize;
	}

	inline RadianceBuffer* GetImageData(void) { return m_pRadianceBuffer; }	
};
//----------------------------------------------------------------------------------------------
