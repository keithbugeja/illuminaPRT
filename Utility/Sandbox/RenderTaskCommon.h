//----------------------------------------------------------------------------------------------
//	Filename:	RenderTaskCommon.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------------------------------
#include <boost/asio.hpp>

#include <Maths/Maths.h>
#include <Geometry/Spline.h>
#include <External/Compression/Compression.h>

#include "Half.h"
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
#define __TaskID "taskid"
#define __Job_Name "job_name"
#define __Job_User "job_user"
#define __Tile_Distribution_Adaptive "tile_distribution_adaptive"
#define __Tile_Distribution_Batchsize "tile_distribution_batchsize"
#define __Tile_Height "tile_height"
#define __Tile_Width "tile_width"
#define __Device_Override "device_override"
#define __Device_Height "device_height"
#define __Device_Width "device_width"
#define __Device_Type "device_type"
#define __Device_Stream_IP "device_stream_ip"
#define __Device_Stream_Port "device_stream_port"
#define __Device_Stream_Bitrate "device_stream_bitrate"
#define __Device_Stream_Codec "device_stream_codec"
#define __Device_Stream_Framerate "device_stream_framerate"
#define __Device_Sequence_BufferedFrames "device_sequence_bufferedframes"
#define __Device_Sequence_Details "device_sequence_details"
#define __Device_Sequence_Format "device_sequence_format"
#define __Device_Sequence_Prefix "device_sequence_prefix"
#define __Device_Image_Prefix "device_image_prefix"
#define __Device_Image_Format "device_image_format"
#define __Device_Image_Timestamp "device_image_timestamp"
#define __Resource_Cap_Max "resource_cap_max"
#define __Resource_Cap_Min "resource_cap_min"
#define __Resource_Deadline_Enabled "resource_deadline_enabled"
#define __Resource_Deadline_Framerate "resource_deadline_fps"
#define __Script_Name "script_name"
//----------------------------------------------------------------------------------------------

#define __PPF_Tonemapping		0x0001
#define __PPF_Discontinuity		0x0002
#define	__PPF_BilateralFilter	0x0004

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
struct SynchronisePacket
{
	Vector3		observerPosition;
	Vector3		observerTarget;
	int			seed;
};
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

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
class CompressedRadianceContext
{
public:
	// unsigned short Direct[3];
	// unsigned short Indirect[3];
	// unsigned short Albedo[3];

	// unsigned int Final;
	unsigned char Final[3];
	
	/*
	unsigned int Direct;
	unsigned int Indirect;
	unsigned int Albedo;

	unsigned short Normal[2];
	unsigned short Distance;

	unsigned short Flags;
	*/
};
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
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
	
	inline unsigned int ToRGBE(Spectrum &p_rgb)
	{
		float v;
		int e;

		float red = p_rgb[0],
			green = p_rgb[1],
			blue = p_rgb[2];

		unsigned int r;
		unsigned char *rgbe = (unsigned char*)&r;

		v = red;
		if (green > v) v = green;
		if (blue > v) v = blue;
		if (v < 1e-32) {
			rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
		}
		else {
			v = frexp(v,&e) * 256.0/v;
			rgbe[0] = (unsigned char) (red * v);
			rgbe[1] = (unsigned char) (green * v);
			rgbe[2] = (unsigned char) (blue * v);
			rgbe[3] = (unsigned char) (e + 128);
		}

		return r;
	}

	inline void FromRGBE(unsigned int p_nRGBE, Spectrum &p_rgb)
	{
		float f;
		unsigned char *rgbe = (unsigned char*)&p_nRGBE;

		if (rgbe[3]) {   /*nonzero pixel*/
			f = ldexp(1.0,rgbe[3]-(int)(128+8));
			p_rgb[0] = rgbe[0] * f;
			p_rgb[1] = rgbe[1] * f;
			p_rgb[2] = rgbe[2] * f;
		}
		else
			p_rgb = 0.0;
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

	inline void CompressSpectrum(Spectrum *p_pSpectrum, unsigned int *p_pCompressedSpectrum)
	{
		*p_pCompressedSpectrum = ToRGBE(*p_pSpectrum);
	}

	inline void DecompressSpectrum(unsigned int *p_pCompressedSpectrum, Spectrum *p_pSpectrum)
	{
		FromRGBE(*p_pCompressedSpectrum, *p_pSpectrum);
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
			// pDst->Flags = pSrc->Flags;

			// pDst->Distance = (pSrc->Distance);
			
			// CompressSpectrum(&(pSrc->Direct), &(pDst->Direct));

			/*
			CompressSpectrum(&(pSrc->Albedo), &(pDst->Albedo));
			CompressSpectrum(&(pSrc->Indirect), &(pDst->Indirect));
			CompressNormal(&(pSrc->Normal), pDst->Normal);
			*/

			// CompressSpectrum(&(pSrc->Final), &(pDst->Final));
			pDst->Final[0] = (unsigned char)(pSrc->Final[0] * 255);
			pDst->Final[1] = (unsigned char)(pSrc->Final[1] * 255);
			pDst->Final[2] = (unsigned char)(pSrc->Final[2] * 255);

			//CompressSpectrum(&(pSrc->Albedo), pDst->Albedo);
			//CompressSpectrum(&(pSrc->Direct), pDst->Direct);
			//CompressSpectrum(&(pSrc->Indirect), pDst->Indirect);
			//CompressNormal(&(pSrc->Normal), pDst->Normal);
			//CompressVector(&(pSrc->Position), pDst->Position);

			pDst++; pSrc++;
		}
	}
	
	void ToRadianceBuffer(RadianceBuffer *p_pRadianceBuffer) 
	{ 
		CompressedRadianceContext *pSrc = this->GetP(0,0);
		RadianceContext *pDst = p_pRadianceBuffer->GetP(0,0);

		for (int i = 0; i < p_pRadianceBuffer->GetArea(); i++)
		{
			pDst->Final[0] = ((float)pSrc->Final[0]) / 255.0f;
			pDst->Final[1] = ((float)pSrc->Final[1]) / 255.0f;
			pDst->Final[2] = ((float)pSrc->Final[2]) / 255.0f;

			// DecompressSpectrum(&(pSrc->Final), &(pDst->Final));

			//pDst->Flags = pSrc->Flags;		
			//pDst->Distance = DecompressFloat(pSrc->Distance);

			//DecompressSpectrum(&(pSrc->Direct), &(pDst->Direct));

			/*
			DecompressSpectrum(&(pSrc->Albedo), &(pDst->Albedo));
			DecompressSpectrum(&(pSrc->Indirect), &(pDst->Indirect));
			DecompressNormal(pSrc->Normal, &(pDst->Normal));
			*/

			//DecompressSpectrum(pSrc->Albedo, &(pDst->Albedo));
			//DecompressSpectrum(pSrc->Direct, &(pDst->Direct));
			//DecompressSpectrum(pSrc->Indirect, &(pDst->Indirect));
			//DecompressNormal(pSrc->Normal, &(pDst->Normal));
			//DecompressVector(pSrc->Position, &(pDst->Position));

			// pDst->Albedo = 1.0f;
			// pDst->Indirect = 0.f;
			// pDst->Distance = 1.f;
			// pDst->Flags = RadianceContext::DF_Direct;

			// pDst->Final = pDst->Direct + pDst->Indirect;
			
			pDst++; pSrc++;
		}
	}
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

		/* 
		 * m_nStagingBufferSize = sizeof(int) * ucHdr_count;
		 * memcpy((void*)m_pCompressedBuffer, (void*)m_pStagingBuffer, m_nStagingBufferSize);
		 * return m_nStagingBufferSize;
		 */

		/* Uncomment to disable compression */
		/* memcpy((void*)m_pCompressedBuffer, (void*)m_pStagingBuffer, m_nStagingBufferSize);
		 * return m_nStagingBufferSize;
		 */
	}

	inline void Decompress(void) 
	{
		Illumina::Core::Compressor::Decompress(m_pCompressedBuffer, m_nStagingBufferSize, (char*)m_pStagingBuffer);

		/* Uncomment to send only control data */
		/* m_nStagingBufferSize = sizeof(int) * ucHdr_count;
		 * memcpy((void*)m_pStagingBuffer, (void*)m_pCompressedBuffer, m_nStagingBufferSize);
		 */

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
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
class Path
{
protected:
	std::vector<Vector3> m_vertexList;
	std::vector<float> m_pivotList;
	float m_fTime;
public:
	bool IsEmpty(void) { return m_vertexList.empty(); }
	void Clear(void) { m_vertexList.clear(); m_pivotList.clear(); Reset(); } 
	void Reset(void) { m_fTime = 0; }
	void Move(float p_fDeltaT) { m_fTime += p_fDeltaT; }

	void AddVertex(const Vector3 &p_pVertex) 
	{ 
		m_vertexList.push_back(p_pVertex); 
	}

	void PreparePath(void)
	{
		Illumina::Core::Interpolator::ComputePivots(m_vertexList, m_pivotList);
	}

	Vector3 GetPosition(float p_fTime)
	{
		if (m_vertexList.size() <= 2)
			return Vector3::Zero;

		return Illumina::Core::Interpolator::Lagrange(m_vertexList, m_pivotList, p_fTime);
	}

	Vector3 GetPosition(void) 
	{
		return GetPosition(m_fTime);
	}
};
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
struct PathVertexEx
{
	Vector3 position;
	Vector3 orientation;

	// size_t size(void) {return 0x06;}
	// float operator[](int index) const { return element[index]; }
	// float& operator[](int index) { return element[index]; }
};

class PathEx
{
protected:
	// std::vector<PathVertexEx> m_vertexList;
	std::vector<Vector3> m_positionList,
		m_orientationList;
	std::vector<float> m_pivotList;
	float m_fTime;
public:
	bool IsEmpty(void) { return m_positionList.empty(); }
	void Clear(void) { 
		m_positionList.clear(); 
		m_orientationList.clear(); 
		m_pivotList.clear(); 
		Reset(); 
	} 
	
	void Reset(void) { m_fTime = 0; }
	void Move(float p_fDeltaT) { m_fTime += p_fDeltaT; }

	void AddVertex(const PathVertexEx &p_pVertex) 
	{ 
		m_positionList.push_back(p_pVertex.position);
		m_orientationList.push_back(p_pVertex.orientation);
	}

	void PreparePath(void)
	{
		Illumina::Core::Interpolator::ComputePivots(m_orientationList, m_pivotList);
	}

	void Get(float p_fTime, Vector3 &p_position, Vector3 &p_lookat)
	{
		if (m_positionList.size() <= 2)
		{
			p_position = Vector3::Zero;
			p_lookat = Vector3::UnitZPos;
		} 
		else
		{
			// PathVertexEx pathVertexEx = Illumina::Core::Interpolator::Lagrange(m_vertexList, m_pivotList, p_fTime);
			p_position = Illumina::Core::Interpolator::Lagrange(m_positionList, m_pivotList, p_fTime);
			p_lookat = Illumina::Core::Interpolator::Lagrange(m_orientationList, m_pivotList, p_fTime);
		}
	}

	void Get(Vector3 &p_position, Vector3 &p_lookat)
	{
		Get(m_fTime, p_position, p_lookat);
	}
};
