#include "BsEditorApplication.h"
#include "BsEditorWindowManager.h"
#include "BsEditorWidgetManager.h"
#include "BsMainEditorWindow.h"
#include "BsRenderWindow.h"
#include "BsBuiltinEditorResources.h"
#include "BsUndoRedo.h"
#include "BsFileSerializer.h"
#include "BsFileSystem.h"
#include "BsPath.h"
#include "BsResourceImporter.h"
#include "BsEditorWidgetLayout.h"
#include "BsSceneEditorWidget.h"

// DEBUG ONLY
#include "DbgEditorWidget1.h"
#include "DbgEditorWidget2.h"
#include "BsResources.h"
#include "BsSceneObject.h"
#include "BsImporter.h"
#include "BsGpuProgram.h"
#include "BsGpuProgramImportOptions.h"
#include "BsShader.h"
#include "BsTexture.h"
#include "BsMaterial.h"
#include "BsTechnique.h"
#include "BsPass.h"
#include "BsRenderable.h"
#include "BsDbgTestGameObjectRef.h"
#include "BsVirtualInput.h"
#include "BsFolderMonitor.h"
#include "BsProjectLibrary.h"
#include "BsCamera.h"
#include "BsGUIWidget.h"
#include "BsGUIArea.h"
#include "BsGUIButton.h"
#include "BsGUILayout.h"
#include "BsEvent.h"
#include "BsRenderer.h"

namespace BansheeEngine
{
	const Path EditorApplication::WIDGET_LAYOUT_PATH = L"Internal\\Layout.asset";

	RENDER_WINDOW_DESC createRenderWindowDesc()
	{
		RENDER_WINDOW_DESC renderWindowDesc;
		renderWindowDesc.videoMode = VideoMode(1280, 720);
		renderWindowDesc.title = "BansheeEditor";
		renderWindowDesc.fullscreen = false;
		renderWindowDesc.border = WindowBorder::None;

		return renderWindowDesc;
	}

	EditorApplication::EditorApplication(RenderSystemPlugin renderSystemPlugin)
		:Application(createRenderWindowDesc(), renderSystemPlugin, RendererPlugin::Default), 
		mActiveRSPlugin(renderSystemPlugin)
	{
		BuiltinEditorResources::startUp(renderSystemPlugin);

		{
			auto inputConfig = VirtualInput::instance().getConfiguration();

			inputConfig->registerButton("Rename", BC_F2);
			inputConfig->registerButton("Undo", BC_Z, VButtonModifier::Ctrl);
			inputConfig->registerButton("Redo", BC_Y, VButtonModifier::Ctrl);
			inputConfig->registerButton("Copy", BC_C, VButtonModifier::Ctrl);
			inputConfig->registerButton("Cut", BC_X, VButtonModifier::Ctrl);
			inputConfig->registerButton("Paste", BC_V, VButtonModifier::Ctrl);
			inputConfig->registerButton("Delete", BC_DELETE);
		}

		ResourceImporter* resourceImporter = bs_new<ResourceImporter>();
		Importer::instance()._registerAssetImporter(resourceImporter);

		ProjectLibrary::startUp(getActiveProjectPath());

		UndoRedo::startUp();
		EditorWindowManager::startUp();
		EditorWidgetManager::startUp();
	}

	EditorApplication::~EditorApplication()
	{
		saveWidgetLayout(EditorWidgetManager::instance().getLayout());

		EditorWidgetManager::shutDown();
		EditorWindowManager::shutDown();
		UndoRedo::shutDown();


		/************************************************************************/
		/* 								DEBUG CODE                      		*/
		/************************************************************************/

		gResources().unload(mTestTexRef);
		gResources().unload(mDbgMeshRef);
		gResources().unload(mFragProgRef);
		gResources().unload(mVertProgRef);
		gResources().unload(mTestMaterial);

		mTestMaterial = nullptr;
		mTestTexRef = nullptr;
		mDbgMeshRef = nullptr;
		mFragProgRef = nullptr;
		mVertProgRef = nullptr;

		mNewPassGL = nullptr;
		mNewTechniqueGL = nullptr;

		mNewPassDX = nullptr;
		mNewTechniqueDX = nullptr;

		mNewPassDX11 = nullptr;
		mNewTechniqueDX11 = nullptr;

		mTestShader = nullptr;

		/************************************************************************/
		/* 							END DEBUG CODE                      		*/
		/************************************************************************/

		ProjectLibrary::shutDown();
		BuiltinEditorResources::shutDown();
	}

	void EditorApplication::onStartUp()
	{
		Application::onStartUp();

		MainEditorWindow* mainWindow = MainEditorWindow::create(getPrimaryWindow());
		loadPlugin("SBansheeEditor"); // Managed part of the editor

		EditorWidgetLayoutPtr layout = loadWidgetLayout();
		if (layout != nullptr)
			EditorWidgetManager::instance().setLayout(layout);

		SceneEditorWidget::open();

		/************************************************************************/
		/* 								DEBUG CODE                      		*/
		/************************************************************************/

		RenderSystem* renderSystem = RenderSystem::instancePtr();

		HSceneObject testModelGO = SceneObject::create("TestMesh");
		HRenderable testRenderable = testModelGO->addComponent<Renderable>();

		WString psLoc;
		WString vsLoc;

		GpuProgramProfile psProfile;
		GpuProgramProfile vsProfile;

		String psEntry;
		String vsEntry;

		String language;

		switch (mActiveRSPlugin)
		{
		case RenderSystemPlugin::DX11:
		{
										 psLoc = L"..\\..\\..\\..\\Data\\hlsl11_ps.gpuprog";
										 vsLoc = L"..\\..\\..\\..\\Data\\hlsl11_vs.gpuprog";
										 language = "hlsl";
										 psProfile = GPP_PS_4_0;
										 vsProfile = GPP_VS_4_0;
										 psEntry = "ps_main";
										 vsEntry = "vs_main";
										 break;
		}
		case RenderSystemPlugin::DX9:
		{
										psLoc = L"..\\..\\..\\..\\Data\\hlsl9_ps.gpuprog";
										vsLoc = L"..\\..\\..\\..\\Data\\hlsl9_vs.gpuprog";
										language = "hlsl";
										psProfile = GPP_PS_2_0;
										vsProfile = GPP_VS_2_0;
										psEntry = "ps_main";
										vsEntry = "vs_main";
										break;
		}
		case RenderSystemPlugin::OpenGL:
		{
										   psLoc = L"..\\..\\..\\..\\Data\\glsl_ps.gpuprog";
										   vsLoc = L"..\\..\\..\\..\\Data\\glsl_vs.gpuprog";
										   language = "glsl";
										   psProfile = GPP_PS_2_0;
										   vsProfile = GPP_VS_2_0;
										   psEntry = "main";
										   vsEntry = "main";
										   break;
		}
		}

		ImportOptionsPtr gpuProgImportOptions = Importer::instance().createImportOptions(psLoc);
		if (rtti_is_of_type<GpuProgramImportOptions>(gpuProgImportOptions))
		{
			GpuProgramImportOptions* importOptions = static_cast<GpuProgramImportOptions*>(gpuProgImportOptions.get());

			importOptions->setEntryPoint(psEntry);
			importOptions->setLanguage(language);
			importOptions->setProfile(psProfile);
			importOptions->setType(GPT_FRAGMENT_PROGRAM);
		}

		mFragProgRef = Importer::instance().import(psLoc, gpuProgImportOptions);

		gpuProgImportOptions = Importer::instance().createImportOptions(vsLoc);
		if (rtti_is_of_type<GpuProgramImportOptions>(gpuProgImportOptions))
		{
			GpuProgramImportOptions* importOptions = static_cast<GpuProgramImportOptions*>(gpuProgImportOptions.get());

			importOptions->setEntryPoint(vsEntry);
			importOptions->setLanguage(language);
			importOptions->setProfile(vsProfile);
			importOptions->setType(GPT_VERTEX_PROGRAM);
		}

		mVertProgRef = Importer::instance().import(vsLoc, gpuProgImportOptions);

		gResources().save(mVertProgRef, L"C:\\vertProgCg.vprog", true);
		gResources().unload(mVertProgRef);
		mVertProgRef = gResources().load(L"C:\\vertProgCg.vprog");

		gResources().save(mFragProgRef, L"C:\\fragProgCg.vprog", true);
		gResources().unload(mFragProgRef);
		mFragProgRef = gResources().load(L"C:\\fragProgCg.vprog");

		mTestShader = Shader::create("TestShader");
		mTestShader->addParameter("matWorldViewProj", "matWorldViewProj", GPDT_MATRIX_4X4, RPS_WorldViewProjTfrm);

		mTestShader->addParameter("samp", "samp", GPOT_SAMPLER2D);
		mTestShader->addParameter("tex", "tex", GPOT_TEXTURE2D);

		mTestShader->setParamBlockAttribs("PerObject", true, GPBU_DYNAMIC, RBS_PerObject);

		mNewTechniqueGL = mTestShader->addTechnique("GLRenderSystem", "BansheeRenderer");
		mNewPassGL = mNewTechniqueGL->addPass();
		mNewPassGL->setVertexProgram(mVertProgRef);
		mNewPassGL->setFragmentProgram(mFragProgRef);

		// TODO - I need to create different techniques for different render systems (and renderers, if there were any),
		// which is redundant as some techniques can be reused. I should add a functionality that supports multiple
		// render systems/renderers per technique
		mNewTechniqueDX = mTestShader->addTechnique("D3D9RenderSystem", "BansheeRenderer");
		mNewPassDX = mNewTechniqueDX->addPass();
		mNewPassDX->setVertexProgram(mVertProgRef);
		mNewPassDX->setFragmentProgram(mFragProgRef);

		mNewTechniqueDX11 = mTestShader->addTechnique("D3D11RenderSystem", "BansheeRenderer");
		mNewPassDX11 = mNewTechniqueDX11->addPass();
		mNewPassDX11->setVertexProgram(mVertProgRef);
		mNewPassDX11->setFragmentProgram(mFragProgRef);

		mTestMaterial = Material::create();
		mTestMaterial->setShader(mTestShader);

		mTestTexRef = static_resource_cast<Texture>(Importer::instance().import(L"..\\..\\..\\..\\Data\\Examples\\Dragon.tga"));
		mDbgMeshRef = static_resource_cast<Mesh>(Importer::instance().import(L"..\\..\\..\\..\\Data\\Examples\\Dragon.fbx"));

		gResources().save(mTestTexRef, L"C:\\ExportTest.tex", true);
		gResources().save(mDbgMeshRef, L"C:\\ExportMesh.mesh", true);

		gResources().unload(mTestTexRef);
		gResources().unload(mDbgMeshRef);

		mTestTexRef = static_resource_cast<Texture>(gResources().loadAsync(L"C:\\ExportTest.tex"));
		mDbgMeshRef = static_resource_cast<Mesh>(gResources().loadAsync(L"C:\\ExportMesh.mesh"));

		mDbgMeshRef.synchronize();
		mTestTexRef.synchronize();

		mTestMaterial->setTexture("tex", mTestTexRef);
		gResources().save(mTestMaterial, L"C:\\ExportMaterial.mat", true);

		gResources().unload(mTestMaterial);

		mTestMaterial = gResources().load(L"C:\\ExportMaterial.mat");

		testRenderable->setMesh(mDbgMeshRef);
		testRenderable->setMaterial(0, mTestMaterial);

		GameObjectHandle<DbgTestGameObjectRef> dbgTestGameObjectRef = testModelGO->addComponent<DbgTestGameObjectRef>();
		dbgTestGameObjectRef->mRenderable = testRenderable;

		HSceneObject clone = testModelGO->clone();
		GameObjectHandle<DbgTestGameObjectRef> clonedDbgTestGameObjectRef = clone->getComponent<DbgTestGameObjectRef>();

		testModelGO->destroy();

		//Win32FolderMonitor* folderMonitor = bs_new<Win32FolderMonitor>();

		//FolderChange folderChanges = (FolderChange)((UINT32)FolderChange::FileName | (UINT32)FolderChange::DirName | 
		//	(UINT32)FolderChange::Creation | (UINT32)FolderChange::LastWrite);
		//folderMonitor->startMonitor(L"D:\\TestDetect", true, folderChanges);

		//HTexture dbgCursor = static_resource_cast<Texture>(Importer::instance().import(L"C:\\CursorDbg.psd"));
		//PixelDataPtr cursorPixelData = dbgCursor->allocateSubresourceBuffer(0);

		//gMainSyncedCA().readSubresource(dbgCursor.getInternalPtr(), 0, cursorPixelData);
		//gMainSyncedCA().submitToCoreThread(true);

		/************************************************************************/
		/* 								MODAL WINDOW                      		*/
		/************************************************************************/

		//RENDER_WINDOW_DESC modalWindowDesc;
		//modalWindowDesc.width = 200;
		//modalWindowDesc.height = 200;
		//modalWindowDesc.left = 0;
		//modalWindowDesc.top = 0;
		//modalWindowDesc.title = "ModalWindow";
		//modalWindowDesc.fullscreen = false;
		//modalWindowDesc.border = WindowBorder::None;
		//modalWindowDesc.toolWindow = true;
		//modalWindowDesc.modal = true;

		//RenderWindowPtr modalWindow = RenderWindow::create(modalWindowDesc, gApplication().getPrimaryWindow());

		//HSceneObject modalSceneObject = SceneObject::create("ModalWindow");

		//HCamera modalCamera = modalSceneObject->addComponent<Camera>();
		//modalCamera->initialize(modalWindow, 0.0f, 0.0f, 1.0f, 1.0f);
		//modalCamera->setNearClipDistance(5);
		//modalCamera->setAspectRatio(1.0f);
		//modalCamera->setIgnoreSceneRenderables(true);

		//HGUIWidget modalGUI = modalSceneObject->addComponent<GUIWidget>(modalCamera->getViewport().get());
		//modalGUI->setDepth(128);

		//modalGUI->setSkin(EditorGUI::instance().getSkin());

		//GUIArea* modalArea = GUIArea::createStretchedXY(*modalGUI, 0, 0, 0, 0, 500);
		//GUIButton* modalButton = GUIButton::create(*modalGUI, HString(L"Close"));
		//modalButton->onClick.connect(std::bind(&EditorApplication::closeModalWindow, modalWindow, modalSceneObject));

		//modalArea->getLayout().addElement(modalButton);

		/************************************************************************/
		/* 							END DEBUG CODE                      		*/
		/************************************************************************/

		DbgEditorWidget1::open(); // DEBUG ONLY
		DbgEditorWidget2::open(); // DEBUG ONLY
	}

	void EditorApplication::startUp(RenderSystemPlugin renderSystemPlugin)
	{
		CoreApplication::startUp<EditorApplication>(renderSystemPlugin);
	}

	void EditorApplication::closeModalWindow(RenderWindowPtr window, HSceneObject sceneObject)
	{
		//sceneObject->destroy();
		window->destroy();
	}

	void EditorApplication::update()
	{
		Application::update();

		ProjectLibrary::instance().update();
		EditorWindowManager::instance().update();	
	}

	bool EditorApplication::isProjectLoaded() const
	{
		return true; // TODO - DEBUG ONLY
	}

	bool EditorApplication::isGameViewFocused() const
	{
		return false; // TODO
	}

	bool EditorApplication::isSceneViewFocused() const
	{
		return true; // TODO
	}

	const Path& EditorApplication::getActiveProjectPath() const
	{
		static Path dummyProjectPath = L"D:\\DummyBansheeProject\\";

		return dummyProjectPath;
	}

	EditorWidgetLayoutPtr EditorApplication::loadWidgetLayout()
	{
		Path layoutPath = getActiveProjectPath();
		layoutPath.append(WIDGET_LAYOUT_PATH);

		if(FileSystem::exists(layoutPath))
		{
			FileSerializer fs;
			return std::static_pointer_cast<EditorWidgetLayout>(fs.decode(layoutPath));
		}

		return nullptr;
	}

	void EditorApplication::saveWidgetLayout(const EditorWidgetLayoutPtr& layout)
	{
		Path layoutPath = getActiveProjectPath();
		layoutPath.append(WIDGET_LAYOUT_PATH);

		FileSerializer fs;
		fs.encode(layout.get(), layoutPath);
	}

	EditorApplication& gEditorApplication()
	{
		return static_cast<EditorApplication&>(EditorApplication::instance());
	}
}