//----------------------------------------------------------------------------------------------
//	Filename:	Version.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "Version.h"

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
/// Constructs a new version object.
//----------------------------------------------------------------------------------------------
Version::Version(void) 
	: m_nMajor(0)
	, m_nMinor(0)
	, m_nBuild(0) 
{ }

//----------------------------------------------------------------------------------------------
/// Constructs a new version object using Major, Minor and Build values.
/// \param p_nMajor Major version number
/// \param p_nMinor Minor version number
/// \param p_nBuild Build number
//----------------------------------------------------------------------------------------------
Version::Version(int p_nMajor, int p_nMinor, int p_nBuild) 
	: m_nMajor(p_nMajor)
	, m_nMinor(p_nMinor)
	, m_nBuild(p_nBuild) 
{ }

//----------------------------------------------------------------------------------------------
/// Copy constructor for version object.
/// \param p_version Reference to version object
//----------------------------------------------------------------------------------------------
Version::Version(const Version& p_version)
{
	*this = p_version;
}
	
//----------------------------------------------------------------------------------------------
/// Sets major, minor and build versions
/// \param p_nMajor Major version number
/// \param p_nMinor Minor version number
/// \param p_nBuild Build number
//----------------------------------------------------------------------------------------------
void Version::Set(int p_nMajor, int p_nMinor, int p_nBuild )
{
	m_nMajor = p_nMajor;
	m_nMinor = p_nMinor;
	m_nBuild = p_nBuild;
}

//----------------------------------------------------------------------------------------------
/// Returns major version number
/// \return Major version number
//----------------------------------------------------------------------------------------------
int Version::GetMajor(void) const
{
	return m_nMajor;
}

//----------------------------------------------------------------------------------------------
/// Returns minor version number
/// \return Minor version number
//----------------------------------------------------------------------------------------------
int Version::GetMinor(void) const
{
	return m_nMinor;
}

//----------------------------------------------------------------------------------------------
/// Returns build number
/// \return Build number
//----------------------------------------------------------------------------------------------
int Version::GetBuild(void) const
{
	return m_nBuild;
}

//----------------------------------------------------------------------------------------------
/// Sets major version number
/// \param p_nMajor Major version number
//----------------------------------------------------------------------------------------------
void Version::SetMajor(int p_nMajor)
{
	m_nMajor = p_nMajor;
}

//----------------------------------------------------------------------------------------------
/// Sets minor version number
/// \param p_nMinor Minor version number
//----------------------------------------------------------------------------------------------
void Version::SetMinor(int p_nMinor)
{
	m_nMinor = p_nMinor;
}

//----------------------------------------------------------------------------------------------
/// Sets build number
/// \param p_nBuild Build number
//----------------------------------------------------------------------------------------------
void Version::SetBuild(int p_nBuild)
{
	m_nBuild = p_nBuild;
}

//----------------------------------------------------------------------------------------------
/// Assignment operator
/// \param p_version Reference to Version object
/// \return Reference to Version object
//----------------------------------------------------------------------------------------------
Version& Version::operator=(const Version& p_version)
{
	m_nMajor = p_version.m_nMajor;
	m_nMinor = p_version.m_nMinor;
	m_nBuild = p_version.m_nBuild;

	return *this;
}

//----------------------------------------------------------------------------------------------
/// Equality operator
/// \param p_version Reference to Version object
/// \return True if equal, false otherwise
//----------------------------------------------------------------------------------------------
bool Version::operator==(const Version& p_version)
{
	return m_nMajor == p_version.m_nMajor && 
		   m_nMinor == p_version.m_nMinor && 
		   m_nBuild == p_version.m_nBuild;
}

//----------------------------------------------------------------------------------------------
/// Inequality operator
/// \param p_version Reference to Version object
/// \return False if equal, true otherwise
//----------------------------------------------------------------------------------------------
bool Version::operator!=(const Version& p_version)
{
	return !(*this == p_version);
}

//----------------------------------------------------------------------------------------------
/// Comparison operator
/// \param p_version Reference to Version object
/// \return True if greater, false otherwise
//----------------------------------------------------------------------------------------------
bool Version::operator>(const Version& p_version)
{
	if (m_nMajor != p_version.m_nMajor)
		return (m_nMajor > p_version.m_nMajor);

	if (m_nMinor != p_version.m_nMinor )
		return (m_nMinor > p_version.m_nMinor);

	return (m_nBuild > p_version.m_nBuild);
}

//----------------------------------------------------------------------------------------------
/// Comparison operator
/// \param p_version Reference to Version object
/// \return True if smaller, false otherwise
//----------------------------------------------------------------------------------------------
bool Version::operator<(const Version& p_version)
{
	if (m_nMajor != p_version.m_nMajor)
		return (m_nMajor < p_version.m_nMajor);

	if (m_nMinor != p_version.m_nMinor)
		return (m_nMinor < p_version.m_nMinor);

	return (m_nBuild < p_version.m_nBuild);
}

//----------------------------------------------------------------------------------------------
/// Comparison operator
/// \param p_version Reference to Version object
/// \return True if less or equal, false otherwise
//----------------------------------------------------------------------------------------------
bool Version::operator<=(const Version& p_version)
{
	return (*this==p_version || *this<p_version);
}

//----------------------------------------------------------------------------------------------
/// Comparison operator
/// \param p_version Reference to Version object
/// \return True if greater or equal, false otherwise
//----------------------------------------------------------------------------------------------
bool Version::operator>=(const Version& p_version)
{
	return (*this==p_version || *this>p_version);
}