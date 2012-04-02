//----------------------------------------------------------------------------------------------
//	Filename:	PostProcessFactories.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <iostream>

#include "Postproc/PostProcess.h"
#include "Postproc/DiscontinuityBuffer.h"

namespace Illumina
{
	namespace Core
	{		
		class DiscontinuityBufferFactory : public Illumina::Core::Factory<Illumina::Core::IPostProcess>
		{
		public:
			Illumina::Core::IPostProcess *CreateInstance(void)
			{
				return new DiscontinuityBuffer();
			}

			// Arguments
			// -- Id
			// -- KernelSize
			Illumina::Core::IPostProcess *CreateInstance(ArgumentMap &p_argumentMap)
			{
				int kernelSize = 1;
				p_argumentMap.GetArgument("KernelSize", kernelSize);

				std::string strId;
				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, kernelSize);

				return CreateInstance(kernelSize);
			}

			Illumina::Core::IPostProcess *CreateInstance(const std::string &p_strId, int p_nKernelSize)
			{
				return new DiscontinuityBuffer(p_strId, p_nKernelSize);
			}

			Illumina::Core::IPostProcess *CreateInstance(int p_nKernelSize)
			{
				return new DiscontinuityBuffer(p_nKernelSize);
			}
		};
	}
}