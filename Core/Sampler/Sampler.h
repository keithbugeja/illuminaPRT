//----------------------------------------------------------------------------------------------
//	Filename:	Sampler.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "System/FactoryManager.h"

#include "Object/Object.h"
#include "Maths/Statistics.h"

#include "Image/ImageIO.h"
#include "Image/ImagePPM.h"
//----------------------------------------------------------------------------------------------
namespace Illumina 
{
	namespace Core
	{
		struct Sample
		{
		protected:
			int m_count;
			float *m_sequence;

		public:
			Sample(int p_nCount) 
				: m_count(p_nCount)
				, m_sequence(new float[m_count])
			{ }

			~Sample(void) { delete m_sequence; }

			inline int Size(void) const {
				return m_count; 
			}

			inline float* GetSequence(void) const {
				return m_sequence;
			}
		
			inline float operator[](int p_nIndex) const
			{
				BOOST_ASSERT(p_nIndex < m_count);
				return m_sequence[p_nIndex];
			}
		};

		//----------------------------------------------------------------------------------------------
		// ISampler : Abstract base class for random sample generation methods. 
		//----------------------------------------------------------------------------------------------
		class ISampler
			: public Object
		{
		protected:
			ISampler(const std::string &p_strName) : Object(p_strName) { }
			ISampler(void) { }

		public:
			virtual void Reset(void) = 0;
			virtual void Reset(unsigned int p_unSeed) = 0;

			virtual void Get2DSamples(Vector2* p_pSamples, int p_nSampleCount) = 0;
			virtual void Get1DSamples(float* p_pSamples, int p_nSampleCount) = 0;
			
			virtual float Get1DSample(void) = 0;
			virtual Vector2 Get2DSample(void) = 0;

			virtual void GetSample(Sample* p_pSample) = 0;

			virtual std::string ToString(void) const { return "ISampler"; }
		};

		//----------------------------------------------------------------------------------------------
		// SamplerManager : All Sampler factories must register with object.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<ISampler> SamplerManager;
	} 
}