//----------------------------------------------------------------------------------------------
//	Filename:	Material.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once
#include <map>
#include "System/IlluminaPRT.h"
#include "Material/BSDF.h"

namespace Illumina
{
	namespace Core
	{
		class IMaterial 
			: public BSDF
		{ 
		protected:
			using BSDF::m_bxdfList;

		public:
			IMaterial(void) : BSDF() { }
			IMaterial(const std::string &p_strId) : BSDF(p_strId) { }

			virtual bool IsComposite(void) const { return false; }
		};
	}
}