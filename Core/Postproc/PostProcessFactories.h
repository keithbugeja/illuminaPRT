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
				int kernelSize = 6;
				
				float angle	= 0.75f,
					distance = 100000.0f;

				p_argumentMap.GetArgument("KernelSize", kernelSize);
				p_argumentMap.GetArgument("AngleThreshold", angle);
				p_argumentMap.GetArgument("DistanceThreshold", distance);

				std::string strId;
				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, kernelSize, angle, distance);

				return CreateInstance(kernelSize, angle, distance);
			}

			Illumina::Core::IPostProcess *CreateInstance(const std::string &p_strId, int p_nKernelSize, float p_fAngle, float p_fDistance)
			{
				return new DiscontinuityBuffer(p_strId, p_nKernelSize, p_fAngle, p_fDistance);
			}

			Illumina::Core::IPostProcess *CreateInstance(int p_nKernelSize, float p_fAngle, float p_fDistance)
			{
				return new DiscontinuityBuffer(p_nKernelSize, p_fAngle, p_fDistance);
			}
		};
	}
}