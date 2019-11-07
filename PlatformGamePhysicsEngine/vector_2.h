#pragma once

#include <iostream>
#include <math.h>

using namespace std;

template<typename T>
struct Vector2
{
	T x;
	T y;

	Vector2();
	Vector2(const Vector2& src);
	Vector2(T x, T y);

	Vector2 operator+(const Vector2& other) const;
	Vector2 operator-(const Vector2& other) const;
	Vector2& operator+=(const Vector2& other);
	Vector2& operator-=(const Vector2& other);
	Vector2& operator/=(T c);
	Vector2 operator*(T c) const;
	Vector2 operator/(T c) const;
	bool operator!=(const Vector2& other) const;

	T dot(const Vector2& other) const;

	T compute_length() const;

	Vector2& normalize();
	Vector2 normalized() const;
	Vector2 ortho() const;

	Vector2& lerp(const Vector2& other, T c);
};

template<typename T>
inline Vector2<T>::Vector2(const Vector2& src) :
	x(src.x),
	y(src.y)
{
}

template<typename T>
inline Vector2<T>::Vector2(T x, T y) :
	x(x),
	y(y)
{
}

template<typename T>
inline Vector2<T> Vector2<T>::operator+(const Vector2<T>& other) const
{
	Vector2<T> r;

	r.x = this->x + other.x;
	r.y = this->y + other.y;

	return r;
}

template<typename T>
inline Vector2<T> Vector2<T>::operator-(const Vector2<T>& other) const
{
	Vector2<T> r;

	r.x = this->x - other.x;
	r.y = this->y - other.y;

	return r;
}

template<typename T>
inline Vector2<T>& Vector2<T>::operator+=(const Vector2<T>& other)
{
	this->x += other.x;
	this->y += other.y;

	return *this;
}

template<typename T>
inline Vector2<T>& Vector2<T>::operator-=(const Vector2<T>& other)
{
	this->x -= other.x;
	this->y -= other.y;

	return *this;
}

template<typename T>
inline Vector2<T>& Vector2<T>::operator/=(T c)
{
	this->x /= c;
	this->y /= c;

	return *this;
}

template<typename T>
inline Vector2<T> Vector2<T>::operator*(T c) const
{
	Vector2<T> r;

	r.x = this->x * c;
	r.y = this->y * c;

	return r;
}

template<typename T>
inline Vector2<T> Vector2<T>::operator/(T c) const
{
	Vector2<T> r;

	r.x = this->x / c;
	r.y = this->y / c;

	return r;
}

template<typename T>
inline bool Vector2<T>::operator!=(const Vector2& other) const
{
	return this->x != other.x || this->y != other.y;
}

template<typename T>
inline T Vector2<T>::dot(const Vector2<T>& other) const
{
	return this->x * other.x + this->y * other.y;
}

template<typename T>
inline Vector2<T> Vector2<T>::ortho() const
{
	Vector2 result;

	result.x = this->y;
	result.y = -this->x;

	return result;
}

template<typename T>
inline Vector2<T>& Vector2<T>::lerp(const Vector2<T>& other, T c)
{
	this->x += (other.x - this->x) * c;
	this->y += (other.y - this->y) * c;

	return *this;
}

template<typename T>
inline ostream& operator<<(ostream& stream, const Vector2<T>& vector)
{
	stream << "Vector2(" << vector.x << ", " << vector.y << ")";

	return stream;
}

typedef Vector2<float> Vector2f;
