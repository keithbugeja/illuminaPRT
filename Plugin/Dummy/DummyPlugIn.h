//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//	Author			Date		Version		Description
//----------------------------------------------------------------------------------------------
//	Keith Bugeja	27/09/2007	1.0.0		First version
//----------------------------------------------------------------------------------------------
#pragma once

#include <iostream>

#include "System/Dummy.h"
#include "System/PlugIn.h"
#include "System/Factory.h"
#include "System/EngineKernel.h"

#include "System/DummyManager.h"

namespace Illumina
{
	namespace PlugIn
	{
		//----------------------------------------------------------------------------------------------
		// Implementation of IDummy
		//----------------------------------------------------------------------------------------------
		class Dummy : public Illumina::Core::IDummy
		{
		public:
			std::string GetName(void)
			{
				return std::string("I am dummy!");
			}
		};

		//----------------------------------------------------------------------------------------------
		// Implementation of Factory<IDummy>
		//----------------------------------------------------------------------------------------------
		class DummyFactory : public Illumina::Core::Factory<Illumina::Core::IDummy>
		{
		public:
			Illumina::Core::IDummy* CreateInstance(void)
			{
				return new Dummy();
			}

			Illumina::Core::IDummy* CreateInstance(ArgumentMap &p_argumentMap)
			{
				return new Dummy();
			}
		};

		//----------------------------------------------------------------------------------------------
		// Dummy Plug In (registers a DummyFactory to the EngineKernel's DummyFactoryManager)
		//----------------------------------------------------------------------------------------------
		class DummyPlugIn : public Illumina::Core::IPlugIn
		{
		public:
			DummyPlugIn()
				: m_pDummyFactory(NULL)
			{ }

			~DummyPlugIn()
			{
				if (m_pDummyFactory)
					delete m_pDummyFactory;
			}

			const std::string& GetName(void) const
			{
				return m_strName;
			}

			const Illumina::Core::Version& GetVersion(void) const
			{
				return m_version;
			}

			void Register(Illumina::Core::EngineKernel* p_pEngineKernel)
			{
				std::cout << "Registering Dummy Factory" << std::endl;

				// NOT THREAD-SAFE!
				if (m_pDummyFactory == NULL)
					m_pDummyFactory = new DummyFactory();

				p_pEngineKernel->GetDummyManager()->RegisterFactory("DummyFactory", m_pDummyFactory);
			}

			void Unregister(Illumina::Core::EngineKernel* p_pEngineKernel)
			{
				std::cout << "Unregistering Dummy Factory" << std::endl;

				p_pEngineKernel->GetDummyManager()->UnregisterFactory("DummyFactory");

				if (m_pDummyFactory)
					delete m_pDummyFactory;

				m_pDummyFactory = NULL;
			}

			void Initialise(void)
			{ }

			void Shutdown(void)
			{ }

		private:
			std::string	m_strName;
			Illumina::Core::Version	m_version;

			DummyFactory* m_pDummyFactory;		
		};
	}
}