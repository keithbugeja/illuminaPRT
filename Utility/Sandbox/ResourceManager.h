//----------------------------------------------------------------------------------------------
//	Filename:	ResourceManager.h
//	Author:		Keith Bugeja
//	Date:		27/07/2012
//----------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------
#include "UniqueId.h"
#include "Controller.h"

//----------------------------------------------------------------------------------------------
class ResourceManager
{
protected:
	UniqueID m_uniqueID;

	std::vector<IResourceController*> m_controllerList;
	std::map<int, IResourceController*> m_controllerMap;

public:
	enum ResourceType
	{
		Resource,
		Master
	};

public:
	ResourceManager(void);
	~ResourceManager(void);

	ResourceType WhatAmI(void);

	template<class T> T* CreateInstance(void);
	void DestroyInstance(IResourceController *p_pController);

	void Initialise(void);
	void Shutdown(void);
};