//----------------------------------------------------------------------------------------------
//	Filename:	Renderer.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class IRenderer
		{
		public:
			virtual void Render(void) = 0;
		};
	}
}