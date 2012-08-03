//----------------------------------------------------------------------------------------------
//	Filename:	Resource.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

class Resource
{
public:
	enum Type 
	{
		Idle,
		Worker,
		Coordinator
	};

protected:
	int m_nResourceID;
	Type m_resourceType;

public:
	Resource(int p_nResourceID, Type p_resourceType);
		/*: m_resourceType(p_resourceType)
		, m_nResourceID(p_nResourceID) 
	{ } */

	int GetID(void) const;// { return m_nResourceID; }
	bool IsIdle(void); //{ return m_resourceType == Idle; }
	bool IsWorker(void); //{ return m_resourceType == Worker; }
	bool IsCoordinator(void);// { return m_resourceType == Coordinator; }
};