#include "BsGUIResourceField.h"
#include "BsGUILayoutX.h"
#include "BsGUILabel.h"
#include "BsGUIDropButton.h"
#include "BsGUIButton.h"
#include "BsBuiltinResources.h"
#include "BsGUIWidget.h"
#include "BsGUIMouseEvent.h"
#include "BsGUIResourceTreeView.h"
#include "BsGUIWidget.h"
#include "BsGameObjectManager.h"
#include "BsScriptAssemblyManager.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsResources.h"
#include "BsProjectLibrary.h"
#include "BsProjectResourceMeta.h"
#include "BsManagedResourceMetaData.h"
#include "BsBuiltinEditorResources.h"
#include "BsScriptTexture2D.h"
#include "BsScriptTexture3D.h"
#include "BsScriptTextureCube.h"
#include "BsScriptSpriteTexture.h"
#include "BsScriptMaterial.h"
#include "BsScriptMesh.h"
#include "BsScriptFont.h"
#include "BsScriptShader.h"
#include "BsScriptPlainText.h"
#include "BsScriptScriptCode.h"
#include "BsScriptStringTable.h"
#include "BsScriptPrefab.h"
#include "BsScriptManagedResource.h"
#include "BsSelection.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	const UINT32 GUIResourceField::DEFAULT_LABEL_WIDTH = 100;

	GUIResourceField::GUIResourceField(const PrivatelyConstruct& dummy, const String& typeNamespace, const String& type, const GUIContent& labelContent, UINT32 labelWidth,
		const String& style, const GUIDimensions& dimensions, bool withLabel)
		:GUIElementContainer(dimensions, style), mLabel(nullptr), mClearButton(nullptr), mDropButton(nullptr), mType(type), mNamespace(typeNamespace)
	{
		mLayout = GUILayoutX::create();
		_registerChildElement(mLayout);

		if (withLabel)
		{
			mLabel = GUILabel::create(labelContent, GUIOptions(GUIOption::fixedWidth(labelWidth)), getSubStyleName(BuiltinEditorResources::ObjectFieldLabelStyleName));
			mLayout->addElement(mLabel);
		}

		mDropButton = GUIDropButton::create((UINT32)DragAndDropType::Resources, GUIOptions(GUIOption::flexibleWidth()), getSubStyleName(BuiltinEditorResources::ObjectFieldDropBtnStyleName));
		mClearButton = GUIButton::create(HString(L""), getSubStyleName(BuiltinEditorResources::ObjectFieldClearBtnStyleName));
		mClearButton->onClick.connect(std::bind(&GUIResourceField::onClearButtonClicked, this));

		mLayout->addElement(mDropButton);
		mLayout->addElement(mClearButton);

		mDropButton->onDataDropped.connect(std::bind(&GUIResourceField::dataDropped, this, _1));
		mDropButton->onClick.connect(std::bind(&GUIResourceField::onDropButtonClicked, this));
	}

	GUIResourceField::~GUIResourceField()
	{

	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const GUIContent& labelContent, UINT32 labelWidth, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, labelContent, labelWidth, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const GUIContent& labelContent, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, labelContent, DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const HString& labelText, UINT32 labelWidth, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, GUIContent(labelText), labelWidth, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const HString& labelText, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, GUIContent(labelText), DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const GUIOptions& options, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, GUIContent(), 0, *curStyle,
			GUIDimensions::create(options), false);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const GUIContent& labelContent, UINT32 labelWidth,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, labelContent, labelWidth, *curStyle,
			GUIDimensions::create(), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const GUIContent& labelContent,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, labelContent, DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const HString& labelText, UINT32 labelWidth,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, GUIContent(labelText), labelWidth, *curStyle,
			GUIDimensions::create(), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const HString& labelText,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, GUIContent(labelText), DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, GUIContent(), 0, *curStyle,
			GUIDimensions::create(), false);
	}

	HResource GUIResourceField::getValue() const
	{
		return Resources::instance().loadFromUUID(mUUID);
	}

	void GUIResourceField::setValue(const HResource& value)
	{
		if (value)
			setUUID(value.getUUID());
		else
			setUUID("");
	}

	void GUIResourceField::setUUID(const String& uuid)
	{ 
		mUUID = uuid;

		Path filePath;
		if (Resources::instance().getFilePathFromUUID(mUUID, filePath))
		{
			WString title = filePath.getWFilename(false) + L" (" + toWString(mType) + L")";
			mDropButton->setContent(GUIContent(HString(title)));
		}
		else
			mDropButton->setContent(GUIContent(HString(L"None (" + toWString(mType) + L")")));

		onValueChanged(mUUID);
	}

	void GUIResourceField::setTint(const Color& color)
	{
		if (mLabel != nullptr)
			mLabel->setTint(color);

		mDropButton->setTint(color);
		mClearButton->setTint(color);
	}

	void GUIResourceField::_updateLayoutInternal(const GUILayoutData& data)
	{
		mLayout->_setLayoutData(data);
		mLayout->_updateLayoutInternal(data);
	}

	Vector2I GUIResourceField::_getOptimalSize() const
	{
		return mLayout->_getOptimalSize();
	}

	void GUIResourceField::onDropButtonClicked()
	{
		if (mUUID == "")
			return;

		Path resPath = ProjectLibrary::instance().uuidToPath(mUUID);
		Selection::instance().ping(resPath);
	}

	void GUIResourceField::dataDropped(void* data)
	{
		DraggedResources* draggedResources = reinterpret_cast<DraggedResources*>(data);
		UINT32 numResources = (UINT32)draggedResources->resourcePaths.size();

		if (numResources <= 0)
			return;

		MonoClass* acceptedClass = MonoManager::instance().findClass(mNamespace, mType);

		for (UINT32 i = 0; i < numResources; i++)
		{
			Path path = draggedResources->resourcePaths[i];

			ProjectLibrary::LibraryEntry* libEntry = ProjectLibrary::instance().findEntry(path);
			if (libEntry == nullptr || libEntry->type == ProjectLibrary::LibraryEntryType::Directory)
				continue;

			ProjectLibrary::ResourceEntry* resEntry = static_cast<ProjectLibrary::ResourceEntry*>(libEntry);

			ProjectResourceMetaPtr meta = resEntry->meta;
			UINT32 typeId = meta->getTypeID();
			String uuid = meta->getUUID();

			bool found = false;
			switch (typeId)
			{
			case TID_Texture:
			{
				// TODO - Need to distinguish from 2D/3D/Cube
				if (ScriptTexture2D::getMetaData()->scriptClass->isSubClassOf(acceptedClass) ||
					ScriptTexture3D::getMetaData()->scriptClass->isSubClassOf(acceptedClass) ||
					ScriptTextureCube::getMetaData()->scriptClass->isSubClassOf(acceptedClass))
				{
					setUUID(uuid);
					found = true;
				}
			}
				break;
			case TID_SpriteTexture:
			{
				if (ScriptSpriteTexture::getMetaData()->scriptClass->isSubClassOf(acceptedClass))
				{
					setUUID(uuid);
					found = true;
				}
			}
				break;
			case TID_Font:
			{
				if (ScriptFont::getMetaData()->scriptClass->isSubClassOf(acceptedClass))
				{
					setUUID(uuid);
					found = true;
				}
			}
				break;
			case TID_PlainText:
			{
				if (ScriptPlainText::getMetaData()->scriptClass->isSubClassOf(acceptedClass))
				{
					setUUID(uuid);
					found = true;
				}
			}
				break;
			case TID_ScriptCode:
			{
				if (ScriptScriptCode::getMetaData()->scriptClass->isSubClassOf(acceptedClass))
				{
					setUUID(uuid);
					found = true;
				}
			}
				break;
			case TID_Shader:
			{
				if (ScriptShader::getMetaData()->scriptClass->isSubClassOf(acceptedClass))
				{
					setUUID(uuid);
					found = true;
				}
			}
				break;
			case TID_Material:
			{
				if (ScriptMaterial::getMetaData()->scriptClass->isSubClassOf(acceptedClass))
				{
					setUUID(uuid);
					found = true;
				}
			}
				break;
			case TID_Mesh:
			{
				if (ScriptMesh::getMetaData()->scriptClass->isSubClassOf(acceptedClass))
				{
					setUUID(uuid);
					found = true;
				}
			}
				break;
			case TID_Prefab:
			{
				if (ScriptPrefab::getMetaData()->scriptClass->isSubClassOf(acceptedClass))
				{
					setUUID(uuid);
					found = true;
				}
			}
			case TID_StringTable:
			{
				if (ScriptStringTable::getMetaData()->scriptClass->isSubClassOf(acceptedClass))
				{
					setUUID(uuid);
					found = true;
				}
			}
				break;
			case TID_ManagedResource:
			{
				ManagedResourceMetaDataPtr managedResMetaData = std::static_pointer_cast<ManagedResourceMetaData>(meta->getResourceMetaData());
				MonoClass* providedClass = MonoManager::instance().findClass(managedResMetaData->typeNamespace, managedResMetaData->typeName);

				if (providedClass->isSubClassOf(acceptedClass))
				{
					setUUID(uuid);
					found = true;
				}
			}
				break;
			default:
				BS_EXCEPT(NotImplementedException, "Unsupported resource type added to resource field.");
			}

			if (found)
				break;
		}
	}

	void GUIResourceField::styleUpdated()
	{
		if (mLabel != nullptr)
			mLabel->setStyle(getSubStyleName(BuiltinEditorResources::ObjectFieldLabelStyleName));

		mDropButton->setStyle(getSubStyleName(BuiltinEditorResources::ObjectFieldDropBtnStyleName));
		mClearButton->setStyle(getSubStyleName(BuiltinEditorResources::ObjectFieldClearBtnStyleName));
	}

	void GUIResourceField::onClearButtonClicked()
	{
		setValue(HResource());
	}

	const String& GUIResourceField::getGUITypeName()
	{
		static String typeName = "GUIResourceField";
		return typeName;
	}
}