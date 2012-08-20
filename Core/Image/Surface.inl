//----------------------------------------------------------------------------------------------
//	Filename:	Surface.inl
//	Author:		Keith Bugeja
//	Date:		22/02/2012
//----------------------------------------------------------------------------------------------
#include <iostream>

using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
template<class T>
TSurface<T>::TSurface(int p_nWidth, int p_nHeight)
	: m_nWidth(p_nWidth)
	, m_nHeight(p_nHeight)
	, m_bIsOwner(true)
{
	m_bitmap = new T[m_nWidth * m_nHeight];
				
	for (int i = 0; i < m_nWidth * m_nHeight; i++)
		m_bitmap[i].Clear();
}
//----------------------------------------------------------------------------------------------
template<class T>
TSurface<T>::TSurface(int p_nWidth, int p_nHeight, const T &p_rgb)
	: m_nWidth(p_nWidth)
	, m_nHeight(p_nHeight)
	, m_bIsOwner(true)
{
	m_bitmap = new T[m_nWidth * m_nHeight];

	for (int i = 0; i < m_nWidth * m_nHeight; i++)
		m_bitmap[i] = p_rgb;
}
//----------------------------------------------------------------------------------------------
template<class T>
TSurface<T>::TSurface(int p_nWidth, int p_nHeight, T *p_pRGBBuffer)
	: m_nWidth(p_nWidth)
	, m_nHeight(p_nHeight)
	, m_bIsOwner(false)
	, m_bitmap(p_pRGBBuffer)
{
	for (int i = 0; i < m_nWidth * m_nHeight; i++)
		m_bitmap[i].Clear();
}
//----------------------------------------------------------------------------------------------
template<class T>
TSurface<T>::~TSurface(void) 
{
	if (m_bIsOwner)
		delete[] m_bitmap;
}
//----------------------------------------------------------------------------------------------
template<class T>
inline void TSurface<T>::Set(int p_x, int p_y, const T &p_colour) {
	m_bitmap[IndexOf(p_x, p_y)] = p_colour;
} 
//----------------------------------------------------------------------------------------------
template<class T>
inline T TSurface<T>::Get(int p_x, int p_y) {
	return m_bitmap[IndexOf(p_x, p_y)];
}
//----------------------------------------------------------------------------------------------
template<class T>
inline void TSurface<T>::Get(int p_x, int p_y, T &p_colour) 
{
	p_colour = m_bitmap[IndexOf(p_x, p_y)];
}
//----------------------------------------------------------------------------------------------
template<class T>
inline int TSurface<T>::IndexOf(int p_x, int p_y) {
	return p_x + p_y * m_nWidth;
}
//----------------------------------------------------------------------------------------------
template<class T>
inline int TSurface<T>::GetWidth(void) const { 
	return m_nWidth; 
}
//----------------------------------------------------------------------------------------------
template<class T>
inline int TSurface<T>::GetHeight(void) const { 
	return m_nHeight; 
}
//----------------------------------------------------------------------------------------------
template<class T>
inline int TSurface<T>::GetArea(void) const { 
	return m_nWidth * m_nHeight; 
}
//----------------------------------------------------------------------------------------------
template<class T>
inline T TSurface<T>::operator[](int p_nIndex) const {
	return RGBPixel(m_bitmap[p_nIndex]);
}
//----------------------------------------------------------------------------------------------
template<class T>
inline T& TSurface<T>::operator[](int p_nIndex) {
	return m_bitmap[p_nIndex];
}
//----------------------------------------------------------------------------------------------
template<class T>
inline T* TSurface<T>::GetSurfaceBuffer(void) const {
	return m_bitmap;
}
//----------------------------------------------------------------------------------------------
template<class T>
inline T* TSurface<T>::GetScanline(int p_nScanline) const {
	return m_bitmap + p_nScanline * m_nWidth;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
