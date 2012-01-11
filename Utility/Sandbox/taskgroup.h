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
			int m_id;

			int m_masterRank,
				m_coordinatorRank;

			int m_cachedCoordinatorRank;
			Task *m_cachedCoordinatorTask;

		public:
			TaskGroup(int p_id = -1, int p_masterRank = -1, int p_coordinatorRank = -1)
				: m_id(p_id)
				, m_masterRank(p_masterRank)
				, m_coordinatorRank(p_coordinatorRank)
				, m_cachedCoordinatorRank(-1)
				, m_cachedCoordinatorTask(NULL)
			{ }

			/*
			 * Group UID
			 */
			void SetId(int p_nId) { m_id = p_nId; }
			int GetId(void) const { return m_id; }

			/* 
			 * Group properties 
			 */
			void SetMasterRank(int p_nMasterRank) 
			{
				// BOOST_ASSERT(p_nMasterRank >= 0 && p_nMasterRank < TaskList.size());
				m_masterRank = p_nMasterRank;
			}

			int GetMasterRank(void) const
			{
				return m_masterRank;
			}

			void SetCoordinatorRank(int p_nCoordinatorRank)
			{
				// BOOST_ASSERT(p_nCoordinatorRank >= 0 && p_nCoordinatorRank < TaskList.size());
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

			Task *GetMasterTask(void)
			{
				return FindTask(m_masterRank);
			}

			Task *GetCoordinatorTask(void) 
			{
				if (m_cachedCoordinatorTask == NULL ||
					m_cachedCoordinatorRank != m_coordinatorRank)
				{
					m_cachedCoordinatorRank = m_coordinatorRank;
					m_cachedCoordinatorTask = FindTask(m_cachedCoordinatorRank);
				}

				return m_cachedCoordinatorTask;
			}

			/*
			 * Group communication
			 */
			void Broadcast(Task *p_sender, const Message &p_message)
			{
				ControlCommunicator communicator(p_sender);

				// std::cout << "[" << p_sender->GetWorkerRank() << "] :: Broadcast " << p_message.ToString() << std::endl;

				for (std::vector<Task*>::iterator taskIterator = TaskList.begin(); 
					 taskIterator != TaskList.end(); taskIterator++)
				{
					if (p_sender == *taskIterator) 
						continue;

					communicator.Send(p_message, (*taskIterator)->GetRank());
				}
			}

			void Barrier(void)
			{
			}

			/*
			 * Group operations
			 */
			void Merge(Task *p_task)
			{
				BOOST_ASSERT(p_task != NULL);

				if (std::find(TaskList.begin(), TaskList.end(), p_task) != TaskList.end())
					return;

				TaskList.push_back(p_task);
			}

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
					p_group->TaskList.push_back(TaskList[index]);

				// Remove tasks from original group
				TaskList.erase(TaskList.begin() + p_startRank, TaskList.begin() + p_endRank + 1);

				p_group->SetMasterRank(this->GetMasterRank());
				p_group->SetCoordinatorRank(p_startRank);
			}

			std::string ToString(void) const
			{
				std::string result;

				for (std::vector<Task*>::const_iterator taskIterator = TaskList.begin(); 
					 taskIterator != TaskList.end(); ++taskIterator)
				{
					result = result + (*taskIterator)->ToString() + "\n";
				}

				return result;
			}
		};

		class TaskGroupList
		{
		protected:
			std::vector<TaskGroup*> m_taskGroupList;
			std::map<int, TaskGroup*> m_taskGroupMap;

		protected:
			bool ContainsKey(int p_id)
			{
				return (m_taskGroupMap.find(p_id) != m_taskGroupMap.end());
			}

			bool ContainsKey(int p_id, TaskGroup **p_taskGroup)
			{
				std::map<int, TaskGroup*>::iterator value;

				if ((value = m_taskGroupMap.find(p_id)) != m_taskGroupMap.end())
				{
					*p_taskGroup = (*value).second;
					return true;
				}

				return false;
			}

		public:
			TaskGroup *GetTaskGroupById(int p_id)
			{
				TaskGroup *value;

				return ContainsKey(p_id, &value) ? value : NULL;
			}

			TaskGroup *GetTaskGroupByIndex(int p_index)
			{
				BOOST_ASSERT(m_taskGroupList.size() > p_index);
				return m_taskGroupList[p_index];
			}

			void AddTaskGroup(int p_id, TaskGroup *p_taskGroup)
			{
				if (!ContainsKey(p_id))
				{
					m_taskGroupMap[p_id] = p_taskGroup;
					m_taskGroupList.push_back(p_taskGroup);
				}
			}

			void RemoveTaskGroup(int p_id, bool p_destroyGroup = false)
			{
				TaskGroup *value;

				std::map<int, TaskGroup*>::iterator mapIterator = m_taskGroupMap.find(p_id);
				
				if (mapIterator != m_taskGroupMap.end())
				{
					if (p_destroyGroup)
					{
						delete (*mapIterator).second;
					}

					m_taskGroupMap.erase(mapIterator);
					
					std::vector<TaskGroup*>::iterator vectorIterator = 
						std::find(m_taskGroupList.begin(), m_taskGroupList.end(), value);
					
					if (vectorIterator != m_taskGroupList.end())
						m_taskGroupList.erase(vectorIterator);
				}
			}

			void RemoveAllTasks(bool p_destroyGroups = false)
			{
				if (p_destroyGroups)
				{
					for (std::vector<TaskGroup*>::iterator iter = m_taskGroupList.begin();
						 iter != m_taskGroupList.end(); ++iter)
					{
						delete *iter;
					}
				}

				m_taskGroupList.clear();
				m_taskGroupMap.clear();
			}

			size_t Size(void) const
			{
				return m_taskGroupList.size();
			}
		};
	}
}