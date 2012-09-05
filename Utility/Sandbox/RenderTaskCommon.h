//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskCommon.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
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
		WorkersRequired;

	RenderTilePackets TilePackets;
};
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
class SerialisableRenderTile
{
protected:
	int *m_pBuffer;
	size_t m_nSize;

	char *m_pCompressedBuffer;
	size_t m_nCompressedSize;

	RadianceBuffer *m_pRadianceBuffer;

public:
	SerialisableRenderTile(int p_nID, int p_nWidth, int p_nHeight)
	{
		m_nSize = p_nWidth * p_nHeight * sizeof(RadianceContext) + sizeof(int);
		m_pBuffer = (int*)new char[m_nSize];
		m_pCompressedBuffer = new char[m_nSize * 2];
		m_pRadianceBuffer = new RadianceBuffer(p_nWidth, p_nHeight, (RadianceContext*)(m_pBuffer + 1));
	}

	~SerialisableRenderTile() 
	{
		delete[] m_pBuffer;
		delete[] m_pCompressedBuffer;
		delete m_pRadianceBuffer;
	}

	inline int GetID(void) const { return m_pBuffer[0]; }
	inline void SetID(int p_nID) { m_pBuffer[0] = p_nID; }

	inline int GetWidth(void) const { return m_pRadianceBuffer->GetWidth(); }
	inline int GetHeight(void) const { return m_pRadianceBuffer->GetHeight(); }

	inline size_t Compress(void) 
	{
		m_nCompressedSize = Illumina::Core::Compressor::Compress((char*)m_pBuffer, m_nSize, m_pCompressedBuffer);
		return m_nCompressedSize;
	}

	inline void Decompress(void) 
	{
		Illumina::Core::Compressor::Decompress(m_pCompressedBuffer, m_nSize, (char*)m_pBuffer);
	}

	inline char* GetUncompressedBuffer(void) const {
		return (char*)m_pBuffer;
	}

	inline char* GetCompressedBuffer(void) const {
		return (char*)m_pCompressedBuffer;
	}

	inline size_t GetUncompressedBufferSize(void) const {
		return m_nSize;
	}

	inline size_t GetCompressedBufferSize(void) const {
		return m_nCompressedSize;
	}

	inline void SetCompressedBufferSize(size_t p_nSize) {
		m_nCompressedSize = p_nSize;
	}

	inline RadianceBuffer* GetImageData(void) { return m_pRadianceBuffer; }	
};
//----------------------------------------------------------------------------------------------
/*
//----------------------------------------------------------------------------------------------
class SerialisableVariableRenderTile
{
protected:
	int *m_pBuffer;
	size_t m_nSize;

	char *m_pCompressedBuffer;
	size_t m_nCompressedSize;

	RadianceBuffer *m_pRadianceBuffer;

public:
	SerialisableVariableRenderTile(int p_nID, int p_nMaxWidth, int p_nMaxHeight)
	{
		m_nSize = p_nWidth * p_nHeight * sizeof(RadianceContext) + sizeof(int);
		m_pBuffer = (int*)new char[m_nSize];
		m_pCompressedBuffer = new char[m_nSize * 2];
		m_pRadianceBuffer = new RadianceBuffer(p_nWidth, p_nHeight, (RadianceContext*)(m_pBuffer + 1));
	}

	~SerialisableVariableRenderTile() 
	{
		delete[] m_pBuffer;
		delete[] m_pCompressedBuffer;
		delete m_pRadianceBuffer;
	}

	inline void Reinterpret(int p_nWidth, int p_nHeight)
	{
		if (m_pRadianceBuffer) delete m_pRadianceBuffer;
		m_pRadianceBuffer = new (p_nWidth, p_nHeight, (RadianceContext*)(m_pBuffer + 1));
	}

	inline int GetID(void) const { return m_pBuffer[0]; }
	inline void SetID(int p_nID) { m_pBuffer[0] = p_nID; }

	inline int GetWidth(void) const { return m_pRadianceBuffer->GetWidth(); }
	inline int GetHeight(void) const { return m_pRadianceBuffer->GetHeight(); }

	inline size_t Compress(void) 
	{
		m_nCompressedSize = Illumina::Core::Compressor::Compress((char*)m_pBuffer, m_nSize, m_pCompressedBuffer);
		return m_nCompressedSize;
	}

	inline void Decompress(void) 
	{
		Illumina::Core::Compressor::Decompress(m_pCompressedBuffer, m_nSize, (char*)m_pBuffer);
	}

	inline char* GetUncompressedBuffer(void) const {
		return (char*)m_pBuffer;
	}

	inline char* GetCompressedBuffer(void) const {
		return (char*)m_pCompressedBuffer;
	}

	inline size_t GetUncompressedBufferSize(void) const {
		return m_nSize;
	}

	inline size_t GetCompressedBufferSize(void) const {
		return m_nCompressedSize;
	}

	inline void SetCompressedBufferSize(size_t p_nSize) {
		m_nCompressedSize = p_nSize;
	}

	inline RadianceBuffer* GetImageData(void) { return m_pRadianceBuffer; }	
};
*/
//----------------------------------------------------------------------------------------------
