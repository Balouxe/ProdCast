#include "Utils/Vec3.h"

namespace ProdCast {
	Vec3 Vec3::operator- (const Vec3& right) {
		return Vec3(this->X - right.X, this->Y - right.Y, this->Z - right.Z);
	}

	Vec3 Cross(Vec3 x, Vec3 y) {
		return Vec3(x.Y * y.Z - y.Y * x.Z,
			x.Z * y.X - y.Z * x.X,
			x.X * y.Y - y.X * x.Y);
	}

	float Dot(Vec3 x, Vec3 y) {
		return x.X * y.X + x.Y * y.Y + x.Z * y.Z;
	}

	Vec3 Normalize(Vec3 vec) {
		float length = sqrtf((vec.X * vec.X) + (vec.Y * vec.Y) + (vec.Z * vec.Z));
		return Vec3(vec.X * length, vec.Y * length, vec.Z * length);
	}
}