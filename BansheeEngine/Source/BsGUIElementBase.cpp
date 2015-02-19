#include "BsGUIElementBase.h"
#include "BsGUILayout.h"
#include "BsGUILayoutX.h"
#include "BsGUILayoutY.h"
#include "BsGUILayoutExplicit.h"
#include "BsGUIElement.h"
#include "BsException.h"
#include "BsGUIWidget.h"
#include "BsGUILayoutUtility.h"

namespace BansheeEngine
{
	GUIElementBase::GUIElementBase()
		:mIsDirty(true), mParentElement(nullptr), mIsDisabled(false), 
		mParentWidget(nullptr), mWidth(0), mHeight(0)
	{

	}

	GUIElementBase::~GUIElementBase()
	{
		Vector<GUIElementBase*> childCopy = mChildren;
		for (auto& child : childCopy)
		{
			if (child->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement* element = static_cast<GUIElement*>(child);
				GUIElement::destroy(element);
			}
			else
			{
				auto iterFind = std::find(mChildren.begin(), mChildren.end(), child);

				if (iterFind != mChildren.end())
					mChildren.erase(iterFind);

				bs_delete(child);
			}
		}
	}

	void GUIElementBase::setOffset(const Vector2I& offset)
	{
		mOffset = offset;
	}

	void GUIElementBase::setWidth(UINT32 width)
	{
		if (mWidth != width)
			markContentAsDirty();

		mWidth = width;
	}

	void GUIElementBase::setHeight(UINT32 height)
	{
		if (mHeight != height)
			markContentAsDirty();

		mHeight = height;
	}

	Rect2I GUIElementBase::getBounds() const
	{
		return GUILayoutUtility::calcBounds(this);
	}

	Rect2I GUIElementBase::getVisibleBounds() const
	{
		return getBounds();
	}

	bool GUIElementBase::_isContentDirty() const
	{
		if((mIsDirty & 0x01) != 0)
			return true;

		for(auto& child : mChildren)
		{
			if(child->_isContentDirty())
				return true;
		}

		return false;
	}

	bool GUIElementBase::_isMeshDirty() const
	{
		return (mIsDirty & 0x02) != 0;
	}

	void GUIElementBase::markContentAsDirty() 
	{ 
		if(_isDisabled())
			return;

		mIsDirty |= 0x01; 
	}

	void GUIElementBase::markMeshAsDirty()
	{
		if(_isDisabled())
			return;

		mIsDirty |= 0x02;
	}

	void GUIElementBase::enableRecursively()
	{
		if (mParentElement != nullptr && mParentElement->mIsDisabled)
			return; // Cannot enable if parent is disabled

		// Make sure to mark everything as dirty, as we didn't track any dirty flags while the element was disabled
		mIsDisabled = false;
		markContentAsDirty();

		for(auto& elem : mChildren)
		{
			elem->enableRecursively();
		}
	}

	void GUIElementBase::disableRecursively()
	{
		markMeshAsDirty(); // Just need to hide the mesh
		mIsDisabled = true;

		for(auto& elem : mChildren)
		{
			elem->disableRecursively();
		}
	}

	void GUIElementBase::_updateLayout(INT32 x, INT32 y, UINT32 width, UINT32 height, Rect2I clipRect, UINT8 widgetDepth, UINT16 areaDepth)
	{
		_updateOptimalLayoutSizes(); // We calculate optimal sizes of all layouts as a pre-processing step, as they are requested often during update
		_updateLayoutInternal(x, y, width, height, clipRect, widgetDepth, areaDepth);
	}

	void GUIElementBase::_updateOptimalLayoutSizes()
	{
		for(auto& child : mChildren)
		{
			child->_updateOptimalLayoutSizes();
		}
	}

	void GUIElementBase::_updateLayoutInternal(INT32 x, INT32 y, UINT32 width, UINT32 height, Rect2I clipRect, UINT8 widgetDepth, UINT16 areaDepth)
	{
		for(auto& child : mChildren)
		{
			child->_updateLayoutInternal(x, y, width, height, clipRect, widgetDepth, areaDepth);
		}
	}

	void GUIElementBase::_getElementAreas(INT32 x, INT32 y, UINT32 width, UINT32 height, Rect2I* elementAreas, UINT32 numElements, const Vector<Vector2I>& optimalSizes) const
	{
		assert(mChildren.size() == 0);
	}

	void GUIElementBase::_setParent(GUIElementBase* parent) 
	{ 
		if(mParentElement != parent)
		{
			mParentElement = parent; 

			if (parent != nullptr)
			{
				if (_getParentWidget() != parent->_getParentWidget())
					_changeParentWidget(parent->_getParentWidget());
			}
			else
				_changeParentWidget(nullptr);
		}
	}

	GUILayout& GUIElementBase::addLayoutXInternal(GUIElementBase* parent)
	{
		GUILayoutX* entry = bs_new<GUILayoutX, PoolAlloc>();
		entry->_setParent(parent);

		mChildren.push_back(entry);
		
		if (mIsDisabled)
			entry->disableRecursively();

		markContentAsDirty();

		return *entry;
	}

	GUILayout& GUIElementBase::addLayoutYInternal(GUIElementBase* parent)
	{
		GUILayoutY* entry = bs_new<GUILayoutY, PoolAlloc>();
		entry->_setParent(parent);

		mChildren.push_back(entry);

		if (mIsDisabled)
			entry->disableRecursively();

		markContentAsDirty();

		return *entry;
	}

	GUILayout& GUIElementBase::addLayoutExplicitInternal(GUIElementBase* parent)
	{
		GUILayoutExplicit* entry = bs_new<GUILayoutExplicit, PoolAlloc>();
		entry->_setParent(parent);

		mChildren.push_back(entry);

		if (mIsDisabled)
			entry->disableRecursively();

		markContentAsDirty();

		return *entry;
	}

	void GUIElementBase::removeLayoutInternal(GUILayout& layout)
	{
		bool foundElem = false;
		for(auto iter = mChildren.begin(); iter != mChildren.end(); ++iter)
		{
			GUIElementBase* child = *iter;

			if(child->_getType() == GUIElementBase::Type::Layout && child == &layout)
			{
				bs_delete<PoolAlloc>(child);

				mChildren.erase(iter);
				foundElem = true;
				markContentAsDirty();
				break;
			}
		}

		if(!foundElem)
			BS_EXCEPT(InvalidParametersException, "Provided element is not a part of this layout.");
	}

	GUILayout& GUIElementBase::insertLayoutXInternal(GUIElementBase* parent, UINT32 idx)
	{
		if(idx < 0 || idx > (UINT32)mChildren.size())
			BS_EXCEPT(InvalidParametersException, "Index out of range: " + toString(idx) + ". Valid range: 0 .. " + toString((UINT32)mChildren.size()));

		GUILayoutX* entry = bs_new<GUILayoutX, PoolAlloc>();
		entry->_setParent(parent);

		mChildren.insert(mChildren.begin() + idx, entry);

		if (mIsDisabled)
			entry->disableRecursively();

		markContentAsDirty();

		return *entry;
	}

	GUILayout& GUIElementBase::insertLayoutYInternal(GUIElementBase* parent, UINT32 idx)
	{
		if(idx < 0 || idx > (UINT32)mChildren.size())
			BS_EXCEPT(InvalidParametersException, "Index out of range: " + toString(idx) + ". Valid range: 0 .. " + toString((UINT32)mChildren.size()));

		GUILayoutY* entry = bs_new<GUILayoutY, PoolAlloc>();
		entry->_setParent(parent);

		mChildren.insert(mChildren.begin() + idx, entry);

		if (mIsDisabled)
			entry->disableRecursively();

		markContentAsDirty();

		return *entry;
	}

	GUILayout& GUIElementBase::insertLayoutExplicitInternal(GUIElementBase* parent, UINT32 idx)
	{
		if (idx < 0 || idx >(UINT32)mChildren.size())
			BS_EXCEPT(InvalidParametersException, "Index out of range: " + toString(idx) + ". Valid range: 0 .. " + toString((UINT32)mChildren.size()));

		GUILayoutExplicit* entry = bs_new<GUILayoutExplicit, PoolAlloc>();
		entry->_setParent(parent);

		mChildren.insert(mChildren.begin() + idx, entry);

		if (mIsDisabled)
			entry->disableRecursively();

		markContentAsDirty();

		return *entry;
	}

	void GUIElementBase::_registerChildElement(GUIElement* element)
	{
		GUIElementBase* parentElement = element->_getParent();
		if(parentElement != nullptr)
		{
			parentElement->_unregisterChildElement(element);
		}

		element->_setParent(this);
		mChildren.push_back(element);

		if (mIsDisabled)
			element->disableRecursively();

		markContentAsDirty();
	}

	void GUIElementBase::_unregisterChildElement(GUIElement* element)
	{
		bool foundElem = false;
		for(auto iter = mChildren.begin(); iter != mChildren.end(); ++iter)
		{
			GUIElementBase* child = *iter;

			if(child->_getType() == GUIElementBase::Type::Element && child == element)
			{
				mChildren.erase(iter);
				element->_setParent(nullptr);
				foundElem = true;

				markContentAsDirty();
				break;
			}
		}

		if(!foundElem)
			BS_EXCEPT(InvalidParametersException, "Provided element is not a part of this element.");
	}

	void GUIElementBase::_changeParentWidget(GUIWidget* widget)
	{
		mParentWidget = widget;

		for(auto& child : mChildren)
		{
			child->_changeParentWidget(widget);
		}

		markContentAsDirty();
	}
}