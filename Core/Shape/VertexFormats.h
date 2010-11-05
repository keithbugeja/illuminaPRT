//----------------------------------------------------------------------------------------------
//	Filename:	TriangleMesh.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <boost/shared_ptr.hpp>

#include "Geometry/Vector2.h"
#include "Geometry/Vector3.h"

namespace Illumina 
{
	namespace Core
	{
		struct VertexFormat {
			static const int Position	= 0x0001;
			static const int Normal		= 0x0002;
			static const int UV			= 0x0004;
		};

		template<int Descriptor> struct VertexBase {
			inline static int GetDescriptor(void) { return Descriptor; }
		};

		struct Vertex 
			: public VertexBase<VertexFormat::Position | VertexFormat::Normal | VertexFormat::UV>
		{
			Vector3	Position;
			Vector3 Normal;
			Vector2 UV;
		};

		struct VertexP 
			: public VertexBase<VertexFormat::Position>
		{
			Vector3 Position;
		};

		struct VertexPN 
			: public VertexBase<VertexFormat::Position | VertexFormat::Normal>
		{
			Vector3 Position;
			Vector3 Normal;
		};
	} 
}
