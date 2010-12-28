//----------------------------------------------------------------------------------------------
//	Filename:	Material.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once
#include <map>

#include "Material/Material.h"

namespace Illumina
{
	namespace Core
	{
		class MaterialGroup
			: public IMaterial
		{
		protected:
			List<IMaterial*> m_materialList;
			std::map<int, IMaterial*> m_materialGroupMap;
			std::map<std::string, IMaterial*> m_materialNameMap;
			std::map<std::string, int> m_materialNameGroupMap;
		
		public:
			MaterialGroup(void) : IMaterial() { }
			MaterialGroup(const std::string &p_strName) : IMaterial(p_strName) { }

			bool IsComposite(void) const { return true; }

			int Size(void) const { 
				return m_materialList.Size(); 
			}
			
			IMaterial* GetByIndex(int p_nIndex) const { 
				return m_materialList.At(p_nIndex); 
			}

			IMaterial* GetByGroupId(int p_nGroupId) 
			{
				return (m_materialGroupMap.find(p_nGroupId) != m_materialGroupMap.end()) 
					? m_materialGroupMap[p_nGroupId]
					: NULL; 
			}
			
			IMaterial* GetByName(const std::string &p_strName)
			{ 
				return (m_materialNameMap.find(p_strName) != m_materialNameMap.end()) 
					? m_materialNameMap[p_strName]
					: NULL; 
			}

			int GetGroupId(const std::string &p_strName)
			{
				return (m_materialNameGroupMap.find(p_strName) != m_materialNameGroupMap.end()) 
					? m_materialNameGroupMap[p_strName]
					: NULL; 
			}

			void Add(IMaterial* p_pMaterial, int p_nGroupId) 
			{ 
				m_materialList.PushBack(p_pMaterial);
				m_materialGroupMap[p_nGroupId] = p_pMaterial;
				m_materialNameMap[p_pMaterial->GetName()] = p_pMaterial;
				m_materialNameGroupMap[p_pMaterial->GetName()] = p_nGroupId;
			}
		};
	}
}