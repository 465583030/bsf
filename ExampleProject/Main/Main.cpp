#include <windows.h>

#include "BsApplication.h"
#include "BsImporter.h"
#include "BsGpuProgramImportOptions.h"
#include "BsTextureImportOptions.h"
#include "BsMaterial.h"
#include "BsShader.h"
#include "BsTechnique.h"
#include "BsPass.h"
#include "BsCoreThreadAccessor.h"
#include "BsApplication.h"
#include "BsVirtualInput.h"
#include "BsCamera.h"
#include "BsRenderable.h"
#include "BsGUIWidget.h"
#include "BsGUIArea.h"
#include "BsGUILayoutX.h"
#include "BsGUILayoutY.h"
#include "BsGUISpace.h"
#include "BsGUILabel.h"
#include "BsGUIButton.h"
#include "BsGUIListBox.h"
#include "BsBuiltinResources.h"
#include "BsRTTIType.h"
#include "BsHString.h"
#include "BsRenderWindow.h"
#include "BsSceneObject.h"
#include "BsCoreThread.h"
#include "BsProfilerOverlay.h"
#include "BsRenderer.h"

#include "CameraFlyer.h"

namespace BansheeEngine
{
	UINT32 resolutionWidth = 1280;
	UINT32 resolutionHeight = 720;

	/**
	 * Imports all of ours assets and prepares GameObject that handle the example logic.
	 */
	void setUpExample();

	/**
	 * Toggles the primary window between full-screen and windowed mode.
	 */
	void toggleFullscreen();

	/**
	 * Triggered whenever a virtual button is released.
	 */
	void buttonUp(const VirtualButton& button, UINT32 deviceIdx);
}

using namespace BansheeEngine;

/**
 * Main entry point into the application.
 */
int CALLBACK WinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE hPrevInstance,
	_In_  LPSTR lpCmdLine,
	_In_  int nCmdShow
	)
{
	// Descriptor used for initializing the primary application window.
	RENDER_WINDOW_DESC renderWindowDesc;
	renderWindowDesc.videoMode = VideoMode(resolutionWidth, resolutionHeight);
	renderWindowDesc.title = "Banshee Example App";
	renderWindowDesc.fullscreen = false;

	// Initializes the application with primary window defined as above and DirectX 11 render system.
	// You may use other render systems than DirectX 11, however this example for simplicity only uses DirectX 11.
	// If you wanted other render systems you would need to create separate shaders for them and import them above
	// along with (or replace) the DX11 ones.
	Application::startUp(renderWindowDesc, RenderSystemPlugin::DX11);

	// Imports all of ours assets and prepares GameObject that handle the example logic.
	setUpExample();
	
	// Runs the main loop that does most of the work. This method will exit when user closes the main
	// window or exits in some other way.
	Application::instance().runMainLoop();

	// Perform cleanup
	Application::shutDown();

	return 0;
}

namespace BansheeEngine
{
	Path exampleModelPath = "..\\..\\..\\..\\Data\\Examples\\Pyromancer.fbx";
	Path exampleTexturePath = "..\\..\\..\\..\\Data\\Examples\\Pyromancer.psd";
	Path exampleFragmentShaderPath = "..\\..\\..\\..\\Data\\Examples\\example_fs.gpuprog";
	Path exampleVertexShaderPath = "..\\..\\..\\..\\Data\\Examples\\example_vs.gpuprog";

	GUIButton* toggleFullscreenButton = nullptr;
	bool fullscreen = false;
	const VideoMode* videoMode = nullptr;

	HMesh exampleModel;
	HTexture exampleTexture;
	HGpuProgram exampleFragmentGPUProg;
	HGpuProgram exampleVertexGPUProg;

	HCamera sceneCamera;
	HProfilerOverlay profilerOverlay;

	VirtualButton toggleCPUProfilerBtn;
	VirtualButton toggleGPUProfilerBtn;

	bool cpuProfilerActive = false;
	bool gpuProfilerActive = false;

	void setUpExample()
	{
		/************************************************************************/
		/* 								IMPORT ASSETS                      		*/
		/************************************************************************/
		// Import mesh, texture and shader from the disk. In a normal application you would want to save the imported assets
		// so next time the application is ran you can just load them directly. This can be done with Resources::save/load.

		// Import an FBX mesh.
		exampleModel = static_resource_cast<Mesh>(Importer::instance().import(exampleModelPath));

		// When importing you may specify optional import options that control how is the asset imported.
		ImportOptionsPtr textureImportOptions = Importer::instance().createImportOptions(exampleTexturePath);

		// rtti_is_of_type checks if the import options are of valid type, in case the provided path is pointing to a non-texture resource.
		// This is similar to dynamic_cast but uses Banshee internal RTTI system for type checking.
		if (rtti_is_of_type<TextureImportOptions>(textureImportOptions))
		{
			TextureImportOptions* importOptions = static_cast<TextureImportOptions*>(textureImportOptions.get());

			// We want the texture to be compressed, just a basic non-alpha format
			importOptions->setFormat(PF_BC1);

			// We want maximum number of mipmaps to be generated
			importOptions->setGenerateMipmaps(true);
		}

		// Import texture with specified import options
		exampleTexture = static_resource_cast<Texture>(Importer::instance().import(exampleTexturePath, textureImportOptions));

		// Create import options for fragment GPU program
		ImportOptionsPtr gpuProgImportOptions = Importer::instance().createImportOptions(exampleFragmentShaderPath);
		if (rtti_is_of_type<GpuProgramImportOptions>(gpuProgImportOptions))
		{
			GpuProgramImportOptions* importOptions = static_cast<GpuProgramImportOptions*>(gpuProgImportOptions.get());

			// Name of the entry function in the GPU program
			importOptions->setEntryPoint("ps_main");

			// Language the GPU program is written in. Can only be hlsl for DX11
			importOptions->setLanguage("hlsl");

			// GPU program profile specifying what feature-set the shader code uses.
			importOptions->setProfile(GPP_PS_4_0);

			// Type of the shader.
			importOptions->setType(GPT_FRAGMENT_PROGRAM);
		}

		// Import fragment GPU program
		exampleFragmentGPUProg = static_resource_cast<GpuProgram>(Importer::instance().import(exampleFragmentShaderPath, gpuProgImportOptions));

		// Create import options for vertex GPU program. Similar as above.
		gpuProgImportOptions = Importer::instance().createImportOptions(exampleVertexShaderPath);
		if (rtti_is_of_type<GpuProgramImportOptions>(gpuProgImportOptions))
		{
			GpuProgramImportOptions* importOptions = static_cast<GpuProgramImportOptions*>(gpuProgImportOptions.get());

			importOptions->setEntryPoint("vs_main");
			importOptions->setLanguage("hlsl");
			importOptions->setProfile(GPP_VS_4_0);
			importOptions->setType(GPT_VERTEX_PROGRAM);
		}

		// Import vertex GPU program
		exampleVertexGPUProg = static_resource_cast<GpuProgram>(Importer::instance().import(exampleVertexShaderPath, gpuProgImportOptions));

		/************************************************************************/
		/* 							CREATE SHADER	                      		*/
		/************************************************************************/
		// Create a shader that references our vertex and fragment GPU programs, and set
		// up shader input parameters. 
		ShaderPtr exampleShader = Shader::create("ExampleShader");

		// Set up shader parameters and renderer semantics.
		// Renderer semantics allow our renderer to automatically populate certain shader parameters (e.g. a world view projection matrix).
		// These semantics are purely optional and depend on the renderer used. Certain renderers expect certain semantics to be set up
		// otherwise they will not render the objects. You always have the option to populate all the parameters manually, but in this example
		// we go with the semantics route as it allows for a "set up and forget" approach.

		// Add a world view projection matrix parameter, which will be populated by the renderer.
		// We map our shader parameter name to the actual GPU program variable, both being "matWorldViewProj" in this case.
		exampleShader->addParameter("matWorldViewProj", "matWorldViewProj", GPDT_MATRIX_4X4, RPS_WorldViewProjTfrm);

		// Add a sampler and a texture semantic that we will populate manually.
		exampleShader->addParameter("samp", "samp", GPOT_SAMPLER2D);
		exampleShader->addParameter("tex", "tex", GPOT_TEXTURE2D);

		// Our GPU programs use parameter blocks (constant buffers in DX11 lingo). Here we notify the renderer
		// that this particular parameter block contains object-specific data (like the world view projection parameter
		// we defined above).
		exampleShader->setParamBlockAttribs("PerObject", true, GPBU_DYNAMIC, RBS_PerObject);

		/************************************************************************/
		/* 							CREATE MATERIAL                      		*/
		/************************************************************************/

		// Create a shader technique. Shader can have many different techniques and the renderer will automatically
		// use the most appropriate technique depending on the active renderer and render system. e.g. you can have different
		// techniques using HLSL9, HLSL11 and GLSL GPU programs for DirectX 9, DirectX 11 and OpenGL render systems respectively.
		TechniquePtr technique = exampleShader->addTechnique(RenderSystemDX11, RendererDefault);

		// Add a new pass to the technique. Each technique can have multiple passes that allow you to render the same
		// object multiple times using different shaders.
		PassPtr pass = technique->addPass();
		pass->setVertexProgram(exampleVertexGPUProg);
		pass->setFragmentProgram(exampleFragmentGPUProg);

		// And finally create a material with the newly created shader
		HMaterial exampleMaterial = Material::create(exampleShader);

		// And set the texture to be used by the "tex" shader parameter. We leave the "samp"
		// parameter at its defaults.
		exampleMaterial->setTexture("tex", exampleTexture);

		/************************************************************************/
		/* 								SCENE OBJECT                      		*/
		/************************************************************************/

		// Now we create a scene object that has a position, orientation, scale and optionally
		// components to govern its logic. In this particular case we are creating a SceneObject
		// with a Renderable component which will render a mesh at the position of the scene object
		// with the provided material.

		// Create new scene object at (0, 0, 0)
		HSceneObject pyromancerSO = SceneObject::create("Pyromancer");

		// Attach the Renderable component and hook up the mesh we imported earlier,
		// and the material we created in the previous section.
		HRenderable renderable = pyromancerSO->addComponent<Renderable>();
		renderable->setMesh(exampleModel);
		renderable->setMaterial(exampleMaterial);

		/************************************************************************/
		/* 									CAMERA	                     		*/
		/************************************************************************/

		// In order something to render on screen we need at least one camera.

		// Like before, we create a new scene object at (0, 0, 0).
		HSceneObject sceneCameraSO = SceneObject::create("SceneCamera");

		// We retrieve the primary render window and add a Camera component that
		// will output whatever it sees into that window (You could also use a render texture
		// or another window you created).
		RenderWindowPtr window = gApplication().getPrimaryWindow();
		sceneCamera = sceneCameraSO->addComponent<Camera>(window);

		// Set up camera component properties

		// Priority determines in what order are cameras rendered in case multiple cameras render to the same render target.
		// We raise the priority slightly because later in code we have defined a GUI camera that we want to render second.
		sceneCamera->setPriority(1);

		// Set closest distance that is visible. Anything below that is clipped.
		sceneCamera->setNearClipDistance(5);

		// Set aspect ratio depending on the current resolution
		sceneCamera->setAspectRatio(resolutionWidth / (float)resolutionHeight); // TODO - This needs to get called whenever resolution changes

		// Add a CameraFlyer component that allows us to move the camera. See CameraFlyer for more information.
		sceneCameraSO->addComponent<CameraFlyer>();

		// Position and orient the camera scene object
		sceneCameraSO->setPosition(Vector3(40.0f, 30.0f, 230.0f));
		sceneCameraSO->lookAt(Vector3(0, 0, 0));

		/************************************************************************/
		/* 									INPUT					       		*/
		/************************************************************************/

		// Register input configuration
		// Banshee allows you to use VirtualInput system which will map input device buttons
		// and axes to arbitrary names, which allows you to change input buttons without affecting
		// the code that uses it, since the code is only aware of the virtual names. 
		// If you want more direct input, see Input class.
		auto inputConfig = VirtualInput::instance().getConfiguration();

		// Camera controls for buttons (digital 0-1 input, e.g. keyboard or gamepad button)
		inputConfig->registerButton("Forward", BC_W);
		inputConfig->registerButton("Back", BC_S);
		inputConfig->registerButton("Left", BC_A);
		inputConfig->registerButton("Right", BC_D);
		inputConfig->registerButton("Forward", BC_UP);
		inputConfig->registerButton("Back", BC_BACK);
		inputConfig->registerButton("Left", BC_LEFT);
		inputConfig->registerButton("Right", BC_RIGHT);
		inputConfig->registerButton("FastMove", BC_LSHIFT);
		inputConfig->registerButton("RotateCam", BC_MOUSE_RIGHT);

		// Camera controls for axes (analog input, e.g. mouse or gamepad thumbstick)
		// These return values in [-1.0, 1.0] range.
		inputConfig->registerAxis("Horizontal", VIRTUAL_AXIS_DESC((UINT32)InputAxis::MouseX));
		inputConfig->registerAxis("Vertical", VIRTUAL_AXIS_DESC((UINT32)InputAxis::MouseY));

		// Controls that toggle the profiler overlays
		inputConfig->registerButton("CPUProfilerOverlay", BC_F1);
		inputConfig->registerButton("GPUProfilerOverlay", BC_F2);

		// Cache the profiler overlay buttons so when a button is pressed we can quickly
		// use these to determine its the one we want
		toggleCPUProfilerBtn = VirtualButton("CPUProfilerOverlay");
		toggleGPUProfilerBtn = VirtualButton("GPUProfilerOverlay");

		// Hook up a callback that gets triggered whenever a virtual button is released
		VirtualInput::instance().onButtonUp.connect(&buttonUp);

		/************************************************************************/
		/* 									GUI		                     		*/
		/************************************************************************/

		// Create a scene object that will contain GUI components
		HSceneObject guiSO = SceneObject::create("Example");

		// First we want another camera that is responsible for rendering GUI
		HCamera guiCamera = guiSO->addComponent<Camera>(window);

		// Set up GUI camera properties. 
		// We don't care about aspect ratio for GUI camera.
		guiCamera->setAspectRatio(1.0f);

		// This camera should ignore any Renderable objects in the scene
		guiCamera->setIgnoreSceneRenderables(true);

		// Don't clear this camera as that would clear anything the main camera has rendered.
		guiCamera->getViewport()->setRequiresClear(false, false, false);

		// Add a GUIWidget, the top-level GUI component, parent to all GUI elements. GUI widgets
		// require you to specify a viewport that they will output rendered GUI elements to.
		HGUIWidget gui = guiSO->addComponent<GUIWidget>(guiCamera->getViewport().get());

		// Depth allows you to control how is a GUI widget rendered in relation to other widgets
		// Lower depth means the widget will be rendered in front of those with higher. In this case we just
		// make the depth mid-range as there are no other widgets.
		gui->setDepth(128);

		// GUI skin defines how are all child elements of the GUI widget renderer. It contains all their styles
		// and default layout properties. We use the default skin that comes built into Banshee.
		gui->setSkin(BuiltinResources::instance().getGUISkin());

		// Create a GUI area that is used for displaying messages about buttons for toggling profiler overlays.
		// This area will stretch the entire surface of its parent widget, even if the widget is resized.
		GUIArea* topArea = GUIArea::createStretchedXY(*gui, 0, 0, 0, 0);

		// Add a vertical layout that will automatically position any child elements top to bottom.
		GUILayout& topLayout = topArea->getLayout().addLayoutY();

		// Add a couple of labels to the layout with the needed messages. Labels expect a HString object that
		// maps into a string table and allows for easily localization. 
		topLayout.addElement(GUILabel::create(HString(L"Press F1 to toggle CPU profiler overlay")));
		topLayout.addElement(GUILabel::create(HString(L"Press F2 to toggle GPU profiler overlay")));

		// Add a flexible space that fills up any remaining area in the area, making the two labels above be aligned
		// to the top of the GUI widget (and the screen).
		topLayout.addFlexibleSpace();

		// Create a GUI area that is used for displaying resolution and fullscreen options.
		GUIArea* rightArea = GUIArea::createStretchedXY(*gui, 0, 0, 0, 0);

		// We want all the GUI elements be right aligned, so we add a flexible space first.
		rightArea->getLayout().addFlexibleSpace();

		// And we want the elements to be vertically placed, top to bottom
		GUILayout& rightLayout = rightArea->getLayout().addLayoutY();

		// Add an empty space of 50 pixels
		rightLayout.addSpace(50);

		// Add a button that will trigger a callback when clicked
		toggleFullscreenButton = GUIButton::create(HString(L"Toggle fullscreen"));
		toggleFullscreenButton->onClick.connect(&toggleFullscreen);
		rightLayout.addElement(toggleFullscreenButton);

		// Add a profiler overlay object that is resposible for displaying CPU and GPU profiling GUI
		profilerOverlay = guiSO->addComponent<ProfilerOverlay>(guiCamera->getViewport());
	}

	void toggleFullscreen()
	{
		RenderWindowPtr window = gApplication().getPrimaryWindow();

		// In order to toggle between full-screen and windowed mode we need to use a CoreAccessor.
		// Banshee is a multi-threaded engine and when you need to communicate between simulation and
		// core thread you will use a CoreAccessor. Calling a core accessor method will essentially
		// queue the method to be executed later. Since RenderWindow is a core object you need to use
		// CoreAccessor to modify and access it from simulation thread, except where noted otherwise.

		// Classes where it is not clear if they are to be used on the core or simulation thread have
		// it noted in their documentation. e.g. RenderWindow::setWindowed method is marked as "Core only".
		// Additional asserts are normally in place for debug builds which make it harder for you to accidentally
		// call something from the wrong thread.
		if (fullscreen)
		{
			gCoreAccessor().setWindowed(window, resolutionWidth, resolutionHeight);
		}
		else
		{
			//gCoreAccessor().setFullscreen(window, *videoMode);
			gCoreAccessor().setFullscreen(window, 1920, 1200);
		}

		fullscreen = !fullscreen;
	}

	void buttonUp(const VirtualButton& button, UINT32 deviceIdx)
	{
		// Check if the pressed button is one of the either buttons we defined
		// in "setUpExample", and toggle profiler overlays accordingly.
		// Device index is ignored for now, as it is assumed the user is using a single keyboard,
		// but if you wanted support for multiple gamepads you would check deviceIdx.
		if (button == toggleCPUProfilerBtn)
		{
			if (cpuProfilerActive)
			{
				profilerOverlay->hide();
				cpuProfilerActive = false;
			}
			else
			{
				profilerOverlay->show(ProfilerOverlayType::CPUSamples);
				cpuProfilerActive = true;
				gpuProfilerActive = false;
			}
		}
		else if (button == toggleGPUProfilerBtn)
		{
			if (gpuProfilerActive)
			{
				profilerOverlay->hide();
				gpuProfilerActive = false;
			}
			else
			{
				profilerOverlay->show(ProfilerOverlayType::GPUSamples);
				gpuProfilerActive = true;
				cpuProfilerActive = false;
			}
		}
	}
}

