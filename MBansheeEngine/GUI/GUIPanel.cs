﻿using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    public sealed class GUIPanel : ScriptObject
    {
        private GUIArea mainArea;
        private GUILayout _mainLayout;

        internal List<GUIArea> childAreas = new List<GUIArea>();

        public GUILayout layout
        {
            get { return _mainLayout; }
        }

        public GUISkin skin; // TODO

        internal GUIPanel()
        {
            Internal_CreateInstance(this);
        }

        internal void Initialize()
        {
            mainArea = AddArea(0, 0, 0, 0);
            _mainLayout = mainArea.layout;
        }

        public GUIArea AddArea(int x, int y, int width, int height, Int16 depth = 0, GUILayoutType layoutType = GUILayoutType.X)
        {
            GUIArea area = GUIArea.Create(this, x, y, width, height, depth, layoutType);
            area.SetParent(this);

            return area;
        }

        public void SetVisible(bool visible)
        {
            for (int i = 0; i < childAreas.Count; i++)
                childAreas[i].SetVisible(visible);
        }

        internal void SetArea(int x, int y, int width, int height)
        {
            Internal_SetArea(mCachedPtr, x, y, width, height);

            mainArea.SetArea(0, 0, width, height);
        }

        // Note: Only to be called from EditorWindow.DestroyPanel
        internal void Destroy()
        {
            GUIArea[] tempAreas = childAreas.ToArray();
            for (int i = 0; i < tempAreas.Length; i++)
                tempAreas[i].Destroy();

            childAreas.Clear();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetArea(IntPtr nativeInstance, int x, int y, int width, int height);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUIPanel instance);
    }
}
