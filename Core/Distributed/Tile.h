#include "Object/object.h"
#include "Geometry/vector2.h"
#include "Geometry/ray.h"

namespace Illumina 
{
	namespace Core 
	{
		class Tile : public Object
		{
		private:
			Vector2 m_regionStart,
				m_regionEnd;

		public:
			Tile(void) { }

			Tile(const Vector2 &p_regionStart, const Vector2 &p_regionEnd)
			{
				m_regionStart = p_regionStart;
				m_regionEnd = p_regionEnd;
			}

			Tile(const Tile &p_tile)
			{
				m_regionStart = p_tile.m_regionStart;
				m_regionEnd = p_tile.m_regionEnd;
			}

			float GetWidth(void) const
			{
				return m_regionEnd.X - m_regionStart.X;
			}

			float GetHeight(void) const
			{
				return m_regionEnd.Y - m_regionStart.Y;
			}

			Vector2 GetRegionStart(void) const
			{
				return m_regionStart;
			}

			Vector2 GetRegionEnd(void) const
			{
				return m_regionEnd;
			}

			std::string ToString(void) 
			{
				std::string strOut = boost::str(boost::format("Region => [%s %s]") % m_regionStart.ToString() % m_regionEnd.ToString());
				return strOut;
			}
		};

		class RenderRequest
		{
		public:
			Tile m_tileRequest;
			float m_fQuantum;
		};

		class RenderResponse
		{
		public:
			Tile m_tileResponse;
			Image m_renderBuffer;
			float m_fElapsed;
		};
	}
}