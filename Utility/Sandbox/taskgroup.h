#pragma once

#include "task.h"

namespace Illumina
{
	namespace Core
	{
		class TaskGroup
		{
		public:
			std::vector<Task*> TaskList;

		protected:
			int m_masterRank,
				m_coordinatorRank;

		public:
			/* 
			 * Group properties 
			 */
			void SetMasterRank(int p_nMasterRank) 
			{
				BOOST_ASSERT(p_nMasterRank >= 0 && p_nMasterRank < TaskList.size());
				m_masterRank = p_nMasterRank;
			}

			int GetMasterRank(void) const
			{
				return m_masterRank;
			}

			void SetCoordinatorRank(int p_nCoordinatorRank)
			{
				BOOST_ASSERT(p_nCoordinatorRank >= 0 && p_nCoordinatorRank < TaskList.size());
				m_coordinatorRank = p_nCoordinatorRank;
			}

			int GetCoordinatorRank(void) const { return m_coordinatorRank; }

			int Size(void) const { return TaskList.size(); }

			/*
			 * Rank-Task resolution
			 */
			Task *FindTask(int p_nRank)
			{
				for (std::vector<Task*>::iterator taskIterator = TaskList.begin();
					 taskIterator != TaskList.end(); ++taskIterator)
				{
					 if ((*taskIterator)->GetWorkerRank() == p_nRank) 
						 return *taskIterator;
				}

				return NULL;
			}

			/*
			 * Group communication
			 */
			void Broadcast(Task *p_sender, const Message &p_message)
			{
				std::cout << "[" << p_sender->Rank << "] :: Broadcast " << p_message.ToString() << std::endl;

				for (std::vector<Task*>::iterator taskIterator = TaskList.begin(); 
					 taskIterator != TaskList.end(); taskIterator++)
				{
					if (p_sender == *taskIterator) continue;
					p_sender->Send(*taskIterator, p_message);
				}
			}

			void Barrier(void)
			{
			}

			/*
			 * Group operations
			 */
			void CreateSubGroup(TaskGroup *p_group, int p_startRank, int p_endRank)
			{
				BOOST_ASSERT(p_group != NULL);

				for (int index = p_startRank; index <= p_endRank; index++)
					p_group->TaskList.push_back(TaskList[index]);

				p_group->SetMasterRank(this->GetMasterRank());
				p_group->SetCoordinatorRank(p_startRank);
			}

			void Split(TaskGroup *p_group, int p_startRank, int p_endRank)
			{
				BOOST_ASSERT(p_group != NULL);

				for (int index = p_startRank; index <= p_endRank; index++)
				{
					std::cout << "Pushing task[" << index << "], with rank = " << TaskList[index]->Rank << std::endl;
					p_group->TaskList.push_back(TaskList[index]);
				}

				// Remove tasks from original group
				TaskList.erase(TaskList.begin() + p_startRank, TaskList.begin() + p_endRank + 1);

				p_group->SetMasterRank(this->GetMasterRank());
				p_group->SetCoordinatorRank(p_startRank);
			}
		};
	}
}