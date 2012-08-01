//----------------------------------------------------------------------------------------------
//	Filename:	Singleton.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

// Disable DLL export warnings
#pragma warning (disable:4251)

namespace Illumina
{
	namespace Core
	{
		template <typename TClass> 
		class TSingleton
		{
		protected:
			static TClass* m_pInstance;

			TSingleton() { }
			virtual ~TSingleton() { };

		private:
			TSingleton(const TSingleton& p_tSingleton) { };

		public:
			//----------------------------------------------------------------------------------------------
			/// Return a pointer to the singleton instance
			//----------------------------------------------------------------------------------------------
			static TClass* GetInstance(void)
			{
				if (m_pInstance == NULL) 
					m_pInstance = new TClass();

				BOOST_ASSERT(m_pInstance != NULL);
				return m_pInstance;
			}

			//----------------------------------------------------------------------------------------------
			/// Delete singleton instance
			//----------------------------------------------------------------------------------------------
			static void Delete(void)
			{
				if (m_pInstance != NULL)
				{
					delete m_pInstance;
					m_pInstance = NULL;
				}
			}
		};

		template <typename TClass> TClass* TSingleton<TClass>::m_pInstance = NULL;
	}
}