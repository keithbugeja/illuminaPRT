#pragma once

#include "lz4.h"

namespace Illumina
{
	namespace Core
	{
		class Compressor
		{
		public:
			static size_t Compress(char *p_pBuffer, size_t p_nBufferSize, char *p_pOutputBuffer)
			{
				return (size_t)LZ4_compress(p_pBuffer, p_pOutputBuffer, p_nBufferSize);
			}

			static size_t Decompress(char *p_pBuffer, size_t p_nOriginalSize, char *p_pOutputBuffer)
			{				
				return (size_t)LZ4_uncompress(p_pBuffer, p_pOutputBuffer, p_nOriginalSize);
			}
		};
	}
}