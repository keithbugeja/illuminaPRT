#pragma once

#include <External/Compression/Compression.h>

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
};

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