//----------------------------------------------------------------------------------------------
//	Filename:	Material.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/IlluminaPRT.h"
#include "System/FactoryManager.h"

#include "Material/BSDF.h"

//----------------------------------------------------------------------------------------------
namespace Illumina
{
	namespace Core
	{		
		//----------------------------------------------------------------------------------------------
		// IMaterial : Abstract base class system materials. 
		//----------------------------------------------------------------------------------------------
		class IMaterial 
			: public BSDF
		{ 
		protected:
			using BSDF::m_bxdfList;

		public:
			IMaterial(void) : BSDF() { }
			IMaterial(const std::string &p_strName) : BSDF(p_strName) { }

			virtual bool IsComposite(void) const { return false; }
		};

		//----------------------------------------------------------------------------------------------
		// MaterialManager : All Material factories must register with object.
		//----------------------------------------------------------------------------------------------
		typedef FactoryManager<IMaterial> MaterialManager;
	}
}