//----------------------------------------------------------------------------------------------
//	Filename:	Vector.inl
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

using namespace Illumina::Core;
//----------------------------------------------------------------------------------------------
inline float Vector3::operator[](int p_nIndex) const { 
	return Element[p_nIndex]; 
}
//----------------------------------------------------------------------------------------------
inline float& Vector3::operator[](int p_nIndex) { 
	return Element[p_nIndex]; 
}
//----------------------------------------------------------------------------------------------
inline void Vector3::Set(float p_x, float p_y, float p_z) {
	X = p_x; Y = p_y; Z = p_z;
}
//----------------------------------------------------------------------------------------------
inline Vector3& Vector3::operator=(const Vector3 &p_vector)
{
	X = p_vector.X;
	Y = p_vector.Y;
	Z = p_vector.Z;
	
	return *this;
}
//----------------------------------------------------------------------------------------------
inline bool Vector3::operator==(const Vector3 &p_vector) const
{
	if (X != p_vector.X) return false;
	if (Y != p_vector.Y) return false;
	if (Z != p_vector.Z) return false;

	return true;
}
//----------------------------------------------------------------------------------------------
inline bool Vector3::operator!=(const Vector3& p_vector) const {
	return !(*this == p_vector);
}
//----------------------------------------------------------------------------------------------
inline Vector3 Vector3::operator*(float p_fValue) const {
	return Vector3(p_fValue * X, p_fValue * Y, p_fValue * Z);
}
//----------------------------------------------------------------------------------------------
inline Vector3 Vector3::operator/(float p_fValue) const {
	//BOOST_ASSERT(p_fValue != 0.0f);
	return Vector3(*this * (1.0f / p_fValue));
}
//----------------------------------------------------------------------------------------------
inline Vector3 Vector3::operator*(const Vector3 &p_vector) const {
	return Vector3(p_vector.X * X, p_vector.Y * Y, p_vector.Z * Z);
}
//----------------------------------------------------------------------------------------------
inline Vector3 Vector3::operator+(const Vector3 &p_vector) const {
	return Vector3(X + p_vector.X, Y + p_vector.Y, Z + p_vector.Z);
}
//----------------------------------------------------------------------------------------------
inline Vector3 Vector3::operator-(const Vector3 &p_vector) const {
	return Vector3(X - p_vector.X, Y - p_vector.Y, Z - p_vector.Z);
}
//----------------------------------------------------------------------------------------------
inline Vector3 Vector3::operator-(void) const {
	return Vector3(-X, -Y, -Z);
}
//----------------------------------------------------------------------------------------------
inline Vector3& Vector3::operator*=(float p_fValue) {
	return *this = *this * p_fValue;
}
//----------------------------------------------------------------------------------------------
inline Vector3& Vector3::operator*=(const Vector3 &p_vector) {
	return *this = *this * p_vector;
}
//----------------------------------------------------------------------------------------------
inline Vector3& Vector3::operator/=(float p_fValue) {
	return *this = *this / p_fValue;
}
//----------------------------------------------------------------------------------------------
inline Vector3& Vector3::operator+=(const Vector3 &p_vector) {
	return *this = *this + p_vector;
}
//----------------------------------------------------------------------------------------------
inline Vector3& Vector3::operator-=(const Vector3 &p_vector) {
	return *this = *this - p_vector;
}
//----------------------------------------------------------------------------------------------
inline bool Vector3::IsOnes() const {
	return (X == Y && Y == Z && Z == 1.0f);
}
//----------------------------------------------------------------------------------------------
inline bool Vector3::IsZero() const {
	return (X == Y && Y == Z && Z == 0.0f);
}
//----------------------------------------------------------------------------------------------
inline float Vector3::MaxComponent() const 
{
	float max = X;
	if (Y > max) max = Y;
	if (Z > max) max = Z;

	return max;
}
//----------------------------------------------------------------------------------------------
inline float Vector3::MinComponent() const
{
	float min = X;
	if (Y < min) min = Y;
	if (Z < min) min = Z;

	return min;
}
//----------------------------------------------------------------------------------------------
inline float Vector3::MaxAbsComponent() const 
{
	float max = Maths::FAbs(X);
	if (Maths::FAbs(Y) > max) max = Maths::FAbs(Y);
	if (Maths::FAbs(Z) > max) max = Maths::FAbs(Z);

	return max;
}
//----------------------------------------------------------------------------------------------
inline float Vector3::MinAbsComponent() const
{
	float min = Maths::FAbs(X);
	if (Maths::FAbs(Y) < min) min = Maths::FAbs(Y);
	if (Maths::FAbs(Z) < min) min = Maths::FAbs(Z);

	return min;
}
//----------------------------------------------------------------------------------------------
inline int Vector3::ArgMaxAbsComponent() const
{
	int argmax = 0;
	float max = Maths::FAbs(X);

	if (Maths::FAbs(Y) > max) { max = Maths::FAbs(Y); argmax = 1; }
	if (Maths::FAbs(Z) > max) { argmax = 2; }

	return argmax;
}
//----------------------------------------------------------------------------------------------
inline int Vector3::ArgMinAbsComponent() const
{
	int argmin = 0;
	float min = Maths::FAbs(X);

	if (Maths::FAbs(Y) < min) { min = Maths::FAbs(Y); argmin = 1; }
	if (Maths::FAbs(Z) < min) { argmin = 2; }

	return argmin;
}
//----------------------------------------------------------------------------------------------
inline int Vector3::ArgMaxComponent() const
{
	int argmax = 0;
	float max = X;

	if (Y > max) { max = Y; argmax = 1; }
	if (Z > max) { argmax = 2; }

	return argmax;
}
//----------------------------------------------------------------------------------------------
inline int Vector3::ArgMinComponent() const
{
	int argmin = 0;
	float min = X;

	if (Y > min) { min = Y; argmin = 1; }
	if (Z > min) { argmin = 2; }

	return argmin;
}
//----------------------------------------------------------------------------------------------
inline float Vector3::Length(void) const {
	return Maths::Sqrt(X * X + Y * Y + Z * Z);
}
//----------------------------------------------------------------------------------------------
inline float Vector3::LengthSquared(void) const {
	return Vector3::Dot(*this, *this);
}
//----------------------------------------------------------------------------------------------
FORCEINLINE void Vector3::Normalize(void) {
	*this = Vector3::Normalize(*this);
}
//----------------------------------------------------------------------------------------------
FORCEINLINE float Vector3::Dot(const Vector3 &p_vector) const {
	return Vector3::Dot(*this, p_vector);
}
//----------------------------------------------------------------------------------------------
FORCEINLINE float Vector3::AbsDot(const Vector3 &p_vector) const {
	return Vector3::AbsDot(*this, p_vector);
}
//----------------------------------------------------------------------------------------------
FORCEINLINE Vector3 Vector3::Cross(const Vector3 &p_vector) const {
	return Vector3::Cross(*this, p_vector);
}
//----------------------------------------------------------------------------------------------
inline void Vector3::Add(const Vector3 &p_vector1, const Vector3 &p_vector2, Vector3 &p_out)
{
	#if defined(SSE_ENABLED)
		__m128 v1, v2, v3;
		v1 = _mm_load_ps(p_vector1.Element);
		v2 = _mm_load_ps(p_vector2.Element);
		v3 = _mm_add_ps(v1, v2);
		_mm_store_ps(p_out.Element, v3);
	#else
		p_out.X = p_vector1.X + p_vector2.X;
		p_out.Y = p_vector1.Y + p_vector2.Y;
		p_out.Z = p_vector1.Z + p_vector2.Z;
	#endif
}
//----------------------------------------------------------------------------------------------
inline void Vector3::Subtract(const Vector3 &p_vector1, const Vector3 &p_vector2, Vector3 &p_out)
{
	#if defined(SSE_ENABLED)
		__m128 v1, v2, v3;
		v1 = _mm_load_ps(p_vector1.Element);
		v2 = _mm_load_ps(p_vector2.Element);
		v3 = _mm_sub_ps(v1, v2);
		_mm_store_ps(p_out.Element, v3);
	#else
		p_out.X = p_vector1.X - p_vector2.X;
		p_out.Y = p_vector1.Y - p_vector2.Y;
		p_out.Z = p_vector1.Z - p_vector2.Z;
	#endif
}
//----------------------------------------------------------------------------------------------
FORCEINLINE float Vector3::Dot(const Vector3 &p_vector1, const Vector3 &p_vector2)
{
	return p_vector1.X * p_vector2.X + 
		p_vector1.Y * p_vector2.Y + 
		p_vector1.Z * p_vector2.Z;
}
//----------------------------------------------------------------------------------------------
FORCEINLINE float Vector3::AbsDot(const Vector3 &p_vector1, const Vector3 &p_vector2)
{
	return Maths::FAbs(p_vector1.X * p_vector2.X +
		p_vector1.Y * p_vector2.Y + 
		p_vector1.Z * p_vector2.Z);
}
//----------------------------------------------------------------------------------------------
FORCEINLINE Vector3 Vector3::Cross(const Vector3 &p_vector1, const Vector3 &p_vector2)
{
	return Vector3(p_vector1.Y * p_vector2.Z - p_vector1.Z * p_vector2.Y,
		p_vector1.X * p_vector2.Z - p_vector1.Z * p_vector2.X,
		p_vector1.X * p_vector2.Y - p_vector1.Y * p_vector2.X);
}
//----------------------------------------------------------------------------------------------
FORCEINLINE void Vector3::Cross(const Vector3 &p_vector1, const Vector3 &p_vector2, Vector3 &p_out)
{
	p_out.Set(p_vector1.Y * p_vector2.Z - p_vector1.Z * p_vector2.Y,
		p_vector1.X * p_vector2.Z - p_vector1.Z * p_vector2.X,
		p_vector1.X * p_vector2.Y - p_vector1.Y * p_vector2.X);
}
//----------------------------------------------------------------------------------------------
inline float Vector3::TripleProduct(const Vector3 &p_vector1, const Vector3 &p_vector2, const Vector3 &p_vector3) {
	return Vector3::Dot(p_vector1, Vector3::Cross(p_vector2, p_vector3));
}
//----------------------------------------------------------------------------------------------
inline void Vector3::Inverse(void) 
{
	X = 1.0f / X;
	Y = 1.0f / Y;
	Z = 1.0f / Z;
}
//----------------------------------------------------------------------------------------------
inline Vector3 Vector3::Inverse(const Vector3 &p_vector) 
{
	return Vector3(1.0f/p_vector.X, 1.0f/p_vector.Y, 1.0f/p_vector.Z);
}
//----------------------------------------------------------------------------------------------
inline void Vector3::Inverse(const Vector3 &p_vector, Vector3 &p_out) 
{
	p_out.X = 1.0f / p_vector.X;
	p_out.Y = 1.0f / p_vector.Y;
	p_out.Z = 1.0f / p_vector.Z;
}
//----------------------------------------------------------------------------------------------
FORCEINLINE Vector3 Vector3::Normalize(const Vector3 &p_vector) {
	return p_vector / p_vector.Length();
}
//----------------------------------------------------------------------------------------------
FORCEINLINE void Vector3::Normalize(const Vector3 &p_vector, Vector3 &p_out)
{
	float length = p_vector.Length();
	p_out.Set(p_vector.X / length, p_vector.Y / length, p_vector.Z / length);
}
//----------------------------------------------------------------------------------------------
inline float Vector3::DistanceSquared(const Vector3& p_point1, const Vector3& p_point2) {
	return (p_point2 - p_point1).LengthSquared();
}
//----------------------------------------------------------------------------------------------
inline float Vector3::Distance(const Vector3& p_point1, const Vector3& p_point2) {
	return (p_point2 - p_point1).Length();
}
//----------------------------------------------------------------------------------------------
inline void Vector3::Max(const Vector3 &p_vector1, const Vector3 &p_vector2, Vector3 &p_out)
{
	p_out.X = Maths::Max(p_vector1.X, p_vector2.X);	
	p_out.Y = Maths::Max(p_vector1.Y, p_vector2.Y);	
	p_out.Z = Maths::Max(p_vector1.Z, p_vector2.Z);	
}
//----------------------------------------------------------------------------------------------
inline void Vector3::Min(const Vector3 &p_vector1, const Vector3 &p_vector2, Vector3 &p_out)
{
	p_out.X = Maths::Min(p_vector1.X, p_vector2.X);	
	p_out.Y = Maths::Min(p_vector1.Y, p_vector2.Y);	
	p_out.Z = Maths::Min(p_vector1.Z, p_vector2.Z);	
}
//----------------------------------------------------------------------------------------------
inline Vector3 Vector3::Max(const Vector3 &p_vector1, const Vector3 &p_vector2) 
{
	return Vector3(Maths::Max(p_vector1.X, p_vector2.X),
		Maths::Max(p_vector1.Y, p_vector2.Y),
		Maths::Max(p_vector1.Z, p_vector2.Z));
}
//----------------------------------------------------------------------------------------------
inline Vector3 Vector3::Min(const Vector3 &p_vector1, const Vector3 &p_vector2) 
{
	return Vector3(Maths::Min(p_vector1.X, p_vector2.X),
		Maths::Min(p_vector1.Y, p_vector2.Y),
		Maths::Min(p_vector1.Z, p_vector2.Z));
}
//----------------------------------------------------------------------------------------------
inline void Vector3::Reflect(const Vector3 &p_vector, const Vector3 &p_normal, Vector3 &p_out)
{
	Vector3::Subtract(p_vector, (2.0f * Vector3::Dot(p_vector, p_normal)) * p_normal, p_out);
}
