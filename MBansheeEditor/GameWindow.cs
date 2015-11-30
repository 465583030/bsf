﻿using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Displays in-game viewport in the editor.
    /// </summary>
    public class GameWindow : EditorWindow
    {
        private const int HeaderHeight = 20;
        private readonly AspectRatio[] aspectRatios =
        {
            new AspectRatio(16, 9), 
            new AspectRatio(16, 10),
            new AspectRatio(5, 4),
            new AspectRatio(4, 3),
            new AspectRatio(3, 2)
        };

        private int selectedAspectRatio = 0;
        private GUIRenderTexture renderTextureGUI;

        /// <summary>
        /// Opens the game window.
        /// </summary>
        [MenuItem("Windows/Game", ButtonModifier.CtrlAlt, ButtonCode.G, 6000)]
        private static void OpenGameWindow()
        {
            OpenWindow<GameWindow>();
        }

        /// <summary>
        /// Starts execution of the game in the game window.
        /// </summary>
        [MenuItem("Tools/Play", 9300)]
        [ToolbarItem("Play", ToolbarIcon.Play, "", 1800, true)]
        private static void Play()
        {
            if (EditorApplication.IsPaused)
                EditorApplication.IsPaused = false;
            else
                EditorApplication.IsPlaying = !EditorApplication.IsPlaying;
        }

        /// <summary>
        /// Pauses the execution of the game on the current frame.
        /// </summary>
        [MenuItem("Tools/Pause", 9299)]
        [ToolbarItem("Pause", ToolbarIcon.Pause, "", 1799)]
        private static void Pause()
        {
            EditorApplication.IsPaused = !EditorApplication.IsPaused;
        }

        /// <summary>
        /// Moves the execution of the game by one frame forward.
        /// </summary>
        [MenuItem("Tools/Step", 9298)]
        [ToolbarItem("Step", ToolbarIcon.Step, "", 1798)]
        private static void Step()
        {
            EditorApplication.FrameStep();
        }

        /// <inheritdoc/>
        protected override LocString GetDisplayName()
        {
            return new LocEdString("Game");
        }

        private void OnInitialize()
        {
            GUILayoutY mainLayout = GUI.AddLayoutY();

            string[] aspectRatioTitles = new string[aspectRatios.Length + 1];
            aspectRatioTitles[0] = "Free";

            for (int i = 0; i < aspectRatios.Length; i++)
                aspectRatioTitles[i + 1] = aspectRatios[i].width + ":" + aspectRatios[i].height;

            GUIListBoxField aspectField = new GUIListBoxField(aspectRatioTitles, new LocEdString("Aspect ratio"));
            aspectField.OnSelectionChanged += OnAspectRatioChanged;
            
            GUILayout buttonLayout = mainLayout.AddLayoutX();
            buttonLayout.AddElement(aspectField);
            buttonLayout.AddFlexibleSpace();

            renderTextureGUI = new GUIRenderTexture(null);

            GUILayoutY alignLayoutY = mainLayout.AddLayoutY();
            alignLayoutY.AddFlexibleSpace();
            GUILayoutX alignLayoutX = alignLayoutY.AddLayoutX();
            alignLayoutX.AddFlexibleSpace();
            alignLayoutX.AddElement(renderTextureGUI);
            alignLayoutX.AddFlexibleSpace();
            alignLayoutY.AddFlexibleSpace();

            UpdateRenderTexture(Width, Height);
        }

        /// <summary>
        /// Creates or rebuilds the main render texture. Should be called at least once before using the
        /// game window. Should be called whenever the window is resized.
        /// </summary>
        /// <param name="width">Width of the scene render target, in pixels.</param>
        /// <param name="height">Height of the scene render target, in pixels.</param>
        private void UpdateRenderTexture(int width, int height)
        {
            width = MathEx.Max(20, width);
            height = MathEx.Max(20, height - HeaderHeight);

            if (selectedAspectRatio != 0) // 0 is free aspect
            {
                AspectRatio aspectRatio = aspectRatios[selectedAspectRatio - 1];

                float aspectInv = aspectRatio.height/(float)aspectRatio.width;
                height = MathEx.RoundToInt(width*aspectInv);
            }

            RenderTexture2D renderTexture = new RenderTexture2D(PixelFormat.R8G8B8A8, width, height) {Priority = 1};

            EditorApplication.MainRenderTarget = renderTexture;
            renderTextureGUI.RenderTexture = renderTexture;
        }

        /// <summary>
        /// Triggered when the user selects a new aspect ratio from the drop down box.
        /// </summary>
        /// <param name="idx">Index of the aspect ratio the user selected.</param>
        private void OnAspectRatioChanged(int idx)
        {
            selectedAspectRatio = idx;
            UpdateRenderTexture(Width, Height);
        }

        /// <inheritdoc/>
        protected override void WindowResized(int width, int height)
        {
            UpdateRenderTexture(width, height - HeaderHeight);

            base.WindowResized(width, height);
        }

        /// <summary>
        /// Camera aspect ratio as numerator and denominator.
        /// </summary>
        struct AspectRatio
        {
            /// <summary>
            /// Creates a new object that holds the aspect ratio.
            /// </summary>
            /// <param name="width">Numerator of the aspect ratio.</param>
            /// <param name="height">Denominator of the aspect ratio.</param>
            public AspectRatio(int width, int height)
            {
                this.width = width;
                this.height = height;
            }

            public int width;
            public int height;
        }
    }
}
