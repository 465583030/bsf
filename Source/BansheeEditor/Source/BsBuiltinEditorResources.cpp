//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsBuiltinEditorResources.h"
#include "BsBuiltinResources.h"
#include "BsGUIElementStyle.h"
#include "BsGUILabel.h"
#include "BsGUITexture.h"
#include "BsGUIButton.h"
#include "BsGUIInputBox.h"
#include "BsGUIToggle.h"
#include "BsGUIColor.h"
#include "BsTextSprite.h"
#include "BsSpriteTexture.h"
#include "BsGUITreeViewEditBox.h"
#include "BsGUIIntField.h"
#include "BsGUIFloatField.h"
#include "BsGUIColorField.h"
#include "BsGUITextField.h"
#include "BsGUIToggleField.h"
#include "BsGUIVector2Field.h"
#include "BsGUIVector3Field.h"
#include "BsGUIVector4Field.h"
#include "BsGUIListBoxField.h"
#include "BsGUISliderField.h"
#include "BsGUIProgressBar.h"
#include "BsGUISlider.h"
#include "BsGUIDropDownContent.h"
#include "BsGUIStatusBar.h"
#include "BsGUIMenuBar.h"
#include "BsGUIListBox.h"
#include "BsCoreThread.h"
#include "BsFont.h"
#include "BsTexture.h"
#include "BsShader.h"
#include "BsMaterial.h"
#include "BsResources.h"
#include "BsFileSystem.h"
#include "BsResourceManifest.h"
#include "BsDataStream.h"
#include "BsGUITooltip.h"

namespace BansheeEngine
{
	static const Path EDITOR_DATA_FOLDER = "Editor\\";

	const String BuiltinEditorResources::ObjectFieldStyleName = "GUIObjectField";
	const String BuiltinEditorResources::ObjectFieldLabelStyleName = "EditorFieldLabel";
	const String BuiltinEditorResources::ObjectFieldDropBtnStyleName = "DropButton";
	const String BuiltinEditorResources::ObjectFieldClearBtnStyleName = "ObjectClearButton";

	const String BuiltinEditorResources::TextureFieldStyleName = "GUITextureField";
	const String BuiltinEditorResources::TextureFieldLabelStyleName = "TextureFieldLabel";
	const String BuiltinEditorResources::TextureFieldDropStyleName = "TextureDrop";
	const String BuiltinEditorResources::TextureFieldClearBtnStyleName = "TextureClearButton";

	const WString BuiltinEditorResources::DefaultFontFilename = L"arial.ttf";
	const WString BuiltinEditorResources::DefaultAAFontFilename = L"arialAA.ttf";
	const UINT32 BuiltinEditorResources::DefaultFontSize = 8;
	const UINT32 BuiltinEditorResources::TitleFontSize = 16;

	const Color BuiltinEditorResources::TextNormalColor = Color(0.7f, 0.7f, 0.7f);
	const Color BuiltinEditorResources::TextActiveColor = Color(0.0f, 0.0f, 0.0f);

	const WString BuiltinEditorResources::GUISkinFile = L"GUISkin";

	const char* BuiltinEditorResources::ShaderFolder = "Shaders\\";
	const char* BuiltinEditorResources::SkinFolder = "Skin\\";
	const char* BuiltinEditorResources::IconFolder = "Skin\\Icons";
	const char* BuiltinEditorResources::ShaderIncludeFolder = "Includes\\";

	const WString BuiltinEditorResources::FolderIconTex = L"FolderIcon.psd";
	const WString BuiltinEditorResources::MeshIconTex = L"MeshIcon.psd";
	const WString BuiltinEditorResources::TextureIconTex = L"TextureIcon.psd";
	const WString BuiltinEditorResources::FontIconTex = L"FontIcon.psd";
	const WString BuiltinEditorResources::PlainTextIconTex = L"TextIcon.psd";
	const WString BuiltinEditorResources::ScriptCodeIconTex = L"CSharpIcon.psd";
	const WString BuiltinEditorResources::ShaderIconTex = L"ShaderIcon.psd";
	const WString BuiltinEditorResources::ShaderIncludeIconTex = L"ShaderIncludeIcon.psd";
	const WString BuiltinEditorResources::MaterialIconTex = L"MaterialIcon.psd";
	const WString BuiltinEditorResources::SpriteTextureIconTex = L"SpriteIcon.psd";
	const WString BuiltinEditorResources::PrefabIconTex = L"PrefabIcon.psd";
	const WString BuiltinEditorResources::GUISkinIconTex = L"GUISkinIcon.psd";
	const WString BuiltinEditorResources::PhysicsMaterialIconTex = L"PhysicsMaterialIcon.psd";
	const WString BuiltinEditorResources::PhysicsMeshIconTex = L"PhysicsMeshIcon.psd";

	const WString BuiltinEditorResources::ButtonNormalTex = L"ButtonNormal.png";
	const WString BuiltinEditorResources::ButtonHoverTex = L"ButtonHover.png";
	const WString BuiltinEditorResources::ButtonActiveTex = L"ButtonActive.png";

	const WString BuiltinEditorResources::ButtonLeftNormalTex = L"ButtonLeftNormal.png";
	const WString BuiltinEditorResources::ButtonLeftHoverTex = L"ButtonLeftHover.png";
	const WString BuiltinEditorResources::ButtonLeftActiveTex = L"ButtonLeftActive.png";

	const WString BuiltinEditorResources::ButtonRightNormalTex = L"ButtonRightNormal.png";
	const WString BuiltinEditorResources::ButtonRightHoverTex = L"ButtonRightHover.png";
	const WString BuiltinEditorResources::ButtonRightActiveTex = L"ButtonRightActive.png";

	const WString BuiltinEditorResources::ToggleNormalTex = L"ToggleNormal.png";
	const WString BuiltinEditorResources::ToggleHoverTex = L"ToggleHover.png";
	const WString BuiltinEditorResources::ToggleNormalOnTex = L"ToggleNormalOn.png";
	const WString BuiltinEditorResources::ToggleHoverOnTex = L"ToggleHoverOn.png";

	const WString BuiltinEditorResources::InputBoxNormalTex = L"InputBoxNormal.png";
	const WString BuiltinEditorResources::InputBoxHoverTex = L"InputBoxHover.png";
	const WString BuiltinEditorResources::InputBoxFocusedTex = L"InputBoxActive.png";

	const WString BuiltinEditorResources::ObjectDropBtnNormalTex = L"ObjectDropNormal.png";
	const WString BuiltinEditorResources::ObjectDropBtnNormalOnTex = L"ObjectDropNormalOn.png";
	const WString BuiltinEditorResources::ObjectClearBtnNormalTex = L"ObjectDropClearNormal.png";
	const WString BuiltinEditorResources::ObjectClearBtnHoverTex = L"ObjectDropClearHover.png";
	const WString BuiltinEditorResources::ObjectClearBtnActiveTex = L"ObjectDropClearActive.png";
	
	const WString BuiltinEditorResources::DropDownBtnNormalTex = L"DropDownButtonNormal.png";
	const WString BuiltinEditorResources::DropDownBtnHoverTex = L"DropDownButtonHover.png";
	const WString BuiltinEditorResources::DropDownBtnActiveTex = L"DropDownButtonActive.png";

	const WString BuiltinEditorResources::SliderHBackgroundTex = L"SliderHBackground.png";
	const WString BuiltinEditorResources::SliderHFillTex = L"SliderHFill.png";
	const WString BuiltinEditorResources::SliderVBackgroundTex = L"SliderVBackground.png";
	const WString BuiltinEditorResources::SliderVFillTex = L"SliderVFill.png";
	const WString BuiltinEditorResources::SliderHandleNormalTex = L"SliderHandleNormal.png";
	const WString BuiltinEditorResources::SliderHandleHoverTex = L"SliderHandleHover.png";
	const WString BuiltinEditorResources::SliderHandleActiveTex = L"SliderHandleActive.png";

	const WString BuiltinEditorResources::FoldoutOpenNormalTex = L"FoldoutNormalOn.png";
	const WString BuiltinEditorResources::FoldoutOpenHoverTex = L"FoldoutHoverOn.png";
	const WString BuiltinEditorResources::FoldoutOpenActiveTex = L"FoldoutHoverOn.png";
	const WString BuiltinEditorResources::FoldoutClosedNormalTex = L"FoldoutNormalOff.png";
	const WString BuiltinEditorResources::FoldoutClosedHoverTex = L"FoldoutHoverOff.png";
	const WString BuiltinEditorResources::FoldoutClosedActiveTex = L"FoldoutHoverOff.png";

	const WString BuiltinEditorResources::WindowBackgroundTex = L"WindowBackground.png";

	const WString BuiltinEditorResources::WindowFrameNormal = L"WindowFrameNormal.png";
	const WString BuiltinEditorResources::WindowFrameFocused = L"WindowFrameFocused.png";

	const WString BuiltinEditorResources::CloseButtonNormalTex = L"CloseButtonNormal.png";
	const WString BuiltinEditorResources::CloseButtonHoverTex = L"CloseButtonHover.png";
	const WString BuiltinEditorResources::CloseButtonActiveTex = L"CloseButtonActive.png";

	const WString BuiltinEditorResources::MinButtonNormalTex = L"MinimizeButtonNormal.png"; 
	const WString BuiltinEditorResources::MinButtonHoverTex = L"MinimizeButtonHover.png"; 
	const WString BuiltinEditorResources::MinButtonActiveTex = L"MinimizeButtonActive.png";

	const WString BuiltinEditorResources::MaxButtonNormalTex = L"MaximizeButtonNormal.png";
	const WString BuiltinEditorResources::MaxButtonHoverTex = L"MaximizeButtonHover.png";
	const WString BuiltinEditorResources::MaxButtonActiveTex = L"MaximizeButtonActive.png";

	const WString BuiltinEditorResources::TabBarBackgroundTex = L"TabBarBackground.png";
	const WString BuiltinEditorResources::TitleBarBackgroundTex = L"TitleBarBackground.png";

	const WString BuiltinEditorResources::TabButtonNormalTex = L"TabButtonNormal.png";
	const WString BuiltinEditorResources::TabButtonFocusedTex = L"TabButtonFocused.png";

	const WString BuiltinEditorResources::ScrollBarUpNormalTex = L"ScrollArrowUpNormal.png";
	const WString BuiltinEditorResources::ScrollBarUpHoverTex = L"ScrollArrowUpHover.png";
	const WString BuiltinEditorResources::ScrollBarUpActiveTex = L"ScrollArrowUpActive.png";

	const WString BuiltinEditorResources::ScrollBarDownNormalTex = L"ScrollArrowDownNormal.png";
	const WString BuiltinEditorResources::ScrollBarDownHoverTex = L"ScrollArrowDownHover.png";
	const WString BuiltinEditorResources::ScrollBarDownActiveTex = L"ScrollArrowDownActive.png";

	const WString BuiltinEditorResources::ScrollBarLeftNormalTex = L"ScrollArrowLeftNormal.png";
	const WString BuiltinEditorResources::ScrollBarLeftHoverTex = L"ScrollArrowLeftHover.png";
	const WString BuiltinEditorResources::ScrollBarLeftActiveTex = L"ScrollArrowLeftActive.png";

	const WString BuiltinEditorResources::ScrollBarRightNormalTex = L"ScrollArrowRightNormal.png";
	const WString BuiltinEditorResources::ScrollBarRightHoverTex = L"ScrollArrowRightHover.png";
	const WString BuiltinEditorResources::ScrollBarRightActiveTex = L"ScrollArrowRightActive.png";

	const WString BuiltinEditorResources::ScrollBarHandleHorzNormalTex = L"ScrollBarHHandleNormal.png";
	const WString BuiltinEditorResources::ScrollBarHandleHorzHoverTex = L"ScrollBarHHandleHover.png";
	const WString BuiltinEditorResources::ScrollBarHandleHorzActiveTex = L"ScrollBarHHandleActive.png";

	const WString BuiltinEditorResources::ScrollBarHandleVertNormalTex = L"ScrollBarVHandleNormal.png";
	const WString BuiltinEditorResources::ScrollBarHandleVertHoverTex = L"ScrollBarVHandleHover.png";
	const WString BuiltinEditorResources::ScrollBarHandleVertActiveTex = L"ScrollBarVHandleActive.png";

	const WString BuiltinEditorResources::ScrollBarHBgTex = L"ScrollBarHBackground.png";
	const WString BuiltinEditorResources::ScrollBarVBgTex = L"ScrollBarVBackground.png";

	const WString BuiltinEditorResources::DropDownBoxBgTex = L"DropDownBoxBg.png";
	const WString BuiltinEditorResources::DropDownBoxSideBgTex = L"DropDownBoxSideBg.png";
	const WString BuiltinEditorResources::DropDownBoxHandleTex = L"DropDownBoxScrollHandle.png";

	const WString BuiltinEditorResources::DropDownBoxEntryNormalTex = L"DropDownBoxEntryNormal.png";
	const WString BuiltinEditorResources::DropDownBoxEntryHoverTex = L"DropDownBoxEntryHover.png";

	const WString BuiltinEditorResources::DropDownBoxEntryToggleNormalTex = L"DropDownBoxEntryToggleNormal.png";
	const WString BuiltinEditorResources::DropDownBoxEntryToggleHoverTex = L"DropDownBoxEntryToggleHover.png";
	const WString BuiltinEditorResources::DropDownBoxEntryToggleNormalOnTex = L"DropDownBoxEntryToggleNormalOn.png";
	const WString BuiltinEditorResources::DropDownBoxEntryToggleHoverOnTex = L"DropDownBoxEntryToggleHoverOn.png";

	const WString BuiltinEditorResources::DropDownBoxBtnUpNormalTex = L"DropDownBoxArrowUpNormal.png";
	const WString BuiltinEditorResources::DropDownBoxBtnUpHoverTex = L"DropDownBoxArrowUpHover.png";

	const WString BuiltinEditorResources::DropDownBoxBtnDownNormalTex = L"DropDownBoxArrowDownNormal.png";
	const WString BuiltinEditorResources::DropDownBoxBtnDownHoverTex = L"DropDownBoxArrowDownHover.png";

	const WString BuiltinEditorResources::DropDownBoxEntryExpNormalTex = L"DropDownBoxExpandBtnNormal.png";
	const WString BuiltinEditorResources::DropDownBoxEntryExpHoverTex = L"DropDownBoxExpandBtnHover.png";

	const WString BuiltinEditorResources::DropDownSeparatorTex = L"DropDownBoxSeparator.png";

	const WString BuiltinEditorResources::MenuBarBgTex = L"MenuBarBackground.png";
	const WString BuiltinEditorResources::MenuBarBansheeLogoTex = L"MenuBarLog.png";

	const WString BuiltinEditorResources::MenuBarBtnNormalTex = L"MenuBarButtonNormal.png";
	const WString BuiltinEditorResources::MenuBarBtnActiveTex = L"MenuBarButtonActive.png";
	const WString BuiltinEditorResources::MenuBarBtnHoverTex = L"MenuBarButtonHover.png";

	const WString BuiltinEditorResources::MenuBarLineNormalTex = L"MenuBarLineNormal.png";
	const WString BuiltinEditorResources::MenuBarLineActiveTex = L"MenuBarLineActive.png";

	const WString BuiltinEditorResources::ToolBarBtnNormalTex = L"ToolBarButtonNormal.png";
	const WString BuiltinEditorResources::ToolBarBtnHoverTex = L"ToolBarButtonHover.png";
	const WString BuiltinEditorResources::ToolBarBtnActiveTex = L"ToolBarButtonActive.png";

	const WString BuiltinEditorResources::ToolBarSeparatorTex = L"ToolBarSeparator.png";

	const WString BuiltinEditorResources::DockSliderNormalTex = L"DockSliderNormal.png";

	const WString BuiltinEditorResources::TreeViewExpandButtonOffNormal = L"ExpandArrowNormalOff.png";
	const WString BuiltinEditorResources::TreeViewExpandButtonOffHover = L"ExpandArrowHoverOff.png";
	const WString BuiltinEditorResources::TreeViewExpandButtonOnNormal = L"ExpandArrowNormalOn.png";
	const WString BuiltinEditorResources::TreeViewExpandButtonOnHover = L"ExpandArrowHoverOn.png";

	const WString BuiltinEditorResources::TreeViewHighlightBackground = L"TreeViewHighlightBackground.psd";
	const WString BuiltinEditorResources::TreeViewEditBox = L"TreeViewEditBox.psd";
	const WString BuiltinEditorResources::TreeViewElementHighlight = L"TreeViewElementHighlight.psd";
	const WString BuiltinEditorResources::TreeViewElementSepHighlight = L"TreeViewElementSepHighlight.psd";

	const WString BuiltinEditorResources::ColorPickerSliderHorzHandleTex = L"ColorPickerSliderHorzHandle.psd";
	const WString BuiltinEditorResources::ColorPickerSliderVertHandleTex = L"ColorPickerSliderVertHandle.psd";
	const WString BuiltinEditorResources::ColorPickerSlider2DHandleTex = L"ColorPicker2DHandle.psd";

	const WString BuiltinEditorResources::ProgressBarFillTex = L"ProgressBarFill.png";
	const WString BuiltinEditorResources::ProgressBarBgTex = L"ProgressBarBg.png";

	const WString BuiltinEditorResources::SelectionAreaTex = L"SelectionHighlight.png";
	const WString BuiltinEditorResources::SelectionBgTex = L"SelectionBg.psd";

	const WString BuiltinEditorResources::TextureDropTex = L"TextureDrop.png";
	const WString BuiltinEditorResources::TextureDropOnTex = L"TextureDropHover.png";

	const WString BuiltinEditorResources::XButtonNormalTex = L"XBtnNormal.png";
	const WString BuiltinEditorResources::XButtonHoverTex = L"XBtnHover.png";
	const WString BuiltinEditorResources::XButtonActiveTex = L"XBtnActive.png";

	const WString BuiltinEditorResources::StatusBarBgTex = L"StatusBarBackground.png";
	const WString BuiltinEditorResources::ScrollAreaBgTex = L"ScrollAreaBg.png";

	const WString BuiltinEditorResources::InspectorTitleBgTex = L"InspectorTitleBg.png";
	const WString BuiltinEditorResources::InspectorContentBgTex = L"InspectorContentBg.png";
	const WString BuiltinEditorResources::InspectorContentBgAlternateTex = L"InspectorContentBgAlternate.png";

	const WString BuiltinEditorResources::LibraryEntryFirstBgTex = L"LibraryEntryFirstBg.png";
	const WString BuiltinEditorResources::LibraryEntryBgTex = L"LibraryEntryBg.png";
	const WString BuiltinEditorResources::LibraryEntryLastBgTex = L"LibraryEntryLastBg.png";

	const WString BuiltinEditorResources::LibraryEntryVertFirstBgTex = L"LibraryEntryVertFirstBg.png";
	const WString BuiltinEditorResources::LibraryEntryVertBgTex = L"LibraryEntryVertBg.png";
	const WString BuiltinEditorResources::LibraryEntryVertLastBgTex = L"LibraryEntryVertLastBg.png";

	/************************************************************************/
	/* 									SHADERS                      		*/
	/************************************************************************/

	const WString BuiltinEditorResources::ShaderDockOverlayFile = L"DockDropOverlay.bsl";
	const WString BuiltinEditorResources::ShaderSceneGridFile = L"SceneGrid.bsl";
	const WString BuiltinEditorResources::ShaderPickingCullNoneFile = L"PickingCullNone.bsl";
	const WString BuiltinEditorResources::ShaderPickingCullCWFile = L"PickingCullCW.bsl";
	const WString BuiltinEditorResources::ShaderPickingCullCCWFile = L"PickingCullCCW.bsl";
	const WString BuiltinEditorResources::ShaderPickingAlphaCullNoneFile = L"PickingAlphaCullNone.bsl";
	const WString BuiltinEditorResources::ShaderPickingAlphaCullCWFile = L"PickingAlphaCullCW.bsl";
	const WString BuiltinEditorResources::ShaderPickingAlphaCullCCWFile = L"PickingAlphaCullCCW.bsl";
	const WString BuiltinEditorResources::ShaderLineGizmoFile = L"LineGizmo.bsl";
	const WString BuiltinEditorResources::ShaderWireGizmoFile = L"WireGizmo.bsl";
	const WString BuiltinEditorResources::ShaderSolidGizmoFile = L"SolidGizmo.bsl";
	const WString BuiltinEditorResources::ShaderLineHandleFile = L"LineHandle.bsl";
	const WString BuiltinEditorResources::ShaderSolidHandleFile = L"SolidHandle.bsl";
	const WString BuiltinEditorResources::ShaderHandleClearAlphaFile = L"ClearHandleAlpha.bsl";
	const WString BuiltinEditorResources::ShaderIconGizmoFile = L"IconGizmo.bsl";
	const WString BuiltinEditorResources::ShaderGizmoPickingFile = L"GizmoPicking.bsl";
	const WString BuiltinEditorResources::ShaderGizmoPickingAlphaFile = L"GizmoPickingAlpha.bsl";
	const WString BuiltinEditorResources::ShaderTextGizmoFile = L"TextGizmo.bsl";
	const WString BuiltinEditorResources::ShaderSelectionFile = L"Selection.bsl";

	/************************************************************************/
	/* 								OTHER							  		*/
	/************************************************************************/
	const WString BuiltinEditorResources::EmptyShaderCodeFile = L"EmptyShaderText.txt";
	const WString BuiltinEditorResources::EmptyCSScriptCodeFile = L"EmptyCSScriptText.txt";

	BuiltinEditorResources::BuiltinEditorResources()
	{
		// Set up paths
		BuiltinRawDataFolder = Paths::getRuntimeDataPath() + L"Raw\\Editor\\";
		EditorRawSkinFolder = BuiltinRawDataFolder + SkinFolder;
		EditorRawShaderFolder = BuiltinRawDataFolder + ShaderFolder;
		EditorRawShaderIncludeFolder = BuiltinRawDataFolder + ShaderIncludeFolder;

		BuiltinDataFolder = Paths::getRuntimeDataPath() + EDITOR_DATA_FOLDER;
		EditorSkinFolder = BuiltinDataFolder + SkinFolder;
		EditorIconFolder = BuiltinDataFolder + IconFolder;
		EditorShaderFolder = BuiltinDataFolder + ShaderFolder;
		EditorShaderIncludeFolder = BuiltinDataFolder + ShaderIncludeFolder;

		ResourceManifestPath = BuiltinDataFolder + "ResourceManifest.asset";

		Path absoluteDataPath = FileSystem::getWorkingDirectoryPath();
		absoluteDataPath.append(BuiltinDataFolder);

		// Update from raw assets if needed
#if BS_DEBUG_MODE
		if (BuiltinResourcesHelper::checkForModifications(BuiltinRawDataFolder, BuiltinDataFolder + L"Timestamp.asset"))
		{
			mResourceManifest = ResourceManifest::create("BuiltinResources");
			gResources().registerResourceManifest(mResourceManifest);

			preprocess();
			BuiltinResourcesHelper::writeTimestamp(BuiltinDataFolder + L"Timestamp.asset");

			Path absoluteDataPath = FileSystem::getWorkingDirectoryPath();
			absoluteDataPath.append(BuiltinDataFolder);

			ResourceManifest::save(mResourceManifest, ResourceManifestPath, absoluteDataPath);
		}
#endif

		// Load manifest
		if (mResourceManifest == nullptr)
		{
			if (FileSystem::exists(ResourceManifestPath))
				mResourceManifest = ResourceManifest::load(ResourceManifestPath, absoluteDataPath);

			if (mResourceManifest == nullptr)
				mResourceManifest = ResourceManifest::create("BuiltinResources");

			gResources().registerResourceManifest(mResourceManifest);
		}

		// Load basic resources
		mShaderDockOverlay = getShader(ShaderDockOverlayFile);
		mShaderSceneGrid = getShader(ShaderSceneGridFile);
		mShaderPicking[(int)CULL_NONE] = getShader(ShaderPickingCullNoneFile);
		mShaderPicking[(int)CULL_CLOCKWISE] = getShader(ShaderPickingCullCWFile);
		mShaderPicking[(int)CULL_COUNTERCLOCKWISE] = getShader(ShaderPickingCullCCWFile);
		mShaderPickingAlpha[(int)CULL_NONE] = getShader(ShaderPickingAlphaCullNoneFile);
		mShaderPickingAlpha[(int)CULL_CLOCKWISE] = getShader(ShaderPickingAlphaCullCWFile);
		mShaderPickingAlpha[(int)CULL_COUNTERCLOCKWISE] = getShader(ShaderPickingAlphaCullCCWFile);
		mShaderGizmoSolid = getShader(ShaderSolidGizmoFile);
		mShaderGizmoLine = getShader(ShaderLineGizmoFile);
		mShaderGizmoWire = getShader(ShaderWireGizmoFile);
		mShaderGizmoIcon = getShader(ShaderIconGizmoFile);
		mShaderGizmoPicking = getShader(ShaderGizmoPickingFile);
		mShaderGizmoAlphaPicking = getShader(ShaderGizmoPickingAlphaFile);
		mShaderGizmoText = getShader(ShaderTextGizmoFile);
		mShaderHandleSolid = getShader(ShaderSolidHandleFile);
		mShaderHandleClearAlpha = getShader(ShaderHandleClearAlphaFile);
		mShaderHandleLine = getShader(ShaderLineHandleFile);
		mShaderSelection = getShader(ShaderSelectionFile);

		mDefaultFont = gResources().load<Font>(BuiltinDataFolder + (DefaultAAFontFilename + L".asset"));
		mSkin = gResources().load<GUISkin>(BuiltinDataFolder + (GUISkinFile + L".asset"));
	}

	BuiltinEditorResources::~BuiltinEditorResources()
	{ }

	void BuiltinEditorResources::preprocess()
	{
		BuiltinResourcesHelper::importAssets(EditorRawShaderIncludeFolder, EditorShaderIncludeFolder, mResourceManifest); // Hidden dependency: Includes must be imported before shaders
		BuiltinResourcesHelper::importAssets(EditorRawShaderFolder, EditorShaderFolder, mResourceManifest);
		BuiltinResourcesHelper::importAssets(EditorRawSkinFolder, EditorSkinFolder, mResourceManifest);

		// Generate different sizes of resource icons
		generateResourceIcons(EditorIconFolder, mResourceManifest);

		// Import fonts
		BuiltinResourcesHelper::importFont(BuiltinRawDataFolder + DefaultFontFilename, DefaultFontFilename, 
			BuiltinDataFolder, { DefaultFontSize }, true, mResourceManifest);

		BuiltinResourcesHelper::importFont(BuiltinRawDataFolder + DefaultFontFilename, DefaultAAFontFilename, 
			BuiltinDataFolder, { TitleFontSize }, true, mResourceManifest);

		// Generate & save GUI sprite textures
		BuiltinResourcesHelper::generateSpriteTextures(EditorSkinFolder, mResourceManifest);

		// Generate & save GUI skin
		{
			GUISkinPtr skin = generateGUISkin();
			Path outputPath = FileSystem::getWorkingDirectoryPath() + BuiltinDataFolder + (GUISkinFile + L".asset");

			HResource skinResource;
			if (FileSystem::exists(outputPath))
				skinResource = gResources().load(outputPath);

			if (skinResource.isLoaded())
				gResources().update(skinResource, skin);
			else
				skinResource = gResources()._createResourceHandle(skin);

			gResources().save(skinResource, outputPath, true);
			mResourceManifest->registerResource(skinResource.getUUID(), outputPath);
		}

		Resources::instance().unloadAllUnused();
	}

	void BuiltinEditorResources::generateResourceIcons(const Path& inputFolder, const ResourceManifestPtr& manifest)
	{
		if (!FileSystem::exists(inputFolder))
			return;

		WString iconsToProcess[] = { FolderIconTex, FontIconTex, MeshIconTex, TextureIconTex, PlainTextIconTex, 
			ScriptCodeIconTex, ShaderIconTex, ShaderIncludeIconTex, MaterialIconTex, SpriteTextureIconTex, PrefabIconTex,
			GUISkinIconTex, PhysicsMaterialIconTex, PhysicsMeshIconTex };

		PixelDataPtr srcData[sizeof(iconsToProcess)/sizeof(iconsToProcess[0])];

		UINT32 idx = 0;
		for (auto& iconName : iconsToProcess)
		{
			Path path = inputFolder + (iconName + L".asset");

			HTexture source = gResources().load<Texture>(path);
			if (source != nullptr)
			{
				srcData[idx] = source->getProperties().allocateSubresourceBuffer(0);
				source->readSubresource(gCoreAccessor(), 0, srcData[idx]);
			}

			idx++;
		}

		gCoreAccessor().submitToCoreThread(true);

		idx = 0;
		for (auto& iconName : iconsToProcess)
		{
			PixelDataPtr src = srcData[idx];

			PixelDataPtr scaled48 = PixelData::create(48, 48, 1, src->getFormat());
			PixelUtil::scale(*src, *scaled48);

			PixelDataPtr scaled32 = PixelData::create(32, 32, 1, src->getFormat());
			PixelUtil::scale(*scaled48, *scaled32);

			PixelDataPtr scaled16 = PixelData::create(16, 16, 1, src->getFormat());
			PixelUtil::scale(*scaled32, *scaled16);

			HTexture tex48 = Texture::create(scaled48);
			HTexture tex32 = Texture::create(scaled32);
			HTexture tex16 = Texture::create(scaled16);

			Path outputPath48 = FileSystem::getWorkingDirectoryPath() + inputFolder + (iconName + L"48.asset");
			Resources::instance().save(tex48, outputPath48, true);
			manifest->registerResource(tex48.getUUID(), outputPath48);

			Path outputPath32 = FileSystem::getWorkingDirectoryPath() + inputFolder + (iconName + L"32.asset");
			Resources::instance().save(tex32, outputPath32, true);
			manifest->registerResource(tex32.getUUID(), outputPath32);

			Path outputPath16 = FileSystem::getWorkingDirectoryPath() + inputFolder + (iconName + L"16.asset");
			Resources::instance().save(tex16, outputPath16, true);
			manifest->registerResource(tex16.getUUID(), outputPath16);

			idx++;
		}
	}

	GUISkinPtr BuiltinEditorResources::generateGUISkin()
	{
		GUISkinPtr skin = GUISkin::_createPtr();

		Path defaultFontPath = FileSystem::getWorkingDirectoryPath();
		defaultFontPath.append(BuiltinDataFolder);
		defaultFontPath.append(DefaultFontFilename + L".asset");

		HFont defaultFont = gResources().load<Font>(defaultFontPath);

		Path defaultAAFontPath = FileSystem::getWorkingDirectoryPath();
		defaultAAFontPath.append(BuiltinDataFolder);
		defaultAAFontPath.append(DefaultAAFontFilename + L".asset");

		HFont defaultAAFont = gResources().load<Font>(defaultAAFontPath);

		// Blank entry
		GUIElementStyle blankStyle;

		skin->setStyle("Blank", blankStyle);
		skin->setStyle(GUITexture::getGUITypeName(), blankStyle);

		// Label
		GUIElementStyle labelStyle;
		labelStyle.font = defaultFont;
		labelStyle.fontSize = DefaultFontSize;
		labelStyle.fixedWidth = false;
		labelStyle.fixedHeight = true;
		labelStyle.height = 14;
		labelStyle.minWidth = 10;
		labelStyle.normal.textColor = TextNormalColor;

		skin->setStyle(GUILabel::getGUITypeName(), labelStyle);

		// Window frame
		GUIElementStyle windowFrameStyle;
		windowFrameStyle.normal.texture = getGUITexture(WindowFrameNormal);
		windowFrameStyle.focused.texture = getGUITexture(WindowFrameFocused);
		windowFrameStyle.border.left = 1;
		windowFrameStyle.border.right = 1;
		windowFrameStyle.border.top = 1;
		windowFrameStyle.border.bottom = 1;

		skin->setStyle("WindowFrame", windowFrameStyle);

		// Button
		GUIElementStyle buttonStyle;
		buttonStyle.normal.texture = getGUITexture(ButtonNormalTex);
		buttonStyle.hover.texture = getGUITexture(ButtonHoverTex);
		buttonStyle.active.texture = getGUITexture(ButtonActiveTex);
		buttonStyle.normalOn.texture = getGUITexture(ButtonActiveTex);
		buttonStyle.hoverOn.texture = getGUITexture(ButtonActiveTex);
		buttonStyle.activeOn.texture = getGUITexture(ButtonActiveTex);
		buttonStyle.normal.textColor = TextNormalColor;
		buttonStyle.hover.textColor = TextNormalColor;
		buttonStyle.active.textColor = TextActiveColor;
		buttonStyle.normalOn.textColor = TextActiveColor;
		buttonStyle.hoverOn.textColor = TextActiveColor;
		buttonStyle.activeOn.textColor = TextActiveColor;
		buttonStyle.border.left = 2;
		buttonStyle.border.right = 2;
		buttonStyle.border.top = 2;
		buttonStyle.border.bottom = 4;
		buttonStyle.contentOffset.top = 2;
		buttonStyle.contentOffset.left = 3;
		buttonStyle.contentOffset.right = 3;
		buttonStyle.margins.bottom = 2;
		buttonStyle.fixedHeight = true;
		buttonStyle.height = 21;
		buttonStyle.minWidth = 20;
		buttonStyle.font = defaultFont;
		buttonStyle.fontSize = DefaultFontSize;
		buttonStyle.textHorzAlign = THA_Center;
		buttonStyle.textVertAlign = TVA_Center;

		skin->setStyle(GUIButton::getGUITypeName(), buttonStyle);

		// Left edge button
		GUIElementStyle leftButtonStyle;
		leftButtonStyle.normal.texture = getGUITexture(ButtonLeftNormalTex);
		leftButtonStyle.hover.texture = getGUITexture(ButtonLeftHoverTex);
		leftButtonStyle.active.texture = getGUITexture(ButtonLeftActiveTex);
		leftButtonStyle.normalOn.texture = getGUITexture(ButtonLeftActiveTex);
		leftButtonStyle.hoverOn.texture = getGUITexture(ButtonLeftActiveTex);
		leftButtonStyle.activeOn.texture = getGUITexture(ButtonLeftActiveTex);
		leftButtonStyle.normal.textColor = TextNormalColor;
		leftButtonStyle.hover.textColor = TextNormalColor;
		leftButtonStyle.active.textColor = TextActiveColor;
		leftButtonStyle.normalOn.textColor = TextActiveColor;
		leftButtonStyle.hoverOn.textColor = TextActiveColor;
		leftButtonStyle.activeOn.textColor = TextActiveColor;
		leftButtonStyle.border.left = 7;
		leftButtonStyle.border.right = 2;
		leftButtonStyle.border.top = 2;
		leftButtonStyle.border.bottom = 4;
		leftButtonStyle.contentOffset.top = 2;
		leftButtonStyle.contentOffset.left = 7;
		leftButtonStyle.contentOffset.right = 3;
		leftButtonStyle.margins.bottom = 2;
		leftButtonStyle.fixedHeight = true;
		leftButtonStyle.height = 21;
		leftButtonStyle.minWidth = 20;
		leftButtonStyle.font = defaultFont;
		leftButtonStyle.fontSize = DefaultFontSize;
		leftButtonStyle.textHorzAlign = THA_Center;
		leftButtonStyle.textVertAlign = TVA_Center;

		skin->setStyle("ButtonLeft", leftButtonStyle);

		// Right edge button
		GUIElementStyle rightButtonStyle;
		rightButtonStyle.normal.texture = getGUITexture(ButtonRightNormalTex);
		rightButtonStyle.hover.texture = getGUITexture(ButtonRightHoverTex);
		rightButtonStyle.active.texture = getGUITexture(ButtonRightActiveTex);
		rightButtonStyle.normalOn.texture = getGUITexture(ButtonRightActiveTex);
		rightButtonStyle.hoverOn.texture = getGUITexture(ButtonRightActiveTex);
		rightButtonStyle.activeOn.texture = getGUITexture(ButtonRightActiveTex);
		rightButtonStyle.normal.textColor = TextNormalColor;
		rightButtonStyle.hover.textColor = TextNormalColor;
		rightButtonStyle.active.textColor = TextActiveColor;
		rightButtonStyle.normalOn.textColor = TextActiveColor;
		rightButtonStyle.hoverOn.textColor = TextActiveColor;
		rightButtonStyle.activeOn.textColor = TextActiveColor;
		rightButtonStyle.border.left = 2;
		rightButtonStyle.border.right = 7;
		rightButtonStyle.border.top = 2;
		rightButtonStyle.border.bottom = 4;
		rightButtonStyle.contentOffset.top = 2;
		rightButtonStyle.contentOffset.left = 3;
		rightButtonStyle.contentOffset.right = 7;
		rightButtonStyle.margins.bottom = 2;
		rightButtonStyle.fixedHeight = true;
		rightButtonStyle.height = 21;
		rightButtonStyle.minWidth = 20;
		rightButtonStyle.font = defaultFont;
		rightButtonStyle.fontSize = DefaultFontSize;
		rightButtonStyle.textHorzAlign = THA_Center;
		rightButtonStyle.textVertAlign = TVA_Center;

		skin->setStyle("ButtonRight", rightButtonStyle);

		// Toggle
		GUIElementStyle toggleStyle;
		toggleStyle.normal.texture = getGUITexture(ToggleNormalTex);
		toggleStyle.hover.texture = getGUITexture(ToggleHoverTex);
		toggleStyle.active.texture = toggleStyle.hover.texture;
		toggleStyle.normalOn.texture = getGUITexture(ToggleNormalOnTex);
		toggleStyle.hoverOn.texture = getGUITexture(ToggleHoverOnTex);
		toggleStyle.activeOn.texture = toggleStyle.hoverOn.texture;
		toggleStyle.fixedHeight = true;
		toggleStyle.fixedWidth = true;
		toggleStyle.margins.bottom = 2;
		toggleStyle.height = 15;
		toggleStyle.width = 13;

		skin->setStyle(GUIToggle::getGUITypeName(), toggleStyle);

		// Color
		GUIElementStyle colorStyle;
		colorStyle.margins.left = 2;
		colorStyle.margins.right = 2;
		colorStyle.margins.top = 2;
		colorStyle.margins.bottom = 2;
		colorStyle.fixedHeight = true;
		colorStyle.height = 15;
		colorStyle.minWidth = 10;

		skin->setStyle(GUIColor::getGUITypeName(), colorStyle);

		// Window background texture
		GUIElementStyle windowBgStyle;
		windowBgStyle.normal.texture = getGUITexture(WindowBackgroundTex);

		skin->setStyle("WindowBackground", windowBgStyle);

		// Window tab bar background
		GUIElementStyle tabBarBgStyle;
		tabBarBgStyle.normal.texture = getGUITexture(TabBarBackgroundTex);
		tabBarBgStyle.fixedHeight = true;
		tabBarBgStyle.height = 16;

		skin->setStyle("TabBarBackground", tabBarBgStyle);

		// Tabbed title bar tab button
		GUIElementStyle tabbedBarButton;
		tabbedBarButton.normal.texture = getGUITexture(TabButtonNormalTex);
		tabbedBarButton.hover.texture = tabbedBarButton.normal.texture;
		tabbedBarButton.active.texture = getGUITexture(TabButtonFocusedTex);
		tabbedBarButton.normalOn.texture = tabbedBarButton.active.texture;
		tabbedBarButton.hoverOn.texture = tabbedBarButton.active.texture;
		tabbedBarButton.activeOn.texture = tabbedBarButton.active.texture;
		tabbedBarButton.normal.textColor = TextNormalColor;
		tabbedBarButton.hover.textColor = TextNormalColor;
		tabbedBarButton.active.textColor = TextActiveColor;
		tabbedBarButton.normalOn.textColor = TextActiveColor;
		tabbedBarButton.hoverOn.textColor = TextActiveColor;
		tabbedBarButton.activeOn.textColor = TextActiveColor;
		tabbedBarButton.border.left = 12;
		tabbedBarButton.border.right = 12;
		tabbedBarButton.margins.left = 5;
		tabbedBarButton.margins.right = 5;
		tabbedBarButton.contentOffset.top = 3;
		tabbedBarButton.contentOffset.left = 6;
		tabbedBarButton.contentOffset.right = 6;
		tabbedBarButton.fixedHeight = true;
		tabbedBarButton.height = 15;
		tabbedBarButton.minWidth = 24;
		tabbedBarButton.maxWidth = 110;
		tabbedBarButton.font = defaultFont;
		tabbedBarButton.fontSize = DefaultFontSize;
		tabbedBarButton.textHorzAlign = THA_Center;
		tabbedBarButton.textVertAlign = TVA_Center;

		skin->setStyle("TabbedBarBtn", tabbedBarButton);

		// Tabbed title bar drag/drop button
		GUIElementStyle tabbedBarDropButton;
		tabbedBarDropButton.fixedHeight = true;
		tabbedBarDropButton.fixedWidth = true;
		tabbedBarDropButton.height = 13;
		tabbedBarDropButton.width = 6;

		skin->setStyle("TabbedBarDropArea", tabbedBarDropButton);

		// Window title bar background
		GUIElementStyle titleBarBgStyle;
		titleBarBgStyle.normal.texture = getGUITexture(TitleBarBackgroundTex);
		titleBarBgStyle.fixedHeight = true;
		titleBarBgStyle.height = 17;

		skin->setStyle("TitleBarBackground", titleBarBgStyle);

		// Window minimize button
		GUIElementStyle winMinButtonStyle;
		winMinButtonStyle.normal.texture = getGUITexture(MinButtonNormalTex);
		winMinButtonStyle.hover.texture = getGUITexture(MinButtonHoverTex);
		winMinButtonStyle.active.texture = getGUITexture(MinButtonActiveTex);
		winMinButtonStyle.fixedHeight = true;
		winMinButtonStyle.fixedWidth = true;
		winMinButtonStyle.height = 14;
		winMinButtonStyle.width = 14;

		skin->setStyle("WinMinimizeBtn", winMinButtonStyle);

		// Window maximize button
		GUIElementStyle winMaxButtonStyle;
		winMaxButtonStyle.normal.texture = getGUITexture(MaxButtonNormalTex);
		winMaxButtonStyle.hover.texture = getGUITexture(MaxButtonHoverTex);
		winMaxButtonStyle.active.texture = getGUITexture(MaxButtonActiveTex);
		winMaxButtonStyle.fixedHeight = true;
		winMaxButtonStyle.fixedWidth = true;
		winMaxButtonStyle.height = 14;
		winMaxButtonStyle.width = 14;

		skin->setStyle("WinMaximizeBtn", winMaxButtonStyle);

		// Window close button
		GUIElementStyle winCloseButtonStyle;
		winCloseButtonStyle.normal.texture = getGUITexture(CloseButtonNormalTex);
		winCloseButtonStyle.hover.texture = getGUITexture(CloseButtonHoverTex);
		winCloseButtonStyle.active.texture = getGUITexture(CloseButtonActiveTex);
		winCloseButtonStyle.fixedHeight = true;
		winCloseButtonStyle.fixedWidth = true;
		winCloseButtonStyle.height = 14;
		winCloseButtonStyle.width = 14;

		skin->setStyle("WinCloseBtn", winCloseButtonStyle);

		// Input box
		GUIElementStyle inputBoxStyle;
		inputBoxStyle.normal.texture = getGUITexture(InputBoxNormalTex);
		inputBoxStyle.hover.texture = getGUITexture(InputBoxHoverTex);
		inputBoxStyle.focused.texture = getGUITexture(InputBoxFocusedTex);
		inputBoxStyle.active.texture = inputBoxStyle.normal.texture;
		inputBoxStyle.normal.textColor = TextNormalColor;
		inputBoxStyle.hover.textColor = TextNormalColor;
		inputBoxStyle.focused.textColor = TextNormalColor;
		inputBoxStyle.active.textColor = TextNormalColor;
		inputBoxStyle.border.left = 4;
		inputBoxStyle.border.right = 4;
		inputBoxStyle.border.top = 4;
		inputBoxStyle.border.bottom = 6;
		inputBoxStyle.contentOffset.left = 4;
		inputBoxStyle.contentOffset.right = 4;
		inputBoxStyle.contentOffset.top = 4;
		inputBoxStyle.contentOffset.bottom = 4;
		inputBoxStyle.margins.bottom = 2;
		inputBoxStyle.fixedHeight = true;
		inputBoxStyle.height = 21;
		inputBoxStyle.minWidth = 10;
		inputBoxStyle.font = defaultFont;
		inputBoxStyle.fontSize = DefaultFontSize;
		inputBoxStyle.textHorzAlign = THA_Left;
		inputBoxStyle.textVertAlign = TVA_Top;

		skin->setStyle(GUIInputBox::getGUITypeName(), inputBoxStyle);

		/************************************************************************/
		/* 								SCROLL BAR                      		*/
		/************************************************************************/

		// Up button
		GUIElementStyle scrollUpBtnStyle;
		scrollUpBtnStyle.normal.texture = getGUITexture(ScrollBarUpNormalTex);
		scrollUpBtnStyle.hover.texture = getGUITexture(ScrollBarUpHoverTex);
		scrollUpBtnStyle.active.texture = getGUITexture(ScrollBarUpActiveTex);
		scrollUpBtnStyle.fixedHeight = true;
		scrollUpBtnStyle.fixedWidth = true;
		scrollUpBtnStyle.height = 11;
		scrollUpBtnStyle.width = 13;

		skin->setStyle("ScrollUpBtn", scrollUpBtnStyle);

		// Down button
		GUIElementStyle scrollDownBtnStyle;
		scrollDownBtnStyle.normal.texture = getGUITexture(ScrollBarDownNormalTex);
		scrollDownBtnStyle.hover.texture = getGUITexture(ScrollBarDownHoverTex);
		scrollDownBtnStyle.active.texture = getGUITexture(ScrollBarDownActiveTex);
		scrollDownBtnStyle.fixedHeight = true;
		scrollDownBtnStyle.fixedWidth = true;
		scrollDownBtnStyle.height = 11;
		scrollDownBtnStyle.width = 13;

		skin->setStyle("ScrollDownBtn", scrollDownBtnStyle);

		// Left button
		GUIElementStyle scrollLeftBtnStyle;
		scrollLeftBtnStyle.normal.texture = getGUITexture(ScrollBarLeftNormalTex);
		scrollLeftBtnStyle.hover.texture = getGUITexture(ScrollBarLeftHoverTex);
		scrollLeftBtnStyle.active.texture = getGUITexture(ScrollBarLeftActiveTex);
		scrollLeftBtnStyle.fixedHeight = true;
		scrollLeftBtnStyle.fixedWidth = true;
		scrollLeftBtnStyle.height = 13;
		scrollLeftBtnStyle.width = 11;

		skin->setStyle("ScrollLeftBtn", scrollLeftBtnStyle);

		// Right button
		GUIElementStyle scrollRightBtnStyle;
		scrollRightBtnStyle.normal.texture = getGUITexture(ScrollBarRightNormalTex);
		scrollRightBtnStyle.hover.texture = getGUITexture(ScrollBarRightHoverTex);
		scrollRightBtnStyle.active.texture = getGUITexture(ScrollBarRightActiveTex);
		scrollRightBtnStyle.fixedHeight = true;
		scrollRightBtnStyle.fixedWidth = true;
		scrollRightBtnStyle.height = 13;
		scrollRightBtnStyle.width = 11;

		skin->setStyle("ScrollRightBtn", scrollRightBtnStyle);

		// Horizontal handle
		GUIElementStyle scrollBarHorzBtnStyle;
		scrollBarHorzBtnStyle.normal.texture = getGUITexture(ScrollBarHandleHorzNormalTex);
		scrollBarHorzBtnStyle.hover.texture = getGUITexture(ScrollBarHandleHorzHoverTex);
		scrollBarHorzBtnStyle.active.texture = getGUITexture(ScrollBarHandleHorzActiveTex);
		scrollBarHorzBtnStyle.fixedHeight = true;
		scrollBarHorzBtnStyle.fixedWidth = false;
		scrollBarHorzBtnStyle.width = 10;
		scrollBarHorzBtnStyle.height = 13;
		scrollBarHorzBtnStyle.border.left = 4;
		scrollBarHorzBtnStyle.border.right = 4;

		skin->setStyle("ScrollBarHorzBtn", scrollBarHorzBtnStyle);

		// Vertical handle
		GUIElementStyle scrollBarVertBtnStyle;
		scrollBarVertBtnStyle.normal.texture = getGUITexture(ScrollBarHandleVertNormalTex);
		scrollBarVertBtnStyle.hover.texture = getGUITexture(ScrollBarHandleVertHoverTex);
		scrollBarVertBtnStyle.active.texture = getGUITexture(ScrollBarHandleVertActiveTex);
		scrollBarVertBtnStyle.fixedHeight = false;
		scrollBarVertBtnStyle.fixedWidth = true;
		scrollBarVertBtnStyle.width = 13;
		scrollBarVertBtnStyle.height = 10;
		scrollBarVertBtnStyle.border.top = 4;
		scrollBarVertBtnStyle.border.bottom = 4;

		skin->setStyle("ScrollBarVertBtn", scrollBarVertBtnStyle);

		// Vertical scroll bar
		GUIElementStyle vertScrollBarStyle;
		vertScrollBarStyle.normal.texture = getGUITexture(ScrollBarVBgTex);
		vertScrollBarStyle.hover.texture = vertScrollBarStyle.normal.texture;
		vertScrollBarStyle.active.texture = vertScrollBarStyle.normal.texture;
		vertScrollBarStyle.fixedHeight = false;
		vertScrollBarStyle.fixedWidth = true;
		vertScrollBarStyle.minHeight = 8;
		vertScrollBarStyle.width = 16;

		skin->setStyle("ScrollBarVert", vertScrollBarStyle);

		// Horizontal scroll bar
		GUIElementStyle horzScrollBarStyle;
		horzScrollBarStyle.normal.texture = getGUITexture(ScrollBarHBgTex);
		horzScrollBarStyle.hover.texture = horzScrollBarStyle.normal.texture;
		horzScrollBarStyle.active.texture = horzScrollBarStyle.normal.texture;
		horzScrollBarStyle.fixedHeight = true;
		horzScrollBarStyle.fixedWidth = false;
		horzScrollBarStyle.minWidth = 8;
		horzScrollBarStyle.height = 16;

		skin->setStyle("ScrollBarHorz", horzScrollBarStyle);

		/************************************************************************/
		/* 								DROP DOWN BOX                      		*/
		/************************************************************************/

		// ListBox button
		GUIElementStyle dropDownListStyle;
		dropDownListStyle.normal.texture = getGUITexture(DropDownBtnNormalTex);
		dropDownListStyle.hover.texture = getGUITexture(DropDownBtnHoverTex);
		dropDownListStyle.active.texture = getGUITexture(DropDownBtnActiveTex);
		dropDownListStyle.normalOn.texture = dropDownListStyle.active.texture;
		dropDownListStyle.hoverOn.texture = dropDownListStyle.active.texture;
		dropDownListStyle.activeOn.texture = dropDownListStyle.active.texture;
		dropDownListStyle.normal.textColor = TextNormalColor;
		dropDownListStyle.hover.textColor = TextNormalColor;
		dropDownListStyle.active.textColor = TextNormalColor;
		dropDownListStyle.normalOn.textColor = TextNormalColor;
		dropDownListStyle.hoverOn.textColor = TextNormalColor;
		dropDownListStyle.activeOn.textColor = TextNormalColor;
		dropDownListStyle.fixedHeight = true;
		dropDownListStyle.fixedWidth = false;
		dropDownListStyle.height = 21;
		dropDownListStyle.minWidth = 20;
		dropDownListStyle.contentOffset.left = 3;
		dropDownListStyle.contentOffset.right = 18;
		dropDownListStyle.contentOffset.top = 2;
		dropDownListStyle.contentOffset.bottom = 2;
		dropDownListStyle.border.left = 2;
		dropDownListStyle.border.right = 16;
		dropDownListStyle.border.top = 2;
		dropDownListStyle.border.bottom = 4;
		dropDownListStyle.margins.bottom = 2;
		dropDownListStyle.font = defaultFont;
		dropDownListStyle.fontSize = DefaultFontSize;
		dropDownListStyle.textHorzAlign = THA_Left;
		dropDownListStyle.textVertAlign = TVA_Center;

		skin->setStyle(GUIListBox::getGUITypeName(), dropDownListStyle);

		// DropDown scroll up button
		GUIElementStyle dropDownScrollUpBtnStyle;
		dropDownScrollUpBtnStyle.normal.texture = getGUITexture(DropDownBoxBtnUpNormalTex);
		dropDownScrollUpBtnStyle.hover.texture = getGUITexture(DropDownBoxBtnUpHoverTex);
		dropDownScrollUpBtnStyle.active.texture = dropDownScrollUpBtnStyle.hover.texture;
		dropDownScrollUpBtnStyle.fixedHeight = true;
		dropDownScrollUpBtnStyle.fixedWidth = true;
		dropDownScrollUpBtnStyle.width = 8;
		dropDownScrollUpBtnStyle.height = 12;

		skin->setStyle("ListBoxScrollUpBtn", dropDownScrollUpBtnStyle);
		skin->setStyle("MenuBarScrollUpBtn", dropDownScrollUpBtnStyle);
		skin->setStyle("ContextMenuScrollUpBtn", dropDownScrollUpBtnStyle);

		// DropDown scroll down button
		GUIElementStyle dropDownScrollDownBtnStyle;
		dropDownScrollDownBtnStyle.normal.texture = getGUITexture(DropDownBoxBtnDownNormalTex);
		dropDownScrollDownBtnStyle.hover.texture = getGUITexture(DropDownBoxBtnDownHoverTex);
		dropDownScrollDownBtnStyle.active.texture = dropDownScrollDownBtnStyle.hover.texture;
		dropDownScrollDownBtnStyle.fixedHeight = true;
		dropDownScrollDownBtnStyle.fixedWidth = true;
		dropDownScrollDownBtnStyle.width = 8;
		dropDownScrollDownBtnStyle.height = 12;

		skin->setStyle("ListBoxScrollDownBtn", dropDownScrollDownBtnStyle);
		skin->setStyle("MenuBarScrollDownBtn", dropDownScrollDownBtnStyle);
		skin->setStyle("ContextMenuScrollDownBtn", dropDownScrollDownBtnStyle);

		// DropDown handle
		GUIElementStyle dropDownScrollHandleStyle;
		dropDownScrollHandleStyle.normal.texture = getGUITexture(DropDownBoxHandleTex);
		dropDownScrollHandleStyle.fixedHeight = false;
		dropDownScrollHandleStyle.fixedWidth = true;
		dropDownScrollHandleStyle.height = 8;
		dropDownScrollHandleStyle.width = 8;

		skin->setStyle("ListBoxHandle", dropDownScrollHandleStyle);
		skin->setStyle("MenuBarHandle", dropDownScrollHandleStyle);
		skin->setStyle("ContextMenuHandle", dropDownScrollHandleStyle);

		// DropDown sidebar background
		GUIElementStyle dropDownSidebarBg;
		dropDownSidebarBg.normal.texture = getGUITexture(DropDownBoxSideBgTex);
		dropDownSidebarBg.fixedHeight = false;
		dropDownSidebarBg.fixedWidth = true;
		dropDownSidebarBg.height = 8;
		dropDownSidebarBg.width = 9;
		dropDownSidebarBg.border.left = 1;
		dropDownSidebarBg.border.top = 1;
		dropDownSidebarBg.border.bottom = 1;

		skin->setStyle("ListBoxSidebarBg", dropDownSidebarBg);
		skin->setStyle("MenuBarSidebarBg", dropDownSidebarBg);
		skin->setStyle("ContextMenuSidebarBg", dropDownSidebarBg);

		// DropDown entry button
		GUIElementStyle dropDownEntryBtnStyle;
		dropDownEntryBtnStyle.normal.texture = getGUITexture(DropDownBoxEntryNormalTex);
		dropDownEntryBtnStyle.hover.texture = getGUITexture(DropDownBoxEntryHoverTex);
		dropDownEntryBtnStyle.active.texture = dropDownEntryBtnStyle.hover.texture;
		dropDownEntryBtnStyle.normalOn.texture = dropDownEntryBtnStyle.hover.texture;
		dropDownEntryBtnStyle.hoverOn.texture = dropDownEntryBtnStyle.hover.texture;
		dropDownEntryBtnStyle.activeOn.texture = dropDownEntryBtnStyle.hover.texture;
		dropDownEntryBtnStyle.normal.textColor = TextNormalColor;
		dropDownEntryBtnStyle.hover.textColor = TextNormalColor;
		dropDownEntryBtnStyle.active.textColor = TextNormalColor;
		dropDownEntryBtnStyle.normalOn.textColor = TextNormalColor;
		dropDownEntryBtnStyle.hoverOn.textColor = TextNormalColor;
		dropDownEntryBtnStyle.activeOn.textColor = TextNormalColor;
		dropDownEntryBtnStyle.fixedHeight = true;
		dropDownEntryBtnStyle.fixedWidth = false;
		dropDownEntryBtnStyle.height = 16;
		dropDownEntryBtnStyle.width = 30;
		dropDownEntryBtnStyle.font = defaultFont;
		dropDownEntryBtnStyle.fontSize = DefaultFontSize;
		dropDownEntryBtnStyle.textHorzAlign = THA_Left;
		dropDownEntryBtnStyle.textVertAlign = TVA_Center;

		skin->setStyle(GUIDropDownContent::ENTRY_STYLE_TYPE, dropDownEntryBtnStyle);

		// DropDown toggle entry button
		GUIElementStyle dropDownToggleEntryBtnStyle;
		dropDownToggleEntryBtnStyle.normal.texture = getGUITexture(DropDownBoxEntryToggleNormalTex);
		dropDownToggleEntryBtnStyle.hover.texture = getGUITexture(DropDownBoxEntryToggleHoverTex);
		dropDownToggleEntryBtnStyle.active.texture = dropDownToggleEntryBtnStyle.hover.texture;
		dropDownToggleEntryBtnStyle.normalOn.texture = getGUITexture(DropDownBoxEntryToggleNormalOnTex);
		dropDownToggleEntryBtnStyle.hoverOn.texture = getGUITexture(DropDownBoxEntryToggleHoverOnTex);
		dropDownToggleEntryBtnStyle.activeOn.texture = dropDownToggleEntryBtnStyle.hoverOn.texture;
		dropDownToggleEntryBtnStyle.normal.textColor = TextNormalColor;
		dropDownToggleEntryBtnStyle.hover.textColor = TextNormalColor;
		dropDownToggleEntryBtnStyle.active.textColor = TextNormalColor;
		dropDownToggleEntryBtnStyle.normalOn.textColor = TextNormalColor;
		dropDownToggleEntryBtnStyle.hoverOn.textColor = TextNormalColor;
		dropDownToggleEntryBtnStyle.activeOn.textColor = TextNormalColor;
		dropDownToggleEntryBtnStyle.fixedHeight = true;
		dropDownToggleEntryBtnStyle.fixedWidth = false;
		dropDownToggleEntryBtnStyle.height = 18;
		dropDownToggleEntryBtnStyle.width = 30;
		dropDownToggleEntryBtnStyle.border.left = 17;
		dropDownToggleEntryBtnStyle.contentOffset.left = 17;
		dropDownToggleEntryBtnStyle.font = defaultFont;
		dropDownToggleEntryBtnStyle.fontSize = DefaultFontSize;
		dropDownToggleEntryBtnStyle.textHorzAlign = THA_Left;
		dropDownToggleEntryBtnStyle.textVertAlign = TVA_Center;

		skin->setStyle(GUIDropDownContent::ENTRY_TOGGLE_STYLE_TYPE, dropDownToggleEntryBtnStyle);

		// DropDown entry button with expand
		GUIElementStyle dropDownEntryExpBtnStyle;
		dropDownEntryExpBtnStyle.normal.texture = getGUITexture(DropDownBoxEntryExpNormalTex);
		dropDownEntryExpBtnStyle.hover.texture = getGUITexture(DropDownBoxEntryExpHoverTex);
		dropDownEntryExpBtnStyle.active.texture = dropDownEntryExpBtnStyle.hover.texture;
		dropDownEntryExpBtnStyle.normalOn.texture = dropDownEntryExpBtnStyle.hover.texture;
		dropDownEntryExpBtnStyle.hoverOn.texture = dropDownEntryExpBtnStyle.hover.texture;
		dropDownEntryExpBtnStyle.activeOn.texture = dropDownEntryExpBtnStyle.hover.texture;
		dropDownEntryExpBtnStyle.normal.textColor = TextNormalColor;
		dropDownEntryExpBtnStyle.hover.textColor = TextNormalColor;
		dropDownEntryExpBtnStyle.active.textColor = TextNormalColor;
		dropDownEntryExpBtnStyle.normalOn.textColor = TextNormalColor;
		dropDownEntryExpBtnStyle.hoverOn.textColor = TextNormalColor;
		dropDownEntryExpBtnStyle.activeOn.textColor = TextNormalColor;
		dropDownEntryExpBtnStyle.fixedHeight = true;
		dropDownEntryExpBtnStyle.fixedWidth = false;
		dropDownEntryExpBtnStyle.height = 16;
		dropDownEntryExpBtnStyle.width = 30;
		dropDownEntryExpBtnStyle.border.right = 13;
		dropDownEntryExpBtnStyle.font = defaultFont;
		dropDownEntryExpBtnStyle.fontSize = DefaultFontSize;
		dropDownEntryExpBtnStyle.textHorzAlign = THA_Left;
		dropDownEntryExpBtnStyle.textVertAlign = TVA_Center;

		skin->setStyle(GUIDropDownContent::ENTRY_EXP_STYLE_TYPE, dropDownEntryExpBtnStyle);

		// Drop down separator
		GUIElementStyle dropDownSeparatorStyle;
		dropDownSeparatorStyle.normal.texture = getGUITexture(DropDownSeparatorTex);
		dropDownSeparatorStyle.fixedHeight = true;
		dropDownSeparatorStyle.fixedWidth = false;
		dropDownSeparatorStyle.height = 3;
		dropDownSeparatorStyle.width = 30;

		skin->setStyle(GUIDropDownContent::SEPARATOR_STYLE_TYPE, dropDownSeparatorStyle);

		// Drop down content
		GUIElementStyle dropDownContentStyle;
		dropDownContentStyle.minWidth = 50;
		dropDownContentStyle.minHeight = 20;
		dropDownContentStyle.subStyles[GUIDropDownContent::ENTRY_TOGGLE_STYLE_TYPE] = GUIDropDownContent::ENTRY_TOGGLE_STYLE_TYPE;
		dropDownContentStyle.subStyles[GUIDropDownContent::ENTRY_STYLE_TYPE] = GUIDropDownContent::ENTRY_STYLE_TYPE;
		dropDownContentStyle.subStyles[GUIDropDownContent::ENTRY_EXP_STYLE_TYPE] = GUIDropDownContent::ENTRY_EXP_STYLE_TYPE;
		dropDownContentStyle.subStyles[GUIDropDownContent::SEPARATOR_STYLE_TYPE] = GUIDropDownContent::SEPARATOR_STYLE_TYPE;

		skin->setStyle("ListBoxContent", dropDownContentStyle);
		skin->setStyle("MenuBarContent", dropDownContentStyle);
		skin->setStyle("ContextMenuContent", dropDownContentStyle);

		// DropDown box frame
		GUIElementStyle dropDownBoxStyle;
		dropDownBoxStyle.normal.texture = getGUITexture(DropDownBoxBgTex);
		dropDownBoxStyle.hover.texture = dropDownBoxStyle.normal.texture;
		dropDownBoxStyle.active.texture = dropDownBoxStyle.hover.texture;
		dropDownBoxStyle.fixedHeight = false;
		dropDownBoxStyle.fixedWidth = false;
		dropDownBoxStyle.border.left = 2;
		dropDownBoxStyle.border.right = 2;
		dropDownBoxStyle.border.top = 2;
		dropDownBoxStyle.border.bottom = 4;
		dropDownBoxStyle.margins.left = 6;
		dropDownBoxStyle.margins.right = 6;
		dropDownBoxStyle.margins.top = 4;
		dropDownBoxStyle.margins.bottom = 6;

		skin->setStyle("ListBoxFrame", dropDownBoxStyle);
		skin->setStyle("MenuBarFrame", dropDownBoxStyle);
		skin->setStyle("ContextMenuFrame", dropDownBoxStyle);

		/************************************************************************/
		/* 								TOOLTIP                      			*/
		/************************************************************************/

		GUIElementStyle tooltipFrameStyle;
		tooltipFrameStyle.normal.texture = getGUITexture(DropDownBoxBgTex);
		tooltipFrameStyle.hover.texture = tooltipFrameStyle.normal.texture;
		tooltipFrameStyle.active.texture = tooltipFrameStyle.hover.texture;
		tooltipFrameStyle.fixedHeight = false;
		tooltipFrameStyle.fixedWidth = false;
		tooltipFrameStyle.border.left = 2;
		tooltipFrameStyle.border.right = 2;
		tooltipFrameStyle.border.top = 2;
		tooltipFrameStyle.border.bottom = 4;
		tooltipFrameStyle.margins.left = 6;
		tooltipFrameStyle.margins.right = 6;
		tooltipFrameStyle.margins.top = 4;
		tooltipFrameStyle.margins.bottom = 6;

		skin->setStyle(GUITooltip::getFrameStyleName(), tooltipFrameStyle);

		/************************************************************************/
		/* 								MENU BAR	                     		*/
		/************************************************************************/

		// MenuBar background
		GUIElementStyle menuBarBgStyle;
		menuBarBgStyle.normal.texture = getGUITexture(MenuBarBgTex);
		menuBarBgStyle.height = 14;
		menuBarBgStyle.width = 2;
		menuBarBgStyle.border.bottom = 2;

		skin->setStyle(GUIMenuBar::getBackgroundStyleType(), menuBarBgStyle);

		// MenuBar separator line
		GUIElementStyle menuBarLineStyle;
		menuBarLineStyle.normal.texture = getGUITexture(MenuBarLineNormalTex);
		menuBarLineStyle.normalOn.texture = getGUITexture(MenuBarLineActiveTex);
		menuBarLineStyle.fixedHeight = true;
		menuBarLineStyle.height = 1;

		skin->setStyle(GUIMenuBar::getLineStyleType(), menuBarLineStyle);

		// MenuBar Banshee logo
		GUIElementStyle menuBarBansheeLogoStyle;
		menuBarBansheeLogoStyle.normal.texture = getGUITexture(MenuBarBansheeLogoTex);
		menuBarBansheeLogoStyle.fixedHeight = true;
		menuBarBansheeLogoStyle.fixedWidth = true;
		menuBarBansheeLogoStyle.height = 46;
		menuBarBansheeLogoStyle.width = 46;

		skin->setStyle(GUIMenuBar::getLogoStyleType(), menuBarBansheeLogoStyle);

		// MenuBar button
		GUIElementStyle menuBarBtnStyle;
		menuBarBtnStyle.normal.texture = getGUITexture(MenuBarBtnNormalTex);
		menuBarBtnStyle.hover.texture = getGUITexture(MenuBarBtnHoverTex);
		menuBarBtnStyle.active.texture = getGUITexture(MenuBarBtnActiveTex);
		menuBarBtnStyle.normalOn.texture = menuBarBtnStyle.active.texture;
		menuBarBtnStyle.hoverOn.texture = menuBarBtnStyle.active.texture;
		menuBarBtnStyle.activeOn.texture = menuBarBtnStyle.active.texture;
		menuBarBtnStyle.normal.textColor = TextNormalColor;
		menuBarBtnStyle.hover.textColor = TextNormalColor;
		menuBarBtnStyle.active.textColor = TextActiveColor;
		menuBarBtnStyle.normalOn.textColor = TextActiveColor;
		menuBarBtnStyle.hoverOn.textColor = TextActiveColor;
		menuBarBtnStyle.activeOn.textColor = TextActiveColor;
		menuBarBtnStyle.fixedHeight = true;
		menuBarBtnStyle.fixedWidth = false;
		menuBarBtnStyle.height = 14;
		menuBarBtnStyle.width = 4;
		menuBarBtnStyle.margins.left = 2;
		menuBarBtnStyle.margins.right = 2;
		menuBarBtnStyle.margins.top = 2;
		menuBarBtnStyle.margins.bottom = 2;
		menuBarBtnStyle.font = defaultFont;
		menuBarBtnStyle.fontSize = DefaultFontSize;
		menuBarBtnStyle.textHorzAlign = THA_Left;
		menuBarBtnStyle.textVertAlign = TVA_Top;

		skin->setStyle(GUIMenuBar::getMenuItemButtonStyleType(), menuBarBtnStyle);

		// ToolBar separator
		GUIElementStyle toolBarSeparator;
		toolBarSeparator.normal.texture = getGUITexture(ToolBarSeparatorTex);
		toolBarSeparator.fixedWidth = true;
		toolBarSeparator.width = 3;
		toolBarSeparator.height = 32;

		skin->setStyle(GUIMenuBar::getToolBarSeparatorStyleType(), toolBarSeparator);

		// ToolBar button
		GUIElementStyle toolBarBtnStyle;
		toolBarBtnStyle.normal.texture = getGUITexture(ToolBarBtnNormalTex);
		toolBarBtnStyle.hover.texture = getGUITexture(ToolBarBtnHoverTex);
		toolBarBtnStyle.active.texture = getGUITexture(ToolBarBtnActiveTex);
		toolBarBtnStyle.normalOn.texture = getGUITexture(ToolBarBtnActiveTex);
		toolBarBtnStyle.hoverOn.texture = getGUITexture(ToolBarBtnActiveTex);
		toolBarBtnStyle.activeOn.texture = getGUITexture(ToolBarBtnActiveTex);
		toolBarBtnStyle.normal.textColor = TextNormalColor;
		toolBarBtnStyle.hover.textColor = TextNormalColor;
		toolBarBtnStyle.active.textColor = TextActiveColor;
		toolBarBtnStyle.normalOn.textColor = TextNormalColor;
		toolBarBtnStyle.hoverOn.textColor = TextNormalColor;
		toolBarBtnStyle.activeOn.textColor = TextActiveColor;
		toolBarBtnStyle.fixedHeight = true;
		toolBarBtnStyle.fixedWidth = true;
		toolBarBtnStyle.height = 32;
		toolBarBtnStyle.width = 32;

		skin->setStyle(GUIMenuBar::getToolBarButtonStyleType(), toolBarBtnStyle);

		/************************************************************************/
		/* 								DOCK SLIDER	                     		*/
		/************************************************************************/

		GUIElementStyle dockSliderBtnStyle;
		dockSliderBtnStyle.normal.texture = getGUITexture(DockSliderNormalTex);
		dockSliderBtnStyle.hover.texture = dockSliderBtnStyle.normal.texture;
		dockSliderBtnStyle.active.texture = dockSliderBtnStyle.normal.texture;
		dockSliderBtnStyle.fixedHeight = false;
		dockSliderBtnStyle.fixedWidth = false;
		dockSliderBtnStyle.height = 2;
		dockSliderBtnStyle.width = 2;

		skin->setStyle("DockSliderBtn", dockSliderBtnStyle);

		/************************************************************************/
		/* 								TREE VIEW	                     		*/
		/************************************************************************/

		// Expand button
		GUIElementStyle treeViewExpandButtonStyle;
		treeViewExpandButtonStyle.normal.texture = getGUITexture(TreeViewExpandButtonOffNormal);
		treeViewExpandButtonStyle.hover.texture = getGUITexture(TreeViewExpandButtonOffHover);
		treeViewExpandButtonStyle.active.texture = treeViewExpandButtonStyle.hover.texture;
		treeViewExpandButtonStyle.normalOn.texture = getGUITexture(TreeViewExpandButtonOnNormal);
		treeViewExpandButtonStyle.hoverOn.texture = getGUITexture(TreeViewExpandButtonOnHover);
		treeViewExpandButtonStyle.activeOn.texture = treeViewExpandButtonStyle.hoverOn.texture;
		treeViewExpandButtonStyle.fixedHeight = true;
		treeViewExpandButtonStyle.fixedWidth = true;
		treeViewExpandButtonStyle.height = 10;
		treeViewExpandButtonStyle.width = 10;

		skin->setStyle("TreeViewFoldoutBtn", treeViewExpandButtonStyle);

		// Entry
		GUIElementStyle treeViewEntryStyle;
		treeViewEntryStyle.font = defaultFont;
		treeViewEntryStyle.fontSize = DefaultFontSize;
		treeViewEntryStyle.fixedWidth = false;
		treeViewEntryStyle.fixedHeight = true;
		treeViewEntryStyle.height = 16;
		treeViewEntryStyle.minWidth = 10;
		treeViewEntryStyle.normal.textColor = TextNormalColor;

		skin->setStyle("TreeViewElementBtn", treeViewEntryStyle);

		// Background
		GUIElementStyle treeViewBackgroundStyle;

		skin->setStyle("TreeViewBackground", treeViewBackgroundStyle);

		// Selection background
		GUIElementStyle treeViewSelBackgroundStyle;
		treeViewSelBackgroundStyle.normal.texture = getGUITexture(SelectionBgTex);
		treeViewSelBackgroundStyle.fixedHeight = false;
		treeViewSelBackgroundStyle.fixedWidth = false;
		treeViewSelBackgroundStyle.height = 2;
		treeViewSelBackgroundStyle.width = 2;

		skin->setStyle("TreeViewSelectionBackground", treeViewSelBackgroundStyle);

		// Highlight background
		GUIElementStyle treeViewHLBackgroundStyle;
		treeViewHLBackgroundStyle.normal.texture = getGUITexture(TreeViewHighlightBackground);
		treeViewHLBackgroundStyle.fixedHeight = false;
		treeViewHLBackgroundStyle.fixedWidth = false;
		treeViewHLBackgroundStyle.height = 2;
		treeViewHLBackgroundStyle.width = 2;

		skin->setStyle("TreeViewHighlightBackground", treeViewHLBackgroundStyle);

		// Edit box
		GUIElementStyle treeViewEditBox;
		treeViewEditBox.normal.texture = getGUITexture(TreeViewEditBox);
		treeViewEditBox.hover.texture = treeViewEditBox.normal.texture;
		treeViewEditBox.focused.texture = treeViewEditBox.normal.texture;
		treeViewEditBox.active.texture = treeViewEditBox.normal.texture;
		treeViewEditBox.normal.textColor = TextNormalColor;
		treeViewEditBox.hover.textColor = TextNormalColor;
		treeViewEditBox.focused.textColor = TextNormalColor;
		treeViewEditBox.active.textColor = TextNormalColor;
		treeViewEditBox.border.left = 1;
		treeViewEditBox.border.right = 1;
		treeViewEditBox.border.top = 1;
		treeViewEditBox.border.bottom = 1;
		treeViewEditBox.margins.left = 1;
		treeViewEditBox.margins.right = 1;
		treeViewEditBox.margins.top = 1;
		treeViewEditBox.margins.bottom = 1;
		treeViewEditBox.fixedHeight = true;
		treeViewEditBox.height = 13;
		treeViewEditBox.minWidth = 10;
		treeViewEditBox.font = defaultFont;
		treeViewEditBox.fontSize = DefaultFontSize;
		treeViewEditBox.textHorzAlign = THA_Left;
		treeViewEditBox.textVertAlign = TVA_Top;

		skin->setStyle(GUITreeViewEditBox::getGUITypeName(), treeViewEditBox);

		// Element highlight
		GUIElementStyle treeViewElementHighlight;
		treeViewElementHighlight.normal.texture = getGUITexture(TreeViewElementHighlight);
		treeViewElementHighlight.border.left = 1;
		treeViewElementHighlight.border.right = 1;
		treeViewElementHighlight.border.top = 1;
		treeViewElementHighlight.border.bottom = 1;

		skin->setStyle("TreeViewElementHighlight", treeViewElementHighlight);

		// Element separator highlight
		GUIElementStyle treeViewElementSepHighlight;
		treeViewElementSepHighlight.normal.texture = getGUITexture(TreeViewElementSepHighlight);
		treeViewElementSepHighlight.border.left = 1;
		treeViewElementSepHighlight.border.right = 1;
		treeViewElementSepHighlight.border.top = 1;
		treeViewElementSepHighlight.border.bottom = 1;

		skin->setStyle("TreeViewElementSepHighlight", treeViewElementSepHighlight);

		/************************************************************************/
		/* 							OBJECT DROP FIELD                      		*/
		/************************************************************************/
		GUIElementStyle objectDropStyle;
		objectDropStyle.normal.texture = getGUITexture(ObjectDropBtnNormalTex);
		objectDropStyle.normalOn.texture = getGUITexture(ObjectDropBtnNormalOnTex);
		objectDropStyle.hover.texture = objectDropStyle.normal.texture;
		objectDropStyle.hoverOn.texture = objectDropStyle.normalOn.texture;
		objectDropStyle.active.texture = objectDropStyle.normal.texture;
		objectDropStyle.activeOn.texture = objectDropStyle.normalOn.texture;
		objectDropStyle.normal.textColor = TextNormalColor;
		objectDropStyle.hover.textColor = TextNormalColor;
		objectDropStyle.active.textColor = TextNormalColor;
		objectDropStyle.normalOn.textColor = TextNormalColor;
		objectDropStyle.hoverOn.textColor = TextNormalColor;
		objectDropStyle.activeOn.textColor = TextNormalColor;
		objectDropStyle.border.left = 2;
		objectDropStyle.border.top = 2;
		objectDropStyle.border.bottom = 4;
		objectDropStyle.contentOffset.left = 3;
		objectDropStyle.contentOffset.right = 1;
		objectDropStyle.margins.bottom = 2;
		objectDropStyle.fixedHeight = true;
		objectDropStyle.height = 21;
		objectDropStyle.minWidth = 20;
		objectDropStyle.font = defaultFont;
		objectDropStyle.fontSize = DefaultFontSize;
		objectDropStyle.textHorzAlign = THA_Center;
		objectDropStyle.textVertAlign = TVA_Center;

		skin->setStyle(ObjectFieldDropBtnStyleName, objectDropStyle);

		GUIElementStyle objectClearBtnStyle;
		objectClearBtnStyle.normal.texture = getGUITexture(ObjectClearBtnNormalTex);
		objectClearBtnStyle.hover.texture = getGUITexture(ObjectClearBtnHoverTex);
		objectClearBtnStyle.active.texture = getGUITexture(ObjectClearBtnActiveTex);
		objectClearBtnStyle.fixedHeight = true;
		objectClearBtnStyle.fixedWidth = true;
		objectClearBtnStyle.height = 21;
		objectClearBtnStyle.width = 16;
		objectClearBtnStyle.margins.bottom = 2;

		skin->setStyle(ObjectFieldClearBtnStyleName, objectClearBtnStyle);

		GUIElementStyle editorObjectFieldStyle;
		editorObjectFieldStyle.fixedHeight = true;
		editorObjectFieldStyle.height = 21;
		editorObjectFieldStyle.minWidth = 30;
		editorObjectFieldStyle.subStyles[ObjectFieldLabelStyleName] = GUITextField::getLabelStyleType();
		editorObjectFieldStyle.subStyles[ObjectFieldDropBtnStyleName] = ObjectFieldDropBtnStyleName;
		editorObjectFieldStyle.subStyles[ObjectFieldClearBtnStyleName] = ObjectFieldClearBtnStyleName;

		skin->setStyle(ObjectFieldStyleName, editorObjectFieldStyle);

		/************************************************************************/
		/* 						TEXTURE DROP FIELD                      		*/
		/************************************************************************/
		GUIElementStyle textureFieldStyle;
		textureFieldStyle.minHeight = 15;
		textureFieldStyle.minWidth = 15;
		textureFieldStyle.subStyles[TextureFieldDropStyleName] = TextureFieldDropStyleName;
		textureFieldStyle.subStyles[TextureFieldClearBtnStyleName] = TextureFieldClearBtnStyleName;
		textureFieldStyle.subStyles[TextureFieldLabelStyleName] = GUITextField::getLabelStyleType();

		skin->setStyle(TextureFieldStyleName, textureFieldStyle);

		GUIElementStyle textureDropStyle;
		textureDropStyle.font = defaultFont;
		textureDropStyle.fontSize = DefaultFontSize;
		textureDropStyle.textHorzAlign = THA_Center;
		textureDropStyle.textVertAlign = TVA_Center;
		textureDropStyle.normal.textColor = Color(95 / 255.0f, 95 / 255.0f, 95 / 255.0f, 1.0f);
		textureDropStyle.normal.texture = getGUITexture(TextureDropTex);
		textureDropStyle.hover.textColor = textureDropStyle.normal.textColor;
		textureDropStyle.hover.texture = textureDropStyle.normal.texture;
		textureDropStyle.active.textColor = textureDropStyle.normal.textColor;
		textureDropStyle.active.texture = textureDropStyle.normal.texture;
		textureDropStyle.normalOn.textColor = Color(95 / 255.0f, 95 / 255.0f, 95 / 255.0f, 1.0f);
		textureDropStyle.normalOn.texture = getGUITexture(TextureDropOnTex);
		textureDropStyle.hoverOn.textColor = textureDropStyle.normalOn.textColor;
		textureDropStyle.hoverOn.texture = textureDropStyle.normalOn.texture;
		textureDropStyle.activeOn.textColor = textureDropStyle.normalOn.textColor;
		textureDropStyle.activeOn.texture = textureDropStyle.normalOn.texture;
		textureDropStyle.height = 84;
		textureDropStyle.width = 82;
		textureDropStyle.fixedHeight = true;
		textureDropStyle.fixedWidth = true;
		textureDropStyle.border.left = 2;
		textureDropStyle.border.right = 2;
		textureDropStyle.border.top = 2;
		textureDropStyle.border.bottom = 4;
		textureDropStyle.contentOffset.left = 4;
		textureDropStyle.contentOffset.right = 4;
		textureDropStyle.contentOffset.top = 4;
		textureDropStyle.contentOffset.bottom = 4;

		skin->setStyle(TextureFieldDropStyleName, textureDropStyle);
		
		GUIElementStyle textureDropClearStyle;
		textureDropClearStyle.normal.texture = getGUITexture(XButtonNormalTex);
		textureDropClearStyle.hover.texture = getGUITexture(XButtonHoverTex);
		textureDropClearStyle.active.texture = getGUITexture(XButtonActiveTex);
		textureDropClearStyle.fixedWidth = true;
		textureDropClearStyle.fixedHeight = true;
		textureDropClearStyle.height = 10;
		textureDropClearStyle.width = 10;

		skin->setStyle(TextureFieldClearBtnStyleName, textureDropClearStyle);

		/************************************************************************/
		/* 								EDITOR FIELDS                      		*/
		/************************************************************************/

		GUIElementStyle editorFieldLabelStyle;
		editorFieldLabelStyle.font = defaultFont;
		editorFieldLabelStyle.fontSize = DefaultFontSize;
		editorFieldLabelStyle.normal.textColor = TextNormalColor;
		editorFieldLabelStyle.fixedWidth = false;
		editorFieldLabelStyle.fixedHeight = true;
		editorFieldLabelStyle.height = 14;
		editorFieldLabelStyle.minWidth = 10;
		editorFieldLabelStyle.textHorzAlign = THA_Left;

		skin->setStyle(GUITextField::getLabelStyleType(), editorFieldLabelStyle);

		GUIElementStyle editorIntFieldStyle;
		editorIntFieldStyle.fixedHeight = true;
		editorIntFieldStyle.height = 21;
		editorIntFieldStyle.minWidth = 30;
		editorIntFieldStyle.subStyles[GUIIntField::getLabelStyleType()] = GUITextField::getLabelStyleType();
		editorIntFieldStyle.subStyles[GUIIntField::getInputStyleType()] = GUIInputBox::getGUITypeName();

		skin->setStyle(GUIIntField::getGUITypeName(), editorIntFieldStyle);

		GUIElementStyle editorFloatFieldStyle;
		editorFloatFieldStyle.fixedHeight = true;
		editorFloatFieldStyle.height = 21;
		editorFloatFieldStyle.minWidth = 30;
		editorFloatFieldStyle.subStyles[GUIFloatField::getLabelStyleType()] = GUITextField::getLabelStyleType();
		editorFloatFieldStyle.subStyles[GUIFloatField::getInputStyleType()] = GUIInputBox::getGUITypeName();

		skin->setStyle(GUIFloatField::getGUITypeName(), editorFloatFieldStyle);

		GUIElementStyle editorTextFieldStyle;
		editorTextFieldStyle.fixedHeight = true;
		editorTextFieldStyle.height = 21;
		editorTextFieldStyle.minWidth = 30;
		editorTextFieldStyle.subStyles[GUITextField::getLabelStyleType()] = GUITextField::getLabelStyleType();
		editorTextFieldStyle.subStyles[GUITextField::getInputStyleType()] = GUIInputBox::getGUITypeName();

		skin->setStyle(GUITextField::getGUITypeName(), editorTextFieldStyle);

		GUIElementStyle editorColorFieldStyle;
		editorColorFieldStyle.fixedHeight = true;
		editorColorFieldStyle.height = 21;
		editorColorFieldStyle.minWidth = 30;
		editorColorFieldStyle.subStyles[GUIColorField::getLabelStyleType()] = GUITextField::getLabelStyleType();
		editorColorFieldStyle.subStyles[GUIColorField::getColorInputStyleType()] = GUIColor::getGUITypeName();

		skin->setStyle(GUIColorField::getGUITypeName(), editorColorFieldStyle);

		GUIElementStyle editorToggleFieldStyle;
		editorToggleFieldStyle.fixedHeight = true;
		editorToggleFieldStyle.height = 15;
		editorToggleFieldStyle.minWidth = 30;
		editorToggleFieldStyle.subStyles[GUIToggleField::getLabelStyleType()] = GUITextField::getLabelStyleType();
		editorToggleFieldStyle.subStyles[GUIToggleField::getToggleStyleType()] = GUIToggle::getGUITypeName();

		skin->setStyle(GUIToggleField::getGUITypeName(), editorToggleFieldStyle);

		GUIElementStyle editorVector2FieldStyle;
		editorVector2FieldStyle.fixedHeight = true;
		editorVector2FieldStyle.height = 35;
		editorVector2FieldStyle.minWidth = 30;
		editorVector2FieldStyle.subStyles[GUIVector2Field::getLabelStyleType()] = GUITextField::getLabelStyleType();
		editorVector2FieldStyle.subStyles[GUIVector2Field::getFloatFieldStyleType()] = GUIFloatField::getGUITypeName();

		skin->setStyle(GUIVector2Field::getGUITypeName(), editorVector2FieldStyle);

		GUIElementStyle editorVector3FieldStyle;
		editorVector3FieldStyle.fixedHeight = true;
		editorVector3FieldStyle.height = 35;
		editorVector3FieldStyle.minWidth = 30;
		editorVector3FieldStyle.subStyles[GUIVector3Field::getLabelStyleType()] = GUITextField::getLabelStyleType();
		editorVector3FieldStyle.subStyles[GUIVector3Field::getFloatFieldStyleType()] = GUIFloatField::getGUITypeName();

		skin->setStyle(GUIVector3Field::getGUITypeName(), editorVector3FieldStyle);

		GUIElementStyle editorVector4FieldStyle;
		editorVector4FieldStyle.fixedHeight = true;
		editorVector4FieldStyle.height = 35;
		editorVector4FieldStyle.minWidth = 30;
		editorVector4FieldStyle.subStyles[GUIVector4Field::getLabelStyleType()] = GUITextField::getLabelStyleType();
		editorVector4FieldStyle.subStyles[GUIVector4Field::getFloatFieldStyleType()] = GUIFloatField::getGUITypeName();

		skin->setStyle(GUIVector4Field::getGUITypeName(), editorVector4FieldStyle);

		GUIElementStyle editorListBoxFieldStyle;
		editorListBoxFieldStyle.fixedHeight = true;
		editorListBoxFieldStyle.height = 21;
		editorListBoxFieldStyle.minWidth = 30;
		editorListBoxFieldStyle.subStyles[GUIListBoxField::getLabelStyleType()] = GUIListBoxField::getLabelStyleType();
		editorListBoxFieldStyle.subStyles[GUIListBoxField::getListBoxStyleType()] = GUIListBox::getGUITypeName();

		skin->setStyle(GUIListBoxField::getGUITypeName(), editorListBoxFieldStyle);

		GUIElementStyle editorSliderFieldStyle;
		editorSliderFieldStyle.fixedHeight = true;
		editorSliderFieldStyle.height = 21;
		editorSliderFieldStyle.minWidth = 30;
		editorSliderFieldStyle.subStyles[GUISliderField::getLabelStyleType()] = GUISliderField::getLabelStyleType();
		editorSliderFieldStyle.subStyles[GUISliderField::getInputStyleType()] = GUIInputBox::getGUITypeName();
		editorSliderFieldStyle.subStyles[GUISliderField::getSliderStyleType()] = GUISliderHorz::getGUITypeName();

		skin->setStyle(GUISliderField::getGUITypeName(), editorSliderFieldStyle);

		/************************************************************************/
		/* 							     FOLDOUT                      		    */
		/************************************************************************/

		GUIElementStyle foldoutStyle;
		foldoutStyle.normal.texture = getGUITexture(FoldoutClosedNormalTex);
		foldoutStyle.hover.texture = getGUITexture(FoldoutClosedHoverTex);
		foldoutStyle.active.texture = getGUITexture(FoldoutOpenActiveTex);
		foldoutStyle.normalOn.texture = getGUITexture(FoldoutOpenNormalTex);
		foldoutStyle.hoverOn.texture = getGUITexture(FoldoutOpenHoverTex);
		foldoutStyle.activeOn.texture = getGUITexture(FoldoutOpenActiveTex);
		foldoutStyle.normal.textColor = TextNormalColor;
		foldoutStyle.hover.textColor = TextNormalColor;
		foldoutStyle.active.textColor = TextNormalColor;
		foldoutStyle.normalOn.textColor = TextNormalColor;
		foldoutStyle.hoverOn.textColor = TextNormalColor;
		foldoutStyle.activeOn.textColor = TextNormalColor;
		foldoutStyle.fixedHeight = true;
		foldoutStyle.fixedWidth = false;
		foldoutStyle.height = 21;
		foldoutStyle.minWidth = 17;
		foldoutStyle.font = defaultFont;
		foldoutStyle.fontSize = DefaultFontSize;
		foldoutStyle.textHorzAlign = THA_Left;
		foldoutStyle.textVertAlign = TVA_Center;
		foldoutStyle.contentOffset = RectOffset(16, 0, 0, 0);
		foldoutStyle.border.left = 15;
		foldoutStyle.border.right = 2;
		foldoutStyle.border.top = 2;
		foldoutStyle.border.bottom = 4;

		skin->setStyle("Foldout", foldoutStyle);

		/************************************************************************/
		/* 								PROGRESS BAR                      		*/
		/************************************************************************/
		GUIElementStyle progressBarBgStyle;
		progressBarBgStyle.fixedHeight = true;
		progressBarBgStyle.height = 17;
		progressBarBgStyle.normal.texture = getGUITexture(ProgressBarBgTex);
		progressBarBgStyle.border.left = 8;
		progressBarBgStyle.border.right = 8;

		skin->setStyle(GUIProgressBar::getBackgroundStyleType(), progressBarBgStyle);

		GUIElementStyle progressBarFillStyle;
		progressBarFillStyle.fixedHeight = true;
		progressBarFillStyle.height = 16;
		progressBarFillStyle.normal.texture = getGUITexture(ProgressBarFillTex);
		progressBarFillStyle.border.left = 9;
		progressBarFillStyle.border.right = 10;

		skin->setStyle(GUIProgressBar::getBarStyleType(), progressBarFillStyle);

		GUIElementStyle progressBarStyle;
		progressBarStyle.fixedHeight = true;
		progressBarStyle.height = 17;
		progressBarStyle.minWidth = 100;

		progressBarStyle.subStyles[GUIProgressBar::getBarStyleType()] = GUIProgressBar::getBarStyleType();
		progressBarStyle.subStyles[GUIProgressBar::getBackgroundStyleType()] = GUIProgressBar::getBackgroundStyleType();

		skin->setStyle(GUIProgressBar::getGUITypeName(), progressBarStyle);

		/************************************************************************/
		/* 								SLIDER                      			*/
		/************************************************************************/

		GUIElementStyle sliderHandleStyle;
		sliderHandleStyle.fixedHeight = true;
		sliderHandleStyle.fixedWidth = true;
		sliderHandleStyle.width = 12;
		sliderHandleStyle.height = 13;
		sliderHandleStyle.normal.texture = getGUITexture(SliderHandleNormalTex);
		sliderHandleStyle.hover.texture = getGUITexture(SliderHandleHoverTex);
		sliderHandleStyle.active.texture = getGUITexture(SliderHandleActiveTex);

		skin->setStyle(GUISlider::getHandleStyleType(), sliderHandleStyle);

		GUIElementStyle sliderHorizontalBgStyle;
		sliderHorizontalBgStyle.fixedHeight = true;
		sliderHorizontalBgStyle.height = 10;
		sliderHorizontalBgStyle.normal.texture = getGUITexture(SliderHBackgroundTex);
		sliderHorizontalBgStyle.border.left = 4;
		sliderHorizontalBgStyle.border.right = 4;

		skin->setStyle("SliderHorzBg", sliderHorizontalBgStyle);

		GUIElementStyle sliderHorizontalFillStyle;
		sliderHorizontalFillStyle.fixedHeight = true;
		sliderHorizontalFillStyle.height = 10;
		sliderHorizontalFillStyle.normal.texture = getGUITexture(SliderHFillTex);
		sliderHorizontalFillStyle.border.left = 6;
		sliderHorizontalFillStyle.border.right = 4;

		skin->setStyle("SliderHorzFill", sliderHorizontalFillStyle);

		GUIElementStyle sliderHorizontalStyle;
		sliderHorizontalStyle.fixedHeight = true;
		sliderHorizontalStyle.height = 13;
		sliderHorizontalStyle.width = 150;
		sliderHorizontalStyle.minWidth = 10;
		sliderHorizontalStyle.subStyles[GUISlider::getHandleStyleType()] = GUISlider::getHandleStyleType();
		sliderHorizontalStyle.subStyles[GUISlider::getBackgroundStyleType()] = "SliderHorzBg";
		sliderHorizontalStyle.subStyles[GUISlider::getFillStyleType()] = "SliderHorzFill";

		skin->setStyle(GUISliderHorz::getGUITypeName(), sliderHorizontalStyle);

		GUIElementStyle sliderVerticalBgStyle;
		sliderVerticalBgStyle.fixedWidth = true;
		sliderVerticalBgStyle.width = 10;
		sliderVerticalBgStyle.normal.texture = getGUITexture(SliderVBackgroundTex);
		sliderVerticalBgStyle.border.top = 4;
		sliderVerticalBgStyle.border.bottom = 4;

		skin->setStyle("SliderVertBg", sliderVerticalBgStyle);

		GUIElementStyle sliderVerticalFillStyle;
		sliderVerticalFillStyle.fixedWidth = true;
		sliderVerticalFillStyle.width = 10;
		sliderVerticalFillStyle.normal.texture = getGUITexture(SliderVFillTex);
		sliderVerticalFillStyle.border.top = 6;
		sliderVerticalFillStyle.border.bottom = 4;

		skin->setStyle("SliderVertFill", sliderVerticalFillStyle);

		GUIElementStyle sliderVerticalStyle;
		sliderVerticalStyle.fixedWidth = true;
		sliderVerticalStyle.width = 13;
		sliderVerticalStyle.height = 150;
		sliderVerticalStyle.minHeight = 10;
		sliderVerticalStyle.subStyles[GUISlider::getHandleStyleType()] = GUISlider::getHandleStyleType();
		sliderVerticalStyle.subStyles[GUISlider::getBackgroundStyleType()] = "SliderVertBg";
		sliderVerticalStyle.subStyles[GUISlider::getFillStyleType()] = "SliderVertFill";

		skin->setStyle(GUISliderVert::getGUITypeName(), sliderVerticalStyle);

		/************************************************************************/
		/* 							COLOR PICKER SLIDER                      	*/
		/************************************************************************/

		GUIElementStyle colorPickerSliderHorzHandleStyle;
		colorPickerSliderHorzHandleStyle.fixedHeight = true;
		colorPickerSliderHorzHandleStyle.fixedWidth = true;
		colorPickerSliderHorzHandleStyle.height = 20;
		colorPickerSliderHorzHandleStyle.width = 7;
		colorPickerSliderHorzHandleStyle.normal.texture = getGUITexture(ColorPickerSliderHorzHandleTex);
		colorPickerSliderHorzHandleStyle.hover.texture = colorPickerSliderHorzHandleStyle.normal.texture;
		colorPickerSliderHorzHandleStyle.active.texture = colorPickerSliderHorzHandleStyle.normal.texture;

		skin->setStyle("ColorSliderHorzHandle", colorPickerSliderHorzHandleStyle);

		GUIElementStyle colorPickerSliderHorzStyle;
		colorPickerSliderHorzStyle.fixedHeight = true;
		colorPickerSliderHorzStyle.height = 32;
		colorPickerSliderHorzStyle.minWidth = 20;
		colorPickerSliderHorzStyle.subStyles[GUISlider::getHandleStyleType()] = "ColorSliderHorzHandle";

		skin->setStyle("ColorSliderHorz", colorPickerSliderHorzStyle);

		GUIElementStyle colorPickerSliderVertHandleStyle;
		colorPickerSliderVertHandleStyle.fixedHeight = true;
		colorPickerSliderVertHandleStyle.fixedWidth = true;
		colorPickerSliderVertHandleStyle.height = 7;
		colorPickerSliderVertHandleStyle.width = 45;
		colorPickerSliderVertHandleStyle.normal.texture = getGUITexture(ColorPickerSliderVertHandleTex);
		colorPickerSliderVertHandleStyle.hover.texture = colorPickerSliderVertHandleStyle.normal.texture;
		colorPickerSliderVertHandleStyle.active.texture = colorPickerSliderVertHandleStyle.normal.texture;

		skin->setStyle("ColorSliderVertHandle", colorPickerSliderVertHandleStyle);

		GUIElementStyle colorPickerSliderVertStyle;
		colorPickerSliderVertStyle.fixedWidth = true;
		colorPickerSliderVertStyle.width = 30;
		colorPickerSliderVertStyle.minHeight = 20;
		colorPickerSliderVertStyle.subStyles[GUISlider::getHandleStyleType()] = "ColorSliderVertHandle";

		skin->setStyle("ColorSliderVert", colorPickerSliderVertStyle);

		GUIElementStyle colorPickerSlider2DHandleStyle;
		colorPickerSlider2DHandleStyle.fixedHeight = true;
		colorPickerSlider2DHandleStyle.fixedWidth = true;
		colorPickerSlider2DHandleStyle.height = 7;
		colorPickerSlider2DHandleStyle.width = 7;
		colorPickerSlider2DHandleStyle.normal.texture = getGUITexture(ColorPickerSlider2DHandleTex);
		colorPickerSlider2DHandleStyle.hover.texture = colorPickerSlider2DHandleStyle.normal.texture;
		colorPickerSlider2DHandleStyle.active.texture = colorPickerSlider2DHandleStyle.normal.texture;

		skin->setStyle("ColorSlider2DHandle", colorPickerSlider2DHandleStyle);

		/************************************************************************/
		/* 								STATUS BAR                      		*/
		/************************************************************************/
		GUIElementStyle statusBarBgStyle;
		statusBarBgStyle.height = 16;
		statusBarBgStyle.border.top = 2;
		statusBarBgStyle.normal.texture = getGUITexture(StatusBarBgTex);

		skin->setStyle(GUIStatusBar::getGUIBackgroundTypeName(), statusBarBgStyle);

		GUIElementStyle statusBarMessageBtnStyle;
		statusBarMessageBtnStyle.font = defaultFont;
		statusBarMessageBtnStyle.fontSize = DefaultFontSize;
		statusBarMessageBtnStyle.fixedHeight = true;
		statusBarMessageBtnStyle.height = 16;
		statusBarMessageBtnStyle.minWidth = 10;
		statusBarMessageBtnStyle.textHorzAlign = THA_Left;
		statusBarMessageBtnStyle.textVertAlign = TVA_Center;
		statusBarMessageBtnStyle.imagePosition = GUIImagePosition::Left;

		skin->setStyle(GUIStatusBar::getGUIMessageTypeName(), statusBarMessageBtnStyle);

		GUIElementStyle statusBarStyle;
		statusBarStyle.height = 16;

		statusBarStyle.subStyles[GUIStatusBar::getGUIBackgroundTypeName()] = GUIStatusBar::getGUIBackgroundTypeName();
		statusBarStyle.subStyles[GUIStatusBar::getGUIMessageTypeName()] = GUIStatusBar::getGUIMessageTypeName();

		skin->setStyle(GUIStatusBar::getGUITypeName(), statusBarStyle);

		/************************************************************************/
		/* 									OTHER                      			*/
		/************************************************************************/

		// Centered label
		GUIElementStyle centeredLabelStyle;
		centeredLabelStyle.font = defaultFont;
		centeredLabelStyle.fontSize = DefaultFontSize;
		centeredLabelStyle.fixedWidth = false;
		centeredLabelStyle.fixedHeight = true;
		centeredLabelStyle.height = 11;
		centeredLabelStyle.minWidth = 10;
		centeredLabelStyle.textHorzAlign = THA_Center;
		centeredLabelStyle.normal.textColor = TextNormalColor;

		skin->setStyle("LabelCentered", centeredLabelStyle);

		// Right-aligned label
		GUIElementStyle rightAlignedLabelStyle;
		rightAlignedLabelStyle.font = defaultFont;
		rightAlignedLabelStyle.fontSize = DefaultFontSize;
		rightAlignedLabelStyle.fixedWidth = false;
		rightAlignedLabelStyle.fixedHeight = true;
		rightAlignedLabelStyle.height = 11;
		rightAlignedLabelStyle.minWidth = 10;
		rightAlignedLabelStyle.textHorzAlign = THA_Right;
		rightAlignedLabelStyle.normal.textColor = TextNormalColor;

		skin->setStyle("RightAlignedLabel", rightAlignedLabelStyle);

		// Multi-line label
		GUIElementStyle multiLinelabelStyle;
		multiLinelabelStyle.font = defaultFont;
		multiLinelabelStyle.fontSize = DefaultFontSize;
		multiLinelabelStyle.fixedWidth = false;
		multiLinelabelStyle.fixedHeight = true;
		multiLinelabelStyle.height = 11;
		multiLinelabelStyle.minWidth = 10;
		multiLinelabelStyle.wordWrap = true;
		multiLinelabelStyle.normal.textColor = TextNormalColor;

		skin->setStyle(BuiltinResources::MultiLineLabelStyle, multiLinelabelStyle);

		// Multi-line centered label
		GUIElementStyle multiLineCenteredLabelStyle;
		multiLineCenteredLabelStyle.font = defaultFont;
		multiLineCenteredLabelStyle.fontSize = DefaultFontSize;
		multiLineCenteredLabelStyle.fixedWidth = false;
		multiLineCenteredLabelStyle.fixedHeight = true;
		multiLineCenteredLabelStyle.height = 11;
		multiLineCenteredLabelStyle.minWidth = 10;
		multiLineCenteredLabelStyle.wordWrap = true;
		multiLineCenteredLabelStyle.textHorzAlign = THA_Center;
		multiLineCenteredLabelStyle.normal.textColor = TextNormalColor;

		skin->setStyle("MultiLineLabelCentered", multiLineCenteredLabelStyle);

		// Title label
		GUIElementStyle titleLabel;
		titleLabel.font = defaultAAFont;
		titleLabel.fontSize = TitleFontSize;
		titleLabel.fixedWidth = false;
		titleLabel.fixedHeight = true;
		titleLabel.height = 20;
		titleLabel.minWidth = 10;
		titleLabel.wordWrap = true;
		titleLabel.textHorzAlign = THA_Center;
		titleLabel.normal.textColor = TextNormalColor;

		skin->setStyle("TitleLabel", titleLabel);

		// Selection area
		GUIElementStyle selectionAreaStyle;
		selectionAreaStyle.normal.texture = getGUITexture(SelectionAreaTex);
		selectionAreaStyle.border.left = 1;
		selectionAreaStyle.border.right = 1;
		selectionAreaStyle.border.top = 1;
		selectionAreaStyle.border.bottom = 1;

		skin->setStyle("SelectionArea", selectionAreaStyle);

		// Selectable label
		GUIElementStyle selectableLabelStyle;
		selectableLabelStyle.normalOn.texture = getGUITexture(SelectionBgTex);
		selectableLabelStyle.hoverOn.texture = selectableLabelStyle.normalOn.texture;
		selectableLabelStyle.activeOn.texture = selectableLabelStyle.normalOn.texture;
		selectableLabelStyle.fixedHeight = true;
		selectableLabelStyle.height = 11;
		selectableLabelStyle.minWidth = 10;
		selectableLabelStyle.font = defaultFont;
		selectableLabelStyle.fontSize = DefaultFontSize;
		selectableLabelStyle.textHorzAlign = THA_Left;
		selectableLabelStyle.normal.textColor = TextNormalColor;

		skin->setStyle("SelectableLabel", selectableLabelStyle);

		// Scroll area background
		GUIElementStyle scrollAreaBg;
		scrollAreaBg.normal.texture = getGUITexture(ScrollAreaBgTex);
		scrollAreaBg.minHeight = 6;
		scrollAreaBg.minWidth = 4;
		scrollAreaBg.border.left = 2;
		scrollAreaBg.border.right = 2;
		scrollAreaBg.border.top = 2;
		scrollAreaBg.border.bottom = 4;
		
		skin->setStyle("ScrollAreaBg", scrollAreaBg);

		// Inspector title area background
		GUIElementStyle inspectorTitleBg;
		inspectorTitleBg.normal.texture = getGUITexture(InspectorTitleBgTex);
		inspectorTitleBg.minHeight = 4;
		inspectorTitleBg.border.bottom = 2;

		skin->setStyle("InspectorTitleBg", inspectorTitleBg);

		// Inspector content background
		GUIElementStyle inspectorContentBg;
		inspectorContentBg.normal.texture = getGUITexture(InspectorContentBgTex);
		inspectorContentBg.minWidth = 4;
		inspectorContentBg.minHeight = 6;
		inspectorContentBg.border.left = 2;
		inspectorContentBg.border.right = 2;
		inspectorContentBg.border.top = 2;
		inspectorContentBg.border.bottom = 4;

		skin->setStyle("InspectorContentBg", inspectorContentBg);

		// Inspector content alternate background
		GUIElementStyle inspectorContentBgAlternate;
		inspectorContentBgAlternate.normal.texture = getGUITexture(InspectorContentBgAlternateTex);
		inspectorContentBgAlternate.minWidth = 4;
		inspectorContentBgAlternate.minHeight = 6;
		inspectorContentBgAlternate.border.left = 2;
		inspectorContentBgAlternate.border.right = 2;
		inspectorContentBgAlternate.border.top = 2;
		inspectorContentBgAlternate.border.bottom = 4;

		skin->setStyle("InspectorContentBgAlternate", inspectorContentBgAlternate);

		// Background for multi-element resource entries in the library window
		GUIElementStyle libraryEntryFirstBg;
		libraryEntryFirstBg.normal.texture = getGUITexture(LibraryEntryFirstBgTex);
		libraryEntryFirstBg.minHeight = 7;
		libraryEntryFirstBg.minWidth = 4;
		libraryEntryFirstBg.border.top = 5;
		libraryEntryFirstBg.border.bottom = 2;
		libraryEntryFirstBg.border.left = 2;

		skin->setStyle("LibraryEntryFirstBg", libraryEntryFirstBg);

		GUIElementStyle libraryEntryBg;
		libraryEntryBg.normal.texture = getGUITexture(LibraryEntryBgTex);
		libraryEntryBg.minHeight = 7;
		libraryEntryBg.minWidth = 4;
		libraryEntryBg.border.top = 5;
		libraryEntryBg.border.bottom = 2;

		skin->setStyle("LibraryEntryBg", libraryEntryBg);

		GUIElementStyle libraryEntryLastBg;
		libraryEntryLastBg.normal.texture = getGUITexture(LibraryEntryLastBgTex);
		libraryEntryLastBg.minHeight = 7;
		libraryEntryFirstBg.minWidth = 4;
		libraryEntryLastBg.border.top = 5;
		libraryEntryLastBg.border.bottom = 2;
		libraryEntryLastBg.border.right = 2;

		skin->setStyle("LibraryEntryLastBg", libraryEntryLastBg);

		GUIElementStyle libraryEntryVertFirstBg;
		libraryEntryVertFirstBg.normal.texture = getGUITexture(LibraryEntryVertFirstBgTex);
		libraryEntryVertFirstBg.minHeight = 5;
		libraryEntryVertFirstBg.minWidth = 6;
		libraryEntryVertFirstBg.border.top = 4;
		libraryEntryVertFirstBg.border.left = 2;
		libraryEntryVertFirstBg.border.right = 4;

		skin->setStyle("LibraryEntryVertFirstBg", libraryEntryVertFirstBg);

		GUIElementStyle libraryEntryVertBg;
		libraryEntryVertBg.normal.texture = getGUITexture(LibraryEntryVertBgTex);
		libraryEntryVertBg.minHeight = 4;
		libraryEntryVertBg.minWidth = 6;
		libraryEntryVertBg.border.left = 2;
		libraryEntryVertBg.border.right = 4;

		skin->setStyle("LibraryEntryVertBg", libraryEntryVertBg);

		GUIElementStyle libraryEntryVertLastBg;
		libraryEntryVertLastBg.normal.texture = getGUITexture(LibraryEntryVertLastBgTex);
		libraryEntryVertLastBg.minHeight = 4;
		libraryEntryVertLastBg.minWidth = 6;
		libraryEntryVertLastBg.border.bottom = 2;
		libraryEntryVertLastBg.border.left = 2;
		libraryEntryVertLastBg.border.right = 4;

		skin->setStyle("LibraryEntryVertLastBg", libraryEntryVertLastBg);

		return skin;
	}

	HSpriteTexture BuiltinEditorResources::getGUITexture(const WString& name) const
	{
		Path texturePath = FileSystem::getWorkingDirectoryPath();
		texturePath.append(EditorSkinFolder);
		texturePath.append(L"sprite_" + name + L".asset");

		return gResources().load<SpriteTexture>(texturePath);
	}

	HSpriteTexture BuiltinEditorResources::getGUIIcon(const WString& name) const
	{
		Path texturePath = FileSystem::getWorkingDirectoryPath();
		texturePath.append(EditorIconFolder);
		texturePath.append(L"sprite_" + name + L".asset");

		return gResources().load<SpriteTexture>(texturePath);
	}

	HShader BuiltinEditorResources::getShader(const WString& name) const
	{
		Path programPath = EditorShaderFolder;
		programPath.append(name + L".asset");

		return gResources().load<Shader>(programPath);
	}

	HMaterial BuiltinEditorResources::createDockDropOverlayMaterial() const
	{
		return Material::create(mShaderDockOverlay);
	}

	HMaterial BuiltinEditorResources::createSceneGridMaterial() const
	{
		return Material::create(mShaderSceneGrid);
	}

	HMaterial BuiltinEditorResources::createPicking(CullingMode cullMode) const
	{
		UINT32 modeIdx = (UINT32)cullMode;

		return Material::create(mShaderPicking[modeIdx]);
	}

	HMaterial BuiltinEditorResources::createPickingAlpha(CullingMode cullMode) const
	{
		UINT32 modeIdx = (UINT32)cullMode;

		return Material::create(mShaderPickingAlpha[modeIdx]);
	}

	HMaterial BuiltinEditorResources::createLineGizmoMat() const
	{
		return Material::create(mShaderGizmoLine);
	}

	HMaterial BuiltinEditorResources::createSolidGizmoMat() const
	{
		return Material::create(mShaderGizmoSolid);
	}

	HMaterial BuiltinEditorResources::createWireGizmoMat() const
	{
		return Material::create(mShaderGizmoWire);
	}

	HMaterial BuiltinEditorResources::createIconGizmoMat() const
	{
		return Material::create(mShaderGizmoIcon);
	}

	HMaterial BuiltinEditorResources::createGizmoPickingMat() const
	{
		return Material::create(mShaderGizmoPicking);
	}

	HMaterial BuiltinEditorResources::createAlphaGizmoPickingMat() const
	{
		return Material::create(mShaderGizmoAlphaPicking);
	}

	HMaterial BuiltinEditorResources::createLineHandleMat() const
	{
		return Material::create(mShaderHandleLine);
	}

	HMaterial BuiltinEditorResources::createTextGizmoMat() const
	{
		return Material::create(mShaderGizmoText);
	}

	HMaterial BuiltinEditorResources::createSolidHandleMat() const
	{
		return Material::create(mShaderHandleSolid);
	}

	HMaterial BuiltinEditorResources::createHandleClearAlphaMat() const
	{
		return Material::create(mShaderHandleClearAlpha);
	}

	HMaterial BuiltinEditorResources::createSelectionMat() const
	{
		return Material::create(mShaderSelection);
	}

	HSpriteTexture BuiltinEditorResources::getLibraryIcon(ProjectIcon icon, int size) const
	{
		WString iconName;

		switch (icon)
		{
		case ProjectIcon::Folder:
			iconName = FolderIconTex;
			break;
		case ProjectIcon::Font:
			iconName = FontIconTex;
			break;
		case ProjectIcon::Mesh:
			iconName = MeshIconTex;
			break;
		case ProjectIcon::Texture:
			iconName = TextureIconTex;
			break;
		case ProjectIcon::PlainText:
			iconName = PlainTextIconTex;
			break;
		case ProjectIcon::ScriptCode:
			iconName = ScriptCodeIconTex;
			break;
		case ProjectIcon::Shader:
			iconName = ShaderIconTex;
			break;
		case ProjectIcon::ShaderInclude:
			iconName = ShaderIncludeIconTex;
			break;
		case ProjectIcon::Material:
			iconName = MaterialIconTex;
			break;
		case ProjectIcon::SpriteTexture:
			iconName = SpriteTextureIconTex;
			break;
		case ProjectIcon::Prefab:
			iconName = PrefabIconTex;
			break;
		case ProjectIcon::GUISkin:
			iconName = GUISkinIconTex;
			break;
		case ProjectIcon::PhysicsMaterial:
			iconName = PhysicsMaterialIconTex;
			break;
		case ProjectIcon::PhysicsMesh:
			iconName = PhysicsMeshIconTex;
			break;
		}

		if (iconName.empty())
			return HSpriteTexture();

		if (size <= 16)
			iconName += L"16";
		else if (size <= 32)
			iconName += L"32";
		else if (size <= 48)
			iconName += L"48";

		return getGUIIcon(iconName);
	}

	HSpriteTexture BuiltinEditorResources::getToolbarIcon(ToolbarIcon icon) const
	{
		switch (icon)
		{
		case ToolbarIcon::NewCamera:
			return getGUIIcon(L"ToolbarNewCameraIcon.png");
		case ToolbarIcon::NewRenderable:
			return getGUIIcon(L"ToolbarNewRenderableIcon.png");
		case ToolbarIcon::NewPointLight:
			return getGUIIcon(L"ToolbarNewPointLightIcon.png");
		case ToolbarIcon::NewDirLight:
			return getGUIIcon(L"ToolbarNewDirectionalLightIcon.png");
		case ToolbarIcon::NewSpotLight:
			return getGUIIcon(L"ToolbarNewSpotLightIcon.png");
		case ToolbarIcon::NewSceneObject:
			return getGUIIcon(L"ToolbarNewSceneObjectIcon.png");
		case ToolbarIcon::NewCube:
			return getGUIIcon(L"ToolbarNewCubeIcon.png");
		case ToolbarIcon::NewSphere:
			return getGUIIcon(L"ToolbarNewSphereIcon.png");
		case ToolbarIcon::NewCone:
			return getGUIIcon(L"ToolbarNewConeIcon.png");
		case ToolbarIcon::NewQuad:
			return getGUIIcon(L"ToolbarNewQuadIcon.png");
		case ToolbarIcon::NewMat:
			return getGUIIcon(L"ToolbarNewMaterialIcon.png");
		case ToolbarIcon::NewCSScript:
			return getGUIIcon(L"ToolbarNewCSScriptIcon.png");
		case ToolbarIcon::NewShader:
			return getGUIIcon(L"ToolbarNewShaderIcon.png");
		case ToolbarIcon::NewSpriteTex:
			return getGUIIcon(L"ToolbarNewSpriteTextureIcon.png");
		case ToolbarIcon::Pause:
			return getGUIIcon(L"ToolbarPauseIcon.png");
		case ToolbarIcon::Play:
			return getGUIIcon(L"ToolbarPlayIcon.png");
		case ToolbarIcon::Step:
			return getGUIIcon(L"ToolbarStepIcon.png");
		case ToolbarIcon::Undo:
			return getGUIIcon(L"ToolbarUndoIcon.png");
		case ToolbarIcon::Redo:
			return getGUIIcon(L"ToolbarRedoIcon.png");
		case ToolbarIcon::OpenProject:
			return getGUIIcon(L"ToolbarOpenProjectIcon.png");
		case ToolbarIcon::SaveProject:
			return getGUIIcon(L"ToolbarSaveProjectIcon.png");
		case ToolbarIcon::SaveScene:
			return getGUIIcon(L"ToolbarSaveSceneIcon.png");
		}

		return HSpriteTexture();
	}

	GUIContentImages BuiltinEditorResources::getSceneWindowIcon(SceneWindowIcon icon) const
	{
		HSpriteTexture off;
		HSpriteTexture on;

		switch (icon)
		{
		case SceneWindowIcon::View:
			off = getGUIIcon(L"SceneViewIcon.png");
			on = getGUIIcon(L"SceneViewIconOn.png");
			break;
		case SceneWindowIcon::Move:
			off = getGUIIcon(L"SceneMoveIcon.png");
			on = getGUIIcon(L"SceneMoveIconOn.png");
			break;
		case SceneWindowIcon::Rotate:
			off = getGUIIcon(L"SceneRotateIcon.png");
			on = getGUIIcon(L"SceneRotateIconOn.png");
			break;
		case SceneWindowIcon::Scale:
			off = getGUIIcon(L"SceneScaleIcon.png");
			on = getGUIIcon(L"SceneScaleIconOn.png");
			break;
		case SceneWindowIcon::Pivot:
			off = getGUIIcon(L"ScenePivotIcon.png");
			on = getGUIIcon(L"ScenePivotIconOn.png");
			break;
		case SceneWindowIcon::Center:
			off = getGUIIcon(L"SceneCenterIcon.png");
			on = getGUIIcon(L"SceneCenterIconOn.png");
			break;
		case SceneWindowIcon::Local:
			off = getGUIIcon(L"SceneLocalIcon.png");
			on = getGUIIcon(L"SceneLocalIconOn.png");
			break;
		case SceneWindowIcon::World:
			off = getGUIIcon(L"SceneWorldIcon.png");
			on = getGUIIcon(L"SceneWorldIconOn.png");
			break;
		case SceneWindowIcon::MoveSnap:
			off = getGUIIcon(L"SceneMoveSnapIcon.png");
			on = getGUIIcon(L"SceneMoveSnapIconOn.png");
			break;
		case SceneWindowIcon::RotateSnap:
			off = getGUIIcon(L"SceneRotateSnapIcon.png");
			on = getGUIIcon(L"SceneRotateSnapIconOn.png");
			break;
		}

		GUIContentImages output;
		output.normal = off;
		output.hover = off;
		output.active = on;
		output.focused = off;
		output.normalOn = on;
		output.hoverOn = on;
		output.activeOn = on;
		output.focusedOn = on;

		return output;
	}

	HSpriteTexture BuiltinEditorResources::getLibraryWindowIcon(LibraryWindowIcon icon) const
	{
		switch (icon)
		{
		case LibraryWindowIcon::Home:
			return getGUIIcon(L"LibraryHomeIcon.png");
		case LibraryWindowIcon::Up:
			return getGUIIcon(L"LibraryUpIcon.png");
		case LibraryWindowIcon::Clear:
			return getGUIIcon(L"LibraryClearSearchIcon.png");
		case LibraryWindowIcon::Options:
			return getGUIIcon(L"LibraryOptionsIcon.png");
		}

		return HSpriteTexture();
	}

	HSpriteTexture BuiltinEditorResources::getInspectorWindowIcon(InspectorWindowIcon icon) const
	{
		switch (icon)
		{
		case InspectorWindowIcon::Create:
			return getGUIIcon(L"InspectorCreateIcon.png");
		case InspectorWindowIcon::Clone:
			return getGUIIcon(L"InspectorCloneIcon.png");
		case InspectorWindowIcon::Clear:
			return getGUIIcon(L"InspectorClearIcon.png");
		case InspectorWindowIcon::Resize:
			return getGUIIcon(L"InspectorResizeIcon.png");
		case InspectorWindowIcon::Delete:
			return getGUIIcon(L"InspectorDeleteIcon.png");
		case InspectorWindowIcon::MoveUp:
			return getGUIIcon(L"InspectorMoveUpIcon.png");
		case InspectorWindowIcon::MoveDown:
			return getGUIIcon(L"InspectorMoveDownIcon.png");
		case InspectorWindowIcon::Edit:
			return getGUIIcon(L"InspectorEditIcon.png");
		case InspectorWindowIcon::Apply:
			return getGUIIcon(L"InspectorApplyIcon.png");
		case InspectorWindowIcon::Add:
			return getGUIIcon(L"InspectorAddIcon.png");
		case InspectorWindowIcon::Cancel:
			return getGUIIcon(L"InspectorCancelIcon.png");
		}

		return HSpriteTexture();
	}

	HSpriteTexture BuiltinEditorResources::getIcon(EditorIcon icon) const
	{
		switch (icon)
		{
		case EditorIcon::XBtn:
			return getGUIIcon(XButtonNormalTex);
		}

		return HSpriteTexture();
	}

	HSpriteTexture BuiltinEditorResources::getLogMessageIcon(LogMessageIcon icon, UINT32 size, bool dark) const
	{
		if (size < 24) // Round to 16
		{
			if (dark)
			{
				switch (icon)
				{
				case LogMessageIcon::Info:
					return getGUIIcon(L"IconInfoDark.png");
				case LogMessageIcon::Warning:
					return getGUIIcon(L"IconWarningDark.png");
				case LogMessageIcon::Error:
					return getGUIIcon(L"IconErrorDark.png");
				}
			}
			else
			{
				switch (icon)
				{
				case LogMessageIcon::Info:
					return getGUIIcon(L"IconInfo.png");
				case LogMessageIcon::Warning:
					return getGUIIcon(L"IconWarning.png");
				case LogMessageIcon::Error:
					return getGUIIcon(L"IconError.png");
				}
			}
		}
		else // Round to 32
		{
			switch (icon)
			{
			case LogMessageIcon::Info:
				return getGUIIcon(L"IconInfo32.png");
			case LogMessageIcon::Warning:
				return getGUIIcon(L"IconWarning32.png");
			case LogMessageIcon::Error:
				return getGUIIcon(L"IconError32.png");
			}
		}

		return HSpriteTexture();
	}

	WString BuiltinEditorResources::getEmptyShaderCode() const
	{
		Path filePath = FileSystem::getWorkingDirectoryPath();
		filePath.append(BuiltinDataFolder);
		filePath.append(EmptyShaderCodeFile);

		DataStreamPtr fileStream = FileSystem::openFile(filePath);
		if (fileStream != nullptr)
			return fileStream->getAsWString();

		return StringUtil::WBLANK;
	}

	WString BuiltinEditorResources::getEmptyCSScriptCode() const
	{
		Path filePath = FileSystem::getWorkingDirectoryPath();
		filePath.append(BuiltinDataFolder);
		filePath.append(EmptyCSScriptCodeFile);

		DataStreamPtr fileStream = FileSystem::openFile(filePath);
		if (fileStream != nullptr)
			return fileStream->getAsWString();

		return StringUtil::WBLANK;
	}

	Path BuiltinEditorResources::getShaderIncludeFolder()
	{
		return Paths::getRuntimeDataPath() + EDITOR_DATA_FOLDER + ShaderIncludeFolder;
	}

	Path BuiltinEditorResources::getDefaultWidgetLayoutPath()
	{
		return Paths::getRuntimeDataPath() + EDITOR_DATA_FOLDER + "Layout.asset";
	}
}