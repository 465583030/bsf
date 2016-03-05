//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCamera.h"
#include "BsCameraRTTI.h"
#include "BsMath.h"
#include "BsMatrix3.h"
#include "BsVector2.h"
#include "BsAABox.h"
#include "BsSphere.h"
#include "BsException.h"
#include "BsRenderAPI.h"
#include "BsSceneObject.h"
#include "BsDebug.h"
#include "BsRendererManager.h"
#include "BsCoreRenderer.h"
#include "BsFrameAlloc.h"

namespace BansheeEngine
{
	const float CameraBase::INFINITE_FAR_PLANE_ADJUST = 0.00001f;

	CameraBase::CameraBase()
		:mProjType(PT_PERSPECTIVE), mHorzFOV(Degree(90.0f)), mFarDist(1000.0f),
		mNearDist(0.05f), mAspect(1.33333333333333f), mOrthoHeight(5), mRecalcFrustum(true), mRecalcFrustumPlanes(true),
		mCustomViewMatrix(false), mCustomProjMatrix(false), mFrustumExtentsManuallySet(false), mPriority(0), 
		mLayers(0xFFFFFFFFFFFFFFFF), mRecalcView(true), mCameraFlags(0), mIsActive(true)
	{
		mViewMatrix = Matrix4::ZERO;
		mProjMatrixRS = Matrix4::ZERO;
		mViewMatrixInv = Matrix4::ZERO;
		mProjMatrixRSInv = Matrix4::ZERO;
		mProjMatrixInv = Matrix4::ZERO;

		invalidateFrustum();
	}

	void CameraBase::setHorzFOV(const Radian& fov)
	{
		mHorzFOV = fov;
		invalidateFrustum();
		_markCoreDirty();
	}

	const Radian& CameraBase::getHorzFOV() const
	{
		return mHorzFOV;
	}

	void CameraBase::setFarClipDistance(float farPlane)
	{
		mFarDist = farPlane;
		invalidateFrustum();
		_markCoreDirty();
	}

	float CameraBase::getFarClipDistance() const
	{
		return mFarDist;
	}

	void CameraBase::setNearClipDistance(float nearPlane)
	{
		if (nearPlane <= 0)
		{
			BS_EXCEPT(InvalidParametersException, "Near clip distance must be greater than zero.");
		}

		mNearDist = nearPlane;
		invalidateFrustum();
		_markCoreDirty();
	}

	float CameraBase::getNearClipDistance() const
	{
		return mNearDist;
	}

	const Matrix4& CameraBase::getProjectionMatrix() const
	{
		updateFrustum();

		return mProjMatrix;
	}

	const Matrix4& CameraBase::getProjectionMatrixInv() const
	{
		updateFrustum();

		return mProjMatrixInv;
	}

	const Matrix4& CameraBase::getProjectionMatrixRS() const
	{
		updateFrustum();

		return mProjMatrixRS;
	}

	const Matrix4& CameraBase::getProjectionMatrixRSInv() const
	{
		updateFrustum();

		return mProjMatrixRSInv;
	}

	const Matrix4& CameraBase::getViewMatrix() const
	{
		updateView();

		return mViewMatrix;
	}

	const Matrix4& CameraBase::getViewMatrixInv() const
	{
		updateView();

		return mViewMatrixInv;
	}

	const ConvexVolume& CameraBase::getFrustum() const
	{
		// Make any pending updates to the calculated frustum planes
		updateFrustumPlanes();

		return mFrustum;
	}

	ConvexVolume CameraBase::getWorldFrustum() const
	{
		const Vector<Plane>& frustumPlanes = getFrustum().getPlanes();
		Matrix4 worldMatrix;
		worldMatrix.setTRS(mPosition, mRotation, Vector3::ONE);

		Vector<Plane> worldPlanes(frustumPlanes.size());
		UINT32 i = 0;
		for (auto& plane : frustumPlanes)
		{
			worldPlanes[i] = worldMatrix.multiplyAffine(plane);
			i++;
		}

		return ConvexVolume(worldPlanes);
	}

	void CameraBase::calcProjectionParameters(float& left, float& right, float& bottom, float& top) const
	{
		if (mCustomProjMatrix)
		{
			// Convert clipspace corners to camera space
			Matrix4 invProj = mProjMatrix.inverse();
			Vector3 topLeft(-0.5f, 0.5f, 0.0f);
			Vector3 bottomRight(0.5f, -0.5f, 0.0f);

			topLeft = invProj.multiply(topLeft);
			bottomRight = invProj.multiply(bottomRight);

			left = topLeft.x;
			top = topLeft.y;
			right = bottomRight.x;
			bottom = bottomRight.y;
		}
		else
		{
			if (mFrustumExtentsManuallySet)
			{
				left = mLeft;
				right = mRight;
				top = mTop;
				bottom = mBottom;
			}
			else if (mProjType == PT_PERSPECTIVE)
			{
				Radian thetaX(mHorzFOV * 0.5f);
				float tanThetaX = Math::tan(thetaX);
				float tanThetaY = tanThetaX / mAspect;

				float half_w = tanThetaX * mNearDist;
				float half_h = tanThetaY * mNearDist;

				left = -half_w;
				right = half_w;
				bottom = -half_h;
				top = half_h;

				mLeft = left;
				mRight = right;
				mTop = top;
				mBottom = bottom;
			}
			else
			{
				float half_w = getOrthoWindowWidth() * 0.5f;
				float half_h = getOrthoWindowHeight() * 0.5f;

				left = -half_w;
				right = half_w;
				bottom = -half_h;
				top = half_h;

				mLeft = left;
				mRight = right;
				mTop = top;
				mBottom = bottom;
			}
		}
	}

	void CameraBase::updateFrustum() const
	{
		if (isFrustumOutOfDate())
		{
			float left, right, bottom, top;

			calcProjectionParameters(left, right, bottom, top);

			if (!mCustomProjMatrix)
			{
				float inv_w = 1 / (right - left);
				float inv_h = 1 / (top - bottom);
				float inv_d = 1 / (mFarDist - mNearDist);

				if (mProjType == PT_PERSPECTIVE)
				{
					float A = 2 * mNearDist * inv_w;
					float B = 2 * mNearDist * inv_h;
					float C = (right + left) * inv_w;
					float D = (top + bottom) * inv_h;
					float q, qn;

					if (mFarDist == 0)
					{
						// Infinite far plane
						q = CameraBase::INFINITE_FAR_PLANE_ADJUST - 1;
						qn = mNearDist * (CameraBase::INFINITE_FAR_PLANE_ADJUST - 2);
					}
					else
					{
						q = -(mFarDist + mNearDist) * inv_d;
						qn = -2 * (mFarDist * mNearDist) * inv_d;
					}

					mProjMatrix = Matrix4::ZERO;
					mProjMatrix[0][0] = A;
					mProjMatrix[0][2] = C;
					mProjMatrix[1][1] = B;
					mProjMatrix[1][2] = D;
					mProjMatrix[2][2] = q;
					mProjMatrix[2][3] = qn;
					mProjMatrix[3][2] = -1;
				}
				else if (mProjType == PT_ORTHOGRAPHIC)
				{
					float A = 2 * inv_w;
					float B = 2 * inv_h;
					float C = -(right + left) * inv_w;
					float D = -(top + bottom) * inv_h;
					float q, qn;

					if (mFarDist == 0)
					{
						// Can not do infinite far plane here, avoid divided zero only
						q = -CameraBase::INFINITE_FAR_PLANE_ADJUST / mNearDist;
						qn = -CameraBase::INFINITE_FAR_PLANE_ADJUST - 1;
					}
					else
					{
						q = -2 * inv_d;
						qn = -(mFarDist + mNearDist)  * inv_d;
					}

					mProjMatrix = Matrix4::ZERO;
					mProjMatrix[0][0] = A;
					mProjMatrix[0][3] = C;
					mProjMatrix[1][1] = B;
					mProjMatrix[1][3] = D;
					mProjMatrix[2][2] = q;
					mProjMatrix[2][3] = qn;
					mProjMatrix[3][3] = 1;
				}
			}

			RenderAPICore* renderAPI = BansheeEngine::RenderAPICore::instancePtr();
			renderAPI->convertProjectionMatrix(mProjMatrix, mProjMatrixRS);
			mProjMatrixInv = mProjMatrix.inverse();
			mProjMatrixRSInv = mProjMatrixRS.inverse();

			// Calculate bounding box (local)
			// Box is from 0, down -Z, max dimensions as determined from far plane
			// If infinite view frustum just pick a far value
			float farDist = (mFarDist == 0) ? 100000 : mFarDist;

			// Near plane bounds
			Vector3 min(left, bottom, -farDist);
			Vector3 max(right, top, 0);

			if (mCustomProjMatrix)
			{
				// Some custom projection matrices can have unusual inverted settings
				// So make sure the AABB is the right way around to start with
				Vector3 tmp = min;
				min.floor(max);
				max.ceil(tmp);
			}

			if (mProjType == PT_PERSPECTIVE)
			{
				// Merge with far plane bounds
				float radio = farDist / mNearDist;
				min.floor(Vector3(left * radio, bottom * radio, -farDist));
				max.ceil(Vector3(right * radio, top * radio, 0));
			}

			mBoundingBox.setExtents(min, max);

			mRecalcFrustum = false;
			mRecalcFrustumPlanes = true;
		}
	}

	bool CameraBase::isFrustumOutOfDate() const
	{
		return mRecalcFrustum;
	}

	void CameraBase::updateView() const
	{
		if (!mCustomViewMatrix && mRecalcView)
		{
			mViewMatrix.makeView(mPosition, mRotation);
			mViewMatrixInv = mViewMatrix.inverseAffine();
			mRecalcView = false;
		}
	}

	void CameraBase::updateFrustumPlanes() const
	{
		updateFrustum();

		if (mRecalcFrustumPlanes)
		{
			Vector<Plane> frustumPlanes(6);
			Matrix4 combo = mProjMatrix;

			frustumPlanes[FRUSTUM_PLANE_LEFT].normal.x = combo[3][0] + combo[0][0];
			frustumPlanes[FRUSTUM_PLANE_LEFT].normal.y = combo[3][1] + combo[0][1];
			frustumPlanes[FRUSTUM_PLANE_LEFT].normal.z = combo[3][2] + combo[0][2];
			frustumPlanes[FRUSTUM_PLANE_LEFT].d = combo[3][3] + combo[0][3];

			frustumPlanes[FRUSTUM_PLANE_RIGHT].normal.x = combo[3][0] - combo[0][0];
			frustumPlanes[FRUSTUM_PLANE_RIGHT].normal.y = combo[3][1] - combo[0][1];
			frustumPlanes[FRUSTUM_PLANE_RIGHT].normal.z = combo[3][2] - combo[0][2];
			frustumPlanes[FRUSTUM_PLANE_RIGHT].d = combo[3][3] - combo[0][3];

			frustumPlanes[FRUSTUM_PLANE_TOP].normal.x = combo[3][0] - combo[1][0];
			frustumPlanes[FRUSTUM_PLANE_TOP].normal.y = combo[3][1] - combo[1][1];
			frustumPlanes[FRUSTUM_PLANE_TOP].normal.z = combo[3][2] - combo[1][2];
			frustumPlanes[FRUSTUM_PLANE_TOP].d = combo[3][3] - combo[1][3];

			frustumPlanes[FRUSTUM_PLANE_BOTTOM].normal.x = combo[3][0] + combo[1][0];
			frustumPlanes[FRUSTUM_PLANE_BOTTOM].normal.y = combo[3][1] + combo[1][1];
			frustumPlanes[FRUSTUM_PLANE_BOTTOM].normal.z = combo[3][2] + combo[1][2];
			frustumPlanes[FRUSTUM_PLANE_BOTTOM].d = combo[3][3] + combo[1][3];

			frustumPlanes[FRUSTUM_PLANE_NEAR].normal.x = combo[3][0] + combo[2][0];
			frustumPlanes[FRUSTUM_PLANE_NEAR].normal.y = combo[3][1] + combo[2][1];
			frustumPlanes[FRUSTUM_PLANE_NEAR].normal.z = combo[3][2] + combo[2][2];
			frustumPlanes[FRUSTUM_PLANE_NEAR].d = combo[3][3] + combo[2][3];

			frustumPlanes[FRUSTUM_PLANE_FAR].normal.x = combo[3][0] - combo[2][0];
			frustumPlanes[FRUSTUM_PLANE_FAR].normal.y = combo[3][1] - combo[2][1];
			frustumPlanes[FRUSTUM_PLANE_FAR].normal.z = combo[3][2] - combo[2][2];
			frustumPlanes[FRUSTUM_PLANE_FAR].d = combo[3][3] - combo[2][3];

			for (UINT32 i = 0; i < 6; i++)
			{
				float length = frustumPlanes[i].normal.normalize();
				frustumPlanes[i].d /= -length;
			}

			mFrustum = ConvexVolume(frustumPlanes);
			mRecalcFrustumPlanes = false;
		}
	}

	float CameraBase::getAspectRatio() const
	{
		return mAspect;
	}

	void CameraBase::setAspectRatio(float r)
	{
		mAspect = r;
		invalidateFrustum();
		_markCoreDirty();
	}

	const AABox& CameraBase::getBoundingBox() const
	{
		updateFrustum();

		return mBoundingBox;
	}

	void CameraBase::setProjectionType(ProjectionType pt)
	{
		mProjType = pt;
		invalidateFrustum();
		_markCoreDirty();
	}

	ProjectionType CameraBase::getProjectionType() const
	{
		return mProjType;
	}

	void CameraBase::setCustomViewMatrix(bool enable, const Matrix4& viewMatrix)
	{
		mCustomViewMatrix = enable;
		if (enable)
		{
			BS_ASSERT(viewMatrix.isAffine());
			mViewMatrix = viewMatrix;
			mViewMatrixInv = mViewMatrix.inverseAffine();
		}

		_markCoreDirty();
	}

	void CameraBase::setCustomProjectionMatrix(bool enable, const Matrix4& projMatrix)
	{
		mCustomProjMatrix = enable;

		if (enable)
			mProjMatrix = projMatrix;

		invalidateFrustum();
		_markCoreDirty();
	}

	void CameraBase::setOrthoWindow(float w, float h)
	{
		mOrthoHeight = h;
		mAspect = w / h;

		invalidateFrustum();
		_markCoreDirty();
	}

	void CameraBase::setOrthoWindowHeight(float h)
	{
		mOrthoHeight = h;

		invalidateFrustum();
		_markCoreDirty();
	}

	void CameraBase::setOrthoWindowWidth(float w)
	{
		mOrthoHeight = w / mAspect;

		invalidateFrustum();
		_markCoreDirty();
	}

	float CameraBase::getOrthoWindowHeight() const
	{
		return mOrthoHeight;
	}

	float CameraBase::getOrthoWindowWidth() const
	{
		return mOrthoHeight * mAspect;
	}

	void CameraBase::setFrustumExtents(float left, float right, float top, float bottom)
	{
		mFrustumExtentsManuallySet = true;
		mLeft = left;
		mRight = right;
		mTop = top;
		mBottom = bottom;

		invalidateFrustum();
		_markCoreDirty();
	}

	void CameraBase::resetFrustumExtents()
	{
		mFrustumExtentsManuallySet = false;

		invalidateFrustum();
		_markCoreDirty();
	}

	void CameraBase::getFrustumExtents(float& outleft, float& outright, float& outtop, float& outbottom) const
	{
		updateFrustum();

		outleft = mLeft;
		outright = mRight;
		outtop = mTop;
		outbottom = mBottom;
	}

	void CameraBase::setPosition(const Vector3& position)
	{
		mPosition = position;

		mRecalcView = true;
		_markCoreDirty();
	}

	void CameraBase::setRotation(const Quaternion& rotation)
	{
		mRotation = rotation;

		mRecalcView = true;
		_markCoreDirty();
	}

	void CameraBase::invalidateFrustum() const
	{
		mRecalcFrustum = true;
		mRecalcFrustumPlanes = true;
	}

	Vector2I CameraBase::worldToScreenPoint(const Vector3& worldPoint) const
	{
		Vector2 ndcPoint = worldToNdcPoint(worldPoint);
		return ndcToScreenPoint(ndcPoint);
	}

	Vector2 CameraBase::worldToNdcPoint(const Vector3& worldPoint) const
	{
		Vector3 viewPoint = worldToViewPoint(worldPoint);
		return viewToNdcPoint(viewPoint);
	}

	Vector3 CameraBase::worldToViewPoint(const Vector3& worldPoint) const
	{
		return getViewMatrix().multiplyAffine(worldPoint);
	}

	Vector3 CameraBase::screenToWorldPoint(const Vector2I& screenPoint, float depth) const
	{
		Vector2 ndcPoint = screenToNdcPoint(screenPoint);
		return ndcToWorldPoint(ndcPoint, depth);
	}

	Vector3 CameraBase::screenToViewPoint(const Vector2I& screenPoint, float depth) const
	{
		Vector2 ndcPoint = screenToNdcPoint(screenPoint);
		return ndcToViewPoint(ndcPoint, depth);
	}

	Vector2 CameraBase::screenToNdcPoint(const Vector2I& screenPoint) const
	{
		Rect2I viewport = getViewportRect();

		Vector2 ndcPoint;
		ndcPoint.x = (float)(((screenPoint.x - viewport.x) / (float)viewport.width) * 2.0f - 1.0f);
		ndcPoint.y = (float)((1.0f - ((screenPoint.y - viewport.y) / (float)viewport.height)) * 2.0f - 1.0f);

		return ndcPoint;
	}

	Vector3 CameraBase::viewToWorldPoint(const Vector3& viewPoint) const
	{
		return getViewMatrix().inverseAffine().multiplyAffine(viewPoint);
	}

	Vector2I CameraBase::viewToScreenPoint(const Vector3& viewPoint) const
	{
		Vector2 ndcPoint = viewToNdcPoint(viewPoint);
		return ndcToScreenPoint(ndcPoint);
	}

	Vector2 CameraBase::viewToNdcPoint(const Vector3& viewPoint) const
	{
		Vector3 projPoint = projectPoint(viewPoint);

		return Vector2(projPoint.x, projPoint.y);
	}

	Vector3 CameraBase::ndcToWorldPoint(const Vector2& ndcPoint, float depth) const
	{
		Vector3 viewPoint = ndcToViewPoint(ndcPoint, depth);
		return viewToWorldPoint(viewPoint);
	}

	Vector3 CameraBase::ndcToViewPoint(const Vector2& ndcPoint, float depth) const
	{
		return unprojectPoint(Vector3(ndcPoint.x, ndcPoint.y, depth));
	}

	Vector2I CameraBase::ndcToScreenPoint(const Vector2& ndcPoint) const
	{
		Rect2I viewport = getViewportRect();

		Vector2I screenPoint;
		screenPoint.x = Math::roundToInt(viewport.x + ((ndcPoint.x + 1.0f) * 0.5f) * viewport.width);
		screenPoint.y = Math::roundToInt(viewport.y + (1.0f - (ndcPoint.y + 1.0f) * 0.5f) * viewport.height);

		return screenPoint;
	}

	Ray CameraBase::screenPointToRay(const Vector2I& screenPoint) const
	{
		Vector2 ndcPoint = screenToNdcPoint(screenPoint);

		Vector3 near = unprojectPoint(Vector3(ndcPoint.x, ndcPoint.y, mNearDist));
		Vector3 far = unprojectPoint(Vector3(ndcPoint.x, ndcPoint.y, mNearDist + 1.0f));

		Ray ray(near, Vector3::normalize(far - near));
		ray.transformAffine(getViewMatrix().inverseAffine());

		return ray;
	}

	Vector3 CameraBase::projectPoint(const Vector3& point) const
	{
		Vector4 projPoint4(point.x, point.y, point.z, 1.0f);
		projPoint4 = getProjectionMatrixRS().multiply(projPoint4);

		if (Math::abs(projPoint4.w) > 1e-7f)
		{
			float invW = 1.0f / projPoint4.w;
			projPoint4.x *= invW;
			projPoint4.y *= invW;
			projPoint4.z *= invW;
		}
		else
		{
			projPoint4.x = 0.0f;
			projPoint4.y = 0.0f;
			projPoint4.z = 0.0f;
		}

		return Vector3(projPoint4.x, projPoint4.y, projPoint4.z);
	}

	Vector3 CameraBase::unprojectPoint(const Vector3& point) const
	{
		Vector4 dir4(point.x, point.y, 0.95f, 1.0f); // 0.95f arbitrary
		dir4 = getProjectionMatrixRS().inverse().multiply(dir4);

		Vector3 dir;
		dir.x = dir4.x;
		dir.y = dir4.y;
		dir.z = dir4.z;

		if (Math::abs(dir4.w) > 1e-7f)
		{
			float invW = 1.0f / dir4.w;
			dir.x *= invW;
			dir.y *= invW;
			dir.z *= invW;

			// Find a point along a ray from camera origin to point on near plane we just found, 
			// at point.z distance from the origin

			float distToPlane = dir.dot(-Vector3::UNIT_Z);
			if (distToPlane >= 0.0f)
			{
				if (mProjType == PT_PERSPECTIVE)
					dir *= point.z / distToPlane;
				else
					dir += Vector3::UNIT_Z * (distToPlane - point.z);
			}
		}
		else
		{
			dir.x = 0.0f;
			dir.y = 0.0f;
			dir.z = 0.0f;
		}

		return Vector3(dir.x, dir.y, dir.z);
	}

	CameraCore::~CameraCore()
	{
		RendererManager::instance().getActive()->_notifyCameraRemoved(this);
	}

	CameraCore::CameraCore(SPtr<RenderTargetCore> target, float left, float top, float width, float height)
	{
		mViewport = ViewportCore::create(target, left, top, width, height);
	}

	CameraCore::CameraCore(const SPtr<ViewportCore>& viewport)
	{
		mViewport = viewport;
	}

	void CameraCore::initialize()
	{
		RendererManager::instance().getActive()->_notifyCameraAdded(this);

		CoreObjectCore::initialize();
	}

	Rect2I CameraCore::getViewportRect() const
	{
		return mViewport->getArea();
	}

	void CameraCore::syncToCore(const CoreSyncData& data)
	{
		RendererManager::instance().getActive()->_notifyCameraRemoved(this);

		char* dataPtr = (char*)data.getBuffer();

		dataPtr = rttiReadElem(mLayers, dataPtr);
		dataPtr = rttiReadElem(mPosition, dataPtr);
		dataPtr = rttiReadElem(mRotation, dataPtr);
		dataPtr = rttiReadElem(mProjType, dataPtr);
		dataPtr = rttiReadElem(mHorzFOV, dataPtr);
		dataPtr = rttiReadElem(mFarDist, dataPtr);
		dataPtr = rttiReadElem(mNearDist, dataPtr);
		dataPtr = rttiReadElem(mAspect, dataPtr);
		dataPtr = rttiReadElem(mOrthoHeight, dataPtr);
		dataPtr = rttiReadElem(mPriority, dataPtr);
		dataPtr = rttiReadElem(mCustomViewMatrix, dataPtr);
		dataPtr = rttiReadElem(mCustomProjMatrix, dataPtr);
		dataPtr = rttiReadElem(mFrustumExtentsManuallySet, dataPtr);
		dataPtr = rttiReadElem(mCameraFlags, dataPtr);
		dataPtr = rttiReadElem(mIsActive, dataPtr);

		mRecalcFrustum = true;
		mRecalcFrustumPlanes = true;
		mRecalcView = true;

		if(mIsActive)
			RendererManager::instance().getActive()->_notifyCameraAdded(this);
	}

	Camera::Camera(RenderTargetPtr target, float left, float top, float width, float height)
		:mLastUpdateHash(0), mMain(false)
	{
		if (target != nullptr)
			target->blockUntilCoreInitialized();

		mViewport = Viewport::create(target, left, top, width, height);
	}

	SPtr<CameraCore> Camera::getCore() const
	{
		return std::static_pointer_cast<CameraCore>(mCoreSpecific);
	}

	CameraPtr Camera::create(RenderTargetPtr target, float left, float top, float width, float height)
	{
		Camera* handler = new (bs_alloc<Camera>()) Camera(target, left, top, width, height);
		CameraPtr handlerPtr = bs_core_ptr<Camera>(handler);
		handlerPtr->_setThisPtr(handlerPtr);
		handlerPtr->initialize();

		return handlerPtr;
	}

	CameraPtr Camera::createEmpty()
	{
		Camera* handler = new (bs_alloc<Camera>()) Camera();
		CameraPtr handlerPtr = bs_core_ptr<Camera>(handler);
		handlerPtr->_setThisPtr(handlerPtr);

		return handlerPtr;
	}

	SPtr<CoreObjectCore> Camera::createCore() const
	{
		Rect2 normArea = mViewport->getNormArea();

		CameraCore* handler = new (bs_alloc<CameraCore>()) CameraCore(mViewport->getCore());
		SPtr<CameraCore> handlerPtr = bs_shared_ptr<CameraCore>(handler);
		handlerPtr->_setThisPtr(handlerPtr);

		return handlerPtr;
	}

	Rect2I Camera::getViewportRect() const
	{
		return mViewport->getArea();
	}

	CoreSyncData Camera::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = 0;
		size += rttiGetElemSize(mLayers);
		size += rttiGetElemSize(mPosition);
		size += rttiGetElemSize(mRotation);
		size += rttiGetElemSize(mProjType);
		size += rttiGetElemSize(mHorzFOV);
		size += rttiGetElemSize(mFarDist);
		size += rttiGetElemSize(mNearDist);
		size += rttiGetElemSize(mAspect);
		size += rttiGetElemSize(mOrthoHeight);
		size += rttiGetElemSize(mPriority);
		size += rttiGetElemSize(mCustomViewMatrix);
		size += rttiGetElemSize(mCustomProjMatrix);
		size += rttiGetElemSize(mFrustumExtentsManuallySet);
		size += rttiGetElemSize(mCameraFlags);
		size += rttiGetElemSize(mIsActive);

		UINT8* buffer = allocator->alloc(size);

		char* dataPtr = (char*)buffer;
		dataPtr = rttiWriteElem(mLayers, dataPtr);
		dataPtr = rttiWriteElem(mPosition, dataPtr);
		dataPtr = rttiWriteElem(mRotation, dataPtr);
		dataPtr = rttiWriteElem(mProjType, dataPtr);
		dataPtr = rttiWriteElem(mHorzFOV, dataPtr);
		dataPtr = rttiWriteElem(mFarDist, dataPtr);
		dataPtr = rttiWriteElem(mNearDist, dataPtr);
		dataPtr = rttiWriteElem(mAspect, dataPtr);
		dataPtr = rttiWriteElem(mOrthoHeight, dataPtr);
		dataPtr = rttiWriteElem(mPriority, dataPtr);
		dataPtr = rttiWriteElem(mCustomViewMatrix, dataPtr);
		dataPtr = rttiWriteElem(mCustomProjMatrix, dataPtr);
		dataPtr = rttiWriteElem(mFrustumExtentsManuallySet, dataPtr);
		dataPtr = rttiWriteElem(mCameraFlags, dataPtr);
		dataPtr = rttiWriteElem(mIsActive, dataPtr);

		return CoreSyncData(buffer, size);
	}

	void Camera::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		dependencies.push_back(mViewport.get());
	}

	void Camera::_markCoreDirty()
	{
		markCoreDirty();
	}

	RTTITypeBase* Camera::getRTTIStatic()
	{
		return CameraRTTI::instance();
	}

	RTTITypeBase* Camera::getRTTI() const
	{
		return Camera::getRTTIStatic();
	}
}
