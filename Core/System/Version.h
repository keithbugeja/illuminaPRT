//----------------------------------------------------------------------------------------------
//	Filename:	Version.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

namespace Illumina
{
	namespace Core
	{
		class Version
		{
		private:
			int	m_nMajor, 
				m_nMinor,
				m_nBuild;
		public:
			Version(void);
			Version(const Version& p_version);
			Version(int p_nMajor, int p_nMinor, int p_nBuild);
	
			void Set(int p_nMajor, int p_nMinor, int p_nBuild);

			int GetMajor(void) const;
			int GetMinor(void) const;
			int GetBuild(void) const;

			void SetMajor(int p_nMajor);
			void SetMinor(int p_nMinor);
			void SetBuild(int p_nBuild);

			Version& operator=(const Version& p_version);
			bool operator==(const Version& p_version);
			bool operator!=(const Version& p_version);
			bool operator>(const Version& p_version);
			bool operator<(const Version& p_version);
			bool operator<=(const Version& p_version);
			bool operator>=(const Version& p_version);
		};
	}
}