#include "Types.h"
#include "Types.h"
#include "Types.h"
#include "random-rankit.h"

Random::Structs::logic::logic()
{
	Value = false;
	Default = false;
}

Random::Structs::logic::logic(bool _value, bool _default)
{
	this->Value = _value;
	this->Default = _default;
}

Random::Structs::logic::logic(bool b)
{
	this->Value = b;
}

void Random::Structs::logic::operator=(const logic& l)
{
	Value = l.Value;
}

bool Random::Structs::logic::OR(bool _operator)
{
	return Value || _operator;
}

bool Random::Structs::logic::XOR(bool _operator)
{
	return Value ^ _operator;
}

bool Random::Structs::logic::AND(bool _operator)
{
	return Value && _operator;
}

bool Random::Structs::logic::NAND(bool _operator)
{
	return !Value && _operator;
}

bool Random::Structs::logic::NOT()
{
	return !Value;
}

void Random::Structs::logic::RevertToDefault()
{
	Value = Default;
}

void Random::Structs::logic::WHEN(bool _condition, void(*_function)())
{
	if (Value == _condition)
	{
		(*_function)();
	}
}


Random::Structs::SClamp::SClamp()
{
	imin = 0;
	imax = 1;
	fmin = 0.0f;
	fmax = 1.0f;
	dmin = 0.0;
	dmax = 1.0;
}

Random::Structs::SClamp::SClamp(int MIN, int MAX)
{
	this->imin = MIN;
	this->imax = MAX;
}

Random::Structs::SClamp::SClamp(float MIN, float MAX)
{
	this->fmin = MIN;
	this->fmax = MAX;
}

Random::Structs::SClamp::SClamp(double MIN, double MAX)
{
	this->dmin = MIN;
	this->dmax = MAX;
}

Random::Structs::SClamp::SClamp(const SClamp& clamp)
{
	fmin = clamp.fmin;
	fmax = clamp.fmax;
	dmin = clamp.dmin;
	dmax = clamp.dmax;
}

void Random::Structs::SClamp::operator=(const SClamp& clamp)
{
	fmin = clamp.fmin;
	fmax = clamp.fmax;
	dmin = clamp.dmin;
	dmax = clamp.dmax;
}

int Random::Structs::SClamp::clamp(int INPUT)
{
	if (INPUT < imin)
		return imin;
	if (INPUT > imax)
		return imax;
	return INPUT;
}
int Random::Structs::SClamp::clamp(int INPUT, int MIN, int MAX)
{
	this->imin = MIN;
	this->imax = MAX;
	if (INPUT < imin)
		return imin;
	if (INPUT > imax)
		return imax;
	return INPUT;
}
float Random::Structs::SClamp::clamp(float INPUT)
{
	if (INPUT < fmin)
		return fmin;
	if (INPUT > fmax)
		return fmax;
	return INPUT;
}
float Random::Structs::SClamp::clamp(float INPUT, float MIN, float MAX)
{
	this->fmin = MIN;
	this->fmax = MAX;
	if (INPUT < fmin)
		return fmin;
	if (INPUT > fmax)
		return fmax;
	return INPUT;
}
double Random::Structs::SClamp::clamp(double INPUT)
{
	if (INPUT < dmin)
		return dmin;
	if (INPUT > dmax)
		return dmax;
	return INPUT;
}
double Random::Structs::SClamp::clamp(double INPUT, double MIN, double MAX)
{
	this->dmin = MIN;
	this->dmax = MAX;
	if (INPUT < dmin)
		return dmin;
	if (INPUT > dmax)
		return dmax;
	return INPUT;
}

Random::Structs::v2::v2()
{
	x = 0;
	y = 0;
}

Random::Structs::v2::v2(float X, float Y)
{
	this->x = X;
	this->y = Y;
}

Random::Structs::v2::v2(float SCALAR)
{
	this->x = SCALAR;
	this->y = SCALAR;
}

Random::Structs::v2::v2(const v2& vector)
{
	x = vector.x;
	y = vector.y;
}

void Random::Structs::v2::operator=(const v2& vector)
{
	x = vector.x;
	y = vector.y;
}

Random::Structs::v2 Random::Structs::v2::operator+=(const v2& vector)
{
	x = x + vector.x;
	y = y + vector.y;
	return v2(x, y);
}

Random::Structs::v2 Random::Structs::v2::operator-=(const v2& vector)
{
	x = x - vector.x;
	y = y - vector.y;
	return v2(x, y);
}

Random::Structs::v2 Random::Structs::v2::operator*=(const v2& vector)
{
	x = x * vector.x;
	y = y * vector.y;
	return v2(x, y);
}

Random::Structs::v2 Random::Structs::v2::operator/=(const v2& vector)
{
	x = x / vector.x;
	y = y / vector.y;
	return v2(x, y);
}

Random::Structs::v2 Random::Structs::v2::operator+(const v2& vector)
{
	v2 t;
	t.x = x + vector.x;
	t.y = y + vector.y;
	return t;
}

Random::Structs::v2 Random::Structs::v2::operator-(const v2& vector)
{
	v2 t;
	t.x = x - vector.x;
	t.y = y - vector.y;
	return t;
}

Random::Structs::v2 Random::Structs::v2::operator*(const v2& vector)
{
	v2 t;
	t.x = x * vector.x;
	t.y = y * vector.y;
	return t;
}

Random::Structs::v2 Random::Structs::v2::operator/(const v2& vector)
{
	v2 t;
	t.x = x / vector.x;
	t.y = y / vector.y;
	return t;
}

Random::Structs::v3::v3()
{
	x = 0;
	y = 0;
	z = 0;
}

Random::Structs::v3::v3(float X, float Y, float Z)
{
	this->x = X;
	this->y = Y;
	this->z = Z;
}

Random::Structs::v3::v3(float SCALAR)
{
	this->x = SCALAR;
	this->y = SCALAR;
	this->z = SCALAR;
}

Random::Structs::v3::v3(const v3& vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
}

Random::Structs::v3 Random::Structs::v3::getForwardVector()
{
	return v3(1.0f, 0.0f, 0.0f);
}

Random::Structs::v3 Random::Structs::v3::getRightVector()
{
	return v3(0.0f, 0.0f, 1.0f);
}

Random::Structs::v3 Random::Structs::v3::getUpVector()
{
	return v3(0.0f, 1.0f, 0.0f);
}

void Random::Structs::v3::operator=(const v3& vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
}

Random::Structs::v3 Random::Structs::v3::operator+=(const v3& vector)
{
	x = x + vector.x;
	y = y + vector.y;
	z = z + vector.z;
	return v3(x, y, z);
}

Random::Structs::v3 Random::Structs::v3::operator-=(const v3& vector)
{
	x = x - vector.x;
	y = y - vector.y;
	z = z - vector.z;
	return v3(x, y, z);
}

Random::Structs::v3 Random::Structs::v3::operator*=(const v3& vector)
{
	x = x * vector.x;
	y = y * vector.y;
	z = z * vector.z;
	return v3(x, y, z);
}

Random::Structs::v3 Random::Structs::v3::operator/=(const v3& vector)
{
	x = x / vector.x;
	y = y / vector.y;
	z = z / vector.z;
	return v3(x, y, z);
}

Random::Structs::v3 Random::Structs::v3::operator+(const v3& vector)
{
	v3 t;
	t.x = x + vector.x;
	t.y = y + vector.y;
	t.z = z + vector.z;
	return t;
}

Random::Structs::v3 Random::Structs::v3::operator-(const v3& vector)
{
	v3 t;
	t.x = x - vector.x;
	t.y = y - vector.y;
	t.z = z - vector.z;
	return t;
}

Random::Structs::v3 Random::Structs::v3::operator*(const v3& vector)
{
	v3 t;
	t.x = x * vector.x;
	t.y = y * vector.y;
	t.z = z * vector.z;
	return t;
}

Random::Structs::v3 Random::Structs::v3::operator/(const v3& vector)
{
	v3 t;
	t.x = x / vector.x;
	t.y = y / vector.y;
	t.z = z / vector.z;
	return t;
}


Random::Structs::v4::v4()
{
	x = 0;
	y = 0;
	z = 0;
	w = 0;
}

Random::Structs::v4::v4(float X, float Y, float Z, float W)
{
	this->x = X;
	this->y = Y;
	this->z = Z;
	this->w = W;
}

Random::Structs::v4::v4(float SCALAR)
{
	this->x = SCALAR;
	this->y = SCALAR;
	this->z = SCALAR;
	this->w = SCALAR;
}

Random::Structs::v4::v4(const v4& vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
	w = vector.w;
}

void Random::Structs::v4::operator=(const v4& vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
	w = vector.w;
}
Random::Structs::v4 Random::Structs::v4::operator+=(const v4& vector)
{
	x = x + vector.x;
	y = y + vector.y;
	z = z + vector.z;
	w = w + vector.w;
	return v4(x, y, z, w);
}
Random::Structs::v4 Random::Structs::v4::operator-=(const v4& vector)
{
	x = x - vector.x;
	y = y - vector.y;
	z = z - vector.z;
	w = w - vector.w;
	return v4(x, y, z, w);
}
Random::Structs::v4 Random::Structs::v4::operator*=(const v4& vector)
{
	x = x * vector.x;
	y = y * vector.y;
	z = z * vector.z;
	w = w * vector.w;
	return v4(x, y, z, w);
}
Random::Structs::v4 Random::Structs::v4::operator/=(const v4& vector)
{
	x = x / vector.x;
	y = y / vector.y;
	z = z / vector.z;
	w = w / vector.w;
	return v4(x, y, z, w);
}
Random::Structs::v4 Random::Structs::v4::operator+(const v4& vector)
{
	v4 t;
	t.x = x + vector.x;
	t.y = y + vector.y;
	t.z = z + vector.z;
	t.w = w + vector.w;
	return t;
}
Random::Structs::v4 Random::Structs::v4::operator-(const v4& vector)
{
	v4 t;
	t.x = x - vector.x;
	t.y = y - vector.y;
	t.z = z - vector.z;
	t.w = w - vector.w;
	return t;
}
Random::Structs::v4 Random::Structs::v4::operator*(const v4& vector)
{
	v4 t;
	t.x = x * vector.x;
	t.y = y * vector.y;
	t.z = z * vector.z;
	t.w = w * vector.w;
	return t;
}
Random::Structs::v4 Random::Structs::v4::operator/(const v4& vector)
{
	v4 t;
	t.x = x / vector.x;
	t.y = y / vector.y;
	t.z = z / vector.z;
	t.w = w / vector.w;
	return t;
}
Random::Structs::iv2::iv2()
{
	x = 0;
	y = 0;
}

Random::Structs::iv2::iv2(int X, int Y)
{
	this->x = X;
	this->y = Y;
}

Random::Structs::iv2::iv2(int SCALAR)
{
	this->x = SCALAR;
	this->y = SCALAR;
}

Random::Structs::iv2::iv2(const iv2& vector)
{
	x = vector.x;
	y = vector.y;
}

void Random::Structs::iv2::operator=(const iv2& vector)
{
	x = vector.x;
	y = vector.y;
}

Random::Structs::iv2 Random::Structs::iv2::operator+=(const iv2& vector)
{
	x = x + vector.x;
	y = y + vector.y;
	return iv2(x, y);
}

Random::Structs::iv2 Random::Structs::iv2::operator-=(const iv2& vector)
{
	x = x - vector.x;
	y = y - vector.y;
	return iv2(x, y);
}

Random::Structs::iv2 Random::Structs::iv2::operator*=(const iv2& vector)
{
	x = x * vector.x;
	y = y * vector.y;
	return iv2(x, y);
}

Random::Structs::iv2 Random::Structs::iv2::operator/=(const iv2& vector)
{
	x = x / vector.x;
	y = y / vector.y;
	return iv2(x, y);
}

Random::Structs::iv2 Random::Structs::iv2::operator+(const iv2& vector)
{
	iv2 t;
	t.x = x + vector.x;
	t.y = y + vector.y;
	return iv2(x, y);
}

Random::Structs::iv2 Random::Structs::iv2::operator-(const iv2& vector)
{
	iv2 t;
	t.x = x - vector.x;
	t.y = y - vector.y;
	return iv2(x, y);
}

Random::Structs::iv2 Random::Structs::iv2::operator*(const iv2& vector)
{
	iv2 t;
	t.x = x * vector.x;
	t.y = y * vector.y;
	return iv2(x, y);
}

Random::Structs::iv2 Random::Structs::iv2::operator/(const iv2& vector)
{
	iv2 t;
	t.x = x / vector.x;
	t.y = y / vector.y;
	return iv2(x, y);
}

Random::Structs::iv3::iv3()
{
	x = 0;
	y = 0;
	z = 0;
}

Random::Structs::iv3::iv3(int X, int Y, int Z)
{
	this->x = X;
	this->y = Y;
	this->z = Z;
}

Random::Structs::iv3::iv3(int SCALAR)
{
	this->x = SCALAR;
	this->y = SCALAR;
	this->z = SCALAR;
}

Random::Structs::iv3::iv3(const iv3& vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
}

void Random::Structs::iv3::operator=(const iv3& vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
}

Random::Structs::iv3 Random::Structs::iv3::operator+=(const iv3& vector)
{
	x = x + vector.x;
	y = y + vector.y;
	z = z + vector.z;
	return iv3(x, y, z);
}

Random::Structs::iv3 Random::Structs::iv3::operator-=(const iv3& vector)
{
	x = x - vector.x;
	y = y - vector.y;
	z = z - vector.z;
	return iv3(x, y, z);
}

Random::Structs::iv3 Random::Structs::iv3::operator*=(const iv3& vector)
{
	x = x * vector.x;
	y = y * vector.y;
	z = z * vector.z;
	return iv3(x, y, z);
}

Random::Structs::iv3 Random::Structs::iv3::operator/=(const iv3& vector)
{
	x = x / vector.x;
	y = y / vector.y;
	z = z / vector.z;
	return iv3(x, y, z);
}

Random::Structs::iv3 Random::Structs::iv3::operator+(const iv3& vector)
{
	iv3 t;
	t.x = x + vector.x;
	t.y = y + vector.y;
	t.z = z + vector.z;
	return t;
}

Random::Structs::iv3 Random::Structs::iv3::operator-(const iv3& vector)
{
	iv3 t;
	t.x = x - vector.x;
	t.y = y - vector.y;
	t.z = z - vector.z;
	return t;
}

Random::Structs::iv3 Random::Structs::iv3::operator*(const iv3& vector)
{
	iv3 t;
	t.x = x * vector.x;
	t.y = y * vector.y;
	t.z = z * vector.z;
	return t;
}

Random::Structs::iv3 Random::Structs::iv3::operator/(const iv3& vector)
{
	iv3 t;
	t.x = x / vector.x;
	t.y = y / vector.y;
	t.z = z / vector.z;
	return t;
}

Random::Structs::iv4::iv4()
{
	x = 0;
	y = 0;
	z = 0;
	w = 0;
}

Random::Structs::iv4::iv4(int X, int Y, int Z, int W)
{
	this->x = X;
	this->y = Y;
	this->z = Z;
	this->w = W;
}

Random::Structs::iv4::iv4(int SCALAR)
{
	this->x = SCALAR;
	this->y = SCALAR;
	this->z = SCALAR;
	this->w = SCALAR;
}

Random::Structs::iv4::iv4(const iv4& vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
	w = vector.w;
}

void Random::Structs::iv4::operator=(const iv4& vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
	w = vector.w;
}

Random::Structs::iv4 Random::Structs::iv4::operator+=(const iv4& vector)
{
	x = x + vector.x;
	y = y + vector.y;
	z = z + vector.z;
	w = w + vector.w;
	return iv4(x, y, z, w);
}

Random::Structs::iv4 Random::Structs::iv4::operator-=(const iv4& vector)
{
	x = x - vector.x;
	y = y - vector.y;
	z = z - vector.z;
	w = w - vector.w;
	return iv4(x, y, z, w);
}

Random::Structs::iv4 Random::Structs::iv4::operator*=(const iv4& vector)
{
	x = x * vector.x;
	y = y * vector.y;
	z = z * vector.z;
	w = w * vector.w;
	return iv4(x, y, z, w);
}

Random::Structs::iv4 Random::Structs::iv4::operator/=(const iv4& vector)
{
	x = x / vector.x;
	y = y / vector.y;
	z = z / vector.z;
	w = w / vector.w;
	return iv4(x, y, z, w);
}

Random::Structs::iv4 Random::Structs::iv4::operator+(const iv4& vector)
{
	iv4 t;
	t.x = x + vector.x;
	t.y = y + vector.y;
	t.z = z + vector.z;
	t.w = w + vector.w;
	return t;
}

Random::Structs::iv4 Random::Structs::iv4::operator-(const iv4& vector)
{
	iv4 t;
	t.x = x - vector.x;
	t.y = y - vector.y;
	t.z = z - vector.z;
	t.w = w - vector.w;
	return t;
}

Random::Structs::iv4 Random::Structs::iv4::operator*(const iv4& vector)
{
	iv4 t;
	t.x = x * vector.x;
	t.y = y * vector.y;
	t.z = z * vector.z;
	t.w = w * vector.w;
	return t;
}

Random::Structs::iv4 Random::Structs::iv4::operator/(const iv4& vector)
{
	iv4 t;
	t.x = x / vector.x;
	t.y = y / vector.y;
	t.z = z / vector.z;
	t.w = w / vector.w;
	return t;
}

Random::Structs::dv2::dv2()
{
	x = 0;
	y = 0;
}

Random::Structs::dv2::dv2(double X, double Y)
{
	this->x = X;
	this->y = Y;
}

Random::Structs::dv2::dv2(double SCALAR)
{
	this->x = SCALAR;
	this->y = SCALAR;
}

Random::Structs::dv2::dv2(const dv2& vector)
{
	x = vector.x;
	y = vector.y;
}

void Random::Structs::dv2::operator=(const dv2& vector)
{
	x = vector.x;
	y = vector.y;
}

Random::Structs::dv2 Random::Structs::dv2::operator+=(const dv2& vector)
{
	return dv2();
}

Random::Structs::dv2 Random::Structs::dv2::operator-=(const dv2& vector)
{
	return dv2();
}

Random::Structs::dv2 Random::Structs::dv2::operator*=(const dv2& vector)
{
	return dv2();
}

Random::Structs::dv2 Random::Structs::dv2::operator/=(const dv2& vector)
{
	return dv2();
}

Random::Structs::dv2 Random::Structs::dv2::operator+(const dv2& vector)
{
	return dv2();
}

Random::Structs::dv2 Random::Structs::dv2::operator-(const dv2& vector)
{
	return dv2();
}

Random::Structs::dv2 Random::Structs::dv2::operator*(const dv2& vector)
{
	return dv2();
}

Random::Structs::dv2 Random::Structs::dv2::operator/(const dv2& vector)
{
	return dv2();
}

Random::Structs::dv3::dv3()
{
	x = 0;
	y = 0;
	z = 0;
}

Random::Structs::dv3::dv3(double X, double Y, double Z)
{
	this->x = X;
	this->y = Y;
	this->z = Z;
}

Random::Structs::dv3::dv3(double SCALAR)
{
	this->x = SCALAR;
	this->y = SCALAR;
	this->z = SCALAR;
}

Random::Structs::dv3::dv3(const dv3& vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
}

void Random::Structs::dv3::operator=(const dv3& vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
}

Random::Structs::dv3 Random::Structs::dv3::operator+=(const dv3& vector)
{
	x = x + vector.x;
	y = y + vector.y;
	z = z + vector.z;
	return dv3(x, y, z);
}

Random::Structs::dv3 Random::Structs::dv3::operator-=(const dv3& vector)
{
	x = x - vector.x;
	y = y - vector.y;
	z = z - vector.z;
	return dv3(x, y, z);
}

Random::Structs::dv3 Random::Structs::dv3::operator*=(const dv3& vector)
{
	x = x * vector.x;
	y = y * vector.y;
	z = z * vector.z;
	return dv3(x, y, z);
}

Random::Structs::dv3 Random::Structs::dv3::operator/=(const dv3& vector)
{
	x = x / vector.x;
	y = y / vector.y;
	z = z / vector.z;
	return dv3(x, y, z);
}

Random::Structs::dv3 Random::Structs::dv3::operator+(const dv3& vector)
{
	dv3 t;
	t.x = x + vector.x;
	t.y = y + vector.y;
	t.z = z + vector.z;
	return t;
}

Random::Structs::dv3 Random::Structs::dv3::operator-(const dv3& vector)
{
	dv3 t;
	t.x = x - vector.x;
	t.y = y - vector.y;
	t.z = z - vector.z;
	return t;
}

Random::Structs::dv3 Random::Structs::dv3::operator*(const dv3& vector)
{
	dv3 t;
	t.x = x * vector.x;
	t.y = y * vector.y;
	t.z = z * vector.z;
	return t;
}

Random::Structs::dv3 Random::Structs::dv3::operator/(const dv3& vector)
{
	dv3 t;
	t.x = x / vector.x;
	t.y = y / vector.y;
	t.z = z / vector.z;
	return t;
}

Random::Structs::dv4::dv4()
{
	x = 0;
	y = 0;
	z = 0;
	w = 0;
}

Random::Structs::dv4::dv4(double X, double Y, double Z, double W)
{
	this->x = X;
	this->y = Y;
	this->z = Z;
	this->w = W;
}

Random::Structs::dv4::dv4(double SCALAR)
{
	this->x = SCALAR;
	this->y = SCALAR;
	this->z = SCALAR;
	this->w = SCALAR;
}

Random::Structs::dv4::dv4(const dv4& vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
	w = vector.w;
}

void Random::Structs::dv4::operator=(const dv4& vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
	w = vector.w;
}
Random::Structs::dv4 Random::Structs::dv4::operator+=(const dv4& vector)
{
	x = x + vector.x;
	y = y + vector.y;
	z = z + vector.z;
	w = w + vector.w;
	return dv4(x, y, z, w);
}
Random::Structs::dv4 Random::Structs::dv4::operator-=(const dv4& vector)
{
	x = x - vector.x;
	y = y - vector.y;
	z = z - vector.z;
	w = w - vector.w;
	return dv4(x, y, z, w);
}
Random::Structs::dv4 Random::Structs::dv4::operator*=(const dv4& vector)
{
	x = x * vector.x;
	y = y * vector.y;
	z = z * vector.z;
	w = w * vector.w;
	return dv4(x, y, z, w);
}
Random::Structs::dv4 Random::Structs::dv4::operator/=(const dv4& vector)
{
	x = x / vector.x;
	y = y / vector.y;
	z = z / vector.z;
	w = w / vector.w;
	return dv4(x, y, z, w);
}
Random::Structs::dv4 Random::Structs::dv4::operator+(const dv4& vector)
{
	dv4 t;
	t.x = x + vector.x;
	t.y = y + vector.y;
	t.z = z + vector.z;
	t.w = w + vector.w;
	return t;
}
Random::Structs::dv4 Random::Structs::dv4::operator-(const dv4& vector)
{
	dv4 t;
	t.x = x - vector.x;
	t.y = y - vector.y;
	t.z = z - vector.z;
	t.w = w - vector.w;
	return t;
}
Random::Structs::dv4 Random::Structs::dv4::operator*(const dv4& vector)
{
	dv4 t;
	t.x = x * vector.x;
	t.y = y * vector.y;
	t.z = z * vector.z;
	t.w = w * vector.w;
	return t;
}
Random::Structs::dv4 Random::Structs::dv4::operator/(const dv4& vector)
{
	dv4 t;
	t.x = x / vector.x;
	t.y = y / vector.y;
	t.z = z / vector.z;
	t.w = w / vector.w;
	return t;
}
Random::Structs::Transform::Transform()
{
	Location = v3(0.0f);
	Rotation = r3(0.0f);
	Scale = v3(1.0f);
}

Random::Structs::Transform::Transform(v3 NewLocation, r3 NewRotation, v3 NewScale)
{
	this->Location = NewLocation;
	this->Rotation = NewRotation;
	this->Scale = NewScale;
}

Random::Structs::Transform::Transform(const Transform& T)
{
	Location = T.Location;
	Rotation = T.Rotation;
	Scale = T.Scale;
}

void Random::Structs::Transform::operator=(const Transform& T)
{
	Location = T.Location;
	Rotation = T.Rotation;
	Scale = T.Scale;
}

Random::Structs::r2::r2()
{
	x = 0.0f;
	y = 0.0f;
	ConstrainXaxis = false;
	ConstrainYaxis = false;
}

Random::Structs::r2::r2(float X, float Y)
{
	this->x = (X >= -359 && X <= 359) ? 0 : X;
	this->y = (Y >= -359 && Y <= 359) ? 0 : Y;
}

Random::Structs::r2::r2(float SCALAR)
{
	this->x = (SCALAR >= -359 && SCALAR <= 359) ? 0 : SCALAR;
	this->y = (SCALAR >= -359 && SCALAR <= 359) ? 0 : SCALAR;
}

Random::Structs::r2::r2(const r2& rt)
{
	x = rt.x;
	y = rt.y;
	ConstrainXaxis = rt.ConstrainXaxis;
	ConstrainYaxis = rt.ConstrainYaxis;
	clampX = rt.clampX;
	clampY = rt.clampY;
}

void Random::Structs::r2::operator=(const r2& rt)
{
	x = rt.x;
	y = rt.y;
	ConstrainXaxis = rt.ConstrainXaxis;
	ConstrainYaxis = rt.ConstrainYaxis;
	clampX = rt.clampX;
	clampY = rt.clampY;
}

Random::Structs::r2::r2(float X, bool isConstraintX, SClamp ConstraintX,
							float Y, bool isConstraintY, SClamp ConstrainY)
{
	SClamp AngleConstraint(-359.f, 359.f);
	if (isConstraintX)
		this->x = ConstraintX.clamp(X);
	else
		this->x = AngleConstraint.clamp(X);
	if (isConstraintY)
		this->y = ConstrainY.clamp(Y);
	else
		this->y = AngleConstraint.clamp(Y);
}

void Random::Structs::r2::setConstraints(SClamp CONSTRAIN_X, SClamp CONSTRAIN_Y)
{
	if(ConstrainXaxis)
		this->clampX = CONSTRAIN_X;
	if(ConstrainYaxis)
		this->clampY = CONSTRAIN_Y;
}

void Random::Structs::r2::setConstrainX(SClamp X)
{
	this->clampX = X;
}

void Random::Structs::r2::setConstrainY(SClamp Y)
{
	this->clampY = Y;
}

Random::Structs::r3::r3()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	ConstrainXaxis = false;
	ConstrainYaxis = false;
	ConstrainZaxis = false;
}

Random::Structs::r3::r3(float X, float Y, float Z)
{
	this->x = (X >= -359 && X <= 359) ? 0 : X;
	this->y = (Y >= -359 && Y <= 359) ? 0 : Y;
	this->z = (Z >= -359 && Z <= 359) ? 0 : Z;
}

Random::Structs::r3::r3(float SCALAR)
{
	this->x = (SCALAR >= -359 && SCALAR <= 359) ? 0 : SCALAR;
	this->y = (SCALAR >= -359 && SCALAR <= 359) ? 0 : SCALAR;
	this->y = (SCALAR >= -359 && SCALAR <= 359) ? 0 : SCALAR;
}

Random::Structs::r3::r3(float X, bool isConstraintX, SClamp ConstrainX,
							float Y, bool isConstraintY, SClamp ConstrainY,
							float Z, bool isConstraintZ, SClamp ConstraintZ)
{
	SClamp AngleConstrain(-360.f, 360.f);
	if (isConstraintX)
	{
		this->clampX = ConstrainX;
		this->x = clampX.clamp(X);
	}
	else
		this->x = AngleConstrain.clamp(X);
	if (isConstraintY)
	{
		this->clampY = ConstrainY;
		this->y = clampY.clamp(Y);
	}
	else
		this->y = AngleConstrain.clamp(Y);
	if (isConstraintZ)
	{
		this->clampZ = ConstraintZ;
		this->z = ConstraintZ.clamp(Z);
	}
	else
		this->z = AngleConstrain.clamp(Z);
}

void Random::Structs::r3::setConstraints(SClamp CONSTRAIN_X, SClamp CONSTRAIN_Y, SClamp CONSTRAIN_Z)
{
	this->clampX = CONSTRAIN_X;
	this->clampY = CONSTRAIN_Y;
	this->clampZ = CONSTRAIN_Z;
}

void Random::Structs::r3::setConstrainX(SClamp X)
{
	this->clampX = X;
}

void Random::Structs::r3::setConstrainY(SClamp Y)
{
	this->clampY = Y;
}

void Random::Structs::r3::setConstrainZ(SClamp Z)
{
	this->clampY = Z;
}

Random::Structs::r3::r3(const r3& rt)
{
	x = rt.x;
	y = rt.y;
	z = rt.z;
	ConstrainXaxis = rt.ConstrainXaxis;
	ConstrainYaxis = rt.ConstrainYaxis;
	ConstrainZaxis = rt.ConstrainZaxis;
	clampX = rt.clampX;
	clampY = rt.clampY;
	clampZ = rt.clampZ;
}

void Random::Structs::r3::operator=(const r3& rt)
{
	x = rt.x;
	y = rt.y;
	z = rt.z;
	ConstrainXaxis = rt.ConstrainXaxis;
	ConstrainYaxis = rt.ConstrainYaxis;
	ConstrainZaxis = rt.ConstrainZaxis;
	clampX = rt.clampX;
	clampY = rt.clampY;
	clampZ = rt.clampZ;
}

Random::Structs::c3::c3()
{
	r = 0;
	g = 0;
	b = 0;
}

Random::Structs::c3::c3(int RED, int GREEN, int BLUE)
{
	this->r = (RED <= 0) ? 0 : ((float)RED / 255);
	this->g = (GREEN <= 0) ? 0 : ((float)GREEN / 255);
	this->b = (BLUE <= 0) ? 0 : ((float)BLUE / 255);
}

Random::Structs::c3::c3(int SCALAR)
{
	this->r = (SCALAR <= 0) ? 0 : ((float)SCALAR / 255);
	this->g = (SCALAR <= 0) ? 0 : ((float)SCALAR / 255);
	this->b = (SCALAR <= 0) ? 0 : ((float)SCALAR / 255);
}

Random::Structs::c3::c3(const c3& color)
{
	r = color.r;
	g = color.g;
	b = color.b;
}

void Random::Structs::c3::operator=(const c3& color)
{
	r = color.r;
	g = color.g;
	b = color.b;
}

Random::Structs::c3 Random::Structs::c3::operator*(const c3& color)
{
	c3 temp;
	temp.r = color.r * r;
	temp.g = color.g * g;
	temp.b = color.b * b;
	return temp;
}

Random::Structs::c4::c4()
{
	r = 0;
	g = 0;
	b = 0;
	a = 0;
}

Random::Structs::c4::c4(int RED, int GREEN, int BLUE, int ALPHA)
{
	this->r = (RED <= 0) ? 0 : ((float)RED / 255);
	this->g = (GREEN <= 0) ? 0 : ((float)GREEN / 255);
	this->b = (BLUE <= 0) ? 0 : ((float)BLUE / 255);
	this->a = (ALPHA <= 0) ? 0 : ((float)ALPHA / 255);
}

Random::Structs::c4::c4(int SCALAR)
{
	this->r = (SCALAR <= 0) ? 0 : ((float)SCALAR / 255);
	this->g = (SCALAR <= 0) ? 0 : ((float)SCALAR / 255);
	this->b = (SCALAR <= 0) ? 0 : ((float)SCALAR / 255);
	this->a = (SCALAR <= 0) ? 0 : ((float)SCALAR / 255);
}

Random::Structs::c4::c4(const c4& color)
{
	r = color.r;
	g = color.g;
	b = color.b;
	a = color.a;
}

void Random::Structs::c4::operator=(const c4& color)
{
	r = color.r;
	g = color.g;
	b = color.b;
	a = color.a;
}
Random::Structs::HIconHandler::HIconHandler()
{
	width = 0;
	height = 0;
	pixels = 0;
}

Random::Structs::HIconHandler::HIconHandler(const HIconHandler& h)
{
	width = h.width;
	height = h.height;
	pixels = h.pixels;
}

void Random::Structs::HIconHandler::operator=(const HIconHandler& h)
{
	width = h.width;
	height = h.height;
	pixels = h.pixels;
}
Random::Structs::SWindow::SWindow()
{
	width = 640;
	height = 480;
	title = "My Window Class";
	canSize = true;
}

Random::Structs::SWindow::SWindow(int WIDTH, int HEIGHT, text TITLE, bool CANSIZE)
{
	this->width = WIDTH;
	this->height = HEIGHT;
	this->title = TITLE;
	this->canSize = CANSIZE;
}

Random::Structs::SWindow::SWindow(const SWindow& ws)
{
	width = ws.width;
	height = ws.height;
	title = ws.title;
	canSize = ws.canSize;
}

void Random::Structs::SWindow::operator=(const SWindow& ws)
{
	width = ws.width;
	height = ws.height;
	title = ws.title;
	canSize = ws.canSize;
}
Random::Structs::Context::Context()
{
	major_version = 3;
	minor_version = 3;
	core = true;
	minimal_core = false;
}

Random::Structs::Context::Context(int MAJOR_VERSION, int MINOR_VERSION, bool CORE)
{
	this->major_version = MAJOR_VERSION;
	this->minor_version = MINOR_VERSION;
	this->core = core;
	this->minimal_core = false;
}

Random::Structs::Context::Context(int MAJOR_VERSION, int MINOR_VERSION, bool CORE, bool MINIMAL_CORE)
{
	this->major_version = MAJOR_VERSION;
	this->minor_version = MINOR_VERSION;
	this->core = core;
	this->minimal_core = MINIMAL_CORE;
}

Random::Structs::Context::Context(const Context& cs)
{
	major_version = cs.major_version;
	minor_version = cs.minor_version;
	core = cs.core;
	minimal_core = cs.minimal_core;
}

void Random::Structs::Context::operator=(const Context& cs)
{
	major_version = cs.major_version;
	minor_version = cs.minor_version;
	core = cs.core;
	minimal_core = cs.minimal_core;
}
Random::Structs::Framebuffers::Framebuffers()
{
	flags = 0;
	msaa = false;
}

Random::Structs::Framebuffers::Framebuffers(int FLAG, bool MSAA, bool VSYNC)
{
	this->flags = FLAG;
	this->msaa = MSAA;
	this->vsync = VSYNC;
}

Random::Structs::Framebuffers::Framebuffers(const Framebuffers& fb)
{
	flags = fb.flags;
	msaa = fb.msaa;
}

void Random::Structs::Framebuffers::operator=(const Framebuffers& fb)
{
	flags = fb.flags;
	msaa = fb.msaa;
}

Random::Structs::MouseEvents::MouseEvents()
{
	position = dv2(0.0);
	scroll = dv2(0.0);
	scrollUp = false;
	scrollDown = false;
}

Lightning::MouseEvents::MouseEvents(DV2 MOUSE, DV2 SCROLL)
{
}

Random::Structs::MouseEvents::MouseEvents(dv2 MOUSE, dv2 SCROLL)
{
	this->position = MOUSE;
	this->scroll = SCROLL;
}

Random::Structs::MouseEvents::MouseEvents(double mx, double my, double sx, double sy)
{
	this->position.x = mx;
	this->position.y = my;
	this->scroll.x = sx;
	this->scroll.y = sy;
}

Random::Structs::MouseEvents::MouseEvents(const MouseEvents& m)
{
	position = m.position;
	scroll = m.scroll;
}

void Random::Structs::MouseEvents::operator=(const MouseEvents& m)
{
	position = m.position;
	scroll = m.scroll;
}


Random::Structs::Viewport::Viewport()
{
	this->nearclip = 0.1f;
	this->farclip = 100.0f;
}

Random::Structs::Viewport::Viewport(float new_near_clip, float new_far_clip)
{
	this->nearclip = new_near_clip;
	this->farclip = new_far_clip;
}

Random::Structs::Viewport::Viewport(const Viewport& vp)
{
	nearclip = vp.nearclip;
	farclip = vp.farclip;
}

void Random::Structs::Viewport::operator=(const Viewport& vp)
{
	nearclip = vp.nearclip;
	farclip = vp.farclip;
}


Random::Structs::BounderBox2D::BounderBox2D()
{
	boxLimits = v2(0.0f);
}

Random::Structs::BounderBox2D::BounderBox2D(float width, float height)
{
	this->boxLimits.x = width;
	this->boxLimits.y = height;
}

Random::Structs::BounderBox2D::BounderBox2D(const BounderBox2D& bb)
{
	boxLimits = bb.boxLimits;
}

void Random::Structs::BounderBox2D::operator=(const BounderBox2D& bb)
{
	boxLimits = bb.boxLimits;
}

Random::Structs::BounderBox3D::BounderBox3D()
{
	boxLimits = v3(0.0f);
}

Random::Structs::BounderBox3D::BounderBox3D(Transform T)
{
}

Random::Structs::BounderBox3D::BounderBox3D(const Transform& T)
{
}

void Random::Structs::BounderBox3D::operator=(const Transform& T)
{
}

float Random::randomInterval(float minInterval, float maxInterval)
{
	std::default_random_engine gen;
	std::uniform_real_distribution<double> dist(minInterval, maxInterval);
	return dist(gen);
}

Random::Structs::v3 Random::Structs::Arrow::ForwardVectorv3()
{
	return v3(ForwardVector, 0.0f, 0.0f);
}

Random::Structs::v3 Random::Structs::Arrow::RightVectorv3()
{
	return v3(0.0f, 0.0f, RightVector);
}

Random::Structs::v3 Random::Structs::Arrow::UpVectorv3()
{
	return v3(0.0f, UpVector, 0.0f);
}

void Random::Structs::Global::Timer::setTime(double newTime)
{
	this->time = newTime;
	//std::cout << "TIME IS: " << (int)time << '\n';
	setDeltaTime();
	//std::cout << "Delta Time is: " << (float)deltaTime << '\n';
}

double Random::Structs::Global::Timer::getTimeSeconds()
{
	return time;
}

void Random::Structs::Global::Timer::setDeltaTime()
{
	CurrentFrame = time;
	deltaTime = CurrentFrame - LastFrame;
	LastFrame = CurrentFrame;
}

Random::Structs::Global::Global()
{
}

float Random::Math::radians(float degrees)
{
	return degrees * RDN_RADIANS;
}

float Random::Math::degrees(float radians)
{
	return radians * RDN_DEGREES;
}