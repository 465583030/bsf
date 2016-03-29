//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"

/** @addtogroup Layers
 *  @{
 */

/** @defgroup Editor 4. Editor
 *	Functionality specific to the Banshee Editor.
 *  @{
 */

/** @defgroup EditorWindow EditorWindow
 *  Creating, managing and interacting with editor windows.
 */

/** @defgroup Utility-Editor Utility
 *  Various utility methods and types used by the editor layer.
 */

/** @defgroup GUI-Editor GUI
  *	Editor specific GUI elements.
  */

/** @defgroup Build Build
  *	Building (publishing) the game from within the editor.
  */

/** @defgroup CodeEditor CodeEditor
  *	Integration of the Banshee Editor with external code editors.
  */

/** @defgroup Handles Handles
  *	Creating and interacting with handles (scene tools).
  */

/** @defgroup Library Library
  *	Management of all resources (assets) within a project.
  */

/** @cond RTTI */
/** @defgroup RTTI-Impl-Editor RTTI types
 *  Types containing RTTI for specific classes.
 */
/** @endcond */

/** @defgroup Scene-Editor Scene
  *	Rendering/interacting with the scene view in editor.
  */

/** @defgroup Settings Settings
  *	Persistent settings for editor and project.
  */

/** @defgroup Testing-Editor Testing
 *  Contains editor unit tests.
 */

/** @defgroup UndoRedo UndoRedo
 *  Undoing and redoing actions within the editor.
 */

/** @defgroup Application-Editor Application
 *  Entry point into the editor application.
 */

/** @defgroup Internal-Editor [INTERNAL]
 *	Low-level classes and methods not meant for normal use, useful for those that are modifying the engine.
 *  @{
 */

/** @defgroup CodeEditor-Internal CodeEditor
  *	Integration of the Banshee Editor with external code editors.
  */

/** @defgroup EditorWindow-Internal EditorWindow
 *  Creating, managing and interacting with editor windows.
 */

/** @defgroup GUI-Editor-Internal GUI
  *	Editor specific GUI elements.
  */

/** @defgroup Handles-Internal Handles
  *	Creating and interacting with handles (scene tools).
  */

/** @defgroup Library-Internal Library
  *	Management of all resources (assets) within a project.
  */

/** @defgroup Scene-Editor-Internal Scene
  *	Rendering/interacting with the scene view in editor.
  */

/** @} */

/** @} */
/** @} */

#if (BS_PLATFORM == BS_PLATFORM_WIN32) && !defined(__MINGW32__)
#	ifdef BS_ED_EXPORTS
#		define BS_ED_EXPORT __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define BS_ED_EXPORT
#       else
#    		define BS_ED_EXPORT __declspec(dllimport)
#       endif
#	endif
#elif defined ( BS_GCC_VISIBILITY )
#    define BS_ED_EXPORT  __attribute__ ((visibility("default")))
#else
#    define BS_ED_EXPORT
#endif

namespace BansheeEngine
{
	class EditorWindowBase;
	class EditorWindow;
	class EditorWidgetBase;
	class EditorWidgetContainer;
	class EditorWidgetManager;
	class EditorWidgetLayout;
	class ModalWindow;
	class GUITabbedTitleBar;
	class GUITabButton;
	class GUIWindowFrame;
	class GUIIntField;
	class GUIFloatField;
	class GUITextField;
	class GUIToggleField;
	class GUIVector2Field;
	class GUIVector3Field;
	class GUIVector4Field;
	class GUIColorField;
	class GUIListBoxField;
	class GUISliderField;
	class GUIColor;
	class GUIStatusBar;
	class GUIDropButton;
	class GUIHoverHitBox;
	class EditorWindowManager;
	class DockManager;
	class DockManagerLayout;
	class MainEditorWindow;
	class WindowFrameWidget;
	class GUIMenuBar;
	class GUIDockSlider;
	class GUISceneTreeView;
	class GUIResourceTreeView;
	class GUITreeViewEditBox;
	class EditorCommand;
	class ProjectFileMeta;
	class ProjectResourceMeta;
	class SceneGrid;
	class HandleSlider;
	class HandleSliderLine;
	class HandleSliderPlane;
	class HandleSliderDisc;
	class HandleSliderManager;
	class HandleDrawManager;
	class SceneCameraController;
	class EditorSettings;
	class SceneViewHandler;
	class SelectionRenderer;
	class DropDownWindow;
	class ProjectSettings;

	typedef std::shared_ptr<ProjectFileMeta> ProjectFileMetaPtr;
	typedef std::shared_ptr<ProjectResourceMeta> ProjectResourceMetaPtr;
	typedef std::shared_ptr<DockManagerLayout> DockManagerLayoutPtr;
	typedef std::shared_ptr<EditorWidgetLayout> EditorWidgetLayoutPtr;
	typedef std::shared_ptr<EditorSettings> EditorSettingsPtr;
	typedef std::shared_ptr<ProjectSettings> ProjectSettingsPtr;

	static const char* EDITOR_ASSEMBLY = "MBansheeEditor";
	static const char* SCRIPT_EDITOR_ASSEMBLY = "MScriptEditor";
	static const Path PROJECT_INTERNAL_DIR = L"Internal\\";
	static const Path INTERNAL_ASSEMBLY_PATH = PROJECT_INTERNAL_DIR + "Assemblies\\";

	/** Types of drag and drop operations. Different types specify different types of dragged data. */
	enum class DragAndDropType
	{
		EditorWidget = 10000,
		SceneObject = 10001,
		Resources = 10002
	};

	/**
	 * Recognized types of external code editors.
	 *
	 * @see		CodeEditorManager
	 */
	enum class CodeEditorType
	{
		VS2008,
		VS2010,
		VS2012,
		VS2013,
		VS2015,
		None
	};

	/**	Type IDs used by the RTTI system for the editor library. */
	enum TypeID_BansheeEditor
	{
		TID_ProjectFileMeta = 40000,
		TID_ProjectLibraryEntries = 40001,
		TID_ProjectLibraryResEntry = 40002,
		TID_ProjectLibraryDirEntry = 40003,
		TID_DockManagerLayout = 40004,
		TID_DockManagerLayoutEntry = 40005,
		TID_EditorWidgetLayout = 40006,
		TID_EditorWidgetLayoutEntry = 40007,
		TID_TestComponentA = 40008,
		TID_TestComponentB = 40009,
		TID_PlatformInfo = 40010,
		TID_WinPlatformInfo = 40011,
		TID_BuildData = 40012,
		TID_TestObjectA = 40013,
		TID_TestObjectB = 40014,
		TID_TestComponentC = 40015,
		TID_TestComponentD = 40016,
		TID_EditorSettings = 40017,
		TID_RecentProject = 40018,
		TID_Settings = 40019,
		TID_ProjectSettings = 40020,
		TID_WindowFrameWidget = 40021,
		TID_ProjectResourceMeta = 40022
	};
}
