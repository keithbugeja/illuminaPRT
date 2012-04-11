//----------------------------------------------------------------------------------------------
//	Filename:	SamplerManager.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <iostream>

#include "Sampler/Sampler.h"
#include "Sampler/JitterSampler.h"
#include "Sampler/RandomSampler.h"

namespace Illumina
{
	namespace Core
	{
		class JitterSamplerFactory : public Illumina::Core::Factory<Illumina::Core::ISampler>
		{
		public:
			Illumina::Core::ISampler *CreateInstance(void)
			{
				return new JitterSampler();
			}

			// Arguments
			// -- Id {String}
			Illumina::Core::ISampler *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId);

				return CreateInstance();
			}

			Illumina::Core::ISampler *CreateInstance(const std::string &p_strId)
			{
				return new JitterSampler(p_strId);
			}
		};

		class MultijitterSamplerFactory : public Illumina::Core::Factory<Illumina::Core::ISampler>
		{
		public:
			Illumina::Core::ISampler *CreateInstance(void)
			{
				return new MultijitterSampler();
			}

			// Arguments
			// -- Id {String}
			Illumina::Core::ISampler *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId);

				return CreateInstance();
			}

			Illumina::Core::ISampler *CreateInstance(const std::string &p_strId)
			{
				return new MultijitterSampler(p_strId);
			}
		};

		class RandomSamplerFactory : public Illumina::Core::Factory<Illumina::Core::ISampler>
		{
		public:
			Illumina::Core::ISampler *CreateInstance(void)
			{
				return new RandomSampler();
			}

			// Arguments
			// -- Id {String}
			Illumina::Core::ISampler *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId);

				return CreateInstance();
			}

			Illumina::Core::ISampler *CreateInstance(const std::string &p_strId)
			{
				return new RandomSampler(p_strId);
			}
		};

		class PrecomputationSamplerFactory : public Illumina::Core::Factory<Illumina::Core::ISampler>
		{
		public:
			Illumina::Core::ISampler *CreateInstance(void)
			{
				return new PrecomputationSampler<16384, 10619863, 3331333>();
			}

			// Arguments
			// -- Id {String}
			Illumina::Core::ISampler *CreateInstance(ArgumentMap &p_argumentMap)
			{
				std::string strId;

				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId);

				return CreateInstance();
			}

			Illumina::Core::ISampler *CreateInstance(const std::string &p_strId)
			{
				return new PrecomputationSampler<16384, 10619863, 3331333>(p_strId);
			}
		};
	}
}