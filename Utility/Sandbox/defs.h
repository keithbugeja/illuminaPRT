//----------------------------------------------------------------------------------------------
// Illumina PRT compilation modes:
//	ILLUMINA_SHM : Compilation for local, multithreaded, shared memory systems (no network support)
//		ILLUMINA_SHMVIEWER : Compile as a shared memory viewer (to be phased out; separate project)
//  ILLUMINA_DSM : Compilation for multiple-client support (cloud deployment)
//  ILLUMINA_P2P : Compilation for peer-to-peer support (client collaboration)
//----------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------
//	Set Illumina PRT compilation mode (SHM, DSM or P2P)
//----------------------------------------------------------------------------------------------
// #define ILLUMINA_P2P
#define ILLUMINA_SHM

#if (!defined ILLUMINA_SHM) || (!defined ILLUMINA_P2P) 
	#define ILLUMINA_DSM
#else
	/* I hate myself for this, but publication deadlines are what they are */
	// #define ILLUMINA_SHMVIEWER
	#if (defined ILLUMINA_SHMVIEWER)
		#include "SHMViewer.h"
	#endif

	#define ILLUMINA_GISERVER
	#if (defined ILLUMINA_GISERVER)
		#include "PointSet.h"
	#endif
	/**/
#endif
