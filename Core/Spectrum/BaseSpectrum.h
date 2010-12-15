#pragma once

namespace Illumina
{
	namespace Core
	{
		// In the future I might wish to extend this 
		// in a way not unlike PBRT's, so I'll keep
		// the base class and derive an RGBSpectrum 
		// from it.
		template<int nSampleCount> 
		class BaseSpectrum
		{
			float operator[](int p_nIndex) const { return m_fSamples[p_nIndex]; }
			float& operator[](int p_nIndex) { return m_fSamples[p_nIndex]; }

			int GetSampleCount(void) const { return nSampleCount; }

			virtual BaseSpectrum& operator+=(const BaseSpectrum &p_spectrum) {
				return *this = *this + p_spectrum;
			}

			virtual BaseSpectrum& operator-=(const BaseSpectrum &p_spectrum) {
				return *this = *this - p_spectrum;
			}

			virtual BaseSpectrum& operator*=(const BaseSpectrum &p_spectrum) {
				return *this = *this * p_spectrum;
			}

			virtual BaseSpectrum& operator/=(const BaseSpectrum &p_spectrum) {
				return *this = *this / p_spectrum;
			}

			virtual BaseSpectrum& operator*=(float p_fScale) {
				return *this = *this * p_fScale;
			}

			virtual BaseSpectrum& operator/=(float p_fScale) {
				return *this = *this / p_fScale;
			}

			virtual BaseSpectrum operator*(float p_fScale) const = 0;
			virtual BaseSpectrum operator/(float p_fScale) const = 0;
			virtual BaseSpectrum operator/(const BaseSpectrum &p_spectrum) const = 0;
			virtual BaseSpectrum operator*(const BaseSpectrum &p_spectrum) const = 0;
			virtual BaseSpectrum operator+(const BaseSpectrum &p_spectrum) const = 0;
			virtual BaseSpectrum operator-(const BaseSpectrum &p_spectrum) const = 0;

			virtual BaseSpectrum& operator=(const BaseSpectrum &p_spectrum) = 0;
			
			virtual void Set(float *p_fValueArray) = 0;

			virtual void Normalize(void) = 0;
			virtual void Clamp(float p_fMin = 0.0f, float p_fMax = 1.0f) = 0;

			virtual std::string ToString(void) = 0;

		protected:
			float m_fSamples[nSampleCount];
		};
	}
}