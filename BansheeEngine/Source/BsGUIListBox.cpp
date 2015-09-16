#include "BsGUIListBox.h"
#include "BsImageSprite.h"
#include "BsCGUIWidget.h"
#include "BsGUISkin.h"
#include "BsSpriteTexture.h"
#include "BsTextSprite.h"
#include "BsGUIDimensions.h"
#include "BsGUIMouseEvent.h"
#include "BsGUIManager.h"
#include "BsGUIHelper.h"
#include "BsGUIDropDownBoxManager.h"
#include "BsTexture.h"

namespace BansheeEngine
{
	const String& GUIListBox::getGUITypeName()
	{
		static String name = "ListBox";
		return name;
	}

	GUIListBox::GUIListBox(const String& styleName, const Vector<HString>& elements, const GUIDimensions& dimensions)
		:GUIButtonBase(styleName, GUIContent(HString(L"")), dimensions), mElements(elements), mSelectedIdx(0), mIsListBoxOpen(false)
	{
		if(elements.size() > 0)
			setContent(GUIContent(mElements[mSelectedIdx]));
	}

	GUIListBox::~GUIListBox()
	{
		closeListBox();
	}

	GUIListBox* GUIListBox::create(const Vector<HString>& elements, const String& styleName)
	{
		return new (bs_alloc<GUIListBox>()) GUIListBox(getStyleName<GUIListBox>(styleName), elements, GUIDimensions::create());
	}

	GUIListBox* GUIListBox::create(const Vector<HString>& elements, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIListBox>()) GUIListBox(getStyleName<GUIListBox>(styleName), elements, GUIDimensions::create(options));
	}

	void GUIListBox::setElements(const Vector<HString>& elements)
	{
		bool wasOpen = mIsListBoxOpen;

		if(mIsListBoxOpen)
			closeListBox();

		mElements = elements;
		mSelectedIdx = 0;

		if(elements.size() > 0)
			setContent(GUIContent(mElements[mSelectedIdx]));

		if(wasOpen)
			openListBox();
	}

	void GUIListBox::selectElement(UINT32 idx)
	{
		elementSelected(idx);
	}

	bool GUIListBox::_mouseEvent(const GUIMouseEvent& ev)
	{
		bool processed = GUIButtonBase::_mouseEvent(ev);

		if(ev.getType() == GUIMouseEventType::MouseDown)
		{
			if(!mIsListBoxOpen)
				openListBox();
			else
				closeListBox();

			processed = true;
		}

		return processed;
	}

	void GUIListBox::elementSelected(UINT32 idx)
	{
		if (idx >= (UINT32)mElements.size())
			return;

		if(!onSelectionChanged.empty())
			onSelectionChanged(idx);

		mSelectedIdx = idx;
		setContent(GUIContent(mElements[idx]));

		closeListBox();
	}

	void GUIListBox::openListBox()
	{
		closeListBox();

		DROP_DOWN_BOX_DESC desc;

		UINT32 i = 0;
		for(auto& elem : mElements)
		{
			WString identifier = toWString(i);
			desc.dropDownData.entries.push_back(GUIDropDownDataEntry::button(identifier, std::bind(&GUIListBox::elementSelected, this, i)));
			desc.dropDownData.localizedNames[identifier] = elem;
			i++;
		}

		CGUIWidget* widget = _getParentWidget();

		desc.camera = widget->getCamera();
		desc.skin = widget->getSkinResource();
		desc.placement = DropDownAreaPlacement::aroundBoundsHorz(_getLayoutData().area);
		
		GameObjectHandle<GUIDropDownMenu> dropDownBox = GUIDropDownBoxManager::instance().openDropDownBox(
			desc, GUIDropDownType::MenuBar, std::bind(&GUIListBox::onListBoxClosed, this));

		_setOn(true);
		mIsListBoxOpen = true;
	}

	void GUIListBox::closeListBox()
	{
		if(mIsListBoxOpen)
		{
			GUIDropDownBoxManager::instance().closeDropDownBox();

			_setOn(false);
			mIsListBoxOpen = false;
		}
	}

	void GUIListBox::onListBoxClosed()
	{
		_setOn(false);
		mIsListBoxOpen = false;
	}
}