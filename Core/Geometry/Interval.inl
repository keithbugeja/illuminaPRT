//----------------------------------------------------------------------------------------------
//	Filename:	Interval.inl
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
using namespace Illumina::Core;

//----------------------------------------------------------------------------------------------
inline void Interval::Set(float p_fValue) {
	Min = Max = p_fValue;
}
//----------------------------------------------------------------------------------------------
inline void Interval::Set(float p_fMin, float p_fMax) {
	Min = p_fMin; Max = p_fMax;
}
//----------------------------------------------------------------------------------------------
inline Interval& Interval::operator=(const Interval &p_interval)
{
	Min = p_interval.Min;
	Max = p_interval.Max;
				
	return *this;
}
//----------------------------------------------------------------------------------------------
inline bool Interval::operator==(const Interval &p_interval) const
{
	if (Min != p_interval.Min) return false;
	if (Max != p_interval.Max) return false;

	return true;
}
//----------------------------------------------------------------------------------------------
inline bool Interval::operator!=(const Interval& p_interval) const {
	return !(*this == p_interval);
}
//----------------------------------------------------------------------------------------------
inline void Interval::Extend(float p_fValue) 
{
	if (Min > p_fValue) Min = p_fValue;
	if (Max < p_fValue) Max = p_fValue;
}
//----------------------------------------------------------------------------------------------
inline float Interval::Median(void) const {
	return (Min + Max) * 0.5f;
}
//----------------------------------------------------------------------------------------------
inline float Interval::Span(void) const {
	return Max - Min;
}
//----------------------------------------------------------------------------------------------
inline bool Interval::Intersects(const Interval& p_interval) const
{
	return Max >= p_interval.Min && Min <= p_interval.Max;
}
//----------------------------------------------------------------------------------------------
inline bool Interval::Contains(float p_fValue) const
{
	return Min <= p_fValue && p_fValue <= Max;
}
//----------------------------------------------------------------------------------------------
