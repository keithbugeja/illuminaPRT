//----------------------------------------------------------------------------------------------
//	Filename:	ImageDevice.cpp
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
#include "Device/VideoDevice.h"
#include "Image/Image.h"
#include "Spectrum/Spectrum.h"

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
// If an FFMPEG helper is created (or rather, when), this function should go in there.
// Also, base it on a look-up table.
//----------------------------------------------------------------------------------------------
CodecID GetFFMPEGCodec(VideoDevice::VideoCodec p_videoCodec)
{
	switch(p_videoCodec)
	{
		case VideoDevice::MPEG1:
			return CODEC_ID_MPEG1VIDEO;

		case VideoDevice::MPEG2:
			return CODEC_ID_MPEG2VIDEO;

		case VideoDevice::H264:
			return CODEC_ID_H264;

		case VideoDevice::VP8:
			return CODEC_ID_VP8;

		default:
			return CODEC_ID_MPEG1VIDEO;
	}
}
//----------------------------------------------------------------------------------------------
VideoDevice::VideoDevice(const std::string &p_strName, int p_nWidth, int p_nHeight, const std::string &p_strFilename, int p_nFramesPerSecond, VideoCodec p_videoCodec)
	: IDevice(p_strName) 
	, m_pImage(new Image(p_nWidth, p_nHeight))
	, m_strFilename(p_strFilename)
	, m_nFramesPerSecond(p_nFramesPerSecond)
	, m_videoCodec(p_videoCodec)
	, m_bIsOpen(false)
{
}
//----------------------------------------------------------------------------------------------
VideoDevice::VideoDevice(int p_nWidth, int p_nHeight, const std::string &p_strFilename, int p_nFramesPerSecond, VideoCodec p_videoCodec)
	: m_pImage(new Image(p_nWidth, p_nHeight))
	, m_strFilename(p_strFilename)
	, m_nFramesPerSecond(p_nFramesPerSecond)
	, m_videoCodec(p_videoCodec)
	, m_bIsOpen(false)
{
}
//----------------------------------------------------------------------------------------------
VideoDevice::~VideoDevice() 
{
	Safe_Delete(m_pImage);
}
//----------------------------------------------------------------------------------------------
int VideoDevice::GetWidth(void) const { 
	return m_pImage->GetWidth(); 
}
//----------------------------------------------------------------------------------------------
int VideoDevice::GetHeight(void) const { 
	return m_pImage->GetHeight(); 
}
//----------------------------------------------------------------------------------------------
IDevice::AccessType VideoDevice::GetAccessType(void) const {
	return IDevice::ReadWrite;
}
//----------------------------------------------------------------------------------------------
bool VideoDevice::Open(void) 
{
	// If device is already open, return error
	if (m_bIsOpen) 
		return false;

	// First find video encoder
	CodecID codecId = GetFFMPEGCodec(m_videoCodec);
	m_pCodec = avcodec_find_encoder(codecId);
	if (!m_pCodec)
	{
		fprintf(stderr, "Error :: VideoDevice::Open() -> Cannot find requested codec!");
		return false;
	}
	
	// Get codec context and allocate video frame
	m_pCodecContext = avcodec_alloc_context3(m_pCodec);
	m_pPicture = avcodec_alloc_frame();

	// Bit rate
	m_pCodecContext->bit_rate = 400000;
	
	// Resolution (must be multiple of two!)
	m_pCodecContext->width = m_pImage->GetWidth();
	m_pCodecContext->height = m_pImage->GetHeight();

	// Frame rate (FPS)
	m_pCodecContext->time_base.num = 1;
	m_pCodecContext->time_base.den = m_nFramesPerSecond;

	// I and B frames
	m_pCodecContext->gop_size = 10;
	m_pCodecContext->max_b_frames = 25;
	m_pCodecContext->pix_fmt = PIX_FMT_YUV420P;

	if (codecId == CODEC_ID_H264)
		av_opt_set(m_pCodecContext->priv_data, "preset", "slow", 0);

	// Open codec
	if (avcodec_open2(m_pCodecContext, m_pCodec, NULL) < 0) 
	{
		fprintf(stderr, "Error :: VideoDevice::Open() -> Cannot open requested codec!");
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
	m_nOutputBufferSize = 100000 + 12 * m_pCodecContext->width * m_pCodecContext->height;
	m_pOutputBuffer = (uint8_t*)malloc(m_nOutputBufferSize);

	// FFMPEG tutorial original comment:
	// the image can be allocated by any means and av_image_alloc() is
	// just the most convenient way if av_malloc() is to be used */
	av_image_alloc(m_pPicture->data, m_pPicture->linesize,
				   m_pCodecContext->width, m_pCodecContext->height, m_pCodecContext->pix_fmt, 1);

	m_nHadOutput = 0;
	m_bIsOpen = true;

	return true;
}
//----------------------------------------------------------------------------------------------
void VideoDevice::Close(void) 
{ 
	BOOST_ASSERT(m_bIsOpen);

	/* get the delayed frames */
	while(m_nOutputSize || !m_nHadOutput)
	{
		// fflush(stdout);

		m_nOutputSize = avcodec_encode_video(m_pCodecContext, m_pOutputBuffer, m_nOutputBufferSize, NULL);
		m_nHadOutput |= m_nOutputBufferSize;
		fwrite(m_pOutputBuffer, 1, m_nOutputSize, m_videoFile);
	}

	// add sequence end code to have a real mpeg file
	m_pOutputBuffer[0] = 0x00;
	m_pOutputBuffer[1] = 0x00;
	m_pOutputBuffer[2] = 0x01;
	m_pOutputBuffer[3] = 0xb7;
	
	fwrite(m_pOutputBuffer, 1, 4, m_videoFile);
	
	fclose(m_videoFile);
	
	free(m_pOutputBuffer);

	avcodec_close(m_pCodecContext);
	av_free(m_pCodecContext);
	av_free(m_pPicture->data[0]);
	av_free(m_pPicture);
}
//----------------------------------------------------------------------------------------------
void VideoDevice::BeginFrame(void) { }
//----------------------------------------------------------------------------------------------
void VideoDevice::EndFrame(void)  
{
	//	   Y =  (0.257 * R) + (0.504 * G) + (0.098 * B) + 16
	//Cr = V =  (0.439 * R) - (0.368 * G) - (0.071 * B) + 128
	//Cb = U = -(0.148 * R) - (0.291 * G) + (0.439 * B) + 128

	// Y
	for (int y = 0; y < m_pCodecContext->height; ++y)
	{
		for (int x = 0; x < m_pCodecContext->width; ++x)
		{
			RGBPixel pixel = m_pImage->Get(x, y);
			
			pixel *= 256;

			m_pPicture->data[0][y * m_pPicture->linesize[0] + x] = (int)((pixel.R * 0.257) + (pixel.G * 0.504) + (pixel.B * 0.098) + 16);
		}
	}

	// Cb and Cr
	for(int y = 0; y < m_pCodecContext->height / 2; ++y) 
	{
		for(int x = 0; x < m_pCodecContext->width / 2; ++x) 
		{
			RGBPixel pixel = m_pImage->Get(x << 1, y << 1);

			pixel *= 256;

			m_pPicture->data[1][y * m_pPicture->linesize[1] + x] = (int)(-(pixel.R * 0.148) - (pixel.G * 0.291) + (pixel.B * 0.439) + 128);
			m_pPicture->data[2][y * m_pPicture->linesize[2] + x] = (int)((pixel.R * 0.439) - (pixel.G * 0.368) - (pixel.B * 0.071) + 128);
		}
	}

	// encode the image
	m_nOutputSize = avcodec_encode_video(m_pCodecContext, m_pOutputBuffer, m_nOutputBufferSize, m_pPicture);
	m_nHadOutput |= m_nOutputSize;
	fwrite(m_pOutputBuffer, 1, m_nOutputSize, m_videoFile);
}
//----------------------------------------------------------------------------------------------
void VideoDevice::Set(int p_nX, int p_nY, const Spectrum &p_spectrum) {
	m_pImage->Set(p_nX, p_nY, RGBPixel(p_spectrum[0], p_spectrum[1], p_spectrum[2]));
}
//----------------------------------------------------------------------------------------------
void VideoDevice::Set(float p_fX, float p_fY, const Spectrum &p_spectrum) {
	Set((int)p_fX, (int)p_fY, p_spectrum); 
}
//----------------------------------------------------------------------------------------------
void VideoDevice::Get(int p_nX, int p_nY, Spectrum &p_spectrum) const
{
	const RGBPixel pixel = m_pImage->Get(p_nX, p_nY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void VideoDevice::Get(float p_fX, float p_fY, Spectrum &p_spectrum) const
{
	const RGBPixel pixel = m_pImage->Get((int)p_fX, (int)p_fY);
	p_spectrum.Set(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum VideoDevice::Get(int p_nX, int p_nY) const
{
	const RGBPixel pixel = m_pImage->Get(p_nX, p_nY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
Spectrum VideoDevice::Get(float p_fX, float p_fY) const
{
	const RGBPixel pixel = m_pImage->Get((int)p_fX, (int)p_fY);
	return Spectrum(pixel.R, pixel.G, pixel.B);
}
//----------------------------------------------------------------------------------------------
void VideoDevice::WriteRadianceBufferToDevice(int p_nRegionX, int p_nRegionY, int p_nRegionWidth, int p_nRegionHeight, 
	RadianceBuffer *p_pRadianceBuffer, int p_nDeviceX, int p_nDeviceY)
{
	int width = m_pImage->GetWidth(),
		height = m_pImage->GetHeight();

	for (int srcY = p_nRegionY, dstY = p_nDeviceY; srcY < p_pRadianceBuffer->GetHeight(); ++srcY, ++dstY)
	{
		for (int srcX = p_nRegionX, dstX = p_nDeviceX; srcX < p_pRadianceBuffer->GetWidth(); ++srcX, ++dstX)
		{
			this->Set(width - (dstX + 1), height - (dstY + 1), p_pRadianceBuffer->Get(srcX, srcY).Final);
		}
	}
}
//----------------------------------------------------------------------------------------------
VideoDevice::VideoCodec VideoDevice::GetCodec(void) const {
	return m_videoCodec;
}
//----------------------------------------------------------------------------------------------
void VideoDevice::SetCodec(VideoDevice::VideoCodec p_videoCodec)
{
	BOOST_ASSERT(!m_bIsOpen);
	m_videoCodec = p_videoCodec;
}
//----------------------------------------------------------------------------------------------
int VideoDevice::GetFrameRate(void) const {
	return m_nFramesPerSecond;
}
//----------------------------------------------------------------------------------------------
void VideoDevice::SetFrameRate(int p_nFramesPerSecond)
{
	BOOST_ASSERT(!m_bIsOpen);
	m_nFramesPerSecond = p_nFramesPerSecond;
}
//----------------------------------------------------------------------------------------------
std::string VideoDevice::GetFilename(void) const {
	return m_strFilename;
}
//----------------------------------------------------------------------------------------------
void VideoDevice::SetFilename(const std::string &p_strFilename) 
{
	BOOST_ASSERT(!m_bIsOpen);
	m_strFilename = p_strFilename;
}
//----------------------------------------------------------------------------------------------
Image *VideoDevice::GetImage(void) const {
	return m_pImage;
}
//----------------------------------------------------------------------------------------------