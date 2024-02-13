#pragma once
#include "Core.h"

namespace ProdCast {
	/*
	NON WORKING TEMPLATE CODE (TODO: FIX)

	template<typename T>
	struct Vec3 {
		Vec3() = default;
		Vec3(T x, T y, T z) : X(x), Y(y), Z(z) {
		}
		T X;
		T Y;
		T Z;

		template<typename U>
		friend Vec3<T> operator- (Vec3<U> left, const Vec3<T>& right) {
			return Vec3<T>(left.X - right.X, left.Y - right.Y, left.Z - right.Z);
		}
	};

	template<typename T>
	Vec3<T> Cross(Vec3<T> x, Vec3<T> y) {
		return Vec3<T>(x.Y * y.Z - y.Y * x.Z,
			x.Z * y.X - y.Z * x.X,
			x.X * y.Y - y.X * x.Y);
	}

	template<typename T>
	float Dot(Vec3<T> x, Vec3<T> y) {
		return x.X * y.X + x.Y * y.Y + x.Z * y.Z;
	}

	template<typename T>
	Vec3<T> Normalize(Vec3<T> vec) {
		float length = sqrtf((vec.X * vec.X) + (vec.Y * vec.Y) + (vec.Z * vec.Z));
		return Vec3<T>(vec.X * length, vec.Y * length, vec.Z * length);
	}
	*/
	
	struct Vec3 {
		Vec3() = default;
		Vec3(float x, float y, float z) : X(x), Y(y), Z(z) {
		}
		float X;
		float Y;
		float Z;

		Vec3 operator- (const Vec3& right);
	};

	Vec3 Cross(Vec3 x, Vec3 y);

	float Dot(Vec3 x, Vec3 y);

	Vec3 Normalize(Vec3 vec);
}

