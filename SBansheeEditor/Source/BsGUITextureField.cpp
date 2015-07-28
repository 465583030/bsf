#include "BsGUITextureField.h"
#include "BsGUILayoutX.h"
#include "BsGUILayoutY.h"
#include "BsGUILabel.h"
#include "BsGUIDropButton.h"
#include "BsGUIButton.h"
#include "BsGUIPanel.h"
#include "BsResources.h"
#include "BsProjectLibrary.h"
#include "BsBuiltinEditorResources.h"
#include "BsGUIResourceTreeView.h"
#include "BsGUISpace.h"
#include "BsProjectResourceMeta.h"
#include "BsSpriteTexture.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	const UINT32 GUITextureField::DEFAULT_LABEL_WIDTH = 100;

	GUITextureField::GUITextureField(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
		const String& style, const GUIDimensions& dimensions, bool withLabel)
		:GUIElementContainer(dimensions, style), mLabel(nullptr), mClearButton(nullptr), mDropButton(nullptr)
	{
		mLayout = GUILayoutX::create();
		_registerChildElement(mLayout);

		if (withLabel)
		{
			mLabel = GUILabel::create(labelContent, GUIOptions(GUIOption::fixedWidth(labelWidth)), getSubStyleName(BuiltinEditorResources::TextureFieldLabelStyleName));
			mLayout->addElement(mLabel);
		}

		mDropButton = GUIDropButton::create((UINT32)DragAndDropType::Resources, GUIOptions(GUIOption::flexibleWidth()), getSubStyleName(BuiltinEditorResources::TextureFieldDropStyleName));
		mClearButton = GUIButton::create(HString(L""), getSubStyleName(BuiltinEditorResources::TextureFieldClearBtnStyleName));
		mClearButton->onClick.connect(std::bind(&GUITextureField::onClearButtonClicked, this));

		GUIPanel* dropTargetPanel = mLayout->addNewElement<GUIPanel>();
		dropTargetPanel->addElement(mDropButton);

		GUIPanel* closeBtnPanel = mLayout->addNewElement<GUIPanel>();
		GUILayoutY* closeBtnLayoutY = closeBtnPanel->addNewElement<GUILayoutY>();
		closeBtnLayoutY->addNewElement<GUIFixedSpace>(5);
		GUILayoutX* closeBtnLayoutX = closeBtnLayoutY->addNewElement<GUILayoutX>();
		closeBtnLayoutY->addNewElement<GUIFlexibleSpace>();
		closeBtnLayoutX->addNewElement<GUIFlexibleSpace>();
		closeBtnLayoutX->addElement(mClearButton);
		closeBtnLayoutX->addNewElement<GUIFixedSpace>(5);

		mDropButton->onDataDropped.connect(std::bind(&GUITextureField::dataDropped, this, _1));
	}

	GUITextureField::~GUITextureField()
	{

	}

	GUITextureField* GUITextureField::create(const GUIContent& labelContent, UINT32 labelWidth, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), labelContent, labelWidth, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUITextureField* GUITextureField::create(const GUIContent& labelContent, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), labelContent, DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUITextureField* GUITextureField::create(const HString& labelText, UINT32 labelWidth, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), GUIContent(labelText), labelWidth, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUITextureField* GUITextureField::create( const HString& labelText, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), GUIContent(labelText), DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUITextureField* GUITextureField::create(const GUIOptions& options, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), GUIContent(), 0, *curStyle,
			GUIDimensions::create(options), false);
	}

	GUITextureField* GUITextureField::create(const GUIContent& labelContent, UINT32 labelWidth,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), labelContent, labelWidth, *curStyle,
			GUIDimensions::create(), true);
	}

	GUITextureField* GUITextureField::create(const GUIContent& labelContent, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), labelContent, DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(), true);
	}

	GUITextureField* GUITextureField::create(const HString& labelText, UINT32 labelWidth, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), GUIContent(labelText), labelWidth, *curStyle,
			GUIDimensions::create(), true);
	}

	GUITextureField* GUITextureField::create(const HString& labelText, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), GUIContent(labelText), DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(), true);
	}

	GUITextureField* GUITextureField::create(const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), GUIContent(), 0, *curStyle, GUIDimensions::create(), false);
	}

	HTexture GUITextureField::getValue() const
	{
		return static_resource_cast<Texture>(Resources::instance().loadFromUUID(mUUID));
	}

	void GUITextureField::setValue(const HTexture& value)
	{
		if (value)
			setUUID(value.getUUID());
		else
			setUUID("");
	}

	void GUITextureField::setUUID(const String& uuid)
	{
		mUUID = uuid;

		HTexture texture;

		Path filePath;
		if (Resources::instance().getFilePathFromUUID(mUUID, filePath))
			texture = Resources::instance().load<Texture>(filePath);
		
		if (texture != nullptr)
		{
			HSpriteTexture sprite = SpriteTexture::create(texture);
			mDropButton->setContent(GUIContent(sprite));
			mClearButton->enableRecursively();
		}
		else
		{
			mDropButton->setContent(GUIContent(HString(L"(None)")));
			mClearButton->disableRecursively();
		}

		onValueChanged(mUUID);
	}

	void GUITextureField::setTint(const Color& color)
	{
		if (mLabel != nullptr)
			mLabel->setTint(color);

		mDropButton->setTint(color);
		mClearButton->setTint(color);
	}

	void GUITextureField::_updateLayoutInternal(const GUILayoutData& data)
	{
		mLayout->_setLayoutData(data);
		mLayout->_updateLayoutInternal(data);
	}

	Vector2I GUITextureField::_getOptimalSize() const
	{
		return mLayout->_getOptimalSize();
	}

	void GUITextureField::dataDropped(void* data)
	{
		DraggedResources* draggedResources = reinterpret_cast<DraggedResources*>(data);
		UINT32 numResources = (UINT32)draggedResources->resourcePaths.size();

		if (numResources <= 0)
			return;

		for (UINT32 i = 0; i < numResources; i++)
		{
			Path path = draggedResources->resourcePaths[i];

			String uuid;
			if (!gResources().getUUIDFromFilePath(draggedResources->resourcePaths[i], uuid))
				continue;

			ProjectResourceMetaPtr meta = ProjectLibrary::instance().findResourceMeta(uuid);
			if (meta == nullptr || meta->getTypeId() != TID_Texture)
				continue;

			setUUID(uuid);
			break;
		}
	}

	void GUITextureField::styleUpdated()
	{
		if (mLabel != nullptr)
			mLabel->setStyle(getSubStyleName(BuiltinEditorResources::TextureFieldLabelStyleName));

		mDropButton->setStyle(getSubStyleName(BuiltinEditorResources::TextureFieldDropStyleName));
		mClearButton->setStyle(getSubStyleName(BuiltinEditorResources::TextureFieldClearBtnStyleName));
	}

	void GUITextureField::onClearButtonClicked()
	{
		setValue(HTexture());
	}

	const String& GUITextureField::getGUITypeName()
	{
		return BuiltinEditorResources::TextureFieldStyleName;
	}
}