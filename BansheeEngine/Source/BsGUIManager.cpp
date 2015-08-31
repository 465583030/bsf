#include "BsGUIManager.h"
#include "BsCGUIWidget.h"
#include "BsGUIElement.h"
#include "BsImageSprite.h"
#include "BsSpriteTexture.h"
#include "BsTime.h"
#include "BsSceneObject.h"
#include "BsMaterial.h"
#include "BsMeshData.h"
#include "BsVertexDataDesc.h"
#include "BsMesh.h"
#include "BsRenderWindowManager.h"
#include "BsPlatform.h"
#include "BsRect2I.h"
#include "BsCoreApplication.h"
#include "BsException.h"
#include "BsInput.h"
#include "BsPass.h"
#include "BsDebug.h"
#include "BsGUIInputCaret.h"
#include "BsGUIInputSelection.h"
#include "BsGUIListBox.h"
#include "BsGUIButton.h"
#include "BsGUIDropDownMenu.h"
#include "BsGUIContextMenu.h"
#include "BsDragAndDropManager.h"
#include "BsGUIDropDownBoxManager.h"
#include "BsGUIContextMenu.h"
#include "BsProfilerCPU.h"
#include "BsMeshHeap.h"
#include "BsTransientMesh.h"
#include "BsVirtualInput.h"
#include "BsCursor.h"
#include "BsCoreThread.h"
#include "BsRendererManager.h"
#include "BsRenderer.h"
#include "BsCamera.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	struct GUIGroupElement
	{
		GUIGroupElement()
		{ }

		GUIGroupElement(GUIElement* _element, UINT32 _renderElement)
			:element(_element), renderElement(_renderElement)
		{ }

		GUIElement* element;
		UINT32 renderElement;
	};

	struct GUIMaterialGroup
	{
		GUIMaterialInfo matInfo;
		UINT32 numQuads;
		UINT32 depth;
		Rect2I bounds;
		Vector<GUIGroupElement> elements;
	};

	const UINT32 GUIManager::DRAG_DISTANCE = 3;
	const UINT32 GUIManager::MESH_HEAP_INITIAL_NUM_VERTS = 16384;
	const UINT32 GUIManager::MESH_HEAP_INITIAL_NUM_INDICES = 49152;

	GUIManager::GUIManager()
		:mSeparateMeshesByWidget(true), mActiveMouseButton(GUIMouseButton::Left),
		mCaretBlinkInterval(0.5f), mCaretLastBlinkTime(0.0f), mCaretColor(1.0f, 0.6588f, 0.0f), mIsCaretOn(false),
		mTextSelectionColor(1.0f, 0.6588f, 0.0f), mInputCaret(nullptr), mInputSelection(nullptr), mDragState(DragState::NoDrag),
		mActiveCursor(CursorType::Arrow), mCoreDirty(false)
	{
		mOnPointerMovedConn = gInput().onPointerMoved.connect(std::bind(&GUIManager::onPointerMoved, this, _1));
		mOnPointerPressedConn = gInput().onPointerPressed.connect(std::bind(&GUIManager::onPointerPressed, this, _1));
		mOnPointerReleasedConn = gInput().onPointerReleased.connect(std::bind(&GUIManager::onPointerReleased, this, _1));
		mOnPointerDoubleClick = gInput().onPointerDoubleClick.connect(std::bind(&GUIManager::onPointerDoubleClick, this, _1));
		mOnTextInputConn = gInput().onCharInput.connect(std::bind(&GUIManager::onTextInput, this, _1)); 
		mOnInputCommandConn = gInput().onInputCommand.connect(std::bind(&GUIManager::onInputCommandEntered, this, _1)); 
		mOnVirtualButtonDown = VirtualInput::instance().onButtonDown.connect(std::bind(&GUIManager::onVirtualButtonDown, this, _1, _2));

		mWindowGainedFocusConn = RenderWindowManager::instance().onFocusGained.connect(std::bind(&GUIManager::onWindowFocusGained, this, _1));
		mWindowLostFocusConn = RenderWindowManager::instance().onFocusLost.connect(std::bind(&GUIManager::onWindowFocusLost, this, _1));
		mMouseLeftWindowConn = RenderWindowManager::instance().onMouseLeftWindow.connect(std::bind(&GUIManager::onMouseLeftWindow, this, _1));

		mInputCaret = bs_new<GUIInputCaret>();
		mInputSelection = bs_new<GUIInputSelection>();

		DragAndDropManager::startUp();
		mDragEndedConn = DragAndDropManager::instance().onDragEnded.connect(std::bind(&GUIManager::onMouseDragEnded, this, _1, _2));

		GUIDropDownBoxManager::startUp();

		mVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		mVertexDesc->addVertElem(VET_FLOAT2, VES_POSITION);
		mVertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

		mMeshHeap = MeshHeap::create(MESH_HEAP_INITIAL_NUM_VERTS, MESH_HEAP_INITIAL_NUM_INDICES, mVertexDesc);

		// Need to defer this call because I want to make sure all managers are initialized first
		deferredCall(std::bind(&GUIManager::updateCaretTexture, this));
		deferredCall(std::bind(&GUIManager::updateTextSelectionTexture, this));

		mCore.store(bs_new<GUIManagerCore>(), std::memory_order_release);
	}

	GUIManager::~GUIManager()
	{
		GUIDropDownBoxManager::shutDown();
		DragAndDropManager::shutDown();

		// Make a copy of widgets, since destroying them will remove them from mWidgets and
		// we can't iterate over an array thats getting modified
		Vector<WidgetInfo> widgetCopy = mWidgets;
		for(auto& widget : widgetCopy)
			widget.widget->destroy();

		// Ensure everything queued get destroyed, loop until queue empties
		while (processDestroyQueue())
		{ }

		mOnPointerPressedConn.disconnect();
		mOnPointerReleasedConn.disconnect();
		mOnPointerMovedConn.disconnect();
		mOnPointerDoubleClick.disconnect();
		mOnTextInputConn.disconnect();
		mOnInputCommandConn.disconnect();
		mOnVirtualButtonDown.disconnect();

		mDragEndedConn.disconnect();

		mWindowGainedFocusConn.disconnect();
		mWindowLostFocusConn.disconnect();

		mMouseLeftWindowConn.disconnect();

		bs_delete(mInputCaret);
		bs_delete(mInputSelection);

		gCoreAccessor().queueCommand(std::bind(&GUIManager::destroyCore, this, mCore.load(std::memory_order_relaxed)));

		assert(mCachedGUIData.size() == 0);
	}

	void GUIManager::destroyCore(GUIManagerCore* core)
	{
		bs_delete(core);
	}

	void GUIManager::registerWidget(CGUIWidget* widget)
	{
		mWidgets.push_back(WidgetInfo(widget));

		const Viewport* renderTarget = widget->getTarget();

		auto findIter = mCachedGUIData.find(renderTarget);

		if(findIter == end(mCachedGUIData))
			mCachedGUIData[renderTarget] = GUIRenderData();

		GUIRenderData& windowData = mCachedGUIData[renderTarget];
		windowData.widgets.push_back(widget);
		windowData.isDirty = true;
	}

	void GUIManager::unregisterWidget(CGUIWidget* widget)
	{
		{
			auto findIter = std::find_if(begin(mWidgets), end(mWidgets), [=] (const WidgetInfo& x) { return x.widget == widget; } );

			if(findIter != mWidgets.end())
				mWidgets.erase(findIter);
		}

		{
			auto findIter = std::find_if(begin(mElementsInFocus), end(mElementsInFocus), [=](const ElementInfo& x) { return x.widget == widget; });

			if (findIter != mElementsInFocus.end())
				findIter->widget = nullptr;
		}

		{
			auto findIter = std::find_if(begin(mElementsUnderPointer), end(mElementsUnderPointer), [=](const ElementInfoUnderPointer& x) { return x.widget == widget; });

			if (findIter != mElementsUnderPointer.end())
				findIter->widget = nullptr;
		}

		{
			auto findIter = std::find_if(begin(mActiveElements), end(mActiveElements), [=](const ElementInfo& x) { return x.widget == widget; });

			if (findIter != mActiveElements.end())
				findIter->widget = nullptr;
		}

		const Viewport* renderTarget = widget->getTarget();
		GUIRenderData& renderData = mCachedGUIData[renderTarget];

		{
			auto findIter = std::find(begin(renderData.widgets), end(renderData.widgets), widget);
			
			if(findIter != end(renderData.widgets))
				renderData.widgets.erase(findIter);
		}

		if(renderData.widgets.size() == 0)
		{
			for (auto& mesh : renderData.cachedMeshes)
			{
				if (mesh != nullptr)
					mMeshHeap->dealloc(mesh);
			}

			mCachedGUIData.erase(renderTarget);
			mCoreDirty = true;
		}
		else
			renderData.isDirty = true;
	}

	void GUIManager::update()
	{
		DragAndDropManager::instance()._update();

		// Update layouts
		gProfilerCPU().beginSample("UpdateLayout");
		for(auto& widgetInfo : mWidgets)
		{
			widgetInfo.widget->_updateLayout();
		}
		gProfilerCPU().endSample("UpdateLayout");

		// Destroy all queued elements (and loop in case any new ones get queued during destruction)
		do
		{
			mNewElementsUnderPointer.clear();
			for (auto& elementInfo : mElementsUnderPointer)
			{
				if (!elementInfo.element->_isDestroyed())
					mNewElementsUnderPointer.push_back(elementInfo);
			}

			mElementsUnderPointer.swap(mNewElementsUnderPointer);

			mNewActiveElements.clear();
			for (auto& elementInfo : mActiveElements)
			{
				if (!elementInfo.element->_isDestroyed())
					mNewActiveElements.push_back(elementInfo);
			}

			mActiveElements.swap(mNewActiveElements);

			mNewElementsInFocus.clear();
			for (auto& elementInfo : mElementsInFocus)
			{
				if (!elementInfo.element->_isDestroyed())
					mNewElementsInFocus.push_back(elementInfo);
			}

			mElementsInFocus.swap(mNewElementsInFocus);

			for (auto& focusElementInfo : mForcedFocusElements)
			{
				if (focusElementInfo.element->_isDestroyed())
					continue;

				if (focusElementInfo.focus)
				{
					auto iterFind = std::find_if(mElementsInFocus.begin(), mElementsInFocus.end(),
						[&](const ElementInfo& x) { return x.element == focusElementInfo.element; });

					if (iterFind == mElementsInFocus.end())
					{
						mElementsInFocus.push_back(ElementInfo(focusElementInfo.element, focusElementInfo.element->_getParentWidget()));

						mCommandEvent = GUICommandEvent();
						mCommandEvent.setType(GUICommandEventType::FocusGained);

						sendCommandEvent(focusElementInfo.element, mCommandEvent);
					}
				}
				else
				{
					mNewElementsInFocus.clear();
					for (auto& elementInfo : mElementsInFocus)
					{
						if (elementInfo.element == focusElementInfo.element)
						{
							mCommandEvent = GUICommandEvent();
							mCommandEvent.setType(GUICommandEventType::FocusLost);

							sendCommandEvent(elementInfo.element, mCommandEvent);
						}
						else
							mNewElementsInFocus.push_back(elementInfo);
					}

					mElementsInFocus.swap(mNewElementsInFocus);
				}
			}

			mForcedFocusElements.clear();

		} while (processDestroyQueue());

		// Blink caret
		float curTime = gTime().getTime();

		if ((curTime - mCaretLastBlinkTime) >= mCaretBlinkInterval)
		{
			mCaretLastBlinkTime = curTime;
			mIsCaretOn = !mIsCaretOn;

			mCommandEvent = GUICommandEvent();
			mCommandEvent.setType(GUICommandEventType::Redraw);

			for (auto& elementInfo : mElementsInFocus)
			{
				sendCommandEvent(elementInfo.element, mCommandEvent);
			}
		}

		PROFILE_CALL(updateMeshes(), "UpdateMeshes");

		// Send potentially updated meshes to core for rendering
		if (mCoreDirty)
		{
			UnorderedMap<SPtr<CameraCore>, Vector<GUICoreRenderData>> corePerCameraData;

			for (auto& viewportData : mCachedGUIData)
			{
				const GUIRenderData& renderData = viewportData.second;

				SPtr<Camera> camera;
				for (auto& widget : viewportData.second.widgets)
				{
					camera = widget->getCamera();
					if (camera != nullptr)
						break;
				}

				if (camera == nullptr)
					continue;

				auto insertedData = corePerCameraData.insert(std::make_pair(camera->getCore(), Vector<GUICoreRenderData>()));
				Vector<GUICoreRenderData>& cameraData = insertedData.first->second;

				UINT32 meshIdx = 0;
				for (auto& mesh : renderData.cachedMeshes)
				{
					GUIMaterialInfo materialInfo = renderData.cachedMaterials[meshIdx];
					CGUIWidget* widget = renderData.cachedWidgetsPerMesh[meshIdx];

					if (materialInfo.material == nullptr || !materialInfo.material.isLoaded())
					{
						meshIdx++;
						continue;
					}

					if (mesh == nullptr)
					{
						meshIdx++;
						continue;
					}

					cameraData.push_back(GUICoreRenderData());
					GUICoreRenderData& newEntry = cameraData.back();

					newEntry.material = materialInfo.material->getCore();
					newEntry.mesh = mesh->getCore();
					newEntry.worldTransform = widget->SO()->getWorldTfrm();

					meshIdx++;
				}
			}

			GUIManagerCore* core = mCore.load(std::memory_order_relaxed);
			gCoreAccessor().queueCommand(std::bind(&GUIManagerCore::updateData, core, corePerCameraData));

			mCoreDirty = false;
		}
	}

	void GUIManager::updateMeshes()
	{
		for(auto& cachedMeshData : mCachedGUIData)
		{
			GUIRenderData& renderData = cachedMeshData.second;

			// Check if anything is dirty. If nothing is we can skip the update
			bool isDirty = renderData.isDirty;
			renderData.isDirty = false;

			for(auto& widget : renderData.widgets)
			{
				if (widget->isDirty(true))
				{
					isDirty = true;
				}
			}

			if(!isDirty)
				continue;

			mCoreDirty = true;

			bs_frame_mark();
			{
				// Make a list of all GUI elements, sorted from farthest to nearest (highest depth to lowest)
				auto elemComp = [](const GUIGroupElement& a, const GUIGroupElement& b)
				{
					UINT32 aDepth = a.element->_getRenderElementDepth(a.renderElement);
					UINT32 bDepth = b.element->_getRenderElementDepth(b.renderElement);

					// Compare pointers just to differentiate between two elements with the same depth, their order doesn't really matter, but std::set
					// requires all elements to be unique
					return (aDepth > bDepth) || 
						(aDepth == bDepth && a.element > b.element) || 
						(aDepth == bDepth && a.element == b.element && a.renderElement > b.renderElement); 
				};

				FrameSet<GUIGroupElement, std::function<bool(const GUIGroupElement&, const GUIGroupElement&)>> allElements(elemComp);

				for (auto& widget : renderData.widgets)
				{
					const Vector<GUIElement*>& elements = widget->getElements();

					for (auto& element : elements)
					{
						if (element->_isDisabled())
							continue;

						UINT32 numRenderElems = element->_getNumRenderElements();
						for (UINT32 i = 0; i < numRenderElems; i++)
						{
							allElements.insert(GUIGroupElement(element, i));
						}
					}
				}

				// Group the elements in such a way so that we end up with a smallest amount of
				// meshes, without breaking back to front rendering order
				FrameUnorderedMap<UINT64, FrameVector<GUIMaterialGroup>> materialGroups;
				for (auto& elem : allElements)
				{
					GUIElement* guiElem = elem.element;
					UINT32 renderElemIdx = elem.renderElement;
					UINT32 elemDepth = guiElem->_getRenderElementDepth(renderElemIdx);

					Rect2I tfrmedBounds = guiElem->_getClippedBounds();
					tfrmedBounds.transform(guiElem->_getParentWidget()->SO()->getWorldTfrm());

					const GUIMaterialInfo& matInfo = guiElem->_getMaterial(renderElemIdx);

					UINT64 materialId = matInfo.material->getInternalID(); 

					// If this is a new material, add a new list of groups
					auto findIterMaterial = materialGroups.find(materialId);
					if (findIterMaterial == end(materialGroups))
						materialGroups[materialId] = FrameVector<GUIMaterialGroup>();

					// Try to find a group this material will fit in:
					//  - Group that has a depth value same or one below elements depth will always be a match
					//  - Otherwise, we search higher depth values as well, but we only use them if no elements in between those depth values
					//    overlap the current elements bounds.
					FrameVector<GUIMaterialGroup>& allGroups = materialGroups[materialId];
					GUIMaterialGroup* foundGroup = nullptr;

					for (auto groupIter = allGroups.rbegin(); groupIter != allGroups.rend(); ++groupIter)
					{
						// If we separate meshes by widget, ignore any groups with widget parents other than mine
						if (mSeparateMeshesByWidget)
						{
							if (groupIter->elements.size() > 0)
							{
								GUIElement* otherElem = groupIter->elements.begin()->element; // We only need to check the first element
								if (otherElem->_getParentWidget() != guiElem->_getParentWidget())
									continue;
							}
						}

						GUIMaterialGroup& group = *groupIter;

						if (group.depth == elemDepth || group.depth == (elemDepth - 1))
						{
							foundGroup = &group;
							break;
						}
						else
						{
							UINT32 startDepth = elemDepth;
							UINT32 endDepth = group.depth;

							Rect2I potentialGroupBounds = group.bounds;
							potentialGroupBounds.encapsulate(tfrmedBounds);

							bool foundOverlap = false;
							for (auto& material : materialGroups)
							{
								for (auto& matGroup : material.second)
								{
									if (&matGroup == &group)
										continue;

									if (matGroup.depth > startDepth && matGroup.depth < endDepth)
									{
										if (matGroup.bounds.overlaps(potentialGroupBounds))
										{
											foundOverlap = true;
											break;
										}
									}
								}
							}

							if (!foundOverlap)
							{
								foundGroup = &group;
								break;
							}
						}
					}

					if (foundGroup == nullptr)
					{
						allGroups.push_back(GUIMaterialGroup());
						foundGroup = &allGroups[allGroups.size() - 1];

						foundGroup->depth = elemDepth;
						foundGroup->bounds = tfrmedBounds;
						foundGroup->elements.push_back(GUIGroupElement(guiElem, renderElemIdx));
						foundGroup->matInfo = matInfo;
						foundGroup->numQuads = guiElem->_getNumQuads(renderElemIdx);
					}
					else
					{
						foundGroup->bounds.encapsulate(tfrmedBounds);
						foundGroup->elements.push_back(GUIGroupElement(guiElem, renderElemIdx));
						foundGroup->depth = std::min(foundGroup->depth, elemDepth);
						foundGroup->numQuads += guiElem->_getNumQuads(renderElemIdx);
					}
				}

				// Make a list of all GUI elements, sorted from farthest to nearest (highest depth to lowest)
				auto groupComp = [](GUIMaterialGroup* a, GUIMaterialGroup* b)
				{
					return (a->depth > b->depth) || (a->depth == b->depth && a > b);
					// Compare pointers just to differentiate between two elements with the same depth, their order doesn't really matter, but std::set
					// requires all elements to be unique
				};

				FrameSet<GUIMaterialGroup*, std::function<bool(GUIMaterialGroup*, GUIMaterialGroup*)>> sortedGroups(groupComp);
				for(auto& material : materialGroups)
				{
					for(auto& group : material.second)
					{
						sortedGroups.insert(&group);
					}
				}

				UINT32 numMeshes = (UINT32)sortedGroups.size();
				UINT32 oldNumMeshes = (UINT32)renderData.cachedMeshes.size();

				if(numMeshes < oldNumMeshes)
				{
					for (UINT32 i = numMeshes; i < oldNumMeshes; i++)
						mMeshHeap->dealloc(renderData.cachedMeshes[i]);

					renderData.cachedMeshes.resize(numMeshes);
				}

				renderData.cachedMaterials.resize(numMeshes);

				if(mSeparateMeshesByWidget)
					renderData.cachedWidgetsPerMesh.resize(numMeshes);

				// Fill buffers for each group and update their meshes
				UINT32 groupIdx = 0;
				for(auto& group : sortedGroups)
				{
					renderData.cachedMaterials[groupIdx] = group->matInfo;

					if(mSeparateMeshesByWidget)
					{
						if(group->elements.size() == 0)
							renderData.cachedWidgetsPerMesh[groupIdx] = nullptr;
						else
						{
							GUIElement* elem = group->elements.begin()->element;
							renderData.cachedWidgetsPerMesh[groupIdx] = elem->_getParentWidget();
						}
					}

					MeshDataPtr meshData = bs_shared_ptr_new<MeshData>(group->numQuads * 4, group->numQuads * 6, mVertexDesc);

					UINT8* vertices = meshData->getElementData(VES_POSITION);
					UINT8* uvs = meshData->getElementData(VES_TEXCOORD);
					UINT32* indices = meshData->getIndices32();
					UINT32 vertexStride = meshData->getVertexDesc()->getVertexStride();
					UINT32 indexStride = meshData->getIndexElementSize();

					UINT32 quadOffset = 0;
					for(auto& matElement : group->elements)
					{
						matElement.element->_fillBuffer(vertices, uvs, indices, quadOffset, group->numQuads, vertexStride, indexStride, matElement.renderElement);

						UINT32 numQuads = matElement.element->_getNumQuads(matElement.renderElement);
						UINT32 indexStart = quadOffset * 6;
						UINT32 indexEnd = indexStart + numQuads * 6;
						UINT32 vertOffset = quadOffset * 4;

						for(UINT32 i = indexStart; i < indexEnd; i++)
							indices[i] += vertOffset;

						quadOffset += numQuads;
					}

					if(groupIdx < (UINT32)renderData.cachedMeshes.size())
					{
						mMeshHeap->dealloc(renderData.cachedMeshes[groupIdx]);
						renderData.cachedMeshes[groupIdx] = mMeshHeap->alloc(meshData);
					}
					else
					{
						renderData.cachedMeshes.push_back(mMeshHeap->alloc(meshData));
					}

					groupIdx++;
				}
			}

			bs_frame_clear();			
		}
	}

	void GUIManager::updateCaretTexture()
	{
		if(mCaretTexture == nullptr)
		{
			HTexture newTex = Texture::create(TEX_TYPE_2D, 1, 1, 0, PF_R8G8B8A8);
			mCaretTexture = SpriteTexture::create(newTex);
		}

		const HTexture& tex = mCaretTexture->getTexture();
		UINT32 subresourceIdx = tex->getProperties().mapToSubresourceIdx(0, 0);
		PixelDataPtr data = tex->getProperties().allocateSubresourceBuffer(subresourceIdx);

		data->setColorAt(mCaretColor, 0, 0);
		tex->writeSubresource(gCoreAccessor(), subresourceIdx, data, false);
	}

	void GUIManager::updateTextSelectionTexture()
	{
		if(mTextSelectionTexture == nullptr)
		{
			HTexture newTex = Texture::create(TEX_TYPE_2D, 1, 1, 0, PF_R8G8B8A8);
			mTextSelectionTexture = SpriteTexture::create(newTex);
		}

		const HTexture& tex = mTextSelectionTexture->getTexture();
		UINT32 subresourceIdx = tex->getProperties().mapToSubresourceIdx(0, 0);
		PixelDataPtr data = tex->getProperties().allocateSubresourceBuffer(subresourceIdx);

		data->setColorAt(mTextSelectionColor, 0, 0);

		tex->writeSubresource(gCoreAccessor(), subresourceIdx, data, false);
	}

	void GUIManager::onMouseDragEnded(const PointerEvent& event, DragCallbackInfo& dragInfo)
	{
		GUIMouseButton guiButton = buttonToGUIButton(event.button);

		if(DragAndDropManager::instance().isDragInProgress() && guiButton == GUIMouseButton::Left)
		{
			for(auto& elementInfo : mElementsUnderPointer)
			{
				Vector2I localPos;

				if(elementInfo.widget != nullptr)
					localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);

				bool acceptDrop = true;
				if(DragAndDropManager::instance().needsValidDropTarget())
				{
					acceptDrop = elementInfo.element->_acceptDragAndDrop(localPos, DragAndDropManager::instance().getDragTypeId());
				}

				if(acceptDrop)
				{
					mMouseEvent.setDragAndDropDroppedData(localPos, DragAndDropManager::instance().getDragTypeId(), DragAndDropManager::instance().getDragData());
					dragInfo.processed = sendMouseEvent(elementInfo.element, mMouseEvent);

					if(dragInfo.processed)
						return;
				}
			}
		}

		dragInfo.processed = false;
	}

	void GUIManager::onPointerMoved(const PointerEvent& event)
	{
		if(event.isUsed())
			return;

		bool buttonStates[(int)GUIMouseButton::Count];
		buttonStates[0] = event.buttonStates[0];
		buttonStates[1] = event.buttonStates[1];
		buttonStates[2] = event.buttonStates[2];

		if(findElementUnderPointer(event.screenPos, buttonStates, event.shift, event.control, event.alt))
			event.markAsUsed();

		if(mDragState == DragState::HeldWithoutDrag)
		{
			UINT32 dist = mLastPointerClickPos.manhattanDist(event.screenPos);

			if(dist > DRAG_DISTANCE)
			{
				for(auto& activeElement : mActiveElements)
				{
					Vector2I localPos = getWidgetRelativePos(activeElement.widget, event.screenPos);
					Vector2I localDragStartPos = getWidgetRelativePos(activeElement.widget, mLastPointerClickPos);

					mMouseEvent.setMouseDragStartData(localPos, localDragStartPos);
					if(sendMouseEvent(activeElement.element, mMouseEvent))
						event.markAsUsed();
				}

				mDragState = DragState::Dragging;
				mDragStartPos = event.screenPos;
			}
		}

		// If mouse is being held down send MouseDrag events
		if(mDragState == DragState::Dragging)
		{
			for(auto& activeElement : mActiveElements)
			{
				if(mLastPointerScreenPos != event.screenPos)
				{
					Vector2I localPos = getWidgetRelativePos(activeElement.widget, event.screenPos);

					mMouseEvent.setMouseDragData(localPos, event.screenPos - mDragStartPos);
					if(sendMouseEvent(activeElement.element, mMouseEvent))
						event.markAsUsed();
				}
			}

			mLastPointerScreenPos = event.screenPos;

			// Also if drag is in progress send DragAndDrop events
			if(DragAndDropManager::instance().isDragInProgress())
			{
				bool acceptDrop = true;
				for(auto& elementInfo : mElementsUnderPointer)
				{
					Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);

					acceptDrop = true;
					if(DragAndDropManager::instance().needsValidDropTarget())
					{
						acceptDrop = elementInfo.element->_acceptDragAndDrop(localPos, DragAndDropManager::instance().getDragTypeId());
					}

					if(acceptDrop)
					{
						mMouseEvent.setDragAndDropDraggedData(localPos, DragAndDropManager::instance().getDragTypeId(), DragAndDropManager::instance().getDragData());
						if(sendMouseEvent(elementInfo.element, mMouseEvent))
						{
							event.markAsUsed();
							break;
						}
					}
				}

				if(acceptDrop)
				{
					if(mActiveCursor != CursorType::ArrowDrag)
					{
						Cursor::instance().setCursor(CursorType::ArrowDrag);
						mActiveCursor = CursorType::ArrowDrag;
					}
				}
				else
				{
					if(mActiveCursor != CursorType::Deny)
					{
						Cursor::instance().setCursor(CursorType::Deny);
						mActiveCursor = CursorType::Deny;
					}
				}				
			}
		}
		else // Otherwise, send MouseMove events if we are hovering over any element
		{
			if(mLastPointerScreenPos != event.screenPos)
			{
				bool moveProcessed = false;
				bool hasCustomCursor = false;
				for(auto& elementInfo : mElementsUnderPointer)
				{
					Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);

					if(!moveProcessed)
					{
						// Send MouseMove event
						mMouseEvent.setMouseMoveData(localPos);
						moveProcessed = sendMouseEvent(elementInfo.element, mMouseEvent);

						if(moveProcessed)
							event.markAsUsed();
					}

					if (mDragState == DragState::NoDrag)
					{
						CursorType newCursor = CursorType::Arrow;
						if(elementInfo.element->_hasCustomCursor(localPos, newCursor))
						{
							if(newCursor != mActiveCursor)
							{
								Cursor::instance().setCursor(newCursor);
								mActiveCursor = newCursor;
							}

							hasCustomCursor = true;
						}
					}

					if(moveProcessed)
						break;
				}

				// While dragging we don't want to modify the cursor
				if (mDragState == DragState::NoDrag)
				{
					if (!hasCustomCursor)
					{
						if (mActiveCursor != CursorType::Arrow)
						{
							Cursor::instance().setCursor(CursorType::Arrow);
							mActiveCursor = CursorType::Arrow;
						}
					}
				}
			}

			mLastPointerScreenPos = event.screenPos;

			if(Math::abs(event.mouseWheelScrollAmount) > 0.00001f)
			{
				for(auto& elementInfo : mElementsUnderPointer)
				{
					mMouseEvent.setMouseWheelScrollData(event.mouseWheelScrollAmount);
					if(sendMouseEvent(elementInfo.element, mMouseEvent))
					{
						event.markAsUsed();
						break;
					}
				}
			}
		}
	}

	void GUIManager::onPointerReleased(const PointerEvent& event)
	{
		if(event.isUsed())
			return;

		bool buttonStates[(int)GUIMouseButton::Count];
		buttonStates[0] = event.buttonStates[0];
		buttonStates[1] = event.buttonStates[1];
		buttonStates[2] = event.buttonStates[2];

		if(findElementUnderPointer(event.screenPos, buttonStates, event.shift, event.control, event.alt))
			event.markAsUsed();

		mMouseEvent = GUIMouseEvent(buttonStates, event.shift, event.control, event.alt);

		GUIMouseButton guiButton = buttonToGUIButton(event.button);

		// Send MouseUp event only if we are over the active element (we don't want to accidentally trigger other elements).
		// And only activate when a button that originally caused the active state is released, otherwise ignore it.
		if(mActiveMouseButton == guiButton)
		{
			for(auto& elementInfo : mElementsUnderPointer)
			{
				auto iterFind2 = std::find_if(mActiveElements.begin(), mActiveElements.end(), 
					[&](const ElementInfo& x) { return x.element == elementInfo.element; });

				if(iterFind2 != mActiveElements.end())
				{
					Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);
					mMouseEvent.setMouseUpData(localPos, guiButton);

					if(sendMouseEvent(elementInfo.element, mMouseEvent))
					{
						event.markAsUsed();
						break;
					}
				}
			}
		}

		// Send DragEnd event to whichever element is active
		bool acceptEndDrag = (mDragState == DragState::Dragging || mDragState == DragState::HeldWithoutDrag) && mActiveMouseButton == guiButton && 
			(guiButton == GUIMouseButton::Left);

		if(acceptEndDrag)
		{
			if(mDragState == DragState::Dragging)
			{
				for(auto& activeElement : mActiveElements)
				{
					Vector2I localPos = getWidgetRelativePos(activeElement.widget, event.screenPos);

					mMouseEvent.setMouseDragEndData(localPos);
					if(sendMouseEvent(activeElement.element, mMouseEvent))
						event.markAsUsed();
				}
			}

			mDragState = DragState::NoDrag;
		}

		if(mActiveMouseButton == guiButton)
		{
			mActiveElements.clear();
			mActiveMouseButton = GUIMouseButton::Left;
		}

		if(mActiveCursor != CursorType::Arrow)
		{
			Cursor::instance().setCursor(CursorType::Arrow);
			mActiveCursor = CursorType::Arrow;
		}
	}

	void GUIManager::onPointerPressed(const PointerEvent& event)
	{
		if(event.isUsed())
			return;

		bool buttonStates[(int)GUIMouseButton::Count];
		buttonStates[0] = event.buttonStates[0];
		buttonStates[1] = event.buttonStates[1];
		buttonStates[2] = event.buttonStates[2];

		if(findElementUnderPointer(event.screenPos, buttonStates, event.shift, event.control, event.alt))
			event.markAsUsed();

		mMouseEvent = GUIMouseEvent(buttonStates, event.shift, event.control, event.alt);

		GUIMouseButton guiButton = buttonToGUIButton(event.button);

		// We only check for mouse down if mouse isn't already being held down, and we are hovering over an element
		if(mActiveElements.size() == 0)
		{
			mNewActiveElements.clear();
			for(auto& elementInfo : mElementsUnderPointer)
			{
				Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);
				mMouseEvent.setMouseDownData(localPos, guiButton);

				bool processed = sendMouseEvent(elementInfo.element, mMouseEvent);

				if(guiButton == GUIMouseButton::Left)
				{
					mDragState = DragState::HeldWithoutDrag;
					mLastPointerClickPos = event.screenPos;
				}

				mNewActiveElements.push_back(ElementInfo(elementInfo.element, elementInfo.widget));
				mActiveMouseButton = guiButton;

				if(processed)
				{
					event.markAsUsed();
					break;
				}
			}

			mActiveElements.swap(mNewActiveElements);
		}

		mNewElementsInFocus.clear();
		mCommandEvent = GUICommandEvent();
		
		// Determine elements that gained focus
		mCommandEvent.setType(GUICommandEventType::FocusGained);

		for(auto& elementInfo : mElementsUnderPointer)
		{
			mNewElementsInFocus.push_back(ElementInfo(elementInfo.element, elementInfo.widget));

			auto iterFind = std::find_if(begin(mElementsInFocus), end(mElementsInFocus), 
				[=] (const ElementInfo& x) { return x.element == elementInfo.element; });

			if(iterFind == mElementsInFocus.end())
			{
				sendCommandEvent(elementInfo.element, mCommandEvent);
			}
		}

		// Determine elements that lost focus
		mCommandEvent.setType(GUICommandEventType::FocusLost);

		for(auto& elementInfo : mElementsInFocus)
		{
			auto iterFind = std::find_if(begin(mNewElementsInFocus), end(mNewElementsInFocus), 
				[=] (const ElementInfo& x) { return x.element == elementInfo.element; });

			if(iterFind == mNewElementsInFocus.end())
			{
				sendCommandEvent(elementInfo.element, mCommandEvent);
			}
		}

		if(mElementsUnderPointer.size() > 0)
			event.markAsUsed();

		mElementsInFocus.swap(mNewElementsInFocus);

		// If right click try to open context menu
		if(buttonStates[2] == true) 
		{
			for(auto& elementInfo : mElementsUnderPointer)
			{
				GUIContextMenuPtr menu = elementInfo.element->_getContextMenu();

				if(menu != nullptr && elementInfo.widget != nullptr)
				{
					const RenderWindow* window = getWidgetWindow(*elementInfo.widget);
					Vector2I windowPos = window->screenToWindowPos(event.screenPos);

					menu->open(windowPos, *elementInfo.widget);
					event.markAsUsed();
					break;
				}
			}
		}
	}

	void GUIManager::onPointerDoubleClick(const PointerEvent& event)
	{
		if(event.isUsed())
			return;

		bool buttonStates[(int)GUIMouseButton::Count];
		buttonStates[0] = event.buttonStates[0];
		buttonStates[1] = event.buttonStates[1];
		buttonStates[2] = event.buttonStates[2];

		if(findElementUnderPointer(event.screenPos, buttonStates, event.shift, event.control, event.alt))
			event.markAsUsed();

		mMouseEvent = GUIMouseEvent(buttonStates, event.shift, event.control, event.alt);

		GUIMouseButton guiButton = buttonToGUIButton(event.button);

		// We only check for mouse down if we are hovering over an element
		for(auto& elementInfo : mElementsUnderPointer)
		{
			Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);

			mMouseEvent.setMouseDoubleClickData(localPos, guiButton);
			if(sendMouseEvent(elementInfo.element, mMouseEvent))
			{
				event.markAsUsed();
				break;
			}
		}
	}

	void GUIManager::onInputCommandEntered(InputCommandType commandType)
	{
		if(mElementsInFocus.size() == 0)
			return;

		mCommandEvent = GUICommandEvent();

		switch(commandType)
		{
		case InputCommandType::Backspace:
			mCommandEvent.setType(GUICommandEventType::Backspace);
			break;
		case InputCommandType::Delete:
			mCommandEvent.setType(GUICommandEventType::Delete);
			break;
		case InputCommandType::Return:
			mCommandEvent.setType(GUICommandEventType::Return);
			break;
		case InputCommandType::Escape:
			mCommandEvent.setType(GUICommandEventType::Escape);
			break;
		case InputCommandType::CursorMoveLeft:
			mCommandEvent.setType(GUICommandEventType::MoveLeft);
			break;
		case InputCommandType::CursorMoveRight:
			mCommandEvent.setType(GUICommandEventType::MoveRight);
			break;
		case InputCommandType::CursorMoveUp:
			mCommandEvent.setType(GUICommandEventType::MoveUp);
			break;
		case InputCommandType::CursorMoveDown:
			mCommandEvent.setType(GUICommandEventType::MoveDown);
			break;
		case InputCommandType::SelectLeft:
			mCommandEvent.setType(GUICommandEventType::SelectLeft);
			break;
		case InputCommandType::SelectRight:
			mCommandEvent.setType(GUICommandEventType::SelectRight);
			break;
		case InputCommandType::SelectUp:
			mCommandEvent.setType(GUICommandEventType::SelectUp);
			break;
		case InputCommandType::SelectDown:
			mCommandEvent.setType(GUICommandEventType::SelectDown);
			break;
		}

		for(auto& elementInfo : mElementsInFocus)
		{
			sendCommandEvent(elementInfo.element, mCommandEvent);
		}		
	}

	void GUIManager::onVirtualButtonDown(const VirtualButton& button, UINT32 deviceIdx)
	{
		mVirtualButtonEvent.setButton(button);
		
		for(auto& elementInFocus : mElementsInFocus)
		{
			bool processed = sendVirtualButtonEvent(elementInFocus.element, mVirtualButtonEvent);

			if(processed)
				break;
		}
	}

	bool GUIManager::findElementUnderPointer(const Vector2I& pointerScreenPos, bool buttonStates[3], bool shift, bool control, bool alt)
	{
		Vector<const RenderWindow*> widgetWindows;
		for(auto& widgetInfo : mWidgets)
			widgetWindows.push_back(getWidgetWindow(*widgetInfo.widget));

#if BS_DEBUG_MODE
		// Checks if all referenced windows actually exist
		Vector<RenderWindow*> activeWindows = RenderWindowManager::instance().getRenderWindows();
		for(auto& window : widgetWindows)
		{
			if(window == nullptr)
				continue;

			auto iterFind = std::find(begin(activeWindows), end(activeWindows), window);

			if(iterFind == activeWindows.end())
			{
				BS_EXCEPT(InternalErrorException, "GUI manager has a reference to a window that doesn't exist. \
												  Please detach all GUIWidgets from windows before destroying a window.");
			}
		}
#endif

		mNewElementsUnderPointer.clear();

		const RenderWindow* windowUnderPointer = nullptr;
		UnorderedSet<const RenderWindow*> uniqueWindows;

		for(auto& window : widgetWindows)
		{
			if(window == nullptr)
				continue;

			uniqueWindows.insert(window);
		}

		for(auto& window : uniqueWindows)
		{
			if(Platform::isPointOverWindow(*window, pointerScreenPos))
			{
				windowUnderPointer = window;
				break;
			}
		}

		if(windowUnderPointer != nullptr)
		{
			Vector2I windowPos = windowUnderPointer->screenToWindowPos(pointerScreenPos);
			Vector4 vecWindowPos((float)windowPos.x, (float)windowPos.y, 0.0f, 1.0f);

			UINT32 widgetIdx = 0;
			for(auto& widgetInfo : mWidgets)
			{
				if(widgetWindows[widgetIdx] == nullptr)
				{
					widgetIdx++;
					continue;
				}

				CGUIWidget* widget = widgetInfo.widget;
				if(widgetWindows[widgetIdx] == windowUnderPointer && widget->inBounds(windowToBridgedCoords(*widget, windowPos)))
				{
					const Vector<GUIElement*>& elements = widget->getElements();
					Vector2I localPos = getWidgetRelativePos(widget, pointerScreenPos);

					// Elements with lowest depth (most to the front) get handled first
					for(auto iter = elements.begin(); iter != elements.end(); ++iter)
					{
						GUIElement* element = *iter;

						if(!element->_isDisabled() && element->_isInBounds(localPos))
						{
							ElementInfoUnderPointer elementInfo(element, widget);

							auto iterFind = std::find_if(mElementsUnderPointer.begin(), mElementsUnderPointer.end(),
								[=](const ElementInfoUnderPointer& x) { return x.element == element; });

							if (iterFind != mElementsUnderPointer.end())
							{
								elementInfo.usesMouseOver = iterFind->usesMouseOver;
								elementInfo.receivedMouseOver = iterFind->receivedMouseOver;
							}

							mNewElementsUnderPointer.push_back(elementInfo);
						}
					}
				}

				widgetIdx++;
			}
		}

		std::sort(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(), 
			[](const ElementInfoUnderPointer& a, const ElementInfoUnderPointer& b)
		{
			return a.element->_getDepth() < b.element->_getDepth();
		});

		// Send MouseOut and MouseOver events

		bool eventProcessed = false;

		for (auto& elementInfo : mNewElementsUnderPointer)
		{
			GUIElement* element = elementInfo.element;
			CGUIWidget* widget = elementInfo.widget;

			if (elementInfo.receivedMouseOver)
			{
				elementInfo.isHovering = true;
				if (elementInfo.usesMouseOver)
					break;

				continue;
			}

			auto iterFind = std::find_if(mActiveElements.begin(), mActiveElements.end(),
				[&](const ElementInfo& x) { return x.element == element; });

			// Send MouseOver event
			if (mActiveElements.size() == 0 || iterFind != mActiveElements.end())
			{
				Vector2I localPos = getWidgetRelativePos(widget, pointerScreenPos);

				mMouseEvent = GUIMouseEvent(buttonStates, shift, control, alt);

				mMouseEvent.setMouseOverData(localPos);
				elementInfo.receivedMouseOver = true;
				elementInfo.isHovering = true;
				if (sendMouseEvent(element, mMouseEvent))
				{
					eventProcessed = true;
					elementInfo.usesMouseOver = true;
					break;
				}
			}
		}

		// Send DragAndDropLeft event - It is similar to MouseOut events but we send it to all
		// elements a user might hover over, while we send mouse over/out events only to active elements while dragging
		if (DragAndDropManager::instance().isDragInProgress())
		{
			for (auto& elementInfo : mElementsUnderPointer)
			{
				auto iterFind = std::find_if(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(),
					[=](const ElementInfoUnderPointer& x) { return x.element == elementInfo.element; });

				if (iterFind == mNewElementsUnderPointer.end())
				{
					Vector2I localPos = getWidgetRelativePos(elementInfo.widget, pointerScreenPos);

					mMouseEvent.setDragAndDropLeftData(localPos, DragAndDropManager::instance().getDragTypeId(), DragAndDropManager::instance().getDragData());
					if (sendMouseEvent(elementInfo.element, mMouseEvent))
					{
						eventProcessed = true;
						break;
					}
				}
			}
		}

		for(auto& elementInfo : mElementsUnderPointer)
		{
			GUIElement* element = elementInfo.element;
			CGUIWidget* widget = elementInfo.widget;

			auto iterFind = std::find_if(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(),
				[=](const ElementInfoUnderPointer& x) { return x.element == element; });

			if (!elementInfo.receivedMouseOver)
				continue;

			if (iterFind == mNewElementsUnderPointer.end() || !iterFind->isHovering)
			{
				auto iterFind2 = std::find_if(mActiveElements.begin(), mActiveElements.end(), 
					[=](const ElementInfo& x) { return x.element == element; });

				// Send MouseOut event
				if(mActiveElements.size() == 0 || iterFind2 != mActiveElements.end())
				{
					Vector2I localPos = getWidgetRelativePos(widget, pointerScreenPos);

					mMouseEvent.setMouseOutData(localPos);
					if (sendMouseEvent(element, mMouseEvent))
					{
						eventProcessed = true;
						break;
					}
				}
			}
		}

		mElementsUnderPointer.swap(mNewElementsUnderPointer);

		return eventProcessed;
	}

	void GUIManager::onTextInput(const TextInputEvent& event)
	{
		mTextInputEvent = GUITextInputEvent();
		mTextInputEvent.setData(event.textChar);

		for(auto& elementInFocus : mElementsInFocus)
		{
			if(sendTextInputEvent(elementInFocus.element, mTextInputEvent))
				event.markAsUsed();
		}
	}

	void GUIManager::onWindowFocusGained(RenderWindow& win)
	{
		for(auto& widgetInfo : mWidgets)
		{
			CGUIWidget* widget = widgetInfo.widget;
			if(getWidgetWindow(*widget) == &win)
				widget->ownerWindowFocusChanged();
		}
	}

	void GUIManager::onWindowFocusLost(RenderWindow& win)
	{
		for(auto& widgetInfo : mWidgets)
		{
			CGUIWidget* widget = widgetInfo.widget;
			if(getWidgetWindow(*widget) == &win)
				widget->ownerWindowFocusChanged();
		}

		mNewElementsInFocus.clear();
		for(auto& focusedElement : mElementsInFocus)
		{
			if (focusedElement.element->_isDestroyed())
				continue;

			if (focusedElement.widget != nullptr && getWidgetWindow(*focusedElement.widget) == &win)
			{
				mCommandEvent = GUICommandEvent();
				mCommandEvent.setType(GUICommandEventType::FocusLost);

				sendCommandEvent(focusedElement.element, mCommandEvent);
			}
			else
				mNewElementsInFocus.push_back(focusedElement);
		}

		mElementsInFocus.swap(mNewElementsInFocus);
	}

	// We stop getting mouse move events once it leaves the window, so make sure
	// nothing stays in hover state
	void GUIManager::onMouseLeftWindow(RenderWindow& win)
	{
		bool buttonStates[3];
		buttonStates[0] = false;
		buttonStates[1] = false;
		buttonStates[2] = false;

		mNewElementsUnderPointer.clear();

		for(auto& elementInfo : mElementsUnderPointer)
		{
			GUIElement* element = elementInfo.element;
			CGUIWidget* widget = elementInfo.widget;

			if (widget != nullptr && widget->getTarget()->getTarget().get() != &win)
			{
				mNewElementsUnderPointer.push_back(elementInfo);
				continue;
			}

			auto iterFind = std::find_if(mActiveElements.begin(), mActiveElements.end(), 
				[&](const ElementInfo& x) { return x.element == element; });

			// Send MouseOut event
			if(mActiveElements.size() == 0 || iterFind != mActiveElements.end())
			{
				Vector2I localPos = getWidgetRelativePos(widget, Vector2I());

				mMouseEvent.setMouseOutData(localPos);
				sendMouseEvent(element, mMouseEvent);
			}
		}

		mElementsUnderPointer.swap(mNewElementsUnderPointer);

		if(mDragState != DragState::Dragging)
		{
			if(mActiveCursor != CursorType::Arrow)
			{
				Cursor::instance().setCursor(CursorType::Arrow);
				mActiveCursor = CursorType::Arrow;
			}
		}
	}

	void GUIManager::queueForDestroy(GUIElement* element)
	{
		mScheduledForDestruction.push(element);
	}

	void GUIManager::setFocus(GUIElement* element, bool focus)
	{
		ElementFocusInfo efi;
		efi.element = element;
		efi.focus = focus;

		mForcedFocusElements.push_back(efi);
	}

	bool GUIManager::processDestroyQueue()
	{
		Stack<GUIElement*> toDestroy = mScheduledForDestruction;
		mScheduledForDestruction = Stack<GUIElement*>();

		while (!toDestroy.empty())
		{
			bs_delete(toDestroy.top());
			toDestroy.pop();
		}

		return !mScheduledForDestruction.empty();
	}

	void GUIManager::setInputBridge(const RenderTexture* renderTex, const GUIElement* element)
	{
		if(element == nullptr)
			mInputBridge.erase(renderTex);
		else
			mInputBridge[renderTex] = element;
	}

	GUIMouseButton GUIManager::buttonToGUIButton(PointerEventButton pointerButton) const
	{
		if(pointerButton == PointerEventButton::Left)
			return GUIMouseButton::Left;
		else if(pointerButton == PointerEventButton::Middle)
			return GUIMouseButton::Middle;
		else if(pointerButton == PointerEventButton::Right)
			return GUIMouseButton::Right;

		BS_EXCEPT(InvalidParametersException, "Provided button is not a GUI supported mouse button.");
	}

	Vector2I GUIManager::getWidgetRelativePos(const CGUIWidget* widget, const Vector2I& screenPos) const
	{
		if (widget == nullptr)
			return screenPos;

		const RenderWindow* window = getWidgetWindow(*widget);
		if(window == nullptr)
			return Vector2I();

		Vector2I windowPos = window->screenToWindowPos(screenPos);
		windowPos = windowToBridgedCoords(*widget, windowPos);

		const Matrix4& worldTfrm = widget->SO()->getWorldTfrm();

		Vector4 vecLocalPos = worldTfrm.inverse().multiplyAffine(Vector4((float)windowPos.x, (float)windowPos.y, 0.0f, 1.0f));
		Vector2I curLocalPos(Math::roundToInt(vecLocalPos.x), Math::roundToInt(vecLocalPos.y));

		return curLocalPos;
	}

	Vector2I GUIManager::windowToBridgedCoords(const CGUIWidget& widget, const Vector2I& windowPos) const
	{
		// This cast might not be valid (the render target could be a window), but we only really need to cast
		// so that mInputBridge map allows us to search through it - we don't access anything unless the target is bridged
		// (in which case we know it is a RenderTexture)
		const RenderTexture* renderTexture = static_cast<const RenderTexture*>(widget.getTarget()->getTarget().get());
		const RenderTargetProperties& rtProps = renderTexture->getProperties();

		auto iterFind = mInputBridge.find(renderTexture);
		if(iterFind != mInputBridge.end()) // Widget input is bridged, which means we need to transform the coordinates
		{
			const GUIElement* bridgeElement = iterFind->second;

			const Matrix4& worldTfrm = bridgeElement->_getParentWidget()->SO()->getWorldTfrm();

			Vector4 vecLocalPos = worldTfrm.inverse().multiplyAffine(Vector4((float)windowPos.x, (float)windowPos.y, 0.0f, 1.0f));
			Rect2I bridgeBounds = bridgeElement->_getLayoutData().area;

			// Find coordinates relative to the bridge element
			float x = vecLocalPos.x - (float)bridgeBounds.x;
			float y = vecLocalPos.y - (float)bridgeBounds.y;

			float scaleX = rtProps.getWidth() / (float)bridgeBounds.width;
			float scaleY = rtProps.getHeight() / (float)bridgeBounds.height;

			return Vector2I(Math::roundToInt(x * scaleX), Math::roundToInt(y * scaleY));
		}

		return windowPos;
	}

	const RenderWindow* GUIManager::getWidgetWindow(const CGUIWidget& widget) const
	{
		// This cast might not be valid (the render target could be a window), but we only really need to cast
		// so that mInputBridge map allows us to search through it - we don't access anything unless the target is bridged
		// (in which case we know it is a RenderTexture)
		const RenderTexture* renderTexture = static_cast<const RenderTexture*>(widget.getTarget()->getTarget().get());

		auto iterFind = mInputBridge.find(renderTexture);
		if(iterFind != mInputBridge.end())
		{
			CGUIWidget* parentWidget = iterFind->second->_getParentWidget();
			if(parentWidget != &widget)
			{
				return getWidgetWindow(*parentWidget);
			}
		}

		RenderTargetPtr renderTarget = widget.getTarget()->getTarget();
		Vector<RenderWindow*> renderWindows = RenderWindowManager::instance().getRenderWindows();

		auto iterFindWin = std::find(renderWindows.begin(), renderWindows.end(), renderTarget.get());
		if(iterFindWin != renderWindows.end())
			return static_cast<RenderWindow*>(renderTarget.get());

		return nullptr;
	}

	bool GUIManager::sendMouseEvent(GUIElement* element, const GUIMouseEvent& event)
	{
		if (element->_isDestroyed())
			return false;

		return element->_mouseEvent(event);
	}

	bool GUIManager::sendTextInputEvent(GUIElement* element, const GUITextInputEvent& event)
	{
		if (element->_isDestroyed())
			return false;

		return element->_textInputEvent(event);
	}

	bool GUIManager::sendCommandEvent(GUIElement* element, const GUICommandEvent& event)
	{
		if (element->_isDestroyed())
			return false;

		return element->_commandEvent(event);
	}

	bool GUIManager::sendVirtualButtonEvent(GUIElement* element, const GUIVirtualButtonEvent& event)
	{
		if (element->_isDestroyed())
			return false;

		return element->_virtualButtonEvent(event);
	}

	GUIManager& gGUIManager()
	{
		return GUIManager::instance();
	}

	GUIManagerCore::~GUIManagerCore()
	{
		CoreRendererPtr activeRenderer = RendererManager::instance().getActive();
		for (auto& cameraData : mPerCameraData)
			activeRenderer->_unregisterRenderCallback(cameraData.first.get(), -30);
	}

	void GUIManagerCore::updateData(const UnorderedMap<SPtr<CameraCore>, Vector<GUIManager::GUICoreRenderData>>& newPerCameraData)
	{
		bs_frame_mark();

		{
			FrameSet<SPtr<CameraCore>> validCameras;

			CoreRendererPtr activeRenderer = RendererManager::instance().getActive();
			for (auto& newCameraData : newPerCameraData)
			{
				UINT32 idx = 0;
				Vector<RenderData>* renderData = nullptr;
				for (auto& oldCameraData : mPerCameraData)
				{
					if (newCameraData.first == oldCameraData.first)
					{
						renderData = &oldCameraData.second;
						validCameras.insert(oldCameraData.first);
						break;
					}

					idx++;
				}

				if (renderData == nullptr)
				{
					SPtr<CameraCore> camera = newCameraData.first;

					auto insertedData = mPerCameraData.insert(std::make_pair(newCameraData.first, Vector<RenderData>()));
					renderData = &insertedData.first->second;

					activeRenderer->_registerRenderCallback(camera.get(), -30, std::bind(&GUIManagerCore::render, this, camera));
					validCameras.insert(camera);
				}

				renderData->clear();

				for (auto& entry : newCameraData.second)
				{
					renderData->push_back(RenderData());
					RenderData& newEntry = renderData->back();

					newEntry.mesh = entry.mesh;
					newEntry.material = entry.material;
					newEntry.worldTransform = entry.worldTransform;
					newEntry.invViewportWidthParam = newEntry.material->getParamFloat("invViewportWidth");
					newEntry.invViewportHeightParam = newEntry.material->getParamFloat("invViewportHeight");
					newEntry.worldTransformParam = newEntry.material->getParamMat4("worldTransform");
				}
			}

			FrameVector<SPtr<CameraCore>> cameraToRemove;
			for (auto& cameraData : mPerCameraData)
			{
				auto iterFind = validCameras.find(cameraData.first);
				if (iterFind == validCameras.end())
					cameraToRemove.push_back(cameraData.first);
			}

			for (auto& camera : cameraToRemove)
			{
				activeRenderer->_unregisterRenderCallback(camera.get(), -30);
				mPerCameraData.erase(camera);
			}
		}

		bs_frame_clear();
	}

	void GUIManagerCore::render(const SPtr<CameraCore>& camera)
	{
		Vector<RenderData>& renderData = mPerCameraData[camera];

		float invViewportWidth = 1.0f / (camera->getViewport()->getWidth() * 0.5f);
		float invViewportHeight = 1.0f / (camera->getViewport()->getHeight() * 0.5f);
		for (auto& entry : renderData)
		{
			entry.invViewportWidthParam.set(invViewportWidth);
			entry.invViewportHeightParam.set(invViewportHeight);
			entry.worldTransformParam.set(entry.worldTransform);

			// TODO - I shouldn't be re-applying the entire material for each entry, instead just check which programs
			// changed, and apply only those + the modified constant buffers and/or texture.

			CoreRenderer::setPass(entry.material, 0);
			CoreRenderer::draw(entry.mesh, entry.mesh->getProperties().getSubMesh(0));
		}
	}
}