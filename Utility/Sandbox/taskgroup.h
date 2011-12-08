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

			void Split(void) { }
			void Merge(void) { }
		};
	}
}