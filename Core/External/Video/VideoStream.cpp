//----------------------------------------------------------------------------------------------
//	Filename:	VideoStream.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
#define __STDC_CONSTANT_MACROS

extern "C" 
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/opt.h>
}

//----------------------------------------------------------------------------------------------
#include "External/Video/VideoStream.h"
#include "Spectrum/Spectrum.h"
#include "Image/Image.h"
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
// If an FFMPEG helper is created (or rather, when), this function should go in there.
// Also, base it on a look-up table.
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		struct VideoStreamState
		{
			// FFMPEG Library objects
			AVFormatContext *m_pFormatContext;
			AVCodecContext *m_pCodecContext;
			AVOutputFormat *m_pOutputFormat;
			AVStream *m_pStream;
			AVPacket *m_pPacket;
			AVFrame *m_pPicture;
			AVCodec *m_pCodec;

			// Encoding
			int m_nOutputSize,
				m_nOutputBufferSize,
				m_nHadOutput;

			uint8_t *m_pOutputBuffer;
		};
	}
}
//----------------------------------------------------------------------------------------------
int IVideoStream::GetCodec(IVideoStream::VideoCodec p_videoCodec)
{
	switch(p_videoCodec)
	{
		case IVideoStream::MPEG1:
			return CODEC_ID_MPEG1VIDEO;

		case IVideoStream::MPEG2:
			return CODEC_ID_MPEG2VIDEO;

		case IVideoStream::MPEG4:
			return CODEC_ID_MPEG4;

		case IVideoStream::H264:
			return CODEC_ID_H264;

		case IVideoStream::VP8:
			return CODEC_ID_VP8;

		default:
			return CODEC_ID_MPEG1VIDEO;
	}
}
//----------------------------------------------------------------------------------------------
void IVideoStream::ConvertImageToFrame(Image *p_pImage, void *p_pFrame)
{
	// Perform colour space transform from RGB (Image) to YCbCr (AVPicture)
	AVPicture *pPicture = (AVPicture*)p_pFrame;

	int x, y,
		width = p_pImage->GetWidth(),
		height = p_pImage->GetHeight();

	// Y channel
	for (y = 0; y < height; ++y)
	{
		for (x = 0; x < width; ++x)
		{
			RGBPixel pixel = p_pImage->Get(x, y) * 256.f;
			pPicture->data[0][y * pPicture->linesize[0] + x] = (int)((pixel.R * 0.257) + (pixel.G * 0.504) + (pixel.B * 0.098) + 16);
		}
	}

	// Cb and Cr
	for (y = 0; y < height >> 1; ++y)
	{
		for (x = 0; x < width >> 1; ++x)
		{
			RGBPixel pixel = p_pImage->Get(x << 1, y << 1) * 256.f;
			pPicture->data[1][y * pPicture->linesize[1] + x] = (int)(-(pixel.R * 0.148) - (pixel.G * 0.291) + (pixel.B * 0.439) + 128);
			pPicture->data[2][y * pPicture->linesize[2] + x] = (int)((pixel.R * 0.439) - (pixel.G * 0.368) - (pixel.B * 0.071) + 128);
		}
	}
}


//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
NetworkVideoStream::NetworkVideoStream(const std::string &p_strNetworkAddress, int p_nPortNumber)
	: m_strAddress(p_strNetworkAddress)
	, m_nPort(p_nPortNumber)
	, m_pVideoStreamState(new VideoStreamState())
{ }
//----------------------------------------------------------------------------------------------
NetworkVideoStream::NetworkVideoStream(void)
	: m_strAddress("127.0.0.1")
	, m_nPort(6666)
	, m_pVideoStreamState(new VideoStreamState())
{ }
//----------------------------------------------------------------------------------------------
NetworkVideoStream::~NetworkVideoStream(void)
{
	Safe_Delete(m_pVideoStreamState);
}
//----------------------------------------------------------------------------------------------
std::string NetworkVideoStream::GetNetworkAddress(void) const {
	return m_strAddress;
}
//----------------------------------------------------------------------------------------------
void NetworkVideoStream::SetNetworkAddress(const std::string &p_strNetworkAddress) {
	m_strAddress = p_strNetworkAddress;
}
//----------------------------------------------------------------------------------------------
int NetworkVideoStream::GetPortNumber(void) const {
	return m_nPort;
}
//----------------------------------------------------------------------------------------------
void NetworkVideoStream::SetPortNumber(int p_nPortNumber) {
	m_nPort = p_nPortNumber;
}
//----------------------------------------------------------------------------------------------
IVideoStream::StreamType NetworkVideoStream::GetStreamType(void) const {
	return IVideoStream::Network;
}
//----------------------------------------------------------------------------------------------
bool NetworkVideoStream::Initialise(int p_nWidth, int p_nHeight, int p_nFramesPerSecond, int p_nBitRate, VideoCodec p_videoCodec) 
{
	std::cout << "Width : " << p_nWidth << ", Height : " << p_nHeight << ", FPS : " << p_nFramesPerSecond << ", rate : " << p_nBitRate << std::endl;

	av_register_all();
	avcodec_register_all();
	avformat_network_init();

	// First find video encoder
	CodecID codecId = (CodecID)GetCodec(p_videoCodec);
	m_pVideoStreamState->m_pCodec = avcodec_find_encoder(codecId);
	if (!m_pVideoStreamState->m_pCodec)
	{
		fprintf(stderr, "Error :: VideoDevice::Open() -> Cannot find requested codec!");
		return false;
	}
	
	// Get codec context and allocate video frame
	m_pVideoStreamState->m_pCodecContext = avcodec_alloc_context3(m_pVideoStreamState->m_pCodec);
	m_pVideoStreamState->m_pPicture = avcodec_alloc_frame();

	// Bit rate
	m_pVideoStreamState->m_pCodecContext->bit_rate = p_nBitRate;
	
	// Resolution (must be multiple of two!)
	m_pVideoStreamState->m_pCodecContext->width = p_nWidth;
	m_pVideoStreamState->m_pCodecContext->height = p_nHeight;

	// Frame rate (FPS)
	m_pVideoStreamState->m_pCodecContext->time_base.num = 1;
	m_pVideoStreamState->m_pCodecContext->time_base.den = p_nFramesPerSecond;

	// I and B frames
	// m_pCodecContext->gop_size = 10;
	// m_pCodecContext->max_b_frames = 25;
	m_pVideoStreamState->m_pCodecContext->pix_fmt = PIX_FMT_YUV420P;

	if (codecId == CODEC_ID_H264)
		av_opt_set(m_pVideoStreamState->m_pCodecContext->priv_data, "preset", "slow", 0);

	// Open codec
	if (avcodec_open2(m_pVideoStreamState->m_pCodecContext, m_pVideoStreamState->m_pCodec, NULL) < 0) 
	{
		fprintf(stderr, "Error :: RTPDevice::Open() -> Cannot open requested codec!");
		return false;
	}

	// Allocate image and output buffer (this is yet magic... I still have to look at the calculation's whys)
	m_pVideoStreamState->m_nOutputBufferSize = 100000 + 12 * m_pVideoStreamState->m_pCodecContext->width * m_pVideoStreamState->m_pCodecContext->height;
	m_pVideoStreamState->m_pOutputBuffer = (uint8_t*)malloc(m_pVideoStreamState->m_nOutputBufferSize);

	// FFMPEG tutorial original comment:
	// the image can be allocated by any means and av_image_alloc() is
	// just the most convenient way if av_malloc() is to be used */
	av_image_alloc(m_pVideoStreamState->m_pPicture->data, m_pVideoStreamState->m_pPicture->linesize,
				   m_pVideoStreamState->m_pCodecContext->width, m_pVideoStreamState->m_pCodecContext->height, 
				   m_pVideoStreamState->m_pCodecContext->pix_fmt, 1);

	// Initalize the AV context
	m_pVideoStreamState->m_pFormatContext = avformat_alloc_context();
	if (!m_pVideoStreamState->m_pFormatContext)
	{
		fprintf(stderr, "Error :: RTPDevice::Open() -> Cannot allocate format context!");
		return false;
	}

	// Get the output format
	m_pVideoStreamState->m_pOutputFormat = av_guess_format("rtp", NULL, NULL);
	if (!m_pVideoStreamState->m_pOutputFormat)
	{
		fprintf(stderr, "Error :: RTPDevice::Open() -> Could not guess output format!");
		return false;
	}
	
	m_pVideoStreamState->m_pFormatContext->oformat = m_pVideoStreamState->m_pOutputFormat;

	// Try to open the RTP
	sprintf(m_pVideoStreamState->m_pFormatContext->filename, "rtp://%s:%d", m_strAddress.c_str(), m_nPort);
	if (avio_open(&(m_pVideoStreamState->m_pFormatContext->pb), m_pVideoStreamState->m_pFormatContext->filename, AVIO_FLAG_WRITE) < 0)
	{
		fprintf(stderr, "Error :: RTPDevice::Open() -> Could not open RTP stream!");
		return false;
	}

	// Add a stream to RTP
	m_pVideoStreamState->m_pStream = av_new_stream(m_pVideoStreamState->m_pFormatContext, 1);
	if (!m_pVideoStreamState->m_pStream)
	{
		fprintf(stderr, "Error :: RTPDevice::Open() -> Could not add stream!");
		return false;
	}

	// Initalize codec
	AVCodecContext* pStreamCodec = m_pVideoStreamState->m_pStream->codec;
	pStreamCodec->codec_id = m_pVideoStreamState->m_pCodecContext->codec_id;
	pStreamCodec->codec_type = m_pVideoStreamState->m_pCodecContext->codec_type;
	pStreamCodec->bit_rate = m_pVideoStreamState->m_pCodecContext->bit_rate;
	pStreamCodec->width = m_pVideoStreamState->m_pCodecContext->width;
	pStreamCodec->height = m_pVideoStreamState->m_pCodecContext->height;
	pStreamCodec->time_base.den = m_pVideoStreamState->m_pCodecContext->time_base.den;
	pStreamCodec->time_base.num = m_pVideoStreamState->m_pCodecContext->time_base.num;

	// Write the header
	avformat_write_header(m_pVideoStreamState->m_pFormatContext, NULL);
		
	// Clear had output
	m_pVideoStreamState->m_nHadOutput = 0;
	
	return true;
}
//----------------------------------------------------------------------------------------------
void NetworkVideoStream::Stream(Image *p_pImage)
{
	// Convert image to a frame for streaming
	IVideoStream::ConvertImageToFrame(p_pImage, (void*)(m_pVideoStreamState->m_pPicture));

	// encode the image
	m_pVideoStreamState->m_nOutputSize = 
		avcodec_encode_video(m_pVideoStreamState->m_pCodecContext, 
							 m_pVideoStreamState->m_pOutputBuffer, 
							 m_pVideoStreamState->m_nOutputBufferSize, 
							 m_pVideoStreamState->m_pPicture);

	m_pVideoStreamState->m_nHadOutput |= m_pVideoStreamState->m_nOutputSize;

	if (m_pVideoStreamState->m_nOutputSize > 0)
	{
		// initalize a packet
		AVPacket packet;
		av_init_packet(&packet);

		packet.data = m_pVideoStreamState->m_pOutputBuffer;
		packet.size = m_pVideoStreamState->m_nOutputSize;
		packet.stream_index = m_pVideoStreamState->m_pStream->index;

		// Write the compressed frame to the media file.
		av_interleaved_write_frame(m_pVideoStreamState->m_pFormatContext, &packet);
	}
}
//----------------------------------------------------------------------------------------------
void NetworkVideoStream::Shutdown(void) 
{ 
	AVPacket packet;

	// get the delayed frames
	while(m_pVideoStreamState->m_nOutputSize || !m_pVideoStreamState->m_nHadOutput)
	{
		m_pVideoStreamState->m_nOutputSize = avcodec_encode_video(m_pVideoStreamState->m_pCodecContext, 
			m_pVideoStreamState->m_pOutputBuffer, m_pVideoStreamState->m_nOutputBufferSize, NULL);
		m_pVideoStreamState->m_nHadOutput |= m_pVideoStreamState->m_nOutputBufferSize;

		// initalize a packet
		av_init_packet(&packet);
		packet.data = m_pVideoStreamState->m_pOutputBuffer;
		packet.size = m_pVideoStreamState->m_nOutputSize;
		packet.stream_index = m_pVideoStreamState->m_pStream->index;

		// send it out
		av_write_frame(m_pVideoStreamState->m_pFormatContext, &packet);
	}

	// add sequence end code to have a real mpeg file
	m_pVideoStreamState->m_pOutputBuffer[0] = 0x00;
	m_pVideoStreamState->m_pOutputBuffer[1] = 0x00;
	m_pVideoStreamState->m_pOutputBuffer[2] = 0x01;
	m_pVideoStreamState->m_pOutputBuffer[3] = 0xb7;

	free(m_pVideoStreamState->m_pOutputBuffer);

	av_free(m_pVideoStreamState->m_pStream);

	// avformat_close_input(&(m_pVideoStreamState->m_pFormatContext));
	av_free(m_pVideoStreamState->m_pFormatContext);

	avcodec_close(m_pVideoStreamState->m_pCodecContext);
	av_free(m_pVideoStreamState->m_pCodecContext);
	av_free(m_pVideoStreamState->m_pPicture->data[0]);
	av_free(m_pVideoStreamState->m_pPicture);
}
//----------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
FileVideoStream::FileVideoStream(const std::string &p_strFilename)
	: m_strFilename(p_strFilename)
	, m_pVideoStreamState(new VideoStreamState())
{ }
//----------------------------------------------------------------------------------------------
FileVideoStream::FileVideoStream(void)
	: m_strFilename("default.mpg")
	, m_pVideoStreamState(new VideoStreamState())
{ }
//----------------------------------------------------------------------------------------------
FileVideoStream::~FileVideoStream(void)
{
	Safe_Delete(m_pVideoStreamState);
}
//----------------------------------------------------------------------------------------------
std::string FileVideoStream::GetFilename(void) const {
	return m_strFilename;
}
//----------------------------------------------------------------------------------------------
void FileVideoStream::SetFilename(const std::string &p_strFilename) {
	m_strFilename = p_strFilename;
}
//----------------------------------------------------------------------------------------------
IVideoStream::StreamType FileVideoStream::GetStreamType(void) const {
	return IVideoStream::Disk;
}
//----------------------------------------------------------------------------------------------
bool FileVideoStream::Initialise(int p_nWidth, int p_nHeight, int p_nFramesPerSecond, int p_nBitRate, VideoCodec p_videoCodec) 
{
	av_register_all();
	avformat_network_init();

	// First find video encoder
	CodecID codecId = (CodecID)GetCodec(p_videoCodec);
	m_pVideoStreamState->m_pCodec = avcodec_find_encoder(codecId);
	if (!m_pVideoStreamState->m_pCodec)
	{
		fprintf(stderr, "Error :: VideoDevice::Open() -> Cannot find requested codec!");
		return false;
	}
	
	// Get codec context and allocate video frame
	m_pVideoStreamState->m_pCodecContext = avcodec_alloc_context3(m_pVideoStreamState->m_pCodec);
	m_pVideoStreamState->m_pPicture = avcodec_alloc_frame();

	// Bit rate
	m_pVideoStreamState->m_pCodecContext->bit_rate = p_nBitRate;
	
	// Resolution (must be multiple of two!)
	m_pVideoStreamState->m_pCodecContext->width = p_nWidth;
	m_pVideoStreamState->m_pCodecContext->height = p_nHeight;

	// Frame rate (FPS)
	m_pVideoStreamState->m_pCodecContext->time_base.num = 1;
	m_pVideoStreamState->m_pCodecContext->time_base.den = p_nFramesPerSecond;

	// I and B frames
	//m_pCodecContext->gop_size = 10;
	//m_pCodecContext->max_b_frames = 25;
	m_pVideoStreamState->m_pCodecContext->pix_fmt = PIX_FMT_YUV420P;

	if (codecId == CODEC_ID_H264)
		av_opt_set(m_pVideoStreamState->m_pCodecContext->priv_data, "preset", "slow", 0);

	// Open codec
	if (avcodec_open2(m_pVideoStreamState->m_pCodecContext, m_pVideoStreamState->m_pCodec, NULL) < 0) 
	{
		fprintf(stderr, "Error :: RTPDevice::Open() -> Cannot open requested codec!");
		return false;
	}

	// Open destination file
	m_videoFile = fopen(m_strFilename.c_str(), "wb");
	if (!m_videoFile)
	{
		fprintf(stderr, "Error :: VideoDevice::Open() -> Cannot open output file!");
		return false;
	}

	// Allocate image and output buffer (this is yet magic... I still have to look at the calculation's whys)
	m_pVideoStreamState->m_nOutputBufferSize = 100000 + 12 * m_pVideoStreamState->m_pCodecContext->width * m_pVideoStreamState->m_pCodecContext->height;
	m_pVideoStreamState->m_pOutputBuffer = (uint8_t*)malloc(m_pVideoStreamState->m_nOutputBufferSize);

	// FFMPEG tutorial original comment:
	// the image can be allocated by any means and av_image_alloc() is
	// just the most convenient way if av_malloc() is to be used */
	av_image_alloc(m_pVideoStreamState->m_pPicture->data, m_pVideoStreamState->m_pPicture->linesize,
				   m_pVideoStreamState->m_pCodecContext->width, m_pVideoStreamState->m_pCodecContext->height, 
				   m_pVideoStreamState->m_pCodecContext->pix_fmt, 1);

	// Clear had output
	m_pVideoStreamState->m_nHadOutput = 0;
	
	return true;
}
//----------------------------------------------------------------------------------------------
void FileVideoStream::Stream(Image *p_pImage)
{
	// Convert image to a frame for streaming
	IVideoStream::ConvertImageToFrame(p_pImage, (void*)(m_pVideoStreamState->m_pPicture));

	// encode the image
	m_pVideoStreamState->m_nOutputSize = avcodec_encode_video(m_pVideoStreamState->m_pCodecContext, 
		m_pVideoStreamState->m_pOutputBuffer, m_pVideoStreamState->m_nOutputBufferSize, m_pVideoStreamState->m_pPicture);
	m_pVideoStreamState->m_nHadOutput |= m_pVideoStreamState->m_nOutputSize;

	fwrite(m_pVideoStreamState->m_pOutputBuffer, 1, m_pVideoStreamState->m_nOutputSize, m_videoFile);
}
//----------------------------------------------------------------------------------------------
void FileVideoStream::Shutdown(void) 
{ 
	AVPacket packet;

	/* get the delayed frames */
	while(m_pVideoStreamState->m_nOutputSize || !m_pVideoStreamState->m_nHadOutput)
	{
		m_pVideoStreamState->m_nOutputSize = avcodec_encode_video(m_pVideoStreamState->m_pCodecContext, 
			m_pVideoStreamState->m_pOutputBuffer, m_pVideoStreamState->m_nOutputBufferSize, NULL);
		m_pVideoStreamState->m_nHadOutput |= m_pVideoStreamState->m_nOutputBufferSize;
		
		fwrite(m_pVideoStreamState->m_pOutputBuffer, 1, m_pVideoStreamState->m_nOutputSize, m_videoFile);
	}

	// add sequence end code to have a real mpeg file
	m_pVideoStreamState->m_pOutputBuffer[0] = 0x00;
	m_pVideoStreamState->m_pOutputBuffer[1] = 0x00;
	m_pVideoStreamState->m_pOutputBuffer[2] = 0x01;
	m_pVideoStreamState->m_pOutputBuffer[3] = 0xb7;
	
	fwrite(m_pVideoStreamState->m_pOutputBuffer, 1, 4, m_videoFile);
	
	fclose(m_videoFile);

	free(m_pVideoStreamState->m_pOutputBuffer);

	avcodec_close(m_pVideoStreamState->m_pCodecContext);
	av_free(m_pVideoStreamState->m_pCodecContext);
	av_free(m_pVideoStreamState->m_pPicture->data[0]);
	av_free(m_pVideoStreamState->m_pPicture);
}
//----------------------------------------------------------------------------------------------