﻿using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Handles rendering of scene axis handles into a GUI element.
    /// </summary>
    internal class SceneAxesGUI
    {
        private RenderTexture2D renderTexture;
        private Camera camera;
        private SceneHandles sceneHandles;

        private GUIRenderTexture renderTextureGUI;
        private Rect2I bounds;

        /// <summary>
        /// Creates a new scene axes GUI.
        /// </summary>
        /// <param name="window">Window in which the GUI is located in.</param>
        /// <param name="panel">Panel onto which to place the GUI element.</param>
        /// <param name="width">Width of the GUI element.</param>
        /// <param name="height">Height of the GUI element.</param>
        public SceneAxesGUI(EditorWindow window, GUIPanel panel, int width, int height)
        {
            renderTexture = new RenderTexture2D(PixelFormat.R8G8B8A8, width, height);
            renderTexture.Priority = 1;

            SceneObject cameraSO = new SceneObject("SceneAxesCamera", true);
            camera = cameraSO.AddComponent<Camera>();
            camera.Target = renderTexture;
            camera.ViewportRect = new Rect2(0.0f, 0.0f, 1.0f, 1.0f);

            cameraSO.Position = new Vector3(0, 0, 5);
            cameraSO.LookAt(new Vector3(0, 0, 0));

            camera.Priority = 2;
            camera.NearClipPlane = 0.05f;
            camera.FarClipPlane = 1000.0f;
            camera.ClearColor = new Color(0.0f, 0.0f, 0.0f, 0.0f);
            camera.ProjectionType = ProjectionType.Orthographic;
            camera.Layers = SceneAxesHandle.LAYER;
            camera.AspectRatio = 1.0f;
            camera.OrthoHeight = 2.0f;

            renderTextureGUI = new GUIRenderTexture(renderTexture);
            panel.AddElement(renderTextureGUI);

            Rect2I bounds = new Rect2I(0, 0, width, height);
            sceneHandles = new SceneHandles(window, camera);
            renderTextureGUI.Bounds = bounds;

            this.bounds = bounds;
        }

        /// <summary>
        /// Selects a handle under the pointer position.
        /// </summary>
        /// <param name="pointerPos">Position of the pointer relative to the parent GUI panel.</param>
        public void TrySelect(Vector2I pointerPos)
        {
            if (!bounds.Contains(pointerPos))
                return;

            pointerPos.x -= bounds.x;
            pointerPos.y -= bounds.y;
            sceneHandles.TrySelect(pointerPos);
        }

        /// <summary>
        /// Checks is any handle currently active.
        /// </summary>
        /// <returns>True if a handle is active.</returns>
        internal bool IsActive()
        {
            return sceneHandles.IsActive();
        }

        /// <summary>
        /// Deselects any currently active handles.
        /// </summary>
        public void ClearSelection()
        {
            sceneHandles.ClearSelection();
        }

        /// <summary>
        /// Updates active handles by moving them as a result of any input.
        /// </summary>
        /// <param name="pointerPos">Position of the pointer relative to the parent GUI panel</param>
        public void UpdateInput(Vector2I pointerPos)
        {
            pointerPos.x -= bounds.x;
            pointerPos.y -= bounds.y;
            sceneHandles.UpdateInput(pointerPos, Input.PointerDelta);
        }

        /// <summary>
        /// Draws the scene axes onto the underlying camera.
        /// </summary>
        public void Draw()
        {
            sceneHandles.Draw();
        }

        /// <summary>
        /// Moves the GUI element to the specified position.
        /// </summary>
        /// <param name="x">Horizontal position of the GUI element relative to the parent panel.</param>
        /// <param name="y">Vertical position of the GUI element relative to the parent panel.</param>
        public void SetPosition(int x, int y)
        {
            bounds.x = x;
            bounds.y = y;
            renderTextureGUI.Bounds = bounds;
        }
    }
}
