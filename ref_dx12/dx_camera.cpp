#include "dx_camera.h"
#include "dx_utils.h"
#include "dx_app.h"

#include <limits>

#ifdef max
#undef max
#endif  

#ifdef min
#undef min
#endif  



void Camera::Init()
{
	Renderer::Inst().GetDrawAreaSize(&width, &height);
}

void Camera::Update(const refdef_t& updateData)
{
	position.x = updateData.vieworg[0];
	position.y = updateData.vieworg[1];
	position.z = updateData.vieworg[2];

	viewangles.x = updateData.viewangles[0];
	viewangles.y = updateData.viewangles[1];
	viewangles.z = updateData.viewangles[2];

	fov.x = updateData.fov_x;
	fov.y = updateData.fov_y;

	width = updateData.width;
	height = updateData.height;
}

XMMATRIX Camera::GenerateViewMatrix() const 
{
	// The reason we have this weird pre transformation here is the difference
	// between id's software coordinate system and DirectX coordinate system
	//
	// id's system:
	//	- X axis = Left/Right
	//	- Y axis = Forward/Backward
	//	- Z axis = Up/Down
	//
	// DirectX coordinate system:
	//	- X axis = Left/Right
	//	- Y axis = Up/Down
	//	- Z axis = Forward/Backward

	XMMATRIX sseViewMat =
		XMMatrixTranslation(-position.x, -position.y, -position.z) *

		XMMatrixRotationAxis(XMLoadFloat4(&Utils::axisZ), XMConvertToRadians(-viewangles.y)) *
		XMMatrixRotationAxis(XMLoadFloat4(&Utils::axisY), XMConvertToRadians(-viewangles.x)) *
		XMMatrixRotationAxis(XMLoadFloat4(&Utils::axisX), XMConvertToRadians(-viewangles.z)) *

		XMMatrixRotationAxis(XMLoadFloat4(&Utils::axisZ), XMConvertToRadians(90.0f)) *
		XMMatrixRotationAxis(XMLoadFloat4(&Utils::axisX), XMConvertToRadians(-90.0f));

	return sseViewMat;

}

XMMATRIX Camera::GenerateProjectionMatrix() const
{
	constexpr int zNear = 4;
	constexpr int zFar = 4096;
	
	return XMMatrixPerspectiveFovRH(XMConvertToRadians(std::max(fov.y, 1.0f)), width / height, zNear, zFar);
}


std::tuple<XMFLOAT4, XMFLOAT4, XMFLOAT4> Camera::GetBasis() const
{
	// Extract basis from view matrix
	XMFLOAT4X4 viewMat;
	XMStoreFloat4x4(&viewMat, XMMatrixTranspose(GenerateViewMatrix()));

	XMFLOAT4 yaw = *reinterpret_cast<XMFLOAT4*>(viewMat.m[1]);
	XMFLOAT4 pitch =* reinterpret_cast<XMFLOAT4*>(viewMat.m[0]);
	XMFLOAT4 roll;
	// Initially point left, we need right
	XMStoreFloat4(&roll, XMVectorScale(
		XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(viewMat.m[2])),
		-1.0f));

	// Eliminate transpose factor
	yaw.w = pitch.w = roll.w = 0.0f;

	return std::make_tuple(yaw, pitch, roll);
}

std::tuple<XMFLOAT4, XMFLOAT4> Camera::GetAABBInWorldSpace() const
{
	std::array<XMVECTOR, 8> frustum = 
	{
		XMVectorSet(-1.0f, -1.0f, 0.0f, 1.0f ),
		XMVectorSet(-1.0f,  1.0f, 0.0f, 1.0f),
		XMVectorSet(1.0f,  1.0f, 0.0f, 1.0f),
		XMVectorSet(1.0f, -1.0f, 0.0f, 1.0f),
		XMVectorSet(-1.0f, -1.0f, 1.0f, 1.0f),
		XMVectorSet(-1.0f,  1.0f, 1.0f, 1.0f),
		XMVectorSet(1.0f,  1.0f, 1.0f, 1.0f),
		XMVectorSet(1.0f, -1.0f, 1.0f, 1.0f)
	};

	XMVECTOR sseCameraTransformDeterminant;
	const XMMATRIX sseCameraInvTransform = XMMatrixInverse(&sseCameraTransformDeterminant,
		XMMatrixMultiply(GenerateViewMatrix(), GenerateProjectionMatrix()));

	assert(XMVectorGetX(sseCameraTransformDeterminant) != 0.0f && "Camera transform inv can't be found. Determinant is zero");

	constexpr float MIN_FLOAT = std::numeric_limits<float>::min();
	constexpr float MAX_FLOAT = std::numeric_limits<float>::max();

	XMVECTOR sseBBMin = XMVectorSet(MAX_FLOAT, MAX_FLOAT, MAX_FLOAT, 1.0f);
	XMVECTOR sseBBMax = XMVectorSet(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT, 1.0f);

	std::for_each(frustum.cbegin(), frustum.cend(),
		[&sseBBMin, &sseBBMax, sseCameraInvTransform](const XMVECTOR& fPoint) 
	{
		XMVECTOR sseFPointInWorldSpace = XMVector4Transform(fPoint, sseCameraInvTransform);
		const float w = XMVectorGetW(sseFPointInWorldSpace);

		sseFPointInWorldSpace = XMVectorDivide(sseFPointInWorldSpace, XMVectorSet(w, w, w, w));

		sseBBMin = XMVectorMin(sseBBMin, sseFPointInWorldSpace);
		sseBBMax = XMVectorMax(sseBBMax, sseFPointInWorldSpace);
	});

	XMFLOAT4 bbMin, bbMax;
	XMStoreFloat4(&bbMin, sseBBMin);
	XMStoreFloat4(&bbMax, sseBBMax);

	return std::make_tuple(bbMin, bbMax);
}
