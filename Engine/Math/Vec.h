#pragma once
#include <DirectXMath.h>

namespace Math {

#pragma region Vec2
	template<typename T>
	struct Vec2
	{
		T x, y;

		Vec2()
		{
			x = 0;
			y = 0;
		};
		Vec2(T X, T Y) :x(X), y(Y) {};

		float Length()const
		{
			return sqrtf(powf(x, 2) + powf(y, 2));
		};
		float Distance(const Vec2& To)const
		{
			return sqrt(pow(To.x - x, 2) + pow(To.y - y, 2));
		};
		Vec2<float> GetNormal()const
		{
			float len = Length();
			if (len == 0.0f)return Vec2<float>(0.0, 0.0);
			return Vec2<float>(x / len, y / len);
		};
		void Normalize()
		{
			float len = Length();
			x /= len;
			y /= len;
		};
		DirectX::XMFLOAT2 ConvertXMFLOAT2()const
		{
			return DirectX::XMFLOAT2(static_cast<float>(x), static_cast<float>(y));
		};
		DirectX::XMVECTOR ConvertXMVECTOR()const
		{
			return DirectX::XMVECTOR({ static_cast<float>(x), static_cast<float>(y), 0.0f, 0.0f });
		};

		Vec2<int>Int()const
		{
			return Vec2<int>(static_cast<int>(x), static_cast<int>(y));
		}
		Vec2<float>Float()const
		{
			return Vec2<float>((float)x, (float)y);
		}

		float Dot(const Vec2<float>& rhs)const
		{
			auto me = Float();
			return me.x * rhs.x + me.y * rhs.y;
		}
		float Cross(const Vec2<float>& rhs)const
		{
			auto me = Float();
			return me.x * rhs.y - me.y * rhs.x;
		}

#pragma region オペレーター演算子
		Vec2 operator-() const
		{
			return Vec2(-x, -y);
		}
		Vec2 operator+(const Vec2& rhs) const
		{
			return Vec2(x + rhs.x, y + rhs.y);
		};
		Vec2 operator+(const float& rhs) const
		{
			return Vec2(x + rhs, y + rhs);
		};
		Vec2 operator-(const Vec2& rhs)const
		{
			return Vec2(x - rhs.x, y - rhs.y);
		};
		Vec2 operator*(const Vec2& rhs)const
		{
			return Vec2(x * rhs.x, y * rhs.y);
		};
		Vec2 operator*(const float& rhs)const
		{
			return Vec2(x * rhs, y * rhs);
		};
		Vec2 operator/(const Vec2& rhs)const
		{
			return Vec2(x / rhs.x, y / rhs.y);
		};
		Vec2 operator/(const float& rhs)const
		{
			return Vec2(x / rhs, y / rhs);
		};
		Vec2 operator%(const Vec2& rhs) const
		{
			return Vec2(fmodf(x, rhs.x), fmodf(y, rhs.y));
		};
		void operator=(const Vec2& rhs)
		{
			x = rhs.x;
			y = rhs.y;
		};
		bool operator==(const Vec2& rhs)const
		{
			return (x == rhs.x && y == rhs.y);
		};
		bool operator!=(const Vec2& rhs)const
		{
			return !(*this == rhs);
		};
		void operator+=(const Vec2& rhs)
		{
			x += rhs.x;
			y += rhs.y;
		};
		void operator-=(const Vec2& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
		};
		void operator*=(const Vec2& rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
		};
		void operator/=(const Vec2& rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
		};
		void operator%=(const Vec2& rhs)
		{
			x = fmodf(x, rhs.x);
			y = fmodf(y, rhs.y);
		};

		void operator+=(const float& rhs)
		{
			x += rhs;
			y += rhs;
		};
		void operator-=(const float& rhs)
		{
			x -= rhs;
			y -= rhs;
		};
		void operator*=(const float& rhs)
		{
			x *= rhs;
			y *= rhs;
		};
		void operator/=(const float& rhs)
		{
			x /= rhs;
			y /= rhs;
		};
		void operator%=(const float& rhs)
		{
			x = fmodf(x, rhs);
			y = fmodf(y, rhs);
		};

		bool operator<(const Vec2<T>& rhs)
		{
			return x < rhs.x&& y < rhs.y;
		}
#pragma endregion
	};
#pragma endregion

#pragma region Vec3
	template<typename T>
	struct Vec3
	{
		T x, y, z;

		Vec3()
		{
			x = 0;
			y = 0;
			z = 0;
		};
		Vec3(T X, T Y, T Z) :x(X), y(Y), z(Z) {};
		float Length() const
		{
			return static_cast<float>(sqrt(pow(x, 2.0f) + pow(y, 2.0f) + pow(z, 2.0f)));
		};
		Vec3(Vec2<T>XY, T Z) :x(XY.x), y(XY.y), z(Z) {};

		float Distance(const Vec3& To)const
		{
			Vec3 lpow;
			lpow.x = { static_cast<float>(pow(To.x - x, 2.0f)) };
			lpow.y = { static_cast<float>(pow(To.y - y, 2.0f)) };
			lpow.z = { static_cast<float>(pow(To.z - z, 2.0f)) };


			return static_cast<float>(sqrt(lpow.x + lpow.y + lpow.z));
		};
		Vec3<float> GetNormal()const
		{
			float len = Length();
			if (len == 0.0f)return Vec3(0, 0, 0);
			return Vec3<float>(x / len, y / len, z / len);
		};
		void Normalize()
		{
			float len = Length();
			x /= len;
			y /= len;
			z /= len;
		};
		DirectX::XMFLOAT3 ConvertXMFLOAT3()const
		{
			return DirectX::XMFLOAT3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
		};
		DirectX::XMFLOAT4 ConvertXMFLOAT4()const
		{
			return DirectX::XMFLOAT4(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 0.0f);
		};
		DirectX::XMVECTOR ConvertXMVECTOR()const
		{
			DirectX::XMVECTOR result = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 0.0f };
			return result;
		};
		Vec2<T> ConvertVec2()const
		{
			Vec2<T> result = { x,y };
			return result;
		};

		Vec3<int>Int()const
		{
			return Vec3<int>(x, y, z);
		}
		Vec3<float>Float()const
		{
			return Vec3<float>((float)x, (float)y, (float)z);
		}

		float Dot(const Vec3<float>& rhs)const
		{
			auto me = Float();
			return me.x * rhs.x + me.y * rhs.y + me.z * rhs.z;
		}
		Vec3<float>Cross(const Vec3<float>& rhs)const
		{
			auto me = Float();
			return Vec3<float>(
				me.y * rhs.z - rhs.y * me.z,
				me.z * rhs.x - rhs.z * me.x,
				me.x * rhs.y - rhs.x * me.y);
		}

		void Abs()
		{
			x = fabs(x);
			y = fabs(y);
			z = fabs(z);
		}

#pragma region オペレーター演算子
		Vec3 operator-()const
		{
			return Vec3(-x, -y, -z);
		}
		Vec3 operator+(const Vec3& rhs)const
		{
			return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
		};
		Vec3 operator+(float rhs)const
		{
			return Vec3(x + rhs, y + rhs, z + rhs);
		};
		Vec3 operator-(const Vec3& rhs)const
		{
			return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
		};
		Vec3 operator*(const Vec3& rhs) const
		{
			return Vec3(x * rhs.x, y * rhs.y, z * rhs.z);
		};
		Vec3 operator*(const float& rhs)const
		{
			return Vec3(x * rhs, y * rhs, z * rhs);
		};
		Vec3 operator/(const Vec3& rhs)const
		{
			return Vec3(x / rhs.x, y / rhs.y, z / rhs.z);
		};
		Vec3 operator/(const float& rhs)const
		{
			return Vec3(x / rhs, y / rhs, z / rhs);
		};
		Vec3 operator%(const Vec3& rhs)const
		{
			return Vec3(fmodf(x, rhs.x), fmodf(y, rhs.y), fmodf(z, rhs.z));
		};
		void operator=(const Vec3& rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
		};
		bool operator==(const Vec3& rhs)const
		{
			return (x == rhs.x && y == rhs.y && z == rhs.z);
		};
		bool operator!=(const Vec3& rhs)const
		{
			return !(*this == rhs);
		};
		void operator+=(const Vec3& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
		};
		void operator+=(const float& rhs)
		{
			x += rhs;
			y += rhs;
			z += rhs;
		};
		void operator-=(const Vec3& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
		};
		void operator-=(const float& rhs)
		{
			x -= rhs;
			y -= rhs;
			z -= rhs;
		};
		void operator*=(const Vec3& rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
		};
		void operator*=(const float& rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
		};
		void operator/=(const Vec3& rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
		};
		void operator/=(const float& rhs)
		{
			x /= rhs;
			y /= rhs;
			z /= rhs;
		};
		void operator%=(const Vec3& rhs)
		{
			x = fmodf(x, rhs.x);
			y = fmodf(y, rhs.y);
			z = fmodf(z, rhs.z);
		};

#pragma endregion
	};
#pragma endregion

#pragma region Vec4
	template<typename T>
	struct Vec4
	{
		T x, y, z, a;

		Vec4()
		{
			x = 0;
			y = 0;
			z = 0;
			a = 0;
		};
		Vec4(T X, T Y, T Z, T A) :x(X), y(Y), z(Z), a(A) {};

		DirectX::XMFLOAT3 ConvertXMFLOAT3()const
		{
			return DirectX::XMFLOAT3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
		};
		DirectX::XMFLOAT4 ConvertXMFLOAT4()const
		{
			return DirectX::XMFLOAT4(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(a));
		};
		DirectX::XMVECTOR ConvertXMVECTOR()
		{
			DirectX::XMVECTOR result = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(a) };
			return result;
		};

		Vec4<int>Int()const
		{
			return Vec4<int>(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z), static_cast<int>(a));
		}
		Vec4<float>Float()const
		{
			return Vec4<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(a));
		}

		void Abs()
		{
			x = fabs(x);
			y = fabs(y);
			z = fabs(z);
			a = fabs(a);
		}

#pragma region オペレーター演算子
		Vec4 operator-()const
		{
			return Vec4(-x, -y, -z);
		}
		Vec4 operator+(const Vec4& rhs)const
		{
			return Vec4(x + rhs.x, y + rhs.y, z + rhs.z, a + rhs.a);
		};
		Vec4 operator-(const Vec4& rhs)const
		{
			return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, a - rhs.a);
		};
		Vec4 operator*(const Vec4& rhs) const
		{
			return Vec4(x * rhs.x, y * rhs.y, z * rhs.z, a * rhs.a);
		};
		Vec4 operator*(const float& rhs)const
		{
			return Vec4(x * rhs, y * rhs, z * rhs, a * rhs);
		};
		Vec4 operator/(const Vec4& rhs)const
		{
			return Vec4(x / rhs.x, y / rhs.y, z / rhs.z);
		};
		Vec4 operator/(const float& rhs)const
		{
			return Vec4(x / rhs, y / rhs, z / rhs, a / rhs);
		};
		Vec4 operator%(const Vec4& rhs)const
		{
			return Vec4(fmodf(x, rhs.x), fmodf(y, rhs.y), fmodf(z, rhs.z));
		};
		void operator=(const Vec4& rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
			a = rhs.a;
		};
		bool operator==(const Vec4& rhs)const
		{
			return (x == rhs.x && y == rhs.y && z == rhs.z && a == rhs.a);
		};
		bool operator!=(const Vec4& rhs)const
		{
			return !(x == rhs.x && y == rhs.y && z == rhs.z && a == rhs.a);
		};
		void operator+=(const Vec4& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
		};
		void operator+=(const float& rhs)
		{
			x += rhs;
			y += rhs;
			z += rhs;
		};
		void operator-=(const Vec4& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
		};
		void operator-=(const float& rhs)
		{
			x -= rhs;
			y -= rhs;
			z -= rhs;
		};
		void operator*=(const Vec4& rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
		};
		void operator*=(const float& rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
		};
		void operator/=(const Vec4& rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
		};
		void operator/=(const float& rhs)
		{
			x /= rhs;
			y /= rhs;
			z /= rhs;
		};
		void operator%=(const Vec4& rhs)
		{
			x = fmodf(x, rhs.x);
			y = fmodf(y, rhs.y);
			z = fmodf(z, rhs.z);
		};
#pragma endregion
	};
#pragma endregion

}