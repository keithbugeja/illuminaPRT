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
#include "Postproc/AutoTone.h"
#include "Postproc/DragoTone.h"
#include "Postproc/AccumulationBuffer.h"
#include "Postproc/DiscontinuityBuffer.h"

namespace Illumina
{
	namespace Core
	{		
		class AccumulationBufferFactory : public Illumina::Core::Factory<Illumina::Core::IPostProcess>
		{
		public:
			Illumina::Core::IPostProcess *CreateInstance(void)
			{
				return new AccumulationBuffer();
			}

			// Arguments
			// -- Id
			Illumina::Core::IPostProcess *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;
				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId);

				return CreateInstance();
			}

			Illumina::Core::IPostProcess *CreateInstance(const std::string &p_strId)
			{
				return new AccumulationBuffer(p_strId);
			}

			Illumina::Core::IPostProcess *CreateInstance(int dummy)
			{
				return new AccumulationBuffer();
			}
		};


		class AutoToneFactory : public Illumina::Core::Factory<Illumina::Core::IPostProcess>
		{
		public:
			Illumina::Core::IPostProcess *CreateInstance(void)
			{
				return new AutoTone();
			}

			// Arguments
			// -- Id
			Illumina::Core::IPostProcess *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;
				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId);

				return CreateInstance();
			}

			Illumina::Core::IPostProcess *CreateInstance(const std::string &p_strId)
			{
				return new AutoTone(p_strId);
			}

			Illumina::Core::IPostProcess *CreateInstance(int dummy)
			{
				return new AutoTone();
			}
		};

		class DragoToneFactory : public Illumina::Core::Factory<Illumina::Core::IPostProcess>
		{
		public:
			Illumina::Core::IPostProcess *CreateInstance(void)
			{
				return new DragoTone();
			}

			// Arguments
			// -- Id
			Illumina::Core::IPostProcess *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;
				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId);

				return CreateInstance();
			}

			Illumina::Core::IPostProcess *CreateInstance(const std::string &p_strId)
			{
				return new DragoTone(p_strId);
			}

			Illumina::Core::IPostProcess *CreateInstance(int dummy)
			{
				return new DragoTone();
			}
		};

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
				int kernelSize = 3;
				
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