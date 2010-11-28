//----------------------------------------------------------------------------------------------
//	Filename:	Queue.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/thread/tss.hpp>

#include "Threading/Atomic.h"
#include "Threading/AtomicReference.h"

#include "Exception/Exception.h"

namespace Illumina
{
	namespace Core
	{
		ALIGN_16 struct QueueNode
		{
			static AtomicInt32 NodeInstanceCount;

			AtomicStampedReference<QueueNode> Next;
			void *Item;

			QueueNode(void)
				: Next(0, NULL)
				, Item(NULL)
			{ NodeInstanceCount++; }

			QueueNode(void *p_pItem, QueueNode *p_pNext = NULL)
				: Next(0, p_pNext)
				, Item(p_pItem)
			{ NodeInstanceCount++; }

			~QueueNode()
			{ NodeInstanceCount--;}
		};

		AtomicInt32 QueueNode::NodeInstanceCount(0);

		class Queue
		{
			AtomicStampedReference<QueueNode> m_head;
			AtomicStampedReference<QueueNode> m_tail;

			boost::thread_specific_ptr< std::vector<QueueNode*> > m_freeNodeList;

		protected:
			QueueNode* AllocateNode(void)
			{
				QueueNode* pNode = NULL;

				// Initialise thread-local free node list
				if (m_freeNodeList.get() == NULL)
				{
					m_freeNodeList.reset(new std::vector<QueueNode*>);
					// std::cout<<"Created Free Node List for Thread : " << GetCurrentThreadId() << " at " << m_freeNodeList.get() <<std::endl;
				}

				// Do we have any node in the list?
				if (m_freeNodeList.get()->size() > 0)
				{
					pNode = m_freeNodeList.get()->back();
					m_freeNodeList.get()->pop_back();
				}
				else
					pNode = new QueueNode();

				return pNode;
			}

			void FreeNode(QueueNode* p_pNode)
			{
				// Initialise thread-local free node list
				if (m_freeNodeList.get() == NULL)
				{
					m_freeNodeList.reset(new std::vector<QueueNode*>);
					//std::cout<<"Created Free Node List for Thread : " << GetCurrentThreadId() << " at " << m_freeNodeList.get() <<std::endl;
				}

				m_freeNodeList.get()->push_back(p_pNode);
			}

		public:
			Queue(void)
			{
				QueueNode *pNode = new QueueNode(NULL);
				pNode->Next.Set(NULL, 0);
				m_head.Set(pNode, 0);
				m_tail.Set(pNode, 0);
			}

			~Queue(void)
			{
				delete m_head.GetReference();
			}

			void Enqueue(void *p_object)
			{
				#if defined(__ARCHITECTURE_X64__)
					Int64 tailStamp, nextStamp;
				#else
					Int32 tailStamp, nextStamp;
				#endif

				QueueNode *pNode = AllocateNode();
				pNode->Next.Set(NULL, 0);
				pNode->Item = p_object;

				while (true)
				{
					QueueNode *pTail = m_tail.Get(&tailStamp),
						*pNext = pTail->Next.Get(&nextStamp);

					// Check if the tail has changed
					if (pTail == m_tail.GetReference() && tailStamp == m_tail.GetStamp())
					{
						// If the tail hasn't changed and it points to no node, it's the tail
						if (pNext == NULL)
						{
							// Make last.next to point to the newly created node
							if (pTail->Next.CompareAndSet(pNext, pNode, nextStamp, nextStamp + 1))
							{
								// Update the tail to point to the newly created node too
								m_tail.CompareAndSet(pTail, pNode, tailStamp, tailStamp + 1);
								return;
							}
						}
						else
						{
							// Help other thread completing the switch such that tail = tail->next
							m_tail.CompareAndSet(pTail, pNext, tailStamp, tailStamp + 1);
						}
					}
				}
			}

			void* Dequeue(void)
			{
				QueueNode *pHead, *pTail, *pNext;

				#if defined(__ARCHITECTURE_X64__)
					Int64 headStamp, tailStamp, nextStamp;
				#else
					Int32 headStamp, tailStamp, nextStamp;
				#endif

				while(true)
				{
					pHead = m_head.Get(&headStamp);
					pTail = m_tail.Get(&tailStamp);
					pNext = pHead->Next.Get(&nextStamp);

					// Check if pFirst has changed - if so, pNext might be invalid
					if (pHead == m_head.GetReference() && headStamp == m_head.GetStamp())
					{
						// Possibly queue is empty
						if (pHead == pTail)
						{
							// Queue is empty
							if (pNext == NULL) return NULL;

							// Queue is not empty - there is a pending operation, which we help to complete
							m_tail.CompareAndSet(pTail, pNext, tailStamp, tailStamp + 1);
						}
						else
						{
							// Retrieve item from node
							void *pItem = pNext->Item;

							// Swap out head node with next node
							if (m_head.CompareAndSet(pHead, pNext, headStamp, headStamp + 1))
							{
								FreeNode(pHead);
								return pItem;
							}
						}
					}
				}
			}
		};
	}
}
