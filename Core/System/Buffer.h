//----------------------------------------------------------------------------------------------
//	Filename:	Buffer.h
//	Author:		Keith Bugeja
//	Date:		11/02/2012
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/shared_ptr.hpp>

#include "System/Platform.h"
#include "Threading/Spinlock.h"
#include "Threading/Atomic.h"
#include "External/Compression/Compression.h"

namespace Illumina 
{
	namespace Core
	{
		template<class T> 
		class Buffer2D
		{
		protected:
			size_t m_width, 
				m_height,
				m_area;
		
			bool m_ownBuffer;

			T* m_buffer;

		protected:
			FORCEINLINE size_t IndexOf(int p_nX, int p_nY) {
				return p_nY * m_width + p_nX;
			}

		public:	
			Buffer2D(int p_nWidth, int p_nHeight)
				: m_width(p_nWidth)
				, m_height(p_nHeight)
				, m_area(p_nWidth * p_nHeight)
				, m_ownBuffer(true)
			{
				m_buffer = (T*)AlignedMalloc(sizeof(T) * m_area, 16);
				BOOST_ASSERT(m_buffer != NULL);
			}

			Buffer2D(int p_nWidth, int p_nHeight, T* p_pBuffer)
				: m_width(p_nWidth)
				, m_height(p_nHeight)
				, m_area(p_nWidth * p_nHeight)
				, m_ownBuffer(false)
				, m_buffer(p_pBuffer)
			{
				BOOST_ASSERT(m_buffer != NULL);
			}

			~Buffer2D(void) 
			{
				if (m_ownBuffer)
					Safe_AlignedFree(m_buffer);
			}

			inline size_t GetWidth(void) const {
				return m_width;
			}

			inline size_t GetHeight(void) const {
				return m_height;
			}

			inline size_t GetArea(void) const {
				return m_area;
			}

			inline T* GetP(int p_nX, int p_nY) {
				return m_buffer + IndexOf(p_nX, p_nY);
			}

			inline T& Get(int p_nX, int p_nY) {
				return m_buffer[IndexOf(p_nX, p_nY)];
			}

			inline void Set(int p_nX, int p_nY, const T &p_element) {
				m_buffer[IndexOf(p_nX, p_nY)] = p_element;
			}

			inline T* GetBuffer(void) {
				return m_buffer;
			}
					
			virtual void Clear(void)
			{
				memset(m_buffer, 0, sizeof(T) * m_area);
			}

			virtual size_t Compress(T *p_pDestination) {
				return Compressor::Compress((char*)m_buffer, sizeof(T) * m_area, (char*)p_pDestination);
			}

			virtual void Decompress(T *p_pSource) {
				Compressor::Decompress((char*)p_pSource, sizeof(T) * m_area, (char*)m_buffer);
			}
		};
	}
}