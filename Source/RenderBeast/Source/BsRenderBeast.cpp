//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderBeast.h"
#include "BsCCamera.h"
#include "BsCRenderable.h"
#include "BsMaterial.h"
#include "BsMesh.h"
#include "BsPass.h"
#include "BsSamplerState.h"
#include "BsCoreApplication.h"
#include "BsViewport.h"
#include "BsRenderTarget.h"
#include "BsRenderQueue.h"
#include "BsCoreThread.h"
#include "BsGpuParams.h"
#include "BsProfilerCPU.h"
#include "BsShader.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsTime.h"
#include "BsRenderableElement.h"
#include "BsCoreObjectManager.h"
#include "BsRenderBeastOptions.h"
#include "BsSamplerOverrides.h"
#include "BsLight.h"
#include "BsRenderTexturePool.h"
#include "BsRenderTargets.h"
#include "BsRendererUtility.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	RenderBeast::RenderBeast()
		: mDefaultMaterial(nullptr), mPointLightInMat(nullptr), mPointLightOutMat(nullptr), mDirLightMat(nullptr)
		, mObjectRenderer(nullptr), mOptions(bs_shared_ptr_new<RenderBeastOptions>()), mOptionsDirty(true)
	{

	}

	const StringID& RenderBeast::getName() const
	{
		static StringID name = "RenderBeast";
		return name;
	}

	void RenderBeast::initialize()
	{
		CoreRenderer::initialize();

		CoreThread::instance().queueCommand(std::bind(&RenderBeast::initializeCore, this));
	}

	void RenderBeast::destroy()
	{
		CoreRenderer::destroy();

		gCoreAccessor().queueCommand(std::bind(&RenderBeast::destroyCore, this));
		gCoreAccessor().submitToCoreThread(true);
	}

	void RenderBeast::initializeCore()
	{
		RendererUtility::startUp();

		mCoreOptions = bs_shared_ptr_new<RenderBeastOptions>();
		mObjectRenderer = bs_new<ObjectRenderer>();

		mDefaultMaterial = bs_new<DefaultMaterial>();
		mPointLightInMat = bs_new<PointLightInMat>();
		mPointLightOutMat = bs_new<PointLightOutMat>();
		mDirLightMat = bs_new<DirectionalLightMat>();

		RenderTexturePool::startUp();
		PostProcessing::startUp();
	}

	void RenderBeast::destroyCore()
	{
		if (mObjectRenderer != nullptr)
			bs_delete(mObjectRenderer);

		mRenderTargets.clear();
		mCameras.clear();
		mRenderables.clear();

		PostProcessing::shutDown();
		RenderTexturePool::shutDown();

		bs_delete(mDefaultMaterial);
		bs_delete(mPointLightInMat);
		bs_delete(mPointLightOutMat);
		bs_delete(mDirLightMat);

		RendererUtility::shutDown();

		assert(mSamplerOverrides.empty());
	}

	void RenderBeast::notifyRenderableAdded(RenderableCore* renderable)
	{
		UINT32 renderableId = (UINT32)mRenderables.size();

		renderable->setRendererId(renderableId);

		mRenderables.push_back(RendererObject());
		mRenderableShaderData.push_back(RenderableShaderData());
		mWorldBounds.push_back(renderable->getBounds());

		RendererObject& rendererObject = mRenderables.back();
		rendererObject.renderable = renderable;

		RenderableShaderData& shaderData = mRenderableShaderData.back();
		shaderData.worldTransform = renderable->getTransform();
		shaderData.invWorldTransform = shaderData.worldTransform.inverseAffine();
		shaderData.worldNoScaleTransform = renderable->getTransformNoScale();
		shaderData.invWorldNoScaleTransform = shaderData.worldNoScaleTransform.inverseAffine();
		shaderData.worldDeterminantSign = shaderData.worldTransform.determinant3x3() >= 0.0f ? 1.0f : -1.0f;

		SPtr<MeshCore> mesh = renderable->getMesh();
		if (mesh != nullptr)
		{
			const MeshProperties& meshProps = mesh->getProperties();
			SPtr<VertexDeclarationCore> vertexDecl = mesh->getVertexData()->vertexDeclaration;

			for (UINT32 i = 0; i < meshProps.getNumSubMeshes(); i++)
			{
				rendererObject.elements.push_back(BeastRenderableElement());
				BeastRenderableElement& renElement = rendererObject.elements.back();

				renElement.mesh = mesh;
				renElement.subMesh = meshProps.getSubMesh(i);
				renElement.renderableId = renderableId;

				renElement.material = renderable->getMaterial(i);
				if (renElement.material == nullptr)
					renElement.material = renderable->getMaterial(0);

				if (renElement.material != nullptr && renElement.material->getShader() == nullptr)
					renElement.material = nullptr;

				// Validate mesh <-> shader vertex bindings
				if (renElement.material != nullptr)
				{
					UINT32 numPasses = renElement.material->getNumPasses();
					for (UINT32 j = 0; j < numPasses; j++)
					{
						SPtr<PassCore> pass = renElement.material->getPass(j);

						SPtr<VertexDeclarationCore> shaderDecl = pass->getVertexProgram()->getInputDeclaration();
						if (!vertexDecl->isCompatible(shaderDecl))
						{
							Vector<VertexElement> missingElements = vertexDecl->getMissingElements(shaderDecl);

							StringStream wrnStream;
							wrnStream << "Provided mesh is missing required vertex attributes to render with the provided shader. Missing elements: " << std::endl;

							for (auto& entry : missingElements)
								wrnStream << "\t" << toString(entry.getSemantic()) << entry.getSemanticIdx() << std::endl;

							LOGWRN(wrnStream.str());
							break;
						}
					}
				}

				// If no material use the default material
				if (renElement.material == nullptr)
					renElement.material = mDefaultMaterial->getMaterial();

				auto iterFind = mSamplerOverrides.find(renElement.material);
				if (iterFind != mSamplerOverrides.end())
				{
					renElement.samplerOverrides = iterFind->second;
					iterFind->second->refCount++;
				}
				else
				{
					MaterialSamplerOverrides* samplerOverrides = SamplerOverrideUtility::generateSamplerOverrides(renElement.material, mCoreOptions);
					mSamplerOverrides[renElement.material] = samplerOverrides;

					renElement.samplerOverrides = samplerOverrides;
					samplerOverrides->refCount++;
				}

				mObjectRenderer->initElement(renElement);
			}
		}
	}

	void RenderBeast::notifyRenderableRemoved(RenderableCore* renderable)
	{
		UINT32 renderableId = renderable->getRendererId();
		RenderableCore* lastRenerable = mRenderables.back().renderable;
		UINT32 lastRenderableId = lastRenerable->getRendererId();

		Vector<BeastRenderableElement>& elements = mRenderables[renderableId].elements;
		for (auto& element : elements)
		{
			auto iterFind = mSamplerOverrides.find(element.material);
			assert(iterFind != mSamplerOverrides.end());

			MaterialSamplerOverrides* samplerOverrides = iterFind->second;
			samplerOverrides->refCount--;
			if (samplerOverrides->refCount == 0)
			{
				SamplerOverrideUtility::destroySamplerOverrides(samplerOverrides);
				mSamplerOverrides.erase(iterFind);
			}

			element.samplerOverrides = nullptr;
		}

		if (renderableId != lastRenderableId)
		{
			// Swap current last element with the one we want to erase
			std::swap(mRenderables[renderableId], mRenderables[lastRenderableId]);
			std::swap(mWorldBounds[renderableId], mWorldBounds[lastRenderableId]);
			std::swap(mRenderableShaderData[renderableId], mRenderableShaderData[lastRenderableId]);

			lastRenerable->setRendererId(renderableId);

			for (auto& element : elements)
				element.renderableId = renderableId;
		}

		// Last element is the one we want to erase
		mRenderables.erase(mRenderables.end() - 1);
		mWorldBounds.erase(mWorldBounds.end() - 1);
		mRenderableShaderData.erase(mRenderableShaderData.end() - 1);
	}

	void RenderBeast::notifyRenderableUpdated(RenderableCore* renderable)
	{
		UINT32 renderableId = renderable->getRendererId();

		RenderableShaderData& shaderData = mRenderableShaderData[renderableId];
		shaderData.worldTransform = renderable->getTransform();
		shaderData.invWorldTransform = shaderData.worldTransform.inverseAffine();
		shaderData.worldNoScaleTransform = renderable->getTransformNoScale();
		shaderData.invWorldNoScaleTransform = shaderData.worldNoScaleTransform.inverseAffine();
		shaderData.worldDeterminantSign = shaderData.worldTransform.determinant3x3() >= 0.0f ? 1.0f : -1.0f;

		mWorldBounds[renderableId] = renderable->getBounds();
	}

	void RenderBeast::notifyLightAdded(LightCore* light)
	{
		if (light->getType() == LightType::Directional)
		{
			UINT32 lightId = (UINT32)mDirectionalLights.size();
			light->setRendererId(lightId);

			mDirectionalLights.push_back(LightData());

			LightData& lightData = mDirectionalLights.back();
			lightData.internal = light;
		}
		else
		{
			UINT32 lightId = (UINT32)mPointLights.size();

			light->setRendererId(lightId);

			mPointLights.push_back(LightData());
			mLightWorldBounds.push_back(light->getBounds());

			LightData& lightData = mPointLights.back();
			lightData.internal = light;
		}
	}

	void RenderBeast::notifyLightUpdated(LightCore* light)
	{
		UINT32 lightId = light->getRendererId();

		if (light->getType() != LightType::Directional)
			mLightWorldBounds[lightId] = light->getBounds();
	}

	void RenderBeast::notifyLightRemoved(LightCore* light)
	{
		UINT32 lightId = light->getRendererId();
		if (light->getType() == LightType::Directional)
		{
			LightCore* lastLight = mDirectionalLights.back().internal;
			UINT32 lastLightId = lastLight->getRendererId();

			if (lightId != lastLightId)
			{
				// Swap current last element with the one we want to erase
				std::swap(mDirectionalLights[lightId], mDirectionalLights[lastLightId]);
				lastLight->setRendererId(lightId);
			}

			// Last element is the one we want to erase
			mDirectionalLights.erase(mDirectionalLights.end() - 1);
		}
		else
		{
			LightCore* lastLight = mPointLights.back().internal;
			UINT32 lastLightId = lastLight->getRendererId();

			if (lightId != lastLightId)
			{
				// Swap current last element with the one we want to erase
				std::swap(mPointLights[lightId], mPointLights[lastLightId]);
				std::swap(mLightWorldBounds[lightId], mLightWorldBounds[lastLightId]);

				lastLight->setRendererId(lightId);
			}

			// Last element is the one we want to erase
			mPointLights.erase(mPointLights.end() - 1);
			mLightWorldBounds.erase(mLightWorldBounds.end() - 1);
		}
	}

	void RenderBeast::notifyCameraAdded(const CameraCore* camera)
	{
		updateCameraData(camera);
	}

	void RenderBeast::notifyCameraUpdated(const CameraCore* camera, UINT32 updateFlag)
	{
		if((updateFlag & (UINT32)CameraDirtyFlag::Everything) != 0)
		{
			updateCameraData(camera);
		}
		else if((updateFlag & (UINT32)CameraDirtyFlag::PostProcess) != 0)
		{
			RendererCamera& rendererCam = mCameras[camera];
			rendererCam.updatePP();
		} 
	}

	void RenderBeast::notifyCameraRemoved(const CameraCore* camera)
	{
		updateCameraData(camera, true);
	}

	void RenderBeast::updateCameraData(const CameraCore* camera, bool forceRemove)
	{
		SPtr<RenderTargetCore> renderTarget = camera->getViewport()->getTarget();
		if(forceRemove)
		{
			mCameras.erase(camera);
			renderTarget = nullptr;
		}
		else
		{
			mCameras[camera] = RendererCamera(camera, mCoreOptions->stateReductionMode);
		}

		// Remove from render target list
		int rtChanged = 0; // 0 - No RT, 1 - RT found, 2 - RT changed
		for (auto iterTarget = mRenderTargets.begin(); iterTarget != mRenderTargets.end(); ++iterTarget)
		{
			RenderTargetData& target = *iterTarget;
			for (auto iterCam = target.cameras.begin(); iterCam != target.cameras.end(); ++iterCam)
			{
				if (camera == *iterCam)
				{
					if (renderTarget != target.target)
					{
						target.cameras.erase(iterCam);
						rtChanged = 2;

					}
					else
						rtChanged = 1;

					break;
				}
			}

			if (target.cameras.empty())
			{
				mRenderTargets.erase(iterTarget);
				break;
			}
		}

		// Register in render target list
		if (renderTarget != nullptr && (rtChanged == 0 || rtChanged == 2))
		{
			auto findIter = std::find_if(mRenderTargets.begin(), mRenderTargets.end(),
				[&](const RenderTargetData& x) { return x.target == renderTarget; });

			if (findIter != mRenderTargets.end())
			{
				findIter->cameras.push_back(camera);
			}
			else
			{
				mRenderTargets.push_back(RenderTargetData());
				RenderTargetData& renderTargetData = mRenderTargets.back();

				renderTargetData.target = renderTarget;
				renderTargetData.cameras.push_back(camera);
			}

			// Sort render targets based on priority
			auto cameraComparer = [&](const CameraCore* a, const CameraCore* b) { return a->getPriority() > b->getPriority(); };
			auto renderTargetInfoComparer = [&](const RenderTargetData& a, const RenderTargetData& b)
			{ return a.target->getProperties().getPriority() > b.target->getProperties().getPriority(); };
			std::sort(begin(mRenderTargets), end(mRenderTargets), renderTargetInfoComparer);

			for (auto& camerasPerTarget : mRenderTargets)
			{
				Vector<const CameraCore*>& cameras = camerasPerTarget.cameras;

				std::sort(begin(cameras), end(cameras), cameraComparer);
			}
		}
	}

	void RenderBeast::setOptions(const SPtr<CoreRendererOptions>& options)
	{
		mOptions = std::static_pointer_cast<RenderBeastOptions>(options);
		mOptionsDirty = true;
	}

	SPtr<CoreRendererOptions> RenderBeast::getOptions() const
	{
		return mOptions;
	}

	void RenderBeast::syncOptions(const RenderBeastOptions& options)
	{
		bool filteringChanged = mCoreOptions->filtering != options.filtering;
		if (options.filtering == RenderBeastFiltering::Anisotropic)
			filteringChanged |= mCoreOptions->anisotropyMax != options.anisotropyMax;

		if (filteringChanged)
			refreshSamplerOverrides(true);

		*mCoreOptions = options;

		for (auto& entry : mCameras)
		{
			RendererCamera& rendererCam = entry.second;
			rendererCam.update(mCoreOptions->stateReductionMode);
		}
	}

	void RenderBeast::renderAll() 
	{
		// Sync all dirty sim thread CoreObject data to core thread
		CoreObjectManager::instance().syncToCore(gCoreAccessor());

		if (mOptionsDirty)
		{
			gCoreAccessor().queueCommand(std::bind(&RenderBeast::syncOptions, this, *mOptions));
			mOptionsDirty = false;
		}

		gCoreAccessor().queueCommand(std::bind(&RenderBeast::renderAllCore, this, gTime().getTime(), gTime().getFrameDelta()));
	}

	void RenderBeast::renderAllCore(float time, float delta)
	{
		THROW_IF_NOT_CORE_THREAD;

		gProfilerCPU().beginSample("renderAllCore");

		// Note: I'm iterating over all sampler states every frame. If this ends up being a performance
		// issue consider handling this internally in MaterialCore which can only do it when sampler states
		// are actually modified after sync
		refreshSamplerOverrides();

		// Update global per-frame hardware buffers
		mObjectRenderer->updatePerFrameBuffers(time);

		// Generate render queues per camera
		for (auto& entry : mCameras)
			entry.second.determineVisible(mRenderables, mWorldBounds);

		// Render everything, target by target
		for (auto& renderTargetData : mRenderTargets)
		{
			SPtr<RenderTargetCore> target = renderTargetData.target;
			Vector<const CameraCore*>& cameras = renderTargetData.cameras;

			RenderAPICore::instance().beginFrame();

			UINT32 numCameras = (UINT32)cameras.size();
			for (UINT32 i = 0; i < numCameras; i++)
			{
				bool isOverlayCamera = cameras[i]->getFlags().isSet(CameraFlag::Overlay);
				if (!isOverlayCamera)
					render(renderTargetData, i, delta);
				else
					renderOverlay(renderTargetData, i, delta);
			}

			RenderAPICore::instance().endFrame();
			RenderAPICore::instance().swapBuffers(target);
		}

		gProfilerCPU().endSample("renderAllCore");
	}

	void RenderBeast::render(RenderTargetData& rtData, UINT32 camIdx, float delta)
	{
		gProfilerCPU().beginSample("Render");

		const CameraCore* camera = rtData.cameras[camIdx];
		RendererCamera& rendererCam = mCameras[camera];
		CameraShaderData cameraShaderData = rendererCam.getShaderData();

		assert(!camera->getFlags().isSet(CameraFlag::Overlay));

		mObjectRenderer->updatePerCameraBuffers(cameraShaderData);
		rendererCam.beginRendering(true);

		SPtr<RenderTargets> renderTargets = rendererCam.getRenderTargets();
		renderTargets->bindGBuffer();

		//// Trigger pre-scene callbacks
		auto iterCameraCallbacks = mRenderCallbacks.find(camera);
		if (iterCameraCallbacks != mRenderCallbacks.end())
		{
			for (auto& callbackPair : iterCameraCallbacks->second)
			{
				const RenderCallbackData& callbackData = callbackPair.second;

				if (callbackData.overlay)
					continue;

				if (callbackPair.first >= 0)
					break;

				callbackData.callback();
			}
		}
		
		//// Render base pass
		const Vector<RenderQueueElement>& opaqueElements = rendererCam.getOpaqueQueue()->getSortedElements();
		for (auto iter = opaqueElements.begin(); iter != opaqueElements.end(); ++iter)
		{
			BeastRenderableElement* renderElem = static_cast<BeastRenderableElement*>(iter->renderElem);
			SPtr<MaterialCore> material = renderElem->material;

			UINT32 rendererId = renderElem->renderableId;
			Matrix4 worldViewProjMatrix = cameraShaderData.viewProj * mRenderableShaderData[rendererId].worldTransform;

			mObjectRenderer->updatePerObjectBuffers(*renderElem, mRenderableShaderData[rendererId], worldViewProjMatrix);

			if (iter->applyPass)
				RendererUtility::instance().setPass(material, iter->passIdx, false);

			SPtr<PassParametersCore> passParams = material->getPassParameters(iter->passIdx);

			if (renderElem->samplerOverrides != nullptr)
				setPassParams(passParams, &renderElem->samplerOverrides->passes[iter->passIdx]);
			else
				setPassParams(passParams, nullptr);

			gRendererUtility().draw(iter->renderElem->mesh, iter->renderElem->subMesh);
		}

		renderTargets->bindSceneColor(true);

		//// Render light pass
		{
			SPtr<GpuParamBlockBufferCore> perCameraBuffer = mObjectRenderer->getPerCameraParams().getBuffer();

			mDirLightMat->bind(renderTargets, perCameraBuffer);
			for (auto& light : mDirectionalLights)
			{
				if (!light.internal->getIsActive())
					continue;

				mDirLightMat->setPerLightParams(light.internal);
				gRendererUtility().drawScreenQuad();
			}

			// Draw point lights which our camera is within
			// TODO - Possibly use instanced drawing here as only two meshes are drawn with various properties
			mPointLightInMat->bind(renderTargets, perCameraBuffer);

			// TODO - Cull lights based on visibility, right now I just iterate over all of them. 
			for (auto& light : mPointLights)
			{
				if (!light.internal->getIsActive())
					continue;

				float distToLight = (light.internal->getBounds().getCenter() - camera->getPosition()).squaredLength();
				float boundRadius = light.internal->getBounds().getRadius() * 1.05f + camera->getNearClipDistance() * 2.0f;

				bool cameraInLightGeometry = distToLight < boundRadius * boundRadius;
				if (!cameraInLightGeometry)
					continue;

				mPointLightInMat->setPerLightParams(light.internal);

				SPtr<MeshCore> mesh = light.internal->getMesh();
				gRendererUtility().draw(mesh, mesh->getProperties().getSubMesh(0));
			}

			// Draw other point lights
			mPointLightOutMat->bind(renderTargets, perCameraBuffer);

			for (auto& light : mPointLights)
			{
				if (!light.internal->getIsActive())
					continue;

				float distToLight = (light.internal->getBounds().getCenter() - camera->getPosition()).squaredLength();
				float boundRadius = light.internal->getBounds().getRadius() * 1.05f + camera->getNearClipDistance() * 2.0f;

				bool cameraInLightGeometry = distToLight < boundRadius * boundRadius;
				if (cameraInLightGeometry)
					continue;

				mPointLightOutMat->setPerLightParams(light.internal);

				SPtr<MeshCore> mesh = light.internal->getMesh();
				gRendererUtility().draw(mesh, mesh->getProperties().getSubMesh(0));
			}
		}

		renderTargets->bindSceneColor(false);
		
		// Render transparent objects (TODO - No lighting yet)
		const Vector<RenderQueueElement>& transparentElements = rendererCam.getTransparentQueue()->getSortedElements();
		for (auto iter = transparentElements.begin(); iter != transparentElements.end(); ++iter)
		{
			BeastRenderableElement* renderElem = static_cast<BeastRenderableElement*>(iter->renderElem);
			SPtr<MaterialCore> material = renderElem->material;

			UINT32 rendererId = renderElem->renderableId;
			Matrix4 worldViewProjMatrix = cameraShaderData.viewProj * mRenderableShaderData[rendererId].worldTransform;

			mObjectRenderer->updatePerObjectBuffers(*renderElem, mRenderableShaderData[rendererId], worldViewProjMatrix);

			if (iter->applyPass)
				RendererUtility::instance().setPass(material, iter->passIdx, false);

			SPtr<PassParametersCore> passParams = material->getPassParameters(iter->passIdx);

			if (renderElem->samplerOverrides != nullptr)
				setPassParams(passParams, &renderElem->samplerOverrides->passes[iter->passIdx]);
			else
				setPassParams(passParams, nullptr);

			gRendererUtility().draw(iter->renderElem->mesh, iter->renderElem->subMesh);
		}

		// Render non-overlay post-scene callbacks
		if (iterCameraCallbacks != mRenderCallbacks.end())
		{
			for (auto& callbackPair : iterCameraCallbacks->second)
			{
				const RenderCallbackData& callbackData = callbackPair.second;

				if (callbackData.overlay || callbackPair.first < 0)
					continue;

				callbackData.callback();
			}
		}

		// TODO - If GBuffer has multiple samples, I should resolve them before post-processing
		PostProcessing::instance().postProcess(renderTargets->getSceneColorRT(),
			camera, rendererCam.getPPInfo(), delta);

		// Render overlay post-scene callbacks
		if (iterCameraCallbacks != mRenderCallbacks.end())
		{
			for (auto& callbackPair : iterCameraCallbacks->second)
			{
				const RenderCallbackData& callbackData = callbackPair.second;

				if (!callbackData.overlay)
					continue;

				callbackData.callback();
			}
		}

		rendererCam.endRendering();

		gProfilerCPU().endSample("Render");
	}

	void RenderBeast::renderOverlay(RenderTargetData& rtData, UINT32 camIdx, float delta)
	{
		gProfilerCPU().beginSample("RenderOverlay");

		const CameraCore* camera = rtData.cameras[camIdx];
		assert(camera->getFlags().isSet(CameraFlag::Overlay));

		SPtr<ViewportCore> viewport = camera->getViewport();
		RendererCamera& rendererCam = mCameras[camera];
		CameraShaderData cameraShaderData = rendererCam.getShaderData();

		mObjectRenderer->updatePerCameraBuffers(cameraShaderData);
		rendererCam.beginRendering(false);

		SPtr<RenderTargetCore> target = rtData.target;

		RenderAPICore::instance().setRenderTarget(target);
		RenderAPICore::instance().setViewport(viewport->getNormArea());

		// If first camera in render target, prepare the render target
		if (camIdx == 0)
		{
			UINT32 clearBuffers = 0;
			if (viewport->getRequiresColorClear())
				clearBuffers |= FBT_COLOR;

			if (viewport->getRequiresDepthClear())
				clearBuffers |= FBT_DEPTH;

			if (viewport->getRequiresStencilClear())
				clearBuffers |= FBT_STENCIL;

			if (clearBuffers != 0)
			{
				RenderAPICore::instance().clearViewport(clearBuffers, viewport->getClearColor(),
					viewport->getClearDepthValue(), viewport->getClearStencilValue());
			}
		}

		// Render overlay post-scene callbacks
		auto iterCameraCallbacks = mRenderCallbacks.find(camera);
		if (iterCameraCallbacks != mRenderCallbacks.end())
		{
			for (auto& callbackPair : iterCameraCallbacks->second)
			{
				const RenderCallbackData& callbackData = callbackPair.second;

				if (!callbackData.overlay)
					continue;

				callbackData.callback();
			}
		}

		rendererCam.endRendering();

		gProfilerCPU().endSample("RenderOverlay");
	}
	
	void RenderBeast::refreshSamplerOverrides(bool force)
	{
		for (auto& entry : mSamplerOverrides)
		{
			SPtr<MaterialCore> material = entry.first;

			if (force)
			{
				SamplerOverrideUtility::destroySamplerOverrides(entry.second);
				entry.second = SamplerOverrideUtility::generateSamplerOverrides(material, mCoreOptions);
			}
			else
			{
				MaterialSamplerOverrides* materialOverrides = entry.second;
				UINT32 numPasses = material->getNumPasses();

				assert(numPasses == materialOverrides->numPasses);
				for (UINT32 i = 0; i < numPasses; i++)
				{
					SPtr<PassParametersCore> passParams = material->getPassParameters(i);
					PassSamplerOverrides& passOverrides = materialOverrides->passes[i];

					for (UINT32 j = 0; j < PassParametersCore::NUM_PARAMS; j++)
					{
						StageSamplerOverrides& stageOverrides = passOverrides.stages[j];

						SPtr<GpuParamsCore> params = passParams->getParamByIdx(j);
						if (params == nullptr)
							continue;

						const GpuParamDesc& paramDesc = params->getParamDesc();

						for (auto iter = paramDesc.samplers.begin(); iter != paramDesc.samplers.end(); ++iter)
						{
							UINT32 slot = iter->second.slot;
							SPtr<SamplerStateCore> samplerState = params->getSamplerState(slot);

							assert(stageOverrides.numStates > slot);

							if (samplerState != stageOverrides.stateOverrides[slot])
							{
								if (samplerState != nullptr)
									stageOverrides.stateOverrides[slot] = SamplerOverrideUtility::generateSamplerOverride(samplerState, mCoreOptions);
								else
									stageOverrides.stateOverrides[slot] = SamplerOverrideUtility::generateSamplerOverride(SamplerStateCore::getDefault(), mCoreOptions);;
							}	
						}
					}
				}
			}
		}
	}

	void RenderBeast::setPassParams(const SPtr<PassParametersCore>& passParams, const PassSamplerOverrides* samplerOverrides)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderAPICore& rs = RenderAPICore::instance();

		struct StageData
		{
			GpuProgramType type;
			SPtr<GpuParamsCore> params;
		};

		const UINT32 numStages = 6;
		StageData stages[numStages] =
		{
			{ GPT_VERTEX_PROGRAM, passParams->mVertParams },
			{ GPT_FRAGMENT_PROGRAM, passParams->mFragParams },
			{ GPT_GEOMETRY_PROGRAM, passParams->mGeomParams },
			{ GPT_HULL_PROGRAM, passParams->mHullParams },
			{ GPT_DOMAIN_PROGRAM, passParams->mDomainParams },
			{ GPT_COMPUTE_PROGRAM, passParams->mComputeParams }
		};

		for (UINT32 i = 0; i < numStages; i++)
		{
			const StageData& stage = stages[i];

			SPtr<GpuParamsCore> params = stage.params;
			if (params == nullptr)
				continue;

			const GpuParamDesc& paramDesc = params->getParamDesc();

			for (auto iter = paramDesc.samplers.begin(); iter != paramDesc.samplers.end(); ++iter)
			{
				SPtr<SamplerStateCore> samplerState;

				if (samplerOverrides != nullptr)
					samplerState = samplerOverrides->stages[i].stateOverrides[iter->second.slot];
				else
					samplerState = params->getSamplerState(iter->second.slot);

				if (samplerState == nullptr)
					rs.setSamplerState(stage.type, iter->second.slot, SamplerStateCore::getDefault());
				else
					rs.setSamplerState(stage.type, iter->second.slot, samplerState);
			}

			for (auto iter = paramDesc.textures.begin(); iter != paramDesc.textures.end(); ++iter)
			{
				SPtr<TextureCore> texture = params->getTexture(iter->second.slot);
				rs.setTexture(stage.type, iter->second.slot, texture);
			}

			for (auto iter = paramDesc.loadStoreTextures.begin(); iter != paramDesc.loadStoreTextures.end(); ++iter)
			{
				SPtr<TextureCore> texture = params->getLoadStoreTexture(iter->second.slot);
				const TextureSurface& surface = params->getLoadStoreSurface(iter->second.slot);

				if (texture == nullptr)
					rs.setLoadStoreTexture(stage.type, iter->second.slot, false, nullptr, surface);
				else
					rs.setLoadStoreTexture(stage.type, iter->second.slot, true, texture, surface);
			}

			for (auto iter = paramDesc.paramBlocks.begin(); iter != paramDesc.paramBlocks.end(); ++iter)
			{
				SPtr<GpuParamBlockBufferCore> blockBuffer = params->getParamBlockBuffer(iter->second.slot);
				blockBuffer->flushToGPU();

				rs.setParamBuffer(stage.type, iter->second.slot, blockBuffer, paramDesc);
			}
		}
	}
}