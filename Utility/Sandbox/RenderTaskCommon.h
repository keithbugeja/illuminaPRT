//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskCommon.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include <boost/asio.hpp>
#include <External/Compression/Compression.h>
#include <Maths/Maths.h>

#include "Half.h"
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

class CompressedRadianceContext
{
public:
	unsigned short Direct[3];
	unsigned short Indirect[3];
	unsigned short Albedo[3];

	unsigned short Normal[2];
	// unsigned short Position[3];

	unsigned short Flags;
};

class CompressedRadianceBuffer
	: public Buffer2D<CompressedRadianceContext>
{ 
protected:
	inline void XYZToRGB(const Spectrum &p_xyz, Spectrum &p_rgb)
	{
		p_rgb[0] = (float)(2.690*p_xyz[0] + -1.276*p_xyz[1] + -0.414*p_xyz[2]);
		p_rgb[0] = (float)(-1.022*p_xyz[0] +  1.978*p_xyz[1] +  0.044*p_xyz[2]);
		p_rgb[0] = (float)( 0.061*p_xyz[0] + -0.224*p_xyz[1] +  1.163*p_xyz[2]);
	}

	inline void RGBToXYZ(const Spectrum &p_rgb, Spectrum &p_xyz)
	{
		p_xyz[0] =  (float)(0.497*p_rgb[0] +  0.339*p_rgb[1] +  0.164*p_rgb[2]);
		p_xyz[1] =  (float)(0.256*p_rgb[0] +  0.678*p_rgb[1] +  0.066*p_rgb[2]);
		p_xyz[2] =  (float)(0.023*p_rgb[0] +  0.113*p_rgb[1] +  0.864*p_rgb[2]);
	}
	
	inline unsigned int ToLogLuv(const Spectrum &p_rgb) 
	{
		Spectrum xyz;

		RGBToXYZ(p_rgb, xyz);

		float xyza = (xyz[0] + xyz[1] + xyz[2]);

		float x = xyz[0] / xyza,
			y = xyz[1] / xyza;

		unsigned short Le = (unsigned short)Maths::Floor(256.f * (Maths::Log(xyz[1] + 64)));

		float d = (-2 * x + 12 * y + 3),
			u = (4 * x) / d,
			v = (9 * y) / d;
		
		unsigned char ue = (unsigned char)(410.f * u);
		unsigned char ve = (unsigned char)(410.f * v);
	
		return (Le << 16) | (ue << 8) | ve; 
	}

	inline void FromLogLuv(unsigned int p_nLogLuv, Spectrum &p_rgb)
	{
	}

	inline unsigned short CompressFloat(float p_fValue)
	{		
		return half_from_float(*(reinterpret_cast<uint32_t*>(&p_fValue)));
	}

	inline float DecompressFloat(unsigned short h)
	{
		uint32_t f = half_to_float(h);
		return *(reinterpret_cast<float*>(&f));		
	}

	inline void CompressVector(Vector3 *p_pVector, unsigned short *p_pCompressedVector)
	{
		p_pCompressedVector[0] = CompressFloat(p_pVector->X);
		p_pCompressedVector[1] = CompressFloat(p_pVector->Y);
		p_pCompressedVector[2] = CompressFloat(p_pVector->Z);
	}

	inline void DecompressVector(unsigned short *p_pCompressedVector, Vector3 *p_pVector)
	{
		p_pVector->X = DecompressFloat(p_pCompressedVector[0]);
		p_pVector->Y = DecompressFloat(p_pCompressedVector[1]);
		p_pVector->Z = DecompressFloat(p_pCompressedVector[2]);
	}

	inline void CompressNormal(Vector3 *p_pNormal, unsigned short *p_pCompressedNormal) 
	{
		p_pCompressedNormal[0] = CompressFloat(p_pNormal->X);
		p_pCompressedNormal[1] = CompressFloat(p_pNormal->Y);
	}

	inline void DecompressNormal(unsigned short *p_pCompressedNormal, Vector3 *p_pNormal) 
	{
		p_pNormal->X = DecompressFloat(p_pCompressedNormal[0]);
		p_pNormal->Y = DecompressFloat(p_pCompressedNormal[1]);
		p_pNormal->Z = Maths::Sqrt(1 - p_pNormal->X - p_pNormal->Y);
	}

	inline void CompressSpectrum(Spectrum *p_pSpectrum, unsigned short *p_pCompressedSpectrum) 
	{
		p_pCompressedSpectrum[0] = CompressFloat((*p_pSpectrum)[0]);
		p_pCompressedSpectrum[1] = CompressFloat((*p_pSpectrum)[1]);
		p_pCompressedSpectrum[2] = CompressFloat((*p_pSpectrum)[2]);
	}

	inline void DecompressSpectrum(unsigned short *p_pCompressedSpectrum, Spectrum *p_pSpectrum) 
	{ 
		(*p_pSpectrum)[0] = DecompressFloat(p_pCompressedSpectrum[0]);
		(*p_pSpectrum)[1] = DecompressFloat(p_pCompressedSpectrum[1]);
		(*p_pSpectrum)[2] = DecompressFloat(p_pCompressedSpectrum[2]);
	}

public:
	CompressedRadianceBuffer(int p_nWidth, int p_nHeight)
		: Buffer2D<CompressedRadianceContext>::Buffer2D(p_nWidth, p_nHeight)
	{ }

	CompressedRadianceBuffer(int p_nWidth, int p_nHeight, CompressedRadianceContext *p_pBuffer)
		: Buffer2D<CompressedRadianceContext>::Buffer2D(p_nWidth, p_nHeight, p_pBuffer)
	{ }

	void FromRadianceBuffer(RadianceBuffer *p_pRadianceBuffer) 
	{
		RadianceContext *pSrc = p_pRadianceBuffer->GetP(0,0);
		CompressedRadianceContext *pDst = this->GetP(0,0);

		for (int i = 0; i < p_pRadianceBuffer->GetArea(); i++)
		{
			CompressSpectrum(&(pSrc->Albedo), pDst->Albedo);
			CompressSpectrum(&(pSrc->Direct), pDst->Direct);
			CompressSpectrum(&(pSrc->Indirect), pDst->Indirect);
			CompressNormal(&(pSrc->Normal), pDst->Normal);
			// CompressVector(&(pSrc->Position), pDst->Position);

			pDst->Flags = pSrc->Flags;

			pDst++; pSrc++;
		}
	}
	
	void ToRadianceBuffer(RadianceBuffer *p_pRadianceBuffer) 
	{ 
		CompressedRadianceContext *pSrc = this->GetP(0,0);
		RadianceContext *pDst = p_pRadianceBuffer->GetP(0,0);

		for (int i = 0; i < p_pRadianceBuffer->GetArea(); i++)
		{
			DecompressSpectrum(pSrc->Albedo, &(pDst->Albedo));
			DecompressSpectrum(pSrc->Direct, &(pDst->Direct));
			DecompressSpectrum(pSrc->Indirect, &(pDst->Indirect));
			DecompressNormal(pSrc->Normal, &(pDst->Normal));
			// DecompressVector(pSrc->Position, &(pDst->Position));

			pDst->Final = pDst->Direct + pDst->Indirect;
			pDst->Flags = pSrc->Flags;
			
			pDst++; pSrc++;
		}
	}
};

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
		*m_pImageBuffer;

	size_t m_nBufferSize;

	int *m_pStagingBuffer,
		*m_pStagingImageBuffer;

	size_t m_nStagingBufferSize;

	char *m_pCompressedBuffer;
	size_t m_nCompressedSize;

	int *m_pTransferBuffer;
	size_t m_nTransferSize;

	RadianceBuffer *m_pRadianceBuffer;
	CompressedRadianceBuffer *m_pStagingRadianceBuffer;

public:
	SerialisableRenderTile(int p_nID, int p_nMaxWidth, int p_nMaxHeight)
	{
		// Buffer, id, width, height
		m_nBufferSize = p_nMaxWidth * p_nMaxHeight * sizeof(RadianceContext) +
			sizeof(int) * ucHdr_count;

		// Staging buffer 
		m_nStagingBufferSize = p_nMaxWidth * p_nMaxHeight * sizeof(CompressedRadianceContext) + 
			sizeof(int) * ucHdr_count;

		// Initialise unknown quantities
		m_nCompressedSize = 
			m_nTransferSize = 0;

		// Buffer allocations
		m_pBuffer = (int*)new char[m_nBufferSize];
		m_pBuffer[ucHdr_id] = -1;
		m_pBuffer[ucHdr_width] = p_nMaxWidth;
		m_pBuffer[ucHdr_height] = p_nMaxHeight;
		m_pImageBuffer = m_pBuffer + ucHdr_count; 

		// Staging buffer
		m_pStagingBuffer = (int*)new char[m_nStagingBufferSize];
		m_pStagingImageBuffer = m_pStagingBuffer + ucHdr_count;

		// Transfer and compressed bufer
		m_pTransferBuffer = (int*)new char[m_nBufferSize * 2 + sizeof(int)];
		m_pCompressedBuffer = (char*)(m_pTransferBuffer + 1);

		m_pRadianceBuffer = new RadianceBuffer(p_nMaxWidth, p_nMaxHeight, (RadianceContext*)m_pImageBuffer);
		m_pStagingRadianceBuffer = new CompressedRadianceBuffer(p_nMaxHeight, p_nMaxHeight, (CompressedRadianceContext*)m_pStagingImageBuffer); 
	}

	~SerialisableRenderTile(void) 
	{
		delete[] m_pBuffer;
		delete[] m_pStagingBuffer;
		delete[] m_pTransferBuffer;
		delete m_pRadianceBuffer;
		delete m_pStagingRadianceBuffer;
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

		m_nBufferSize = width * height * sizeof(RadianceContext) +
			sizeof(int) * ucHdr_count;

		m_nStagingBufferSize = width * height * sizeof(CompressedRadianceContext) +
			sizeof(int) * ucHdr_count;

		Safe_Delete(m_pRadianceBuffer);
		Safe_Delete(m_pStagingRadianceBuffer);

		m_pRadianceBuffer = new RadianceBuffer(width, height, (RadianceContext*)m_pImageBuffer);
		m_pStagingRadianceBuffer = new CompressedRadianceBuffer(width, height, (CompressedRadianceContext*)m_pStagingImageBuffer);
	}

	inline int GetID(void) const { return m_pBuffer[ucHdr_id]; }
	inline void SetID(int p_nID) { m_pBuffer[ucHdr_id] = p_nID; }

	inline int GetWidth(void) const { return m_pBuffer[ucHdr_width]; }
	inline int GetHeight(void) const { return m_pBuffer[ucHdr_height]; }

	inline void Package(void) 
	{
		m_pStagingRadianceBuffer->FromRadianceBuffer(m_pRadianceBuffer);
		m_pStagingBuffer[ucHdr_id] = m_pBuffer[ucHdr_id];
		m_pStagingBuffer[ucHdr_width] = m_pBuffer[ucHdr_width];
		m_pStagingBuffer[ucHdr_height] = m_pBuffer[ucHdr_height];

		m_nCompressedSize = Compress();

		m_pTransferBuffer[cHdr_size] = m_nStagingBufferSize;
		m_nTransferSize = m_nCompressedSize + sizeof(int);
	}

	inline void Unpackage(void)
	{
		m_nStagingBufferSize = m_pTransferBuffer[cHdr_size];
		
		Decompress();

		m_pBuffer[ucHdr_id]	= m_pStagingBuffer[ucHdr_id];
		m_pBuffer[ucHdr_width] = m_pStagingBuffer[ucHdr_width];
		m_pBuffer[ucHdr_height]	= m_pStagingBuffer[ucHdr_height];
		
		Reinterpret();
	
		m_pStagingRadianceBuffer->ToRadianceBuffer(m_pRadianceBuffer);
	}

	inline size_t Compress(void) 
	{
		return Illumina::Core::Compressor::Compress((char*)m_pStagingBuffer, m_nStagingBufferSize, m_pCompressedBuffer);

		/* Uncomment to disable compression */
		/* memcpy((void*)m_pCompressedBuffer, (void*)m_pStagingBuffer, m_nStagingBufferSize);
		 * return m_nStagingBufferSize;
		 */
	}

	inline void Decompress(void) 
	{
		Illumina::Core::Compressor::Decompress(m_pCompressedBuffer, m_nStagingBufferSize, (char*)m_pStagingBuffer);
		
		/* Uncomment to disable compression */
		/* memcpy((void*)m_pStagingBuffer, (void*)m_pCompressedBuffer, m_nStagingBufferSize);
		 */
	}

	inline char* GetTransferBuffer(void) const {
		return (char*)m_pTransferBuffer;
	}

	inline size_t GetTransferBufferSize(void) const {
		return m_nTransferSize;
	}

	inline RadianceBuffer* GetImageData(void) { return m_pRadianceBuffer; }	
};

/*
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
*/
//----------------------------------------------------------------------------------------------
