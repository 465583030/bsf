﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BansheeEngine
{
    /// <summary>
    /// Determines type of data to display on the profiler overlay.
    /// </summary>
    public enum ProfilerOverlayType // Note: Must match the C++ enum ProfilerOverlayType
	{
		CPUSamples,
		GPUSamples
	};

    /// <summary>
    /// Component that displays a profiler overlay on the main game window.
    /// </summary>
    [RunInEditor]
    public sealed class ProfilerOverlay : Component
    {
        private ProfilerOverlayInternal impl;

        /// <summary>
        /// Controls whether the overlay is getting updated or not.
        /// </summary>
        public bool Paused { get; set; }

        /// <summary>
        /// Changes the type of data displayed by the overlay.
        /// </summary>
        /// <param name="type">Type that determines the type of data to display.</param>
        public void SetType(ProfilerOverlayType type)
        {
            impl.SetType(type);   
        }

        private void OnReset()
        {
            if (impl != null)
                impl.Destroy();

            Camera cam = SceneObject.GetComponent<Camera>();
            impl = new ProfilerOverlayInternal(cam);
        }

        private void OnUpdate()
        {
            if(!Paused)
                impl.Update();
        }

        private void OnDestroy()
        {
            impl.Destroy();
        }
    }
}
