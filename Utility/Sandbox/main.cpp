//----------------------------------------------------------------------------------------------
//	Filename:	main.cpp
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
// TODO:
// Double check ILight-derived classes ... some methods have not been tested properly.
// ?? DistributedRenderer should not instantiate MPI - change it to have it passed to the object
// Polish object factories
// Move factories to CorePlugins.dll
// Finish scene loaders
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		const int Major = 0;
		const int Minor = 5;
		const int Build = 0;
	}
}
//----------------------------------------------------------------------------------------------
#include <omp.h>
#include <iostream>
#include <sstream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>

#include <vlc/vlc.h>

//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
// Required in both scheduler and renderer mode
#include "Environment.h"
#include "Logger.h"

extern "C" 
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavformat/avio.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/opt.h>
}

//----------------------------------------------------------------------------------------------
// #define TEST_SCHEDULER
#define TEST_TILERENDER
//----------------------------------------------------------------------------------------------
#if (!defined(TEST_SCHEDULER))
//----------------------------------------------------------------------------------------------
static void video_stream_example(const std::string& rtp_addr, int rtp_port, int codec_id)
{
	AVCodecContext *c = NULL;
	AVCodec *codec;
	AVFrame *picture;

	int i, x, y, 
		out_size, 
		outbuf_size,
		had_output = 0;

	uint8_t *outbuf;

	// find the video encoder
	codec = avcodec_find_encoder((CodecID)codec_id);
	
	if (!codec) 
	{
		fprintf(stderr, "codec not found\n");
		exit(1);
	}

	// get codec context and allocate frame
	c = avcodec_alloc_context3(codec);
	picture = avcodec_alloc_frame();

	// put sample parameters
	c->bit_rate = 400000;
	
	// resolution must be a multiple of two
	c->width = 640;
	c->height = 480;

	// frames per second
	c->time_base.num = 1; c->time_base.den = 25;
	
	c->gop_size = 10; /* emit one intra frame every ten frames */
	c->max_b_frames = 25;
	c->pix_fmt = PIX_FMT_YUV420P;

	if(codec_id == CODEC_ID_H264)
		av_opt_set (c->priv_data, "preset", "slow", 0);

	// open codec
	if (avcodec_open2(c, codec, NULL) < 0) 
	{
		fprintf(stderr, "could not open codec\n");
		exit(1);
	}

	// allocate image and output buffer
	outbuf_size = 100000 + 12 * c->width * c->height;
	outbuf = (uint8_t*)malloc(outbuf_size);

	
	// the image can be allocated by any means and av_image_alloc() is
	// just the most convenient way if av_malloc() is to be used */
	av_image_alloc(picture->data, picture->linesize,
				   c->width, c->height, c->pix_fmt, 1);



	// initalize the AV context
	AVFormatContext *oc = avformat_alloc_context();
	if (!oc)
	{
		fprintf(stderr, "could not allocate format context");
		exit(1);
	}

	// get the output format
	AVOutputFormat *of = av_guess_format("rtp", NULL, NULL);
	if (!of)
	{
		fprintf(stderr, "could guess output format");
		exit(1);
	}
	
	oc->oformat = of;

	// try to open the RTP stream
	sprintf(oc->filename, "rtp://%s:%d", rtp_addr.c_str(), rtp_port);
	if (avio_open(&(oc->pb), oc->filename, AVIO_FLAG_WRITE) < 0)
	{
		fprintf(stderr, "could open rtp");
		exit(1);
	}

	// add a stream
	AVStream *s = av_new_stream(oc, 1);
	if (!s)
	{
		fprintf(stderr, "could add stream");
		exit(1);
	}

	// initalize codec
	AVCodecContext* occ = s->codec;
	occ->codec_id = c->codec_id;
	occ->codec_type = c->codec_type;
	occ->bit_rate = c->bit_rate;
	occ->width = c->width;
	occ->height = c->height;
	occ->time_base.den = c->time_base.den;
	occ->time_base.num = c->time_base.num;

	// write the header
	avformat_write_header(oc, NULL);


	// encode video
	for(i = 0;; i++) 
	{
		fflush(stdout);
		// prepare a dummy image

		for (y = 0; y < c->height; ++y)
		{
			for (x = 0; x < c->width; ++x)
			{
				picture->data[0][y * picture->linesize[0] + x] = (x * i) % 256;
				
				int x2 = x >> 1, y2 = y >> 1;

				picture->data[1][y2 * picture->linesize[1] + x2] = i * (x + y) % 256;
				picture->data[2][y2 * picture->linesize[2] + x2] = i * (x + y) % 128;
			}
		}

		/* Y */
		for(y=0;y<c->height;y++) 
		{
			for(x=0;x<c->width;x++) 
			{
				picture->data[0][y * picture->linesize[0] + x] = x + y + i * 3;
			}
		}

		/* Cb and Cr */
		for(y=0;y<c->height/2;y++) 
		{
			for(x=0;x<c->width/2;x++) 
			{
				picture->data[1][y * picture->linesize[1] + x] = 128 + y + i * 2;
				picture->data[2][y * picture->linesize[2] + x] = 64 + x + i * 5;
			}
		}

		// encode the image
		out_size = avcodec_encode_video(c, outbuf, outbuf_size, picture);
		had_output |= out_size;

		// initalize a packet
		AVPacket p;
		av_init_packet(&p);
		p.data = outbuf;
		p.size = out_size;
		p.stream_index = s->index;

		// send it out
		av_write_frame(oc, &p);
	}

	/* get the delayed frames */
	for(; out_size || !had_output; i++) 
	{
		fflush(stdout);

		out_size = avcodec_encode_video(c, outbuf, outbuf_size, NULL);
		had_output |= out_size;

		// initalize a packet
		AVPacket p;
		av_init_packet(&p);
		p.data = outbuf;
		p.size = out_size;
		p.stream_index = s->index;

		// send it out
		av_write_frame(oc, &p);
	}

	// add sequence end code to have a real mpeg file
	outbuf[0] = 0x00;
	outbuf[1] = 0x00;
	outbuf[2] = 0x01;
	outbuf[3] = 0xb7;
	//fwrite(outbuf, 1, 4, f);
	free(outbuf);

	avcodec_close(c);
	av_free(c);
	av_free(picture->data[0]);
	av_free(picture);
	printf("\n");
}
static void video_encode_example(const char *filename, int codec_id)
{
	AVCodecContext *c = NULL;
	AVCodec *codec;
	AVFrame *picture;

	int i, x, y, 
		out_size, 
		outbuf_size,
		had_output = 0;

	uint8_t *outbuf;

	FILE *f;

	// Debug out
	printf("Encode video file %s\n", filename);

	// find the video encoder
	codec = avcodec_find_encoder((CodecID)codec_id);
	
	if (!codec) 
	{
		fprintf(stderr, "codec not found\n");
		exit(1);
	}

	// get codec context and allocate frame
	c = avcodec_alloc_context3(codec);
	picture = avcodec_alloc_frame();

	// put sample parameters
	c->bit_rate = 400000;
	
	// resolution must be a multiple of two
	c->width = 640;
	c->height = 480;

	// frames per second
	c->time_base.num = 1; c->time_base.den = 25;
	
	c->gop_size = 10; /* emit one intra frame every ten frames */
	c->max_b_frames = 25;
	c->pix_fmt = PIX_FMT_YUV420P;

	if(codec_id == CODEC_ID_H264)
		av_opt_set (c->priv_data, "preset", "slow", 0);

	// open codec
	if (avcodec_open2(c, codec, NULL) < 0) 
	{
		fprintf(stderr, "could not open codec\n");
		exit(1);
	}

	// open file for writing
	f = fopen(filename, "wb");
	if (!f) 
	{
		fprintf(stderr, "could not open %s\n", filename);
		exit(1);
	}

	// allocate image and output buffer
	outbuf_size = 100000 + 12 * c->width * c->height;
	outbuf = (uint8_t*)malloc(outbuf_size);

	
	// the image can be allocated by any means and av_image_alloc() is
	// just the most convenient way if av_malloc() is to be used */
	av_image_alloc(picture->data, picture->linesize,
				   c->width, c->height, c->pix_fmt, 1);

	// encode video
	for(i = 0; i < 25 * 25; i++) 
	{
		fflush(stdout);
		// prepare a dummy image

		for (y = 0; y < c->height; ++y)
		{
			for (x = 0; x < c->width; ++x)
			{
				picture->data[0][y * picture->linesize[0] + x] = (x * i) % 256;
				
				int x2 = x >> 1, y2 = y >> 1;

				picture->data[1][y2 * picture->linesize[1] + x2] = i * (x + y) % 256;
				picture->data[2][y2 * picture->linesize[2] + x2] = i * (x + y) % 128;
			}
		}

		/* Y */
		for(y=0;y<c->height;y++) 
		{
			for(x=0;x<c->width;x++) 
			{
				picture->data[0][y * picture->linesize[0] + x] = x + y + i * 3;
			}
		}

		/* Cb and Cr */
		for(y=0;y<c->height/2;y++) 
		{
			for(x=0;x<c->width/2;x++) 
			{
				picture->data[1][y * picture->linesize[1] + x] = 128 + y + i * 2;
				picture->data[2][y * picture->linesize[2] + x] = 64 + x + i * 5;
			}
		}

		// encode the image
		out_size = avcodec_encode_video(c, outbuf, outbuf_size, picture);
		had_output |= out_size;
		printf("encoding frame %3d (size=%5d)\n", i, out_size);
		fwrite(outbuf, 1, out_size, f);
	}

	/* get the delayed frames */
	for(; out_size || !had_output; i++) 
	{
		fflush(stdout);

		out_size = avcodec_encode_video(c, outbuf, outbuf_size, NULL);
		had_output |= out_size;
		printf("write frame %3d (size=%5d)\n", i, out_size);
		fwrite(outbuf, 1, out_size, f);
	}

	// add sequence end code to have a real mpeg file
	outbuf[0] = 0x00;
	outbuf[1] = 0x00;
	outbuf[2] = 0x01;
	outbuf[3] = 0xb7;
	fwrite(outbuf, 1, 4, f);
	fclose(f);
	free(outbuf);

	avcodec_close(c);
	av_free(c);
	av_free(picture->data[0]);
	av_free(picture);
	printf("\n");
}
void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[32];
  int  y;
  
  // Open file
  sprintf(szFilename, "frame%d.ppm", iFrame);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
	return;
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  for(y=0; y<height; y++)
	fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
  
  // Close file
  fclose(pFile);
}

char maybuffer[320 * 240 * 3];

int myImemGetCallback (void *data, const char *cookie, int64_t *dts, int64_t *pts, unsigned *flags, size_t * bufferSize, void ** buffer)
{
	static unsigned char a = 255;
	static int64_t e = 0, 
		f = (int64_t) 1000000.f * (1.f / 50.f);
	
	e += f;

	char *mbuffer = maybuffer; // (char*)data;
	
	for (int p = 0; p < 320 * 240 * 3; ++p)
		mbuffer[p] = (a+p) % 255;

	*dts = *pts = e;

	*bufferSize = 320 * 250 * 3;
	*buffer = mbuffer;
	a++;

	std::cout << "A: " << a << ", E: " << e << std::endl;

	return 0;
}
int myImemReleaseCallback (void *data, const char *cookie, size_t bufferSize, void * buffer)
{
	return 0;
}

static void video_vlc_test(void)
{
	libvlc_instance_t * inst;
	libvlc_media_player_t *mp;
	libvlc_media_t *m;
 
	std::vector<char*> arguments;

	char *args[] = {
		"--imem-width=320", "--imem-height=240"
		, "--imem-codec=RGB2", "--imem-cat=2"
		, "--imem-id=1", "--imem-group=1"
		, "--verbose=2"
	};

	std::stringstream get;
		get << "--imem-get=" << (long long int)myImemGetCallback;

	std::stringstream release;
		release << "--imem-release=" << (long long int)myImemReleaseCallback;

	arguments.push_back(args[0]);
	arguments.push_back(args[1]);
	arguments.push_back(args[2]);
	arguments.push_back(args[3]);
	arguments.push_back(args[4]);
	arguments.push_back(args[5]);
	// arguments.push_back(args[6]);

	char *pget = new char[get.str().length() + 1];
	char *prelease = new char[release.str().length() + 1];

	memset(pget, 0, get.str().length() + 1);
	memset(prelease, 0, release.str().length() + 1);

	strncpy (prelease, (char*)release.str().c_str(), release.str().length());
	strncpy (pget, (char*)get.str().c_str(), get.str().length());

	arguments.push_back(pget);
	arguments.push_back(prelease);

	std::cout << "::::::" << get.str() << " : " << release.str() << std::endl;
	std::cout << "::::::" << myImemGetCallback << " : " << myImemReleaseCallback << std::endl;

	std::stringstream imemArgs;

	/* imemArgs << "imem:// :imem-get=" << myImemGetCallback << " :imem-release=" << myImemReleaseCallback << 
		" :imem-width=320 :imem-height=240" <<
		" :imem-codec=RGB2 :imem-cat=2 :imem-id=1 :imem-group=1"; */

	//arguments.push_back((char*)(get.str().c_str()));
	//arguments.push_back((char*)(release.str().c_str()));

	/*
	sprintf(arg[0], "--imem-width=320");
	sprintf(arg[1], "--imem-height=240");
	sprintf(arg[2], "--imem-get=%ld", myImemGetCallback);
	sprintf(arg[3], "--imem-release=%ld", myImemReleaseCallback);
	sprintf(arg[4], "--imem-codec=RGB2");
	sprintf(arg[5], "--imem-data=%ld", maybuffer); 
	sprintf(arg[6], "--imem-cookie=LIBA"); 
	sprintf(arg[7], "--imem-cat=2");
	sprintf(arg[8], " --imem-id=1");
	sprintf(arg[9], " --imem-group=1");
	*/

	/* Load the VLC engine */
	std::cout << "Creating VLC Instance ... ";
	inst = libvlc_new (arguments.size(), (const char *const *)&arguments[0]);
	//inst = libvlc_new (0, NULL);
	std::cout << "[DONE]" << std::endl;
  
	 /* Create a new item */
	std::cout << "Creating Media Instance ... ";
	//m = libvlc_media_new_path (inst, "C:\\Users\\Keith\\Dropbox\\Public\\sponza.mpg");
	m = libvlc_media_new_path (inst, "imem:// :transcode{vcodec=mp2v,scale=1,acodec=none}" /*(const char*)imemArgs.str().c_str()*/ );// "imem://");
	std::cout << "[DONE]" << std::endl;        
	
	/* Create a media player playing environement */
	std::cout << "Creating Media Player Instance ... ";
	mp = libvlc_media_player_new_from_media (m);
	std::cout << "[DONE]";
	
	///* No need to keep the media now */
	//libvlc_media_release (m);
 
	 /* play the media_player */
	std::cout << "Media Player Play ... ";
	char *vlcOptions[] = {""};
	libvlc_vlm_add_broadcast(inst, "tc_smart", "imem://", "#transcode{vcodec=mp2v,scale=1,acodec=none}:rtp{dst=192.168.17.99,port=10000,mux=ts}", 0, vlcOptions, 1, 0);
	libvlc_vlm_play_media(inst, "tc_smart");
	libvlc_media_player_play (mp);
	
	std::getchar();
		
	/* Stop playing */
	libvlc_media_player_stop (mp);
	std::cout << " and Stopped " << std::endl;
 
	 /* Free the media_player */
	libvlc_media_player_release (mp);
 
	libvlc_release (inst);
 
	std::getchar();
 }

//static void video_vlc_test(void)
//{
//	libvlc_instance_t * inst;
//	libvlc_media_player_t *mp;
//	libvlc_media_t *m;
// 
//	/* Load the VLC engine */
//	std::cout << "Creating VLC Instance ... ";
//	inst = libvlc_new (0, NULL);
//	std::cout << "[DONE]" << std::endl;
//  
//	 /* Create a new item */
//	std::cout << "Creating Media Instance ... ";
//	m = libvlc_media_new_path (inst, "C:\\Users\\Keith\\Dropbox\\Public\\sponza.mpg");
//	std::cout << "[DONE]" << std::endl;        
//	
//	/* Create a media player playing environement */
//	std::cout << "Creating Media Player Instance ... ";
//	mp = libvlc_media_player_new_from_media (m);
//	std::cout << "[DONE]";
//	
//	/* No need to keep the media now */
//	libvlc_media_release (m);
// 
//	 /* play the media_player */
//	std::cout << "Media Player Play ... ";
//	char *vlcOptions[] = {""};
//	libvlc_vlm_add_broadcast(inst, "tc_smart", "C:\\Users\\Keith\\Dropbox\\Public\\sponza.mpg", "#rtp{dst=192.168.17.212,port=10000,mux=ts}", 0, vlcOptions, 1, 0);
//	libvlc_vlm_play_media(inst, "tc_smart");
//	libvlc_media_player_play (mp);
//	
//	std::getchar();
//		
//	/* Stop playing */
//	libvlc_media_player_stop (mp);
//	std::cout << " and Stopped " << std::endl;
// 
//	 /* Free the media_player */
//	libvlc_media_player_release (mp);
// 
//	libvlc_release (inst);
// 
//	std::getchar();
// }

void IlluminaPRT(bool p_bVerbose, int p_nIterations, std::string p_strScript)
{
	//av_register_all();
	//avformat_network_init();
	//video_stream_example("127.0.0.1", 6666, CODEC_ID_MPEG2VIDEO);
	//video_encode_example("Z:\\test.mpeg", CODEC_ID_MPEG2VIDEO);

	// video_vlc_test();

	// std::getchar();
	// return;

	//----------------------------------------------------------------------------------------------
	// Illumina sandbox environment 
	//----------------------------------------------------------------------------------------------
	SandboxEnvironment sandbox;

	sandbox.Initialise(p_bVerbose);
	sandbox.LoadScene(p_strScript, p_bVerbose);

	//----------------------------------------------------------------------------------------------
	// Alias required components
	//----------------------------------------------------------------------------------------------
	IIntegrator *pIntegrator = sandbox.GetEnvironment()->GetIntegrator();
	IRenderer *pRenderer = sandbox.GetEnvironment()->GetRenderer();
	ICamera *pCamera = sandbox.GetEnvironment()->GetCamera();
	ISpace *pSpace = sandbox.GetEnvironment()->GetSpace();

	Environment *pEnvironment = sandbox.GetEnvironment();
	EngineKernel *pEngineKernel = sandbox.GetEngineKernel();

	// Open output device
	pRenderer->GetDevice()->Open();

	// Initialisation complete
	Logger::Message("Initialisation complete. Rendering in progress...", p_bVerbose);

	//----------------------------------------------------------------------------------------------
	// Initialise timing
	//----------------------------------------------------------------------------------------------
	float fTotalFramesPerSecond = 0.f;
	double start, elapsed = 0, eventStart, eventComplete;

	//----------------------------------------------------------------------------------------------
	// Render loop
	//----------------------------------------------------------------------------------------------
	RadianceBuffer *pRadianceBuffer = new RadianceBuffer(
		pRenderer->GetDevice()->GetWidth(), pRenderer->GetDevice()->GetHeight()),
		*pRadianceAccumulationBuffer = new RadianceBuffer(
		pRenderer->GetDevice()->GetWidth(), pRenderer->GetDevice()->GetHeight());

	IPostProcess *pDiscontinuityBuffer = pEngineKernel->GetPostProcessManager()->CreateInstance("Discontinuity", "DiscontinuityBuffer", "");
	IPostProcess *pAutoTone = pEngineKernel->GetPostProcessManager()->CreateInstance("AutoTone", "AutoTone", "");
	IPostProcess *pDragoTone = pEngineKernel->GetPostProcessManager()->CreateInstance("DragoTone", "DragoTone", "");
	IPostProcess *pReconstructionBuffer = pEngineKernel->GetPostProcessManager()->CreateInstance("Reconstruction", "ReconstructionBuffer", "");
	
	AccumulationBuffer *pAccumulationBuffer = (AccumulationBuffer*)pEngineKernel->GetPostProcessManager()->CreateInstance("Accumulation", "AccumulationBuffer", "");
	pAccumulationBuffer->SetAccumulationBuffer(pRadianceAccumulationBuffer);
	pAccumulationBuffer->Reset();

	float alpha = Maths::Pi;
	Matrix3x3 rotation;
	
	struct regioninfo_t 
	{
		double lastActual;
		double lastPredicted;
		double nextTime;
		double frameBudget;
	};
	
	const int regionWidth = 32;
	const int regionHeight = 32;

	const int regionX = pRenderer->GetDevice()->GetWidth() / regionWidth;
	const int regionY = pRenderer->GetDevice()->GetHeight() / regionHeight;

	const int regions = regionX * regionY;

	float totalBudget = 0.5f;
	float requiredBudget = 0.f;
	std::vector<regioninfo_t> reg(regions);
	
	for (int j = 0; j < regions; j++)
	{
		reg[j].lastActual = 0;
		reg[j].lastPredicted = 0;
		reg[j].nextTime = 0;
		reg[j].frameBudget = 0;
	}

	//pRenderer->SetRenderBudget(1e+20f);
	pRenderer->SetRenderBudget(0.05f / ((float)regions * 0.33f));
	//pRenderer->SetRenderBudget(10.f);
	Vector3 observer = pCamera->GetObserver();

	for (int nFrame = 0; nFrame < p_nIterations; ++nFrame)
	{
		#if (defined(TEST_TILERENDER))
			
			// Animate scene 
			alpha += Maths::PiTwo / 180.f;
		
			rotation.MakeRotation(Vector3::UnitYPos, alpha);

			////((GeometricPrimitive*)pSpace->PrimitiveList[0])->WorldTransform.SetScaling(Vector3::Ones * 20.0f);
			//// ((GeometricPrimitive*)pSpace->PrimitiveList[0])->WorldTransform.SetRotation(rotation);

			////pCamera->MoveTo(lookFrom);
			////pCamera->MoveTo(Vector3(Maths::Cos(alpha) * lookFrom.X, lookFrom.Y, Maths::Sin(alpha) * lookFrom.Z));
			////pCamera->LookAt(lookAt);
			Vector3 observer_ = observer;
			observer_.Z += Maths::Cos(alpha) * 4.f;
			observer_.X += Maths::Sin(alpha) * 2.f;
			pCamera->MoveTo(observer_);

			// Start timer
			start = Platform::GetTime();

			// Prepare integrator
			pIntegrator->Prepare(pEnvironment->GetScene());

			if (p_bVerbose) 
			{
				eventComplete = Platform::GetTime();
				elapsed = Platform::ToSeconds(eventComplete - start); 
				std::cout << std::endl << "-- Frame " << nFrame;
				std::cout << std::endl << "-- Integrator Preparation Time : [" << elapsed << "s]" << std::endl;

				eventStart = Platform::GetTime();
			}

			// Update space
			pSpace->Update();

			if (p_bVerbose) 
			{
				eventComplete = Platform::GetTime();
				elapsed = Platform::ToSeconds(eventComplete - eventStart); 
				std::cout << "-- Space Update Time : [" << elapsed << "s]" << std::endl;
				
				eventStart = Platform::GetTime();
			}
	 
			// Render frame
			#pragma omp parallel for schedule(static, 8) num_threads(2)
			for (int y = 0; y < regionY; y++)
			{
				for (int x = 0; x < regionX; x++)
				{
					double regionStart = Platform::GetTime();
					pRenderer->RenderRegion(pRadianceBuffer, x * regionWidth, y * regionHeight, regionWidth, regionHeight, x * regionWidth, y * regionHeight);
					double regionEnd = Platform::GetTime();

					double lastActual = Platform::ToSeconds(regionEnd - regionStart);
					reg[x + y * regionX].lastActual = lastActual;
					reg[x + y * regionX].nextTime = reg[x + y * regionX].lastPredicted * 0.5 + lastActual * 0.5;
					reg[x + y * regionX].lastPredicted = reg[x + y * regionX].nextTime;
				}
			}

			requiredBudget = 0.f;
			
			for (int j = 0; j < regions; j++)
			{
				requiredBudget += reg[j].nextTime;
			}

			if (p_bVerbose) 
			{
				eventComplete = Platform::GetTime();
				elapsed = Platform::ToSeconds(eventComplete - eventStart); 
				std::cout << "-- Radiance Computation Time : [" << elapsed << "s]" << std::endl;

				eventStart = Platform::GetTime();
			}

			// Post-process frame
			pReconstructionBuffer->Apply(pRadianceBuffer, pRadianceBuffer);
			pDiscontinuityBuffer->Apply(pRadianceBuffer, pRadianceBuffer);

			pEnvironment->GetScene()->GetSampler()->Reset();
			//pAccumulationBuffer->Reset();
			//pAccumulationBuffer->Apply(pRadianceBuffer, pRadianceBuffer);

			pDragoTone->Apply(pRadianceBuffer, pRadianceBuffer);
			// pAutoTone->Apply(pRadianceBuffer, pRadianceBuffer);

			if (p_bVerbose) 
			{
				eventComplete = Platform::GetTime();
				elapsed = Platform::ToSeconds(eventComplete - eventStart); 
				std::cout << "-- Post-processing Time : [" << elapsed << "s]" << std::endl;

				eventStart = Platform::GetTime();
			}

			// Commit frame
			static int frameId = 4;
			// if (frameId++ % 5 == 0)
				pRenderer->Commit(pRadianceBuffer);

			// Compute frames per second
			elapsed = Platform::ToSeconds(Platform::GetTime() - start);
			fTotalFramesPerSecond += (float)(1.f/elapsed);
		
			if (p_bVerbose)
			{
				std::cout << "-- Frame Render Time : [" << elapsed << "s]" << std::endl;
				std::cout << "-- Frames per second : [" << fTotalFramesPerSecond / (nFrame + 1)<< "]" << std::endl;

				for (int j = 0; j < regions; j++)
				{
					reg[j].frameBudget = reg[j].nextTime / requiredBudget;
					// std::cout << "[Region " << j << "] : B:[" << reg[j].frameBudget << "], T:[" << reg[j].lastActual << "s], P:[" << reg[j].lastPredicted << "s], N:[" << reg[j].nextTime << "]" << std::endl;  
				}
			}
		#else
			// Update space acceleration structure
			pSpace->Update();

			// Render image
			pRenderer->Render();
		#endif
	}

	//----------------------------------------------------------------------------------------------
	// Shutdown system
	//----------------------------------------------------------------------------------------------
	// Close output device
	pRenderer->GetDevice()->Close();

	//----------------------------------------------------------------------------------------------
	// Shutdown renderer and integrator
	pRenderer->Shutdown();
	pIntegrator->Shutdown();

	//----------------------------------------------------------------------------------------------
	// Shutdown snadbox
	sandbox.Shutdown(p_bVerbose);

	//----------------------------------------------------------------------------------------------
	if (p_bVerbose)
	{
		Logger::Message("Complete :: Press enter to continue", true);
		int v = std::getchar();
	}
}
//----------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	std::cout << "Illumina Renderer : Version " << Illumina::Core::Major << "." << Illumina::Core::Minor << "." << Illumina::Core::Build << " http://www.illuminaprt.codeplex.com " << std::endl;
	std::cout << "Copyright (C) 2010-2012 Keith Bugeja" << std::endl << std::endl;

	// default options
	int nIterations = 1;
	bool bVerbose = false;
	std::string strScript("default.ilm");

	// Declare the supported options.
	boost::program_options::options_description description("Allowed Settings");

	description.add_options()
		("help", "show this message")
		("verbose", boost::program_options::value<bool>(), "show extended information")
		("script", boost::program_options::value<std::string>(), "script file to render")
		("workdir", boost::program_options::value<std::string>(), "working directory")
		("iterations", boost::program_options::value<int>(), "iterations to execute")
		;

	// Declare variable map
	boost::program_options::variables_map variableMap;

	// Parse command line options
	try 
	{
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), variableMap);
		boost::program_options::notify(variableMap);
	} 
	catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::unknown_option> > &exception) 
	{
		std::cout << "Unknown option [" << exception.get_option_name() << "] : Please use --help to display help message." << std::endl;
		return 1;
	}
	catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::invalid_option_value> > &exception) 
	{
		std::cout << "Error parsing input for [" << exception.get_option_name() << "] : Invalid argument value." << std::endl;
		return 1;
	}

	// --help
	if (variableMap.count("help"))
	{
		std::cout << description << std::endl;
		return 1;
	}

	// --verbose
	if (variableMap.count("verbose"))
	{
		bVerbose = variableMap["verbose"].as<bool>();
		std::cout << "Verbose mode [" << (bVerbose ? "ON]" : "OFF]") << std::endl;
	}

	// --iterations
	if (variableMap.count("iterations"))
	{
		try {
			nIterations = variableMap["iterations"].as<int>();
		} catch (...) { nIterations = 1; } 
		std::cout << "Iterations [" << nIterations << "]" << std::endl;
	}

	// --script
	if (variableMap.count("script"))
	{
		strScript = variableMap["script"].as<std::string>();
		std::cout << "Script [" << strScript << "]" << std::endl;
	}

	// --workdir
	boost::filesystem::path cwdPath;

	if (variableMap.count("workdir"))
	{
		cwdPath = boost::filesystem::path(variableMap["workdir"].as<std::string>());
	}
	else
	{
		// Setting working directory
		boost::filesystem::path scriptPath(strScript);
		cwdPath = boost::filesystem::path(scriptPath.parent_path());
	}

	try {
		boost::filesystem::current_path(cwdPath);
		std::cout << "Working directory [" << cwdPath.string() << "]" << std::endl;;
	} catch (...) { std::cerr << "Error : Unable to set working directory to " << cwdPath.string() << std::endl; }

	// -- start rendering
	IlluminaPRT(bVerbose, nIterations, strScript);

	// Exit
	return 1;
}
//----------------------------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------------------------
void MessageOut(const std::string& p_strMessage, bool p_bVerbose)
{
	if (p_bVerbose) std::cout << p_strMessage << std::endl;
}
//----------------------------------------------------------------------------------------------

#include "scheduling.h"

//----------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	std::cout << "Illumina Renderer Service : Version " << Major << "." << Minor << "." << Build << " http://www.illuminaprt.codeplex.com " << std::endl;
	std::cout << "Copyright (C) 2010-2012 Keith Bugeja" << std::endl << std::endl;

	// default options
	int nIterations = 1;
	bool bVerbose = false;
	std::string strScript("default.ilm");

	// Declare the supported options.
	boost::program_options::options_description description("Allowed Settings");

	description.add_options()
		("help", "show this message")
		("verbose", boost::program_options::value<bool>(), "show extended information")
		("script", boost::program_options::value<std::string>(), "script file to render")
		("workdir", boost::program_options::value<std::string>(), "working directory")
		("iterations", boost::program_options::value<int>(), "interations to execute")
		;

	// Declare variable map
	boost::program_options::variables_map variableMap;

	// Parse command line options
	try 
	{
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), variableMap);
		boost::program_options::notify(variableMap);
	} 
	catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::unknown_option> > &exception) 
	{
		std::cout << "Unknown option [" << exception.get_option_name() << "] : Please use --help to display help message." << std::endl;
		return 1;
	}
	catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::invalid_option_value> > &exception) 
	{
		std::cout << "Error parsing input for [" << exception.get_option_name() << "] : Invalid argument value." << std::endl;
		return 1;
	}

	// --help
	if (variableMap.count("help"))
	{
		std::cout << description << std::endl;
		return 1;
	}

	// --verbose
	if (variableMap.count("verbose"))
	{
		bVerbose = variableMap["verbose"].as<bool>();
		std::cout << "Verbose mode [" << (bVerbose ? "ON]" : "OFF]") << std::endl;
	}

	// --iterations
	if (variableMap.count("iterations"))
	{
		try {
			nIterations = variableMap["iterations"].as<int>();
		} catch (...) { nIterations = 1; } 
		std::cout << "Iterations [" << nIterations << "]" << std::endl;
	}

	// --script
	if (variableMap.count("script"))
	{
		strScript = variableMap["script"].as<std::string>();
		std::cout << "Script [" << strScript << "]" << std::endl;
	}

	// --workdir
	boost::filesystem::path cwdPath;

	if (variableMap.count("workdir"))
	{
		cwdPath = boost::filesystem::path(variableMap["workdir"].as<std::string>());
	}
	else
	{
		// Setting working directory
		boost::filesystem::path scriptPath(strScript);
		cwdPath = boost::filesystem::path(scriptPath.parent_path());
	}

	try {
		boost::filesystem::current_path(cwdPath);
		std::cout << "Working directory [" << cwdPath.string() << "]" << std::endl;;
	} catch (...) { std::cerr << "Error : Unable to set working directory to " << cwdPath.string() << std::endl; }

	// -- start service
	RunAsServer(argc, argv, bVerbose);
	return 0;
}
#endif