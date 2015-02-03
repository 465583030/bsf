﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BansheeEngine;

namespace BansheeEditor
{
    internal sealed class SceneWindow : EditorWindow
    {
        private const int HeaderHeight = 20;

        private Camera camera;
        private SceneCamera cameraController;
        private RenderTexture2D renderTexture;
        private GUILayoutY mainLayout;

        private GUIRenderTexture renderTextureGUI;
        private SceneViewHandler sceneViewHandler;

        private GUIToggle viewButton;
        private GUIToggle moveButton;
        private GUIToggle rotateButton;
        private GUIToggle scaleButton;

        private GUIToggle localCoordButton;
        private GUIToggle worldCoordButton;

        private GUIToggle pivotButton;
        private GUIToggle centerButton;

        private GUIToggle moveSnapButton;
        private GUIFloatField moveSnapInput;

        private GUIToggle rotateSnapButton;
        private GUIFloatField rotateSnapInput;

        private int editorSettingsHash = int.MaxValue;

        public Camera GetCamera()
        {
            return camera;
        }

        internal SceneWindow()
        { }

        private void OnInitialize()
        {
            mainLayout = GUI.layout.AddLayoutY();

            GUIToggleGroup handlesTG = new GUIToggleGroup();
            viewButton = new GUIToggle("V", handlesTG, EditorStyles.Button);
            moveButton = new GUIToggle("M", handlesTG, EditorStyles.Button);
            rotateButton = new GUIToggle("R", handlesTG, EditorStyles.Button);
            scaleButton = new GUIToggle("S", handlesTG, EditorStyles.Button);

            GUIToggleGroup coordModeTG = new GUIToggleGroup();
            localCoordButton = new GUIToggle("L", coordModeTG, EditorStyles.Button);
            worldCoordButton = new GUIToggle("W", coordModeTG, EditorStyles.Button);

            GUIToggleGroup pivotModeTG = new GUIToggleGroup();
            pivotButton = new GUIToggle("P", pivotModeTG, EditorStyles.Button);
            centerButton = new GUIToggle("C", pivotModeTG, EditorStyles.Button);

            moveSnapButton = new GUIToggle("MS", EditorStyles.Button);
            moveSnapInput = new GUIFloatField();

            rotateSnapButton = new GUIToggle("RS", EditorStyles.Button);
            rotateSnapInput = new GUIFloatField();

            viewButton.OnClick += () => OnSceneToolButtonClicked(SceneViewTool.View);
            moveButton.OnClick += () => OnSceneToolButtonClicked(SceneViewTool.Move);
            rotateButton.OnClick += () => OnSceneToolButtonClicked(SceneViewTool.Rotate);
            scaleButton.OnClick += () => OnSceneToolButtonClicked(SceneViewTool.Scale);

            localCoordButton.OnClick += () => OnCoordinateModeButtonClicked(HandleCoordinateMode.Local);
            worldCoordButton.OnClick += () => OnCoordinateModeButtonClicked(HandleCoordinateMode.World);

            pivotButton.OnClick += () => OnPivotModeButtonClicked(HandlePivotMode.Pivot);
            centerButton.OnClick += () => OnPivotModeButtonClicked(HandlePivotMode.Center);

            moveSnapButton.OnToggled += (bool active) => OnMoveSnapToggled(active);
            moveSnapInput.OnChanged += (float value) => OnMoveSnapValueChanged(value);

            rotateSnapButton.OnToggled += (bool active) => OnRotateSnapToggled(active);
            rotateSnapInput.OnChanged += (float value) => OnRotateSnapValueChanged(value);

            GUILayout handlesLayout = mainLayout.AddLayoutX();
            handlesLayout.AddElement(viewButton);
            handlesLayout.AddElement(moveButton);
            handlesLayout.AddElement(rotateButton);
            handlesLayout.AddElement(scaleButton);
            handlesLayout.AddSpace(10);
            handlesLayout.AddElement(localCoordButton);
            handlesLayout.AddElement(worldCoordButton);
            handlesLayout.AddSpace(10);
            handlesLayout.AddElement(pivotButton);
            handlesLayout.AddElement(centerButton);
            handlesLayout.AddFlexibleSpace();
            handlesLayout.AddElement(moveSnapButton);
            handlesLayout.AddElement(moveSnapInput);
            handlesLayout.AddSpace(10);
            handlesLayout.AddElement(rotateSnapButton);
            handlesLayout.AddElement(rotateSnapInput);

            UpdateRenderTexture(Width, Height - HeaderHeight);
        }

        private void OnDestroy()
        {
            if (camera != null)
            {
                camera.sceneObject.Destroy();
            }
        }

        private bool ScreenToScenePos(Vector2I screenPos, out Vector2I scenePos)
        {
            scenePos = screenPos;
            Vector2I windowPos = ScreenToWindowPos(screenPos);

            Rect2I bounds = GUILayoutUtility.CalculateBounds(renderTextureGUI);

            if (bounds.Contains(windowPos))
            {
                scenePos.x = windowPos.x - bounds.x;
                scenePos.y = windowPos.y - bounds.y;

                return true;
            }

            return false;
        }

        private void EditorUpdate()
        {
            // Refresh GUI buttons if needed (in case someones changes the values from script)
            if (editorSettingsHash != EditorSettings.Hash)
            {
                UpdateButtonStates();
                editorSettingsHash = EditorSettings.Hash;
            }

            // Update scene view handles and selection
            sceneViewHandler.Update();

            bool handleActive = false;
            if (Input.IsButtonUp(ButtonCode.MouseLeft))
            {
                if (sceneViewHandler.IsHandleActive())
                {
                    sceneViewHandler.ClearHandleSelection();
                    handleActive = true;
                }
            }

            if (!HasFocus)
                return;

            Vector2I scenePos;
            if (ScreenToScenePos(Input.PointerPosition, out scenePos))
            {
                if (Input.IsButtonDown(ButtonCode.MouseLeft))
                {
                    sceneViewHandler.TrySelectHandle(scenePos);
                }
                else if (Input.IsButtonUp(ButtonCode.MouseLeft))
                {
                    if (!handleActive)
                    {
                        bool ctrlHeld = Input.IsButtonHeld(ButtonCode.LeftControl) ||
                            Input.IsButtonHeld(ButtonCode.RightControl);

                        sceneViewHandler.PickObject(scenePos, ctrlHeld);
                    }
                }
            }

            sceneViewHandler.UpdateHandle(scenePos, Input.PointerDelta);
        }

        protected override void WindowResized(int width, int height)
        {
            UpdateRenderTexture(width, height - HeaderHeight);

            base.WindowResized(width, height);
        }

        protected override void FocusChanged(bool inFocus)
        {
            if (!inFocus)
            {
                sceneViewHandler.ClearHandleSelection();
            }
        }

        private void OnSceneToolButtonClicked(SceneViewTool tool)
        {
            EditorApplication.ActiveSceneTool = tool;
            editorSettingsHash = EditorSettings.Hash;
        }

        private void OnCoordinateModeButtonClicked(HandleCoordinateMode mode)
        {
            EditorApplication.ActiveCoordinateMode = mode;
            editorSettingsHash = EditorSettings.Hash;
        }

        private void OnPivotModeButtonClicked(HandlePivotMode mode)
        {
            EditorApplication.ActivePivotMode = mode;
            editorSettingsHash = EditorSettings.Hash;
        }

        private void OnMoveSnapToggled(bool active)
        {
            Handles.MoveHandleSnapActive = active;
            editorSettingsHash = EditorSettings.Hash;
        }

        private void OnMoveSnapValueChanged(float value)
        {
            Handles.MoveSnapAmount = MathEx.Clamp(value, 0.01f, 1000.0f);
            editorSettingsHash = EditorSettings.Hash;
        }

        private void OnRotateSnapToggled(bool active)
        {
            Handles.RotateHandleSnapActive = active;
            editorSettingsHash = EditorSettings.Hash;
        }

        private void OnRotateSnapValueChanged(float value)
        {
            Handles.RotateSnapAmount = MathEx.Clamp(value, 0.01f, 360.0f);
            editorSettingsHash = EditorSettings.Hash;
        }

        private void UpdateButtonStates()
        {
            switch (EditorApplication.ActiveSceneTool)
            {
                case SceneViewTool.View:
                    viewButton.ToggleOn();
                    break;
                case SceneViewTool.Move:
                    moveButton.ToggleOn();
                    break;
                case SceneViewTool.Rotate:
                    rotateButton.ToggleOn();
                    break;
                case SceneViewTool.Scale:
                    scaleButton.ToggleOn();
                    break;
            }

            switch (EditorApplication.ActiveCoordinateMode)
            {
                case HandleCoordinateMode.Local:
                    localCoordButton.ToggleOn();
                    break;
                case HandleCoordinateMode.World:
                    worldCoordButton.ToggleOn();
                    break;
            }

            switch (EditorApplication.ActivePivotMode)
            {
                case HandlePivotMode.Center:
                    centerButton.ToggleOn();
                    break;
                case HandlePivotMode.Pivot:
                    pivotButton.ToggleOn();
                    break;
            }

            if (Handles.MoveHandleSnapActive)
                moveSnapButton.ToggleOn();
            else
                moveSnapButton.ToggleOff();

            moveSnapInput.Value = Handles.MoveSnapAmount;

            if (Handles.RotateHandleSnapActive)
                rotateSnapButton.ToggleOn();
            else
                rotateSnapButton.ToggleOff();

            moveSnapInput.Value = Handles.RotateSnapAmount.Degrees;
        }

        private void UpdateRenderTexture(int width, int height)
	    {
            width = MathEx.Max(20, width);
            height = MathEx.Max(20, height);

            renderTexture = new RenderTexture2D(PixelFormat.R8G8B8A8, width, height);
            renderTexture.Priority = 1;

		    if (camera == null)
		    {
                SceneObject sceneCameraSO = new SceneObject("SceneCamera");
                camera = sceneCameraSO.AddComponent<Camera>();
                camera.Target = renderTexture;
                camera.ViewportRect = new Rect2(0.0f, 0.0f, 1.0f, 1.0f);

                sceneCameraSO.Position = new Vector3(0, 0.5f, 1);
                sceneCameraSO.LookAt(new Vector3(0, 0, 0));

                camera.Priority = 1;
                camera.NearClipPlane = 0.005f;
                camera.FarClipPlane = 1000.0f;

                cameraController = sceneCameraSO.AddComponent<SceneCamera>();

                renderTextureGUI = new GUIRenderTexture(renderTexture);
                mainLayout.AddElement(renderTextureGUI);

                sceneViewHandler = new SceneViewHandler(this, camera);
		    }
		    else
		    {
		        camera.Target = renderTexture;
		        renderTextureGUI.RenderTexture = renderTexture;
		    }

		    // TODO - Consider only doing the resize once user stops resizing the widget in order to reduce constant
		    // render target destroy/create cycle for every single pixel.

		    camera.AspectRatio = width / (float)height;
	    }
    }
}
