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

			void UniversalBarrier(std::string& p_comment)
			{
				int rank = GetUniversalRank();
				std::cout << "[" << rank << "] :: Arrived at barrer :: [" << p_comment << "]" << std::endl;
				MPI_Barrier(MPI_COMM_WORLD);
				std::cout << "[" << rank << "] :: Proceeded beyond barrer :: [" << p_comment << "]" << std::endl;
			}

			void Split(int p_splitCount)
			{
				int rank = GetUniversalRank();				
				UniversalBarrier("Split");

				std::cout << "[" << rank << "] :: Split :: [Group Size = " << Size <<", Split Count = " << p_splitCount << "]" << std::endl;		

				// Determine rank zero in current group
				int rankZero = this->GetRankZero();
		
				// Rank zero is within specified range
				if (rankZero > 0 && rankZero < p_splitSize)
				{
					int leftBound = rankZero - 1,
						rightBound = p_splitSize;

					int range[3][3];

					range[0][0] = 0; range[0][1] = leftBound; range[0][2] = 1;
					range[1][0] = rankZero + 1; range[1][1] = rightBound; range[1][2] = 1;
					range[2][0] = rankZero, range[2][1] = rankZero; range[2][2] = 1;

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

					if (!rankZero) { range[0][0] = 1; range[0][1] = p_splitSize; range[0][2] = 1; }
					else { range[0][0] = 0; range[0][1] = p_splitSize - 1; range[0][2] = 1; }
					range[1][0] = rankZero; range[1][1] = rankZero; range[1][2] = 1;

					std::cout << "[" << rank << "] :: Split ranges :: [ " << 
						range[0][0] << "," << range[0][1] << "," << range[0][2] << "," <<
						range[1][0] << "," << range[1][1] << "," << range[1][2] << "]" << std::endl; 
			
					MPI_Group_range_incl(Group, 2, range, &p_subgroup->Group);
					MPI_Group_range_excl(Group, 1, range, &Group);
				}

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

				std::cout << "[" << rank << "] :: Split Successful :: [This size = " << Size << ", Split Size = " << p_subgroup->Size << "]" << std::endl;
			}

			void Merge(void) { }
		};
	}
}