//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScenePicking.h"
#include "BsSceneManager.h"
#include "BsColor.h"
#include "BsMatrix4.h"
#include "BsDebug.h"
#include "BsMath.h"
#include "BsCRenderable.h"
#include "BsSceneObject.h"
#include "BsMesh.h"
#include "BsConvexVolume.h"
#include "BsCCamera.h"
#include "BsCoreThread.h"
#include "BsRenderAPI.h"
#include "BsMaterial.h"
#include "BsPass.h"
#include "BsRasterizerState.h"
#include "BsRenderTarget.h"
#include "BsMultiRenderTexture.h"
#include "BsPixelData.h"
#include "BsGpuParams.h"
#include "BsGpuParamsSet.h"
#include "BsBuiltinEditorResources.h"
#include "BsShader.h"
#include "BsCoreRenderer.h"
#include "BsGizmoManager.h"
#include "BsRendererUtility.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	const float ScenePickingCore::ALPHA_CUTOFF = 0.5f;

	ScenePicking::ScenePicking()
	{
		mCore = bs_new<ScenePickingCore>();

		for (UINT32 i = 0; i < 3; i++)
		{
			HMaterial matPicking = BuiltinEditorResources::instance().createPicking((CullingMode)i);
			HMaterial matPickingAlpha = BuiltinEditorResources::instance().createPickingAlpha((CullingMode)i);

			mCore->mMaterialData[i].mMatPickingCore = matPicking->getCore();
			mCore->mMaterialData[i].mMatPickingAlphaCore = matPickingAlpha->getCore();
		}

		gCoreAccessor().queueCommand(std::bind(&ScenePickingCore::initialize, mCore));
	}

	ScenePicking::~ScenePicking()
	{
		gCoreAccessor().queueCommand(std::bind(&ScenePickingCore::destroy, mCore));
	}

	HSceneObject ScenePicking::pickClosestObject(const SPtr<Camera>& cam, const Vector2I& position, const Vector2I& area, Vector<HSceneObject> ignoreRenderables, SnapData* data)
	{
		Vector<HSceneObject> selectedObjects = pickObjects(cam, position, area, ignoreRenderables, data);
		if (selectedObjects.size() == 0)
			return HSceneObject();
		if (data != nullptr)
		{
			Matrix3 rotation;
			selectedObjects[0]->getWorldRotation().toRotationMatrix(rotation);
			data->normal = rotation.inverse().transpose().transform(data->normal);
		}
		return selectedObjects[0];
	}

	Vector<HSceneObject> ScenePicking::pickObjects(const SPtr<Camera>& cam, const Vector2I& position, const Vector2I& area, Vector<HSceneObject> ignoreRenderables, SnapData* data)
	{
		auto comparePickElement = [&] (const ScenePicking::RenderablePickData& a, const ScenePicking::RenderablePickData& b)
		{
			// Sort by alpha setting first, then by cull mode, then by index
			if (a.alpha == b.alpha)
			{
				if (a.cullMode == b.cullMode)
					return a.index > b.index;
				else
					return (UINT32)a.cullMode > (UINT32)b.cullMode;
			}
			else
				return (UINT32)a.alpha > (UINT32)b.alpha;
		};

		Matrix4 viewProjMatrix = cam->getProjectionMatrixRS() * cam->getViewMatrix();

		const Map<Renderable*, SceneRenderableData>& renderables = SceneManager::instance().getAllRenderables();
		RenderableSet pickData(comparePickElement);
		Map<UINT32, HSceneObject> idxToRenderable;

		for (auto& renderableData : renderables)
		{
			SPtr<Renderable> renderable = renderableData.second.renderable;
			HSceneObject so = renderableData.second.sceneObject;

			if (!so->getActive())
				continue;

			HMesh mesh = renderable->getMesh();
			if (!mesh.isLoaded())
				continue;

			bool found = false;
			for (int i = 0; i < ignoreRenderables.size(); i++)
			{
				if (ignoreRenderables[i] == so)
				{
					found = true;
					break;
				}
			}
			if (found)
				continue;

			Bounds worldBounds = mesh->getProperties().getBounds();
			Matrix4 worldTransform = so->getWorldTfrm();
			worldBounds.transformAffine(worldTransform);

			// TODO - I could limit the frustum to the visible area we're rendering for a speed boost
			// but this is unlikely to be a performance bottleneck
			const ConvexVolume& frustum = cam->getWorldFrustum();
			if (frustum.intersects(worldBounds.getSphere()))
			{
				// More precise with the box
				if (frustum.intersects(worldBounds.getBox()))
				{
					for (UINT32 i = 0; i < mesh->getProperties().getNumSubMeshes(); i++)
					{
						UINT32 idx = (UINT32)pickData.size();

						bool useAlphaShader = false;
						SPtr<RasterizerState> rasterizerState;

						HMaterial originalMat = renderable->getMaterial(i);
						if (originalMat != nullptr && originalMat->getNumPasses() > 0)
						{
							SPtr<Pass> firstPass = originalMat->getPass(0); // Note: We only ever check the first pass, problem?
							useAlphaShader = firstPass->hasBlending();

							if (firstPass->getRasterizerState() == nullptr)
								rasterizerState = RasterizerState::getDefault();
							else
								rasterizerState = firstPass->getRasterizerState();
						}
						else
							rasterizerState = RasterizerState::getDefault();

						CullingMode cullMode = rasterizerState->getProperties().getCullMode();

						HTexture mainTexture;
						if (useAlphaShader)
						{
							const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams = originalMat->getShader()->getTextureParams();
							for (auto& objectParam : textureParams)
							{
								if (objectParam.second.rendererSemantic == RPS_Diffuse)
								{
									mainTexture = originalMat->getTexture(objectParam.first);
									break;
								}
							}
						}

						idxToRenderable[idx] = so;

						Matrix4 wvpTransform = viewProjMatrix * worldTransform;
						pickData.insert({ mesh->getCore(), idx, wvpTransform, useAlphaShader, cullMode, mainTexture });
					}
				}
			}
		}

		UINT32 firstGizmoIdx = (UINT32)pickData.size();

		SPtr<RenderTargetCore> target = cam->getViewport()->getTarget()->getCore();
		gCoreAccessor().queueCommand(std::bind(&ScenePickingCore::corePickingBegin, mCore, target,
			cam->getViewport()->getNormArea(), std::cref(pickData), position, area));

		GizmoManager::instance().renderForPicking(cam, [&](UINT32 inputIdx) { return encodeIndex(firstGizmoIdx + inputIdx); });

		AsyncOp op = gCoreAccessor().queueReturnCommand(std::bind(&ScenePickingCore::corePickingEnd, mCore, target, 
			cam->getViewport()->getNormArea(), position, area, data != nullptr, _1));
		gCoreAccessor().submitToCoreThread(true);

		assert(op.hasCompleted());

		PickResults returnValue = op.getReturnValue<PickResults>();

		if (data != nullptr)
		{
			Vector3 pos = returnValue.data.pickPosition;
			Vector2 ndcPoint = cam->screenToNdcPoint(Vector2I(pos.x, pos.y));
			Vector4 worldPoint(ndcPoint.x, ndcPoint.y, pos.z, 1.0f);
			worldPoint = cam->getProjectionMatrixRS().inverse().multiply(worldPoint);
			Vector3 worldPoint3D;

			if (Math::abs(worldPoint.w) > 1e-7f)
			{
				float invW = 1.0f / worldPoint.w;

				worldPoint3D.x = worldPoint.x * invW;
				worldPoint3D.y = worldPoint.y * invW;
				worldPoint3D.z = worldPoint.z * invW;
			}
			pos = cam->viewToWorldPoint(worldPoint3D);
			*data = returnValue.data;
			data->pickPosition = pos;
			//Todo: camera to world ray if object is too far
		}

		Vector<UINT32> selectedObjects = returnValue.objects;
		Vector<HSceneObject> results;

		for (auto& selectedObjectIdx : selectedObjects)
		{
			if (selectedObjectIdx < firstGizmoIdx)
			{
				auto iterFind = idxToRenderable.find(selectedObjectIdx);

				if (iterFind != idxToRenderable.end())
					results.push_back(iterFind->second);
			}
			else
			{
				UINT32 gizmoIdx = selectedObjectIdx - firstGizmoIdx;

				HSceneObject so = GizmoManager::instance().getSceneObject(gizmoIdx);
				if (so)
					results.push_back(so);
			}
		}

		return results;
	}

	Color ScenePicking::encodeIndex(UINT32 index)
	{
		Color encoded;
		encoded.r = (index & 0xFF) / 255.0f;
		encoded.g = ((index >> 8) & 0xFF) / 255.0f;
		encoded.b = ((index >> 16) & 0xFF) / 255.0f;
		encoded.a = 1.0f;

		if (((index >> 24) & 0xFF))
			LOGERR("Index when picking out of valid range.");

		return encoded;
	}

	UINT32 ScenePicking::decodeIndex(Color color)
	{
		UINT32 r = Math::roundToInt(color.r * 255.0f);
		UINT32 g = Math::roundToInt(color.g * 255.0f);
		UINT32 b = Math::roundToInt(color.b * 255.0f);

		return (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16);
	}

	void ScenePickingCore::initialize()
	{
		for (UINT32 i = 0; i < 3; i++)
		{
			MaterialData& md = mMaterialData[i];

			{
				md.mPickingParams = md.mMatPickingCore->createParamsSet();
				SPtr<GpuParamsCore> vertParams = md.mPickingParams->getGpuParams(GPT_VERTEX_PROGRAM);
				SPtr<GpuParamsCore> fragParams = md.mPickingParams->getGpuParams(GPT_FRAGMENT_PROGRAM);

				vertParams->getParam("matWorldViewProj", md.mParamPickingWVP);
				fragParams->getParam("colorIndex", md.mParamPickingColor);
			}

			{
				md.mPickingAlphaParams = md.mMatPickingAlphaCore->createParamsSet();
				SPtr<GpuParamsCore> vertParams = md.mPickingAlphaParams->getGpuParams(GPT_VERTEX_PROGRAM);
				SPtr<GpuParamsCore> fragParams = md.mPickingAlphaParams->getGpuParams(GPT_FRAGMENT_PROGRAM);

				vertParams->getParam("matWorldViewProj", md.mParamPickingAlphaWVP);
				fragParams->getParam("colorIndex", md.mParamPickingAlphaColor);
				fragParams->getTextureParam("mainTexture", md.mParamPickingAlphaTexture);

				GpuParamFloatCore alphaCutoffParam;
				fragParams->getParam("alphaCutoff", alphaCutoffParam);
				alphaCutoffParam.set(ALPHA_CUTOFF);
			}
		}
	}

	void ScenePickingCore::destroy()
	{
		bs_delete(this);
	}

	void ScenePickingCore::corePickingBegin(const SPtr<RenderTargetCore>& target, const Rect2& viewportArea,
		const ScenePicking::RenderableSet& renderables, const Vector2I& position, const Vector2I& area)
	{
		RenderAPICore& rs = RenderAPICore::instance();

		SPtr<RenderTextureCore> rtt = std::static_pointer_cast<RenderTextureCore>(target);
		SPtr<TextureCore> outputTexture = rtt->getBindableColorTexture();
		TextureProperties outputTextureProperties = outputTexture->getProperties();
		SPtr<TextureCore> depthTexture = rtt->getBindableDepthStencilTexture();

		MULTI_RENDER_TEXTURE_CORE_DESC multiTextureDescription;
		multiTextureDescription.colorSurfaces.resize(2);
		multiTextureDescription.colorSurfaces[0].face = 0;
		multiTextureDescription.colorSurfaces[0].texture = outputTexture;
		multiTextureDescription.colorSurfaces[1].face = 0;
		SPtr<TextureCore> core = TextureCore::create(TextureType::TEX_TYPE_2D, outputTextureProperties.getWidth(),
			outputTextureProperties.getHeight(), 0, PixelFormat::PF_R8G8B8A8, TU_RENDERTARGET, false, 1);
		multiTextureDescription.colorSurfaces[1].texture = core;
		RENDER_SURFACE_CORE_DESC depthStencilDescription;
		depthStencilDescription.face = 0;
		depthStencilDescription.texture = depthTexture;
		multiTextureDescription.depthStencilSurface = depthStencilDescription;
		
		mNormalsTexture = MultiRenderTextureCore::create(multiTextureDescription);

		rs.beginFrame();
		rs.setRenderTarget(mNormalsTexture);
		rs.setViewport(viewportArea);
		rs.clearRenderTarget(FBT_COLOR | FBT_DEPTH | FBT_STENCIL, Color::White);
		rs.setScissorRect(position.x, position.y, position.x + area.x, position.y + area.y);

		gRendererUtility().setPass(mMaterialData[0].mMatPickingCore, 0);
		gRendererUtility().setPassParams(mMaterialData[0].mPickingParams, 0);

		bool activeMaterialIsAlpha = false;
		CullingMode activeMaterialCull = (CullingMode)0;

		for (auto& renderable : renderables)
		{
			if (activeMaterialIsAlpha != renderable.alpha || activeMaterialCull != renderable.cullMode)
			{
				activeMaterialIsAlpha = renderable.alpha;
				activeMaterialCull = renderable.cullMode;

				if (activeMaterialIsAlpha)
					gRendererUtility().setPass(mMaterialData[(UINT32)activeMaterialCull].mMatPickingAlphaCore, 0);
				else
					gRendererUtility().setPass(mMaterialData[(UINT32)activeMaterialCull].mMatPickingCore, 0);
			}

			Color color = ScenePicking::encodeIndex(renderable.index);
			MaterialData& md = mMaterialData[(UINT32)activeMaterialCull];

			if (activeMaterialIsAlpha)
			{
				md.mParamPickingAlphaWVP.set(renderable.wvpTransform);
				md.mParamPickingAlphaColor.set(color);
				md.mParamPickingAlphaTexture.set(renderable.mainTexture->getCore());

				gRendererUtility().setPassParams(md.mPickingAlphaParams);
			}
			else
			{
				md.mParamPickingWVP.set(renderable.wvpTransform);
				md.mParamPickingColor.set(color);

				gRendererUtility().setPassParams(md.mPickingParams);
			}

			UINT32 numSubmeshes = renderable.mesh->getProperties().getNumSubMeshes();

			for (UINT32 i = 0; i < numSubmeshes; i++)
				gRendererUtility().draw(renderable.mesh, renderable.mesh->getProperties().getSubMesh(i));
		}
	}

	void ScenePickingCore::corePickingEnd(const SPtr<RenderTargetCore>& target, const Rect2& viewportArea, const Vector2I& position,
		const Vector2I& area, bool gatherSnapData, AsyncOp& asyncOp)
	{
		const RenderTargetProperties& rtProps = target->getProperties();
		RenderAPICore& rs = RenderAPICore::instance();

		rs.endFrame();
		rs.setRenderTarget(nullptr);

		if (rtProps.isWindow())
		{
			// TODO: When I do implement this then I will likely want a method in RenderTarget that unifies both render window and render texture readback
			BS_EXCEPT(NotImplementedException, "Picking is not supported on render windows as framebuffer readback methods aren't implemented");
		}

		SPtr<TextureCore> outputTexture = mNormalsTexture->getBindableColorTexture(0)->getTexture();
		SPtr<TextureCore> normalsTexture = mNormalsTexture->getBindableColorTexture(1)->getTexture();
		SPtr<TextureCore> depthTexture = mNormalsTexture->getBindableDepthStencilTexture()->getTexture();

		if (position.x < 0 || position.x >= (INT32)outputTexture->getProperties().getWidth() ||
			position.y < 0 || position.y >= (INT32)outputTexture->getProperties().getHeight())
		{
			asyncOp._completeOperation(Vector<UINT32>());
			return;
		}

		SPtr<PixelData> outputPixelData = outputTexture->getProperties().allocateSubresourceBuffer(0);
		SPtr<PixelData> normalsPixelData;
		SPtr<PixelData> depthPixelData;
		if (gatherSnapData)
		{
			normalsPixelData = normalsTexture->getProperties().allocateSubresourceBuffer(0);
			depthPixelData = depthTexture->getProperties().allocateSubresourceBuffer(0);
		}

		outputTexture->readSubresource(0, *outputPixelData);

		Map<UINT32, UINT32> selectionScores;
		UINT32 maxWidth = std::min((UINT32)(position.x + area.x), outputPixelData->getWidth());
		UINT32 maxHeight = std::min((UINT32)(position.y + area.y), outputPixelData->getHeight());

		bool needs = rtProps.requiresTextureFlipping();
		if (rtProps.requiresTextureFlipping())
		{
			UINT32 vertOffset = outputPixelData->getHeight() - 1;

			for (UINT32 y = maxHeight; y > (UINT32)position.y; y--)
			{
				for (UINT32 x = (UINT32)position.x; x < maxWidth; x++)
				{
					Color color = outputPixelData->getColorAt(x, vertOffset - y);
					UINT32 index = ScenePicking::decodeIndex(color);

					if (index == 0x00FFFFFF) // Nothing selected
						continue;

					auto iterFind = selectionScores.find(index);
					if (iterFind == selectionScores.end())
						selectionScores[index] = 1;
					else
						iterFind->second++;
				}
			}
		}
		else
		{
			for (UINT32 y = (UINT32)position.y; y < maxHeight; y++)
			{
				for (UINT32 x = (UINT32)position.x; x < maxWidth; x++)
				{
					Color color = outputPixelData->getColorAt(x, y);
					UINT32 index = ScenePicking::decodeIndex(color);

					if (index == 0x00FFFFFF) // Nothing selected
						continue;

					auto iterFind = selectionScores.find(index);
					if (iterFind == selectionScores.end())
						selectionScores[index] = 1;
					else
						iterFind->second++;
				}
			}
		}

		// Sort by score
		struct SelectedObject { UINT32 index; UINT32 score; };

		Vector<SelectedObject> selectedObjects(selectionScores.size());
		UINT32 idx = 0;
		for (auto& selectionScore : selectionScores)
		{
			selectedObjects[idx++] = { selectionScore.first, selectionScore.second };
		}

		std::sort(selectedObjects.begin(), selectedObjects.end(),
			[&](const SelectedObject& a, const SelectedObject& b)
		{
			return b.score < a.score;
		});

		Vector<UINT32> objects;
		for (auto& selectedObject : selectedObjects)
			objects.push_back(selectedObject.index);
		
		PickResults result;
		if (gatherSnapData)
		{
			depthTexture->readSubresource(0, *depthPixelData);
			normalsTexture->readSubresource(0, *normalsPixelData);
			float depth;
			Color normal;
			if (rtProps.requiresTextureFlipping())
			{
				depth = depthPixelData->getDepthAt(position.x, depthPixelData->getHeight() - position.y);
				normal = normalsPixelData->getColorAt(position.x, depthPixelData->getHeight() - position.y);
			}
			else
			{
				depth = depthPixelData->getDepthAt(position.x, position.y);
				normal = normalsPixelData->getColorAt(position.x, position.y);
			}

			SnapData data;
			const RenderAPIInfo& rapiInfo = rs.getAPIInfo();
			depth = depth * Math::abs(rapiInfo.getMaximumDepthInputValue() - rapiInfo.getMinimumDepthInputValue()) + rapiInfo.getMinimumDepthInputValue();
			data.pickPosition = Vector3(position.x, position.y, depth);
			data.normal = Vector3((normal.r * 2) - 1, (normal.g * 2) - 1, (normal.b * 2) - 1);
			result.data = data;
		}
		result.objects = objects;
		asyncOp._completeOperation(result);
		mNormalsTexture = nullptr;
	}
}