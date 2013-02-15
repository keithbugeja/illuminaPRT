//----------------------------------------------------------------------------------------------
//	Filename:	VideoStream.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#define __VIDEOSTREAM_USE_VLC__
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#if (defined __VIDEOSTREAM_USE_FFMPEG__)
	#define __STDC_CONSTANT_MACROS
	extern "C" 
	{
		#include <libavcodec/avcodec.h>
		#include <libavformat/avformat.h>
		#include <libavutil/imgutils.h>
		#include <libavutil/opt.h>
	}
#elif (defined __VIDEOSTREAM_USE_VLC__)
	extern "C" {
		#include <vlc/vlc.h>
	}
#endif
//----------------------------------------------------------------------------------------------
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
//----------------------------------------------------------------------------------------------
#include "External/Video/VideoStream.h"
#include "Spectrum/Spectrum.h"
#include "Image/Surface.h"
#include "Image/Image.h"
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		struct VideoStreamState
		{
			#if (defined __VIDEOSTREAM_USE_FFMPEG__)
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

			#elif (defined __VIDEOSTREAM_USE_VLC__)
				libvlc_instance_t *m_pInstance;
				libvlc_media_player_t *m_pMediaPlayer;
				libvlc_media_t *m_pMedia;

				unsigned char *m_pFrame;
				Image *m_pImage;

				int64_t m_i64FrameTime;
				int64_t m_i64FrameNext;

				VideoStreamState()
					: m_pFrame(NULL)
					, m_pImage(NULL)
					, m_i64FrameTime(0)
					, m_i64FrameNext(0)
				{ }

			#endif
		};
	}
}
//----------------------------------------------------------------------------------------------
// Callback method VLC uses to request a new frame for streaming
//----------------------------------------------------------------------------------------------
#if (defined __VIDEOSTREAM_USE_VLC__)
int IMEMGetCallback (void *data, const char *cookie, int64_t *dts, int64_t *pts, unsigned *flags, size_t * bufferSize, void ** buffer)
{
	VideoStreamState *pVideoStreamState = (VideoStreamState*)data;

	*dts = *pts = pVideoStreamState->m_i64FrameNext;
	pVideoStreamState->m_i64FrameNext += pVideoStreamState->m_i64FrameTime;

	if (pVideoStreamState->m_pImage != NULL)
	{
		*bufferSize = pVideoStreamState->m_pImage->GetArea() * 3;
		*buffer = (void*)pVideoStreamState->m_pFrame;
	}
	else
		*bufferSize = 0;

	return 0;
}
//----------------------------------------------------------------------------------------------
// Callback method VLC uses to release data
//----------------------------------------------------------------------------------------------
int IMEMReleaseCallback (void *data, const char *cookie, size_t bufferSize, void * buffer)
{
	return 0;
}
#endif
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
// Convert from IVideoStream codec to library specific codec 
//----------------------------------------------------------------------------------------------
#if (defined __VIDEOSTREAM_USE_FFMPEG__)
int GetCodec(IVideoStream::VideoCodec p_videoCodec)
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
#endif
//----------------------------------------------------------------------------------------------
std::string IVideoStream::GetFourCC(VideoCodec p_videoCodec)
{
	switch(p_videoCodec)
	{
	case IVideoStream::MPEG1:
		return "mp1v";
	case IVideoStream::MPEG2:
		return "mp2v";
	case IVideoStream::MPEG4:
		return "mp4v";
	case IVideoStream::H264:
		return "h264";
	case IVideoStream::VP8:
		return "vp8";
	default:
		return "mp1v";
	}
}
//----------------------------------------------------------------------------------------------
IVideoStream::VideoCodec IVideoStream::GetCodec(const std::string &p_strFourCC)
{
	std::string strFourCC = p_strFourCC;

	boost::algorithm::to_lower(strFourCC);

	if (strFourCC == "mp1v")
		return IVideoStream::MPEG1;
	else if (strFourCC == "mp2v")
		return IVideoStream::MPEG2;
	else if (strFourCC == "mp4v")
		return IVideoStream::MPEG4;
	else if (strFourCC == "h264")
		return IVideoStream::H264;
	else if (strFourCC == "vp8")
		return IVideoStream::VP8;

	return IVideoStream::MPEG1;
}
//----------------------------------------------------------------------------------------------
void IVideoStream::ConvertImageToFrame(Image *p_pImage, void *p_pFrame)
{
#if (defined __VIDEOSTREAM_USE_FFMPEG__)
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
#elif (defined __VIDEOSTREAM_USE_VLC__)	
	unsigned char *pFrame = (unsigned char*)p_pFrame;
	RGBPixel4F *pImagePixel = p_pImage->GetSurfaceBuffer();

	for (int size = p_pImage->GetArea(); size > 0; --size, pImagePixel++, pFrame+=3)
	{
		pFrame[0] = (int)(pImagePixel->B * 255);
		pFrame[1] = (int)(pImagePixel->G * 255);
		pFrame[2] = (int)(pImagePixel->R * 255);
	}
#endif
}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
DisplayVideoStream::DisplayVideoStream(void)
	: m_pVideoStreamState(new VideoStreamState())
{ }
//----------------------------------------------------------------------------------------------
DisplayVideoStream::~DisplayVideoStream(void)
{
	Safe_Delete(m_pVideoStreamState);
}
//----------------------------------------------------------------------------------------------
IVideoStream::StreamType DisplayVideoStream::GetStreamType(void) const {
	return IVideoStream::Display;
}

//----------------------------------------------------------------------------------------------
bool DisplayVideoStream::Initialise(int p_nWidth, int p_nHeight, int p_nFramesPerSecond, int p_nBitRate, VideoCodec p_videoCodec)
{
#if (defined __VIDEOSTREAM_USE_VLC__)
	std::vector<char*> arguments;
	std::stringstream argStream;
		
	argStream 
		<< " --imem-id=1"
		<< " --imem-group=1"
		<< " --imem-cat=2"
		<< " --imem-width=" << p_nWidth 
		<< " --imem-height=" << p_nHeight
		<< " --imem-codec=RV24"
		<< " --imem-get=" << (long long int)IMEMGetCallback
		<< " --imem-release=" << (long long int)IMEMReleaseCallback
		<< " --imem-data=" << (long long int)m_pVideoStreamState
		<< std::endl;

	std::string args = argStream.str();

	boost::char_separator<char> separator(" ");
	boost::tokenizer<boost::char_separator<char> > tokens(args, separator);

	size_t tokenSize;
	char *pToken;

	for (boost::tokenizer<boost::char_separator<char> >::iterator tokenIterator = tokens.begin();
			tokenIterator != tokens.end(); ++tokenIterator)
	{
		tokenSize = (*tokenIterator).size() + 1;
		pToken = new char[tokenSize];
		memset(pToken, 0, tokenSize);
		strncpy(pToken, (*tokenIterator).c_str(), tokenSize - 1);

		arguments.push_back(pToken);
	}

	// Create new RGBA surface
	m_pVideoStreamState->m_pFrame = new unsigned char [p_nWidth * p_nHeight * 3];
	m_pVideoStreamState->m_i64FrameNext = 0;
	m_pVideoStreamState->m_i64FrameTime = (int64_t)((1.f / p_nFramesPerSecond) * 1e+6);
	m_pVideoStreamState->m_pImage = NULL;

	// Load the VLC Engine
	m_pVideoStreamState->m_pInstance = libvlc_new(arguments.size(), (const char *const *)&arguments[0]);
	
	// Create media instance
	m_pVideoStreamState->m_pMedia = libvlc_media_new_path(m_pVideoStreamState->m_pInstance, "imem://");

	// Create a media player playing environement
	m_pVideoStreamState->m_pMediaPlayer = 
		libvlc_media_player_new_from_media(m_pVideoStreamState->m_pMedia);

	libvlc_media_player_play(m_pVideoStreamState->m_pMediaPlayer);

	// Free argument strings
	for (std::vector<char*>::iterator stringIterator = arguments.begin();
		 stringIterator != arguments.end(); ++stringIterator)
		 delete *stringIterator;

	arguments.clear();

	// Debug output
	std::cout << "DisplayVideoStreaming :: " << p_nWidth << " x " << p_nHeight << " x " << p_nFramesPerSecond 
		<<  "/s, bitrate = " << p_nBitRate << std::endl;
#endif

	return true;
}
//----------------------------------------------------------------------------------------------
void DisplayVideoStream::Stream(Image *p_pImage)
{
#if (defined __VIDEOSTREAM_USE_VLC__)
	// Save image and covert
	m_pVideoStreamState->m_pImage = p_pImage;
	IVideoStream::ConvertImageToFrame(m_pVideoStreamState->m_pImage, (void*)(m_pVideoStreamState->m_pFrame));
#endif
}
//----------------------------------------------------------------------------------------------
void DisplayVideoStream::Shutdown(void) 
{ 
#if (defined __VIDEOSTREAM_USE_VLC__)
	libvlc_media_player_stop (m_pVideoStreamState->m_pMediaPlayer);
	libvlc_media_player_release (m_pVideoStreamState->m_pMediaPlayer);
	libvlc_media_release (m_pVideoStreamState->m_pMedia);
	libvlc_release(m_pVideoStreamState->m_pInstance);

	Safe_Delete(m_pVideoStreamState->m_pFrame);
#endif
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
NetworkVideoStream::NetworkVideoStream(const std::string &p_strNetworkAddress, int p_nPortNumber)
	: m_pVideoStreamState(new VideoStreamState())
	, m_nPort(p_nPortNumber)
	, m_strAddress(p_strNetworkAddress)
{ }
//----------------------------------------------------------------------------------------------
NetworkVideoStream::NetworkVideoStream(void)
	: m_pVideoStreamState(new VideoStreamState())
	, m_nPort(10000)
	, m_strAddress("127.0.0.1")
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
#if (defined __VIDEOSTREAM_USE_FFMPEG__)

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
	// m_pVideoStreamState->m_pCodecContext->max_b_frames = 25;
	m_pVideoStreamState->m_pCodecContext->gop_size = 12;
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
	m_pVideoStreamState->m_nOutputBufferSize = 65535 + 12 * m_pVideoStreamState->m_pCodecContext->width * m_pVideoStreamState->m_pCodecContext->height;
	m_pVideoStreamState->m_pOutputBuffer = (uint8_t*)malloc(m_pVideoStreamState->m_nOutputBufferSize);

	// FFMPEG tutorial original comment:
	// the image can be allocated by any means and av_image_alloc() is
	// just the most convenient way if av_malloc() is to be used
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
	int result = avformat_write_header(m_pVideoStreamState->m_pFormatContext, NULL);
		
	// Clear had output
	m_pVideoStreamState->m_nHadOutput = 0;

#elif (defined __VIDEOSTREAM_USE_VLC__)
	std::vector<char*> arguments;
	std::stringstream argStream;
		
	argStream 
		<< " --imem-id=1"
		<< " --imem-group=1"
		<< " --imem-cat=2"
		<< " --imem-width=" << p_nWidth 
		<< " --imem-height=" << p_nHeight
		<< " --imem-codec=RV24" 
		<< " --imem-get=" << (long long int)IMEMGetCallback
		<< " --imem-release=" << (long long int)IMEMReleaseCallback
		<< " --imem-data=" << (long long int)m_pVideoStreamState
		<< std::endl;

	std::string args = argStream.str();

	boost::char_separator<char> separator(" ");
	boost::tokenizer<boost::char_separator<char> > tokens(args, separator);

	size_t tokenSize;
	char *pToken;

	for (boost::tokenizer<boost::char_separator<char> >::iterator tokenIterator = tokens.begin();
			tokenIterator != tokens.end(); ++tokenIterator)
	{
		tokenSize = (*tokenIterator).size() + 1;
		pToken = new char[tokenSize];
		memset(pToken, 0, tokenSize);
		strncpy(pToken, (*tokenIterator).c_str(), tokenSize - 1);

		arguments.push_back(pToken);
	}

	// Create new RGBA surface
	m_pVideoStreamState->m_pFrame = new unsigned char [p_nWidth * p_nHeight * 3];
	m_pVideoStreamState->m_i64FrameNext = 0;
	m_pVideoStreamState->m_i64FrameTime = (int64_t)((1.f / p_nFramesPerSecond) * 1e+6);
	m_pVideoStreamState->m_pImage = NULL;

	// Load the VLC Engine
	m_pVideoStreamState->m_pInstance = libvlc_new(arguments.size(), (const char *const *)&arguments[0]);
	
	// Play media player
	char *vlcOptions[] = {""},
		pOutput[256];

	sprintf(pOutput, "#transcode{vcodec=%s,fps=%d,vb=%d,acodec=none}:rtp{dst=%s,port=%d,mux=ts}",
		GetFourCC(p_videoCodec).c_str(), p_nFramesPerSecond, p_nBitRate, m_strAddress.c_str(), m_nPort);

	libvlc_vlm_add_broadcast(m_pVideoStreamState->m_pInstance,
		"illumina_stream", "imem://", pOutput,
		0, vlcOptions, 1, 0);

	libvlc_vlm_play_media(m_pVideoStreamState->m_pInstance, "illumina_stream");

	// Free argument strings
	for (std::vector<char*>::iterator stringIterator = arguments.begin();
		 stringIterator != arguments.end(); ++stringIterator)
		 delete *stringIterator;

	arguments.clear();

	// Debug output
	std::cout << "NetworkVideoStreaming :: " << p_nWidth << " x " << p_nHeight << " x " << p_nFramesPerSecond 
		<<  "/s, bitrate = " << p_nBitRate << std::endl
		<< "streaming " << GetFourCC(p_videoCodec) << " to " << m_strAddress << " : " << m_nPort << std::endl;
#endif

	return true;
}
//----------------------------------------------------------------------------------------------
void NetworkVideoStream::Stream(Image *p_pImage)
{
#if (defined __VIDEOSTREAM_USE_FFMPEG__)
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
		packet.flags |= AV_PKT_FLAG_KEY;
		packet.pts = packet.dts = 0;

		// Write the compressed frame to the media file.
		av_interleaved_write_frame(m_pVideoStreamState->m_pFormatContext, &packet);
		avio_flush(m_pVideoStreamState->m_pFormatContext->pb);
	}

#elif (defined __VIDEOSTREAM_USE_VLC__)
	// Save image and covert
	m_pVideoStreamState->m_pImage = p_pImage;
	IVideoStream::ConvertImageToFrame(m_pVideoStreamState->m_pImage, (void*)(m_pVideoStreamState->m_pFrame));

#endif
}
//----------------------------------------------------------------------------------------------
void NetworkVideoStream::Shutdown(void) 
{ 
#if (defined __VIDEOSTREAM_USE_FFMPEG__)
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
	av_write_trailer(m_pVideoStreamState->m_pFormatContext);

	free(m_pVideoStreamState->m_pOutputBuffer);
	av_free(m_pVideoStreamState->m_pStream);
	av_free(m_pVideoStreamState->m_pFormatContext);

	avcodec_close(m_pVideoStreamState->m_pCodecContext);
	av_free(m_pVideoStreamState->m_pCodecContext);
	av_free(m_pVideoStreamState->m_pPicture->data[0]);
	av_free(m_pVideoStreamState->m_pPicture);
	
#elif (defined __VIDEOSTREAM_USE_VLC__)
	libvlc_vlm_stop_media(m_pVideoStreamState->m_pInstance, "illumina_stream");
	libvlc_vlm_release(m_pVideoStreamState->m_pInstance);
	libvlc_release(m_pVideoStreamState->m_pInstance);

	Safe_Delete(m_pVideoStreamState->m_pFrame);
#endif
}
//----------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
FileVideoStream::FileVideoStream(const std::string &p_strFilename)
	: m_pVideoStreamState(new VideoStreamState())
	, m_strFilename(p_strFilename)
{ }
//----------------------------------------------------------------------------------------------
FileVideoStream::FileVideoStream(void)
	: m_pVideoStreamState(new VideoStreamState())
	, m_strFilename("default.mpg")
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
	#if (defined __VIDEOSTREAM_USE_FFMPEG__)

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

	#elif (defined __VIDEOSTREAM_USE_VLC__)

	#endif
	
	return true;
}
//----------------------------------------------------------------------------------------------
void FileVideoStream::Stream(Image *p_pImage)
{
	#if (defined __VIDEOSTREAM_USE_FFMPEG__)

		// Convert image to a frame for streaming
		IVideoStream::ConvertImageToFrame(p_pImage, (void*)(m_pVideoStreamState->m_pPicture));

		// encode the image
		m_pVideoStreamState->m_nOutputSize = avcodec_encode_video(m_pVideoStreamState->m_pCodecContext, 
			m_pVideoStreamState->m_pOutputBuffer, m_pVideoStreamState->m_nOutputBufferSize, m_pVideoStreamState->m_pPicture);
		m_pVideoStreamState->m_nHadOutput |= m_pVideoStreamState->m_nOutputSize;

		fwrite(m_pVideoStreamState->m_pOutputBuffer, 1, m_pVideoStreamState->m_nOutputSize, m_videoFile);
	
	#elif (defined __VIDEOSTREAM_USE_VLC__)
	#endif
}
//----------------------------------------------------------------------------------------------
void FileVideoStream::Shutdown(void) 
{ 
	#if (defined __VIDEOSTREAM_USE_FFMPEG__)

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

	#elif (defined __VIDEOSTREAM_USE_VLC__)
	#endif
}
//----------------------------------------------------------------------------------------------
