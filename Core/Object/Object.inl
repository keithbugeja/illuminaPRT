//----------------------------------------------------------------------------------------------
//	Filename:	Object.inl
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
inline const UniqueID& Object::GetID(void) const {
	return m_uid;
}
//----------------------------------------------------------------------------------------------
inline const std::string& Object::GetName(void) const {
	return m_strName;
}
//----------------------------------------------------------------------------------------------
inline void Object::Initialise(UniqueIDType p_uidType)
{
	switch (p_uidType)
	{
		case Illumina::Core::Transient:
			m_uid = UniqueIDFactory<TransientUID>::GetUniqueID();
			break;

        case Illumina::Core::Persistent:
            // #pragma message("Persistent unique-id system not implemented")
            break;
	}
}
//----------------------------------------------------------------------------------------------
