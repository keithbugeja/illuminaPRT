//----------------------------------------------------------------------------------------------
//	Filename:	Image.inl
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	A simple image class to store an array of RGB colours that can be accessed
//  via one or two-dimensional indexing. 
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
inline int Image::IndexOf(int p_x, int p_y) 
{
	BOOST_ASSERT(p_x + p_y * m_nWidth >= 0 &&
		p_x + p_y * m_nWidth < m_nHeight * m_nWidth);
			
	return p_x + p_y * m_nWidth;
}
//----------------------------------------------------------------------------------------------
inline int Image::GetWidth(void) const { 
	return m_nWidth; 
}
//----------------------------------------------------------------------------------------------
inline int Image::GetHeight(void) const { 
	return m_nHeight; 
}
//----------------------------------------------------------------------------------------------
inline int Image::GetLength(void) const { 
	return m_nWidth * m_nHeight; 
}
//----------------------------------------------------------------------------------------------
inline RGBPixel Image::operator[](int p_nIndex) const 
{
	BOOST_ASSERT(p_nIndex >= 0 && p_nIndex < m_nWidth * m_nHeight);
	return RGBPixel(m_bitmap[p_nIndex]);
}
//----------------------------------------------------------------------------------------------
inline RGBPixel& Image::operator[](int p_nIndex) 
{
	BOOST_ASSERT(p_nIndex >= 0 && p_nIndex < m_nWidth * m_nHeight);
	return m_bitmap[p_nIndex];
}
//----------------------------------------------------------------------------------------------
