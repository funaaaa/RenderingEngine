#pragma once
#include "Vec.h"

namespace Math {

	struct Transform3D
	{

		Vec3<float> pos;
		Vec3<float> scale;
		DirectX::XMVECTOR quaternion;

		Transform3D() :
			pos(Vec3<float>(0.0f, 0.0f, 0.0f)),
			scale(Vec3<float>(1.0f, 1.0f, 1.0f)),
			quaternion(DirectX::XMQuaternionIdentity())
		{
		};
		Transform3D(const Vec3<float>& arg_pos) :
			pos(arg_pos),
			scale(Vec3<float>(1.0f, 1.0f, 1.0f)),
			quaternion(DirectX::XMQuaternionIdentity())
		{
		};
		Transform3D(const Vec3<float>& arg_pos, const Vec3<float>& arg_scale) :
			pos(arg_pos),
			scale(arg_scale),
			quaternion(DirectX::XMQuaternionIdentity())
		{
		};

		//XMVECTORをVec3<float>に変換
		Vec3<float> TransformVec3(DirectX::XMVECTOR arg_input) {
			return Vec3<float>(arg_input.m128_f32[0], arg_input.m128_f32[1], arg_input.m128_f32[2]);
		}

		//姿勢から各方向ベクトルを取得
		Vec3<float> GetRight() { return TransformVec3(DirectX::XMVector3Transform({ 1,0,0 }, DirectX::XMMatrixRotationQuaternion(quaternion))); }
		Vec3<float> GetUp() { return TransformVec3(DirectX::XMVector3Transform({ 0,1,0 }, DirectX::XMMatrixRotationQuaternion(quaternion))); }
		Vec3<float> GetFront() { return TransformVec3(DirectX::XMVector3Transform({ 0,0,1 }, DirectX::XMMatrixRotationQuaternion(quaternion))); }

		//姿勢に任意のベクトル軸の回転をかける。(加算する感じ)
		void Rotation(Vec3<float> arg_axis, float arg_radian) {
			quaternion = DirectX::XMQuaternionMultiply(quaternion, DirectX::XMQuaternionRotationAxis(arg_axis.ConvertXMVECTOR(), arg_radian));
		}

		void operator=(const Transform3D& arg_obj)
		{
			pos = arg_obj.pos;
			scale = arg_obj.scale;
			quaternion = arg_obj.quaternion;
		}

	};

}