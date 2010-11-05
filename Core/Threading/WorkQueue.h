//----------------------------------------------------------------------------------------------
//	Filename:	WorkQueue.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include "System/Platform.h"

namespace Illumina {
	namespace Core 
	{
		struct WorkQueueItem
		{
			WorkQueueItem *m_pNext, 
				*m_pPrevious;

			int m_nPriority;
			void *p_pTask;
		};

		class WorkQueue
		{
			WorkQueueItem *m_pHead,
				*m_pTail;

			WorkQueue(void) 
				: m_pHead(NULL)
				, m_pTail(NULL)
			{ }

			void Enqueue(WorkQueueItem* p_pItem)
			{
				if (m_pHead == NULL)
				{
					m_pHead = m_pTail = p_pItem;
					p_pItem->m_pPrevious = NULL;
					p_pItem->m_pNext = NULL;
				} else {
					WorkQueueItem *pPrevTail = m_pTail;
					m_pTail = p_pItem;
					
					p_pItem->m_pPrevious = pPrevTail;
					p_pItem->m_pNext = NULL;

					pPrevTail->m_pNext = p_pItem;
				}
			}

			WorkQueueItem* Dequeue(void)
			{
				if (m_pHead == NULL || m_pTail == NULL)
					return NULL;

				WorkQueueItem *pReturn = m_pTail;
				m_pTail = m_pTail->m_pPrevious;
				
				if (m_pTail == NULL)
				{
					m_pHead == NULL;
					return pReturn;
				}

				m_pTail->m_pNext = NULL;
			}
		};
	}
}
