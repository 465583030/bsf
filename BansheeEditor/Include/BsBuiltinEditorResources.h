#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUISkin.h"
#include "BsModule.h"
#include "BsApplication.h"

namespace BansheeEngine
{
	/**
	 * @brief	Types of valid icons used when viewing the project library
	 */
	enum class ProjectIcon
	{
		Folder, Mesh, Font, Texture, PlainText, ScriptCode, SpriteTexture, Shader, ShaderInclude, Material, Prefab
	};

	class BS_ED_EXPORT BuiltinEditorResources : public BansheeEngine::Module<BuiltinEditorResources>
	{
	public:
		BuiltinEditorResources();
		~BuiltinEditorResources();

		const HGUISkin& getSkin() const { return mSkin; }

		/**
		 * @brief	Creates a material used for docking drop overlay used by the editor.
		 */
		HMaterial createDockDropOverlayMaterial() const;

		/**
		 * @brief	Creates a material used for rendering the scene grid.
		 */
		HMaterial createSceneGridMaterial() const;

		/**
		 * @brief	Creates a material used for picking non-transparent objects in scene view.
		 */
		HMaterial createPicking(CullingMode cullMode) const;

		/**
		 * @brief	Creates a material used for picking transparent objects in scene view.
		 */
		HMaterial createPickingAlpha(CullingMode cullMode) const;

		/**
		 * @brief	Creates a material used for rendering wireframe gizmos.
		 */
		HMaterial createWireGizmoMat() const;

		/**
		 * @brief	Creates a material used for rendering solid gizmos.
		 */
		HMaterial createSolidGizmoMat() const;

		/**
		 * @brief	Creates a material used for rendering icon gizmos.
		 */
		HMaterial createIconGizmoMat() const;

		/**
		 * @brief	Creates a material used for picking non-transparent gizmos.
		 */
		HMaterial createGizmoPickingMat() const;

		/**
		 * @brief	Creates a material used for picking transparent gizmos.
		 */
		HMaterial createAlphaGizmoPickingMat() const;

		/**
		 * @brief	Creates a material used for rendering wireframe handles.
		 */
		HMaterial createWireHandleMat() const;

		/**
		 * @brief	Creates a material used for rendering solid handles.
		 */
		HMaterial createSolidHandleMat() const;

		/**
		 * @brief	Creates a material used for displaying selected objects.
		 */
		HMaterial createSelectionMat() const;

		/**
		 * @brief	Retrieves an icon that represents a specific resource type
		 *			that may be displayed when viewing the project library.
		 */
		HSpriteTexture getLibraryIcon(ProjectIcon icon) const;

		static const String ObjectFieldStyleName;
		static const String ObjectFieldLabelStyleName;
		static const String ObjectFieldDropBtnStyleName;
		static const String ObjectFieldClearBtnStyleName;

		static const Path BuiltinDataFolder;
		static const Path EditorSkinFolder;
		static const Path EditorIconFolder;
		static const Path EditorShaderFolder;
		static const Path EditorShaderIncludeFolder;

	private:
		/**
		 * @brief	Imports all necessary resources and converts them to engine-ready format.
		 *
		 * @note	Normally you only want to use this during development phase and then ship
		 *			with engine-ready format only.
		 */
		void preprocess();

		/**
		 * @brief	Generates the default editor skin and all GUI element styles.
		 */
		HGUISkin generateGUISkin();

		/**
		 * @brief	Loads a GUI skin texture with the specified filename.
		 */
		static HSpriteTexture getGUITexture(const WString& name);

		/**
		 * @brief	Loads a GUI icon with the specified filename.
		 */
		static HSpriteTexture getGUIIcon(const WString& name);

		/**
		 * @brief	Loads a shader with the specified filename
		 */
		static HShader getShader(const WString& name);

		HShader mShaderDockOverlay;
		HShader mShaderSceneGrid;
		HShader mShaderPicking[3];
		HShader mShaderPickingAlpha[3];
		HShader mShaderGizmoSolid;
		HShader mShaderGizmoWire;
		HShader mShaderGizmoIcon;
		HShader mShaderGizmoPicking;
		HShader mShaderGizmoAlphaPicking;
		HShader mShaderHandleSolid;
		HShader mShaderHandleWire;
		HShader mShaderSelection;

		HGUISkin mSkin;

		ResourceManifestPtr mResourceManifest;

		static const Path ShaderFolder;
		static const Path SkinFolder;
		static const Path IconFolder;
		static const Path ShaderIncludeFolder;

		static const Path BuiltinRawDataFolder;
		static const Path EditorRawSkinFolder;
		static const Path EditorRawShaderIncludeFolder;
		static const Path EditorRawShaderFolder;

		static const Path ResourceManifestPath;

		static const WString DefaultFontFilename;
		static const UINT32 DefaultFontSize;

		static const WString GUISkinFile;

		static const WString FolderIconTex;
		static const WString MeshIconTex;
		static const WString TextureIconTex;
		static const WString FontIconTex;
		static const WString PlainTextIconTex;
		static const WString ScriptCodeIconTex;
		static const WString ShaderIconTex;
		static const WString ShaderIncludeIconTex;
		static const WString MaterialIconTex;
		static const WString SpriteTextureIconTex;
		static const WString PrefabIconTex;

		static const WString WindowBackgroundTexture;

		static const WString WindowFrameNormal;
		static const WString WindowFrameFocused;

		static const WString WindowTitleBarBg;

		static const WString WindowCloseButtonNormal;
		static const WString WindowCloseButtonHover;

		static const WString WindowMinButtonNormal;
		static const WString WindowMinButtonHover;

		static const WString WindowMaxButtonNormal;
		static const WString WindowMaxButtonHover;

		static const WString TabbedBarBtnNormal;
		static const WString TabbedBarBtnActive;

		static const WString ButtonNormalTex;
		static const WString ButtonHoverTex;
		static const WString ButtonActiveTex;

		static const WString ToggleNormalTex;
		static const WString ToggleHoverTex;
		static const WString ToggleActiveTex;
		static const WString ToggleNormalOnTex;
		static const WString ToggleHoverOnTex;
		static const WString ToggleActiveOnTex;

		static const WString ObjectDropBtnNormalTex;
		static const WString ObjectDropBtnNormalOnTex;
		static const WString ObjectClearBtnNormalTex;
		static const WString ObjectClearBtnHoverTex;
		static const WString ObjectClearBtnActiveTex;

		static const WString FoldoutOpenNormalTex;
		static const WString FoldoutOpenHoverTex;
		static const WString FoldoutClosedNormalTex;
		static const WString FoldoutClosedHoverTex;

		static const WString CmpFoldoutOpenNormalTex;
		static const WString CmpFoldoutOpenHoverTex;
		static const WString CmpFoldoutOpenActiveTex;
		static const WString CmpFoldoutClosedNormalTex;
		static const WString CmpFoldoutClosedHoverTex;
		static const WString CmpFoldoutClosedActiveTex;

		static const WString InputBoxNormalTex;
		static const WString InputBoxHoverTex;
		static const WString InputBoxFocusedTex;

		static const WString ScrollBarUpNormalTex;
		static const WString ScrollBarUpHoverTex;
		static const WString ScrollBarUpActiveTex;

		static const WString ScrollBarDownNormalTex;
		static const WString ScrollBarDownHoverTex;
		static const WString ScrollBarDownActiveTex;

		static const WString ScrollBarLeftNormalTex;
		static const WString ScrollBarLeftHoverTex;
		static const WString ScrollBarLeftActiveTex;

		static const WString ScrollBarRightNormalTex;
		static const WString ScrollBarRightHoverTex;
		static const WString ScrollBarRightActiveTex;

		static const WString ScrollBarHandleHorzNormalTex;
		static const WString ScrollBarHandleHorzHoverTex;
		static const WString ScrollBarHandleHorzActiveTex;

		static const WString ScrollBarHandleVertNormalTex;
		static const WString ScrollBarHandleVertHoverTex;
		static const WString ScrollBarHandleVertActiveTex;

		static const WString ScrollBarBgTex;

		static const WString DropDownBtnNormalTex;
		static const WString DropDownBtnHoverTex;

		static const WString DropDownBoxBgTex;
		static const WString DropDownBoxEntryNormalTex;
		static const WString DropDownBoxEntryHoverTex;

		static const WString DropDownBoxBtnUpNormalTex;
		static const WString DropDownBoxBtnUpHoverTex;

		static const WString DropDownBoxBtnDownNormalTex;
		static const WString DropDownBoxBtnDownHoverTex;

		static const WString DropDownBoxEntryExpNormalTex;
		static const WString DropDownBoxEntryExpHoverTex;

		static const WString DropDownSeparatorTex;

		static const WString DropDownBoxBtnUpArrowTex;
		static const WString DropDownBoxBtnDownArrowTex;

		static const WString MenuBarBgTex;

		static const WString MenuBarBtnNormalTex;
		static const WString MenuBarBtnHoverTex;

		static const WString MenuBarBansheeLogoTex;

		static const WString DockSliderNormalTex;

		static const WString TreeViewExpandButtonOffNormal;
		static const WString TreeViewExpandButtonOffHover;
		static const WString TreeViewExpandButtonOnNormal;
		static const WString TreeViewExpandButtonOnHover;

		static const WString TreeViewSelectionBackground;
		static const WString TreeViewEditBox;

		static const WString TreeViewElementHighlight;
		static const WString TreeViewElementSepHighlight;

		static const WString ColorPickerSliderHorzHandleTex;
		static const WString ColorPickerSliderVertHandleTex;
		static const WString ColorPickerSlider2DHandleTex;

		static const WString ProgressBarFillTex;
		static const WString ProgressBarBgTex;

		static const WString SelectionAreaTex;

		static const WString ShaderDockOverlayFile;
		static const WString ShaderSceneGridFile;
		static const WString ShaderPickingCullNoneFile;
		static const WString ShaderPickingCullCWFile;
		static const WString ShaderPickingCullCCWFile;
		static const WString ShaderPickingAlphaCullNoneFile;
		static const WString ShaderPickingAlphaCullCWFile;
		static const WString ShaderPickingAlphaCullCCWFile;
		static const WString ShaderWireGizmoFile;
		static const WString ShaderSolidGizmoFile;
		static const WString ShaderWireHandleFile;
		static const WString ShaderSolidHandleFile;
		static const WString ShaderIconGizmoFile;
		static const WString ShaderGizmoPickingFile;
		static const WString ShaderGizmoPickingAlphaFile;
		static const WString ShaderSelectionFile;
	};
}