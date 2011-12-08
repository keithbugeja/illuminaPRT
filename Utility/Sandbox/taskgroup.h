#pragma once

#include <map>
#include <vector>

#include <boost/thread.hpp>
#include <boost/mpi.hpp>
namespace mpi = boost::mpi;

#include "../../Core/Object/Object.h"

namespace Illumina
{
	namespace Core
	{
		class TaskGroup
		{
		public:
			MPI_Group Group;
			MPI_Comm Communicator;
			int Size;

			std::vector<int> GroupRankList;
		
		public:
			int GetRank(void)
			{
				int rank; MPI_Group_rank(Group, &rank);
				return rank;
			}

			int GetUniversalRank(void)
			{
				int rank; MPI_Group_rank(GetUniversalGroup(), &rank);
				return rank;
			}

			int GetMasterRank(void)
			{
				int rankZero = 0, 
					rankZeroTranslated;

				MPI_Group group = GetUniversalGroup();
				MPI_Group_translate_ranks(group, 1, &rankZero, Group, &rankZeroTranslated);

				return rankZeroTranslated;
			}

			MPI_Group GetUniversalGroup(void)
			{
				MPI_Group group;
				MPI_Comm_group(MPI_COMM_WORLD, &group);
				return group;
			}
		
		public:
			TaskGroup(void)
				: Group(MPI_UNDEFINED)
				, Communicator(MPI_UNDEFINED)
				, Size(0)
				, MasterRank(-1)
			{ }

			bool IsMember(void)
			{
				return GetRank() != MPI_UNDEFINED;
			}

			void UniversalBarrier(const std::string &p_comment)
			{
				int rank = GetUniversalRank();
				std::cout << "[" << rank << "] :: Arrived at barrer :: [" << p_comment << "]" << std::endl;
				MPI_Barrier(MPI_COMM_WORLD);
				std::cout << "[" << rank << "] :: Proceeded beyond barrer :: [" << p_comment << "]" << std::endl;
			}

			void Split(int p_groupSize, bool p_distributeMasterRank = true)
			{
				this->UniversalBarrier("Range Split");

				if (p_distributeMasterRank)
				{
					int rank = this->GetUniversalRank();
					int masterRank = this->GetMasterRank();
					int coordinatorRank = 0;

					// We need to preserve zero (coodinator), 
					// We need to go through task range to determine
				}
				else
				{
				}
			}


			void Split2(int p_splitCount)
			{
				this->UniversalBarrier("Split");

				int rank = this->GetUniversalRank();				
				std::cout << "[" << rank << "] :: Split :: [Group Size = " << Size <<", Split Count = " << p_splitCount << "]" << std::endl;		

				// Determine rank zero in current group
				int masterRank = this->GetMasterRank();
		
				// Rank zero is within specified range
				if (masterRank > 0 && masterRank < p_splitCount)
				{
					int leftBound = masterRank - 1,
						rightBound = p_splitCount;

					int range[3][3];

					range[0][0] = 0; range[0][1] = leftBound; range[0][2] = 1;
					range[1][0] = masterRank + 1; range[1][1] = rightBound; range[1][2] = 1;
					range[2][0] = masterRank, range[2][1] = masterRank; range[2][2] = 1;

					std::cout << "[" << rank << "] :: Split ranges :: [ " << 
						range[0][0] << "," << range[0][1] << "," << range[0][2] << "," <<
						range[1][0] << "," << range[1][1] << "," << range[1][2] << "," <<
						range[2][0] << "," << range[2][1] << "," << range[2][2] << "]" << std::endl; 

					MPI_Group_range_incl(Group, 3, range, &p_subgroup->Group);
					MPI_Group_range_excl(Group, 2, range, &Group);
				}
				else
				{
					int range[2][3];

					if (!masterRank) { range[0][0] = 1; range[0][1] = p_splitCount; range[0][2] = 1; }
					else { range[0][0] = 0; range[0][1] = p_splitCount - 1; range[0][2] = 1; }
					range[1][0] = masterRank; range[1][1] = masterRank; range[1][2] = 1;

					std::cout << "[" << rank << "] :: Split ranges :: [ " << 
						range[0][0] << "," << range[0][1] << "," << range[0][2] << "," <<
						range[1][0] << "," << range[1][1] << "," << range[1][2] << "]" << std::endl; 
			
					MPI_Group_range_incl(Group, 2, range, &p_subgroup->Group);
					MPI_Group_range_excl(Group, 1, range, &Group);
				}

				this->UniversalBarrier("Split -> Created Groups");
				std::cout << "[" << rank << "] :: " << std::hex << this->Group << ", " << p_subgroup->Group << std::dec << std::endl;

				if (this->IsMember())
				{
					MPI_Comm_create(MPI_COMM_WORLD, this->Group, &this->Communicator);
				} else {
					MPI_Comm_create(MPI_COMM_WORLD, MPI_GROUP_EMPTY, &this->Communicator);
				}

				if (p_subgroup->IsMember())
				{
					MPI_Comm_create(MPI_COMM_WORLD, p_subgroup->Group, &p_subgroup->Communicator);
				} else {
					MPI_Comm_create(MPI_COMM_WORLD, MPI_GROUP_EMPTY, &p_subgroup->Communicator);
				}

				// Update group size
				MPI_Group_size(p_subgroup->Group, &p_subgroup->Size);
				MPI_Group_size(Group, &Size);

				this->UniversalBarrier("Split -> Complete");
				std::cout << "[" << rank << "] :: Split Successful :: [This size = " << Size << ", Split Size = " << p_subgroup->Size << "]" << std::endl;
			}

			void Merge(void) { }
		};
	}
}