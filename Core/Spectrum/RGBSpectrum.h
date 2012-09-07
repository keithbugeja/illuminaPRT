//----------------------------------------------------------------------------------------------
//	Filename:	RGBSpectrum.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <string>
#include <boost/format.hpp>

#include "Spectrum/BaseSpectrum.h"
#include "Maths/Maths.h"
//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{
		class RGBSpectrum
		{
		protected:
			static const int SampleCount = 3;
			float m_fSamples[SampleCount];

		public:
			RGBSpectrum(void)
			{
				m_fSamples[0] = m_fSamples[1] = m_fSamples[2] = 0.0f;
			}

			RGBSpectrum(float p_fValue)
			{
				m_fSamples[0] = m_fSamples[1] = m_fSamples[2] = p_fValue;
			}

			RGBSpectrum(float p_fRed, float p_fGreen, float p_fBlue)
			{
				m_fSamples[0] = p_fRed; 
				m_fSamples[1] = p_fGreen; 
				m_fSamples[2] = p_fBlue;
			}

			RGBSpectrum(const RGBSpectrum &p_rgbSpectrum)
			{ 
				*this = p_rgbSpectrum; 
			}

			int GetSampleCount(void) const { return SampleCount; }

			float operator[](int p_nIndex) const { return m_fSamples[p_nIndex]; }
			float& operator[](int p_nIndex) { return m_fSamples[p_nIndex]; }

			RGBSpectrum& operator+=(const RGBSpectrum &p_spectrum) {
				return *this = *this + p_spectrum;
			}

			RGBSpectrum& operator-=(const RGBSpectrum &p_spectrum) {
				return *this = *this - p_spectrum;
			}

			RGBSpectrum& operator*=(const RGBSpectrum &p_spectrum) {
				return *this = *this * p_spectrum;
			}

			RGBSpectrum& operator/=(const RGBSpectrum &p_spectrum) {
				return *this = *this / p_spectrum;
			}

			RGBSpectrum& operator*=(float p_fScale) {
				return *this = *this * p_fScale;
			}

			RGBSpectrum& operator/=(float p_fScale) {
				return *this = *this / p_fScale;
			}

			RGBSpectrum& operator/=(int p_nScale) {
				return *this = *this / (float)p_nScale;
			}

			bool IsBlack(void) const
			{
				if (Maths::FAbs(m_fSamples[0]) > Maths::Epsilon) return false;
				if (Maths::FAbs(m_fSamples[1]) > Maths::Epsilon) return false;
				if (Maths::FAbs(m_fSamples[2]) > Maths::Epsilon) return false;

				return true;
			}

			RGBSpectrum& operator=(float p_fValue)
			{
				m_fSamples[0] = m_fSamples[1] = m_fSamples[2] = p_fValue;

				return *this;
			}

			RGBSpectrum& operator=(const RGBSpectrum &p_spectrum)
			{
				m_fSamples[0] = p_spectrum.m_fSamples[0];
				m_fSamples[1] = p_spectrum.m_fSamples[1];
				m_fSamples[2] = p_spectrum.m_fSamples[2];

				return *this;
			}

			RGBSpectrum operator*(float p_fScale) const {
				return RGBSpectrum(m_fSamples[0] * p_fScale, m_fSamples[1] * p_fScale, m_fSamples[2] * p_fScale);
			}

			RGBSpectrum operator/(float p_fScale) const {
				//BOOST_ASSERT(p_fScale > 0);
				return *this * (1.0f / p_fScale);
			}

			RGBSpectrum operator/(int p_nScale) const {
				return RGBSpectrum(m_fSamples[0] / p_nScale, m_fSamples[1] / p_nScale, m_fSamples[2] / p_nScale);
			}

			RGBSpectrum operator/(const RGBSpectrum &p_spectrum) const
			{
				return RGBSpectrum(m_fSamples[0] / p_spectrum.m_fSamples[0], m_fSamples[1] / p_spectrum.m_fSamples[1], m_fSamples[2] / p_spectrum.m_fSamples[2]);
			}

			RGBSpectrum operator*(const RGBSpectrum &p_spectrum) const 
			{
				return RGBSpectrum(m_fSamples[0] * p_spectrum.m_fSamples[0], m_fSamples[1] * p_spectrum.m_fSamples[1], m_fSamples[2] * p_spectrum.m_fSamples[2]);
			}

			RGBSpectrum operator+(const RGBSpectrum &p_spectrum) const 
			{
				return RGBSpectrum(m_fSamples[0] + p_spectrum.m_fSamples[0], m_fSamples[1] + p_spectrum.m_fSamples[1], m_fSamples[2] + p_spectrum.m_fSamples[2]);
			}

			RGBSpectrum operator-(const RGBSpectrum &p_spectrum) const 
			{
				return RGBSpectrum(m_fSamples[0] - p_spectrum.m_fSamples[0], m_fSamples[1] - p_spectrum.m_fSamples[1], m_fSamples[2] - p_spectrum.m_fSamples[2]);
			}

			void Set(float *p_fValueArray) 
			{
				m_fSamples[0] = p_fValueArray[0];
				m_fSamples[1] = p_fValueArray[1];
				m_fSamples[2] = p_fValueArray[2];
			}

			void Set(float p_fRed, float p_fGreen, float p_fBlue)
			{
				m_fSamples[0] = p_fRed;
				m_fSamples[1] = p_fGreen;
				m_fSamples[2] = p_fBlue;
			}

			void Normalize(void)
			{
				float length = 0;

				length =  m_fSamples[0]*m_fSamples[0] + m_fSamples[1]*m_fSamples[1] + m_fSamples[2]*m_fSamples[2];

				if (length > 0.0f)
				{
					float invLength = 1.0f / Maths::Sqrt(length);

					m_fSamples[0] *= invLength;
					m_fSamples[1] *= invLength;
					m_fSamples[2] *= invLength;
				}
			}

			void Clamp(float p_fMin = 0.0f, float p_fMax = 1.0f)
			{
				if (m_fSamples[0] < p_fMin) m_fSamples[0] = p_fMin;
				else if (m_fSamples[0] > p_fMax) m_fSamples[0] = p_fMax;

				if (m_fSamples[1] < p_fMin) m_fSamples[1] = p_fMin;
				else if (m_fSamples[1] > p_fMax) m_fSamples[1] = p_fMax;

				if (m_fSamples[2] < p_fMin) m_fSamples[2] = p_fMin;
				else if (m_fSamples[2] > p_fMax) m_fSamples[2] = p_fMax;
			}

			void Max(const RGBSpectrum &p_spectrum1, const RGBSpectrum &p_spectrum2)
			{
				m_fSamples[0] = Maths::Max(p_spectrum1.m_fSamples[0], p_spectrum2.m_fSamples[0]);
				m_fSamples[1] = Maths::Max(p_spectrum1.m_fSamples[1], p_spectrum2.m_fSamples[1]);
				m_fSamples[2] = Maths::Max(p_spectrum1.m_fSamples[2], p_spectrum2.m_fSamples[2]);
			}

			void Min(const RGBSpectrum &p_spectrum1, const RGBSpectrum &p_spectrum2)
			{
				m_fSamples[0] = Maths::Min(p_spectrum1.m_fSamples[0], p_spectrum2.m_fSamples[0]);
				m_fSamples[1] = Maths::Min(p_spectrum1.m_fSamples[1], p_spectrum2.m_fSamples[1]);
				m_fSamples[2] = Maths::Min(p_spectrum1.m_fSamples[2], p_spectrum2.m_fSamples[2]);
			}

			std::string ToString(void) const
			{
				std::string strOut = boost::str(boost::format("[%d %d %d]") % m_fSamples[0] % m_fSamples[1] % m_fSamples[2]);
				return strOut;
			}
		};
	}
}