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
#include "Sampler/RandomSampler.h"
#include "Sampler/JitterSampler.h"
#include "Sampler/MultijitterSampler.h"
#include "Sampler/PrecomputationSampler.h"
#include "Sampler/LowDiscrepancySampler.h"

namespace Illumina
{
	namespace Core
	{
		//----------------------------------------------------------------------------------------------
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
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
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
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		class LowDiscrepancySamplerFactory : public Illumina::Core::Factory<Illumina::Core::ISampler>
		{
		public:
			Illumina::Core::ISampler *CreateInstance(void)
			{
				return new LowDiscrepancySampler();
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
				return new LowDiscrepancySampler(p_strId);
			}
		};

		//----------------------------------------------------------------------------------------------
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
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		class PrecomputedHaltonSamplerFactory : public Illumina::Core::Factory<Illumina::Core::ISampler>
		{
		public:
			Illumina::Core::ISampler *CreateInstance(void)
			{
				return new PrecomputedHaltonSampler();
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
				return new PrecomputedHaltonSampler;
			}
		};
		//----------------------------------------------------------------------------------------------
		
		//----------------------------------------------------------------------------------------------
		class PrecomputedSobolSamplerFactory : public Illumina::Core::Factory<Illumina::Core::ISampler>
		{
		public:
			Illumina::Core::ISampler *CreateInstance(void)
			{
				return new PrecomputedSobolSampler();
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
				return new PrecomputedSobolSampler;
			}
		};
		//----------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------
		class PrecomputedRandomSamplerFactory : public Illumina::Core::Factory<Illumina::Core::ISampler>
		{
		public:
			Illumina::Core::ISampler *CreateInstance(void)
			{
				return new PrecomputedRandomSampler();
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
				return new PrecomputedRandomSampler;
			}
		};
		//----------------------------------------------------------------------------------------------
	}
}