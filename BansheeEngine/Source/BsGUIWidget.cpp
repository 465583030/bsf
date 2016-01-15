#include "BsGUIWidget.h"
#include "BsGUIManager.h"
#include "BsGUISkin.h"
#include "BsGUILabel.h"
#include "BsGUIPanel.h"
#include "BsCoreThreadAccessor.h"
#include "BsVector2I.h"
#include "BsCCamera.h"
#include "BsViewport.h"
#include "BsSceneObject.h"
#include "BsBuiltinResources.h"

namespace BansheeEngine
{
	GUIWidget::GUIWidget(const CameraPtr& camera)
		:mWidgetIsDirty(false), mCamera(camera), mDepth(0), mPanel(nullptr), mIsActive(true)
	{
		construct(camera);
	}

	GUIWidget::GUIWidget(const HCamera& camera)
		:mWidgetIsDirty(false), mCamera(camera->_getCamera()), mDepth(0), mPanel(nullptr), mIsActive(true)
	{
		construct(mCamera);
	}

	void GUIWidget::construct(const CameraPtr& camera)
	{
		if(mCamera != nullptr)
			mOwnerTargetResizedConn = mCamera->getViewport()->getTarget()->onResized.connect(std::bind(&GUIWidget::ownerTargetResized, this));

		GUIManager::instance().registerWidget(this);

		mPanel = GUIPanel::create();
		mPanel->_changeParentWidget(this);
		updateRootPanel();
	}

	GUIWidget::~GUIWidget()
	{
		_destroy();
	}

	SPtr<GUIWidget> GUIWidget::create(const CameraPtr& camera)
	{
		return bs_shared_ptr(new (bs_alloc<GUIWidget>()) GUIWidget(camera));
	}

	SPtr<GUIWidget> GUIWidget::create(const HCamera& camera)
	{
		return bs_shared_ptr(new (bs_alloc<GUIWidget>()) GUIWidget(camera));
	}

	void GUIWidget::_destroy()
	{
		if (mPanel != nullptr)
		{
			GUILayout::destroy(mPanel);
			mPanel = nullptr;
		}

		if (mCamera != nullptr)
		{
			GUIManager::instance().unregisterWidget(this);
			mOwnerTargetResizedConn.disconnect();

			mCamera = nullptr;
		}

		mElements.clear();
		mDirtyContents.clear();
	}

	void GUIWidget::setDepth(UINT8 depth)
	{
		mDepth = depth; 
		mWidgetIsDirty = true;

		updateRootPanel();
	}

	Viewport* GUIWidget::getTarget() const
	{
		if(mCamera != nullptr)
			return mCamera->getViewport().get();

		return nullptr;
	}

	void GUIWidget::_updateTransform(const HSceneObject& parent)
	{
		// If the widgets parent scene object moved, we need to mark it as dirty
		// as the GUIManager batching relies on object positions, so it needs to be updated.
		const float diffEpsilon = 0.0001f;

		Vector3 position = parent->getWorldPosition();
		Quaternion rotation = parent->getWorldRotation();
		Vector3 scale = parent->getWorldScale();

		if(!mWidgetIsDirty)
		{
			Vector3 posDiff = mPosition - position;
			if(!Math::approxEquals(mPosition, position, diffEpsilon))
			{
				mWidgetIsDirty = true;
			}
			else
			{
				Quaternion rotDiff = mRotation - rotation;
				if(!Math::approxEquals(mRotation, rotation, diffEpsilon))
				{
					mWidgetIsDirty = true;
				}
				else
				{
					Vector3 scaleDiff = mScale - scale;
					if(Math::approxEquals(mScale, scale))
					{
						mWidgetIsDirty = true;
					}
				}
			}
		}

		mPosition = position;
		mRotation = rotation;
		mScale = scale;
		mTransform = parent->getWorldTfrm();
	}

	void GUIWidget::_updateLayout()
	{
		bs_frame_mark();

		// Determine dirty contents and layouts
		FrameStack<GUIElementBase*> todo;
		todo.push(mPanel);

		while (!todo.empty())
		{
			GUIElementBase* currentElem = todo.top();
			todo.pop();

			if (currentElem->_isDirty())
			{
				GUIElementBase* updateParent = currentElem->_getUpdateParent();
				assert(updateParent != nullptr || currentElem == mPanel);

				if (updateParent != nullptr)
					_updateLayout(updateParent);
				else // Must be root panel
					_updateLayout(mPanel);
			}
			else
			{
				UINT32 numChildren = currentElem->_getNumChildren();
				for (UINT32 i = 0; i < numChildren; i++)
					todo.push(currentElem->_getChild(i));
			}
		}

		bs_frame_clear();
	}

	void GUIWidget::_updateLayout(GUIElementBase* elem)
	{
		GUIElementBase* parent = elem->_getParent();
		bool isPanelOptimized = parent != nullptr && parent->_getType() == GUIElementBase::Type::Panel;

		GUIElementBase* updateParent = nullptr;

		if (isPanelOptimized)
			updateParent = parent;
		else
			updateParent = elem;

		// For GUIPanel we can do a an optimization and update only the element in question instead
		// of all the children
		if (isPanelOptimized)
		{
			GUIPanel* panel = static_cast<GUIPanel*>(updateParent);

			GUIElementBase* dirtyElement = elem;
			dirtyElement->_updateOptimalLayoutSizes();

			LayoutSizeRange elementSizeRange = panel->_getElementSizeRange(dirtyElement);
			Rect2I elementArea = panel->_getElementArea(panel->_getLayoutData().area, dirtyElement, elementSizeRange);

			GUILayoutData childLayoutData = panel->_getLayoutData();
			panel->_updateDepthRange(childLayoutData);
			childLayoutData.area = elementArea;

			panel->_updateChildLayout(dirtyElement, childLayoutData);
		}
		else
		{
			GUILayoutData childLayoutData = updateParent->_getLayoutData();
			updateParent->_updateLayout(childLayoutData);
		}
		
		// Mark dirty contents
		bs_frame_mark();
		{
			FrameStack<GUIElementBase*> todo;
			todo.push(elem);

			while (!todo.empty())
			{
				GUIElementBase* currentElem = todo.top();
				todo.pop();

				if (currentElem->_getType() == GUIElementBase::Type::Element)
					mDirtyContents.insert(static_cast<GUIElement*>(currentElem));

				currentElem->_markAsClean();

				UINT32 numChildren = currentElem->_getNumChildren();
				for (UINT32 i = 0; i < numChildren; i++)
					todo.push(currentElem->_getChild(i));
			}
		}
		bs_frame_clear();
	}

	void GUIWidget::_registerElement(GUIElementBase* elem)
	{
		assert(elem != nullptr && !elem->_isDestroyed());

		if (elem->_getType() == GUIElementBase::Type::Element)
		{
			mElements.push_back(static_cast<GUIElement*>(elem));
			mWidgetIsDirty = true;
		}
	}

	void GUIWidget::_unregisterElement(GUIElementBase* elem)
	{
		assert(elem != nullptr);

		auto iterFind = std::find(begin(mElements), end(mElements), elem);

		if (iterFind != mElements.end())
		{
			mElements.erase(iterFind);
			mWidgetIsDirty = true;
		}

		if (elem->_getType() == GUIElementBase::Type::Element)
			mDirtyContents.erase(static_cast<GUIElement*>(elem));
	}

	void GUIWidget::_markMeshDirty(GUIElementBase* elem)
	{
		mWidgetIsDirty = true;
	}

	void GUIWidget::_markContentDirty(GUIElementBase* elem)
	{
		if (elem->_getType() == GUIElementBase::Type::Element)
			mDirtyContents.insert(static_cast<GUIElement*>(elem));
	}

	void GUIWidget::setSkin(const HGUISkin& skin)
	{
		mSkin = skin;

		for(auto& element : mElements)
			element->_refreshStyle();
	}

	const GUISkin& GUIWidget::getSkin() const
	{
		if(mSkin.isLoaded())
			return *mSkin;
		else
			return *BuiltinResources::instance().getEmptyGUISkin();
	}

	void GUIWidget::setCamera(const CameraPtr& camera)
	{
		if (mCamera == camera)
			return;

		GUIManager::instance().unregisterWidget(this);

		mOwnerTargetResizedConn.disconnect();

		mCamera = camera;

		Viewport* viewport = getTarget();
		if (viewport != nullptr && viewport->getTarget() != nullptr)
			mOwnerTargetResizedConn = viewport->getTarget()->onResized.connect(std::bind(&GUIWidget::ownerTargetResized, this));

		GUIManager::instance().registerWidget(this);

		updateRootPanel();
	}

	void GUIWidget::setIsActive(bool active)
	{
		mIsActive = active;
	}

	bool GUIWidget::isDirty(bool cleanIfDirty)
	{
		if (!mIsActive)
			return false;

		bool dirty = mWidgetIsDirty || mDirtyContents.size() > 0;

		if(cleanIfDirty && dirty)
		{
			mWidgetIsDirty = false;

			for (auto& dirtyElement : mDirtyContents)
				dirtyElement->_updateRenderElements();

			mDirtyContents.clear();
			updateBounds();
		}
		
		return dirty;
	}

	bool GUIWidget::inBounds(const Vector2I& position) const
	{
		Viewport* target = getTarget();
		if (target == nullptr)
			return false;

		// Technically GUI widget bounds can be larger than the viewport, so make sure we clip to viewport first
		if(!target->getArea().contains(position))
			return false;

		Vector3 vecPos((float)position.x, (float)position.y, 0.0f);
		vecPos = mTransform.inverse().multiplyAffine(vecPos);

		Vector2I localPos(Math::roundToInt(vecPos.x), Math::roundToInt(vecPos.y));
		return mBounds.contains(localPos);
	}

	void GUIWidget::updateBounds() const
	{
		if(mElements.size() > 0)
			mBounds = mElements[0]->_getClippedBounds();

		for(auto& elem : mElements)
		{
			Rect2I elemBounds = elem->_getClippedBounds();
			mBounds.encapsulate(elemBounds);
		}
	}

	void GUIWidget::ownerTargetResized()
	{
		updateRootPanel();

		onOwnerTargetResized();
	}

	void GUIWidget::ownerWindowFocusChanged()
	{
		onOwnerWindowFocusChanged();
	}

	void GUIWidget::updateRootPanel()
	{
		Viewport* target = getTarget();
		if (target == nullptr)
			return;

		UINT32 width = target->getWidth();
		UINT32 height = target->getHeight();

		GUILayoutData layoutData;
		layoutData.area.width = width;
		layoutData.area.height = height;
		layoutData.clipRect = Rect2I(0, 0, width, height);
		layoutData.setWidgetDepth(mDepth);

		mPanel->setWidth(width);
		mPanel->setHeight(height);

		mPanel->_setLayoutData(layoutData);
		mPanel->_markLayoutAsDirty();
	}
}