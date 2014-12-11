﻿using System;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    public class EditorWindow : ScriptObject
    {
        public int Width { get { return Internal_GetWidth(mCachedPtr); } }
        public int Height { get { return Internal_GetHeight(mCachedPtr); } }
        public bool HasFocus { get { return Internal_HasFocus(mCachedPtr); } }

        protected GUIPanel GUI;

        public static T OpenWindow<T>() where T : EditorWindow
        {
            return (T)Internal_CreateOrGetInstance(typeof(T).Namespace, typeof(T).Name);
        }

        public Vector2I ScreenToWindowPos(Vector2I screenPos)
        {
            Vector2I windowPos;
            Internal_ScreenToWindowPos(mCachedPtr, screenPos, out windowPos);
            return windowPos;
        }

        public Vector2I WindowToScreenPos(Vector2I windowPos)
        {
            Vector2I screenPos;
            Internal_WindowToScreenPos(mCachedPtr, windowPos, out screenPos);
            return screenPos;
        }

        protected EditorWindow()
        {
            GUI = CreatePanel(0, 0, Width, Height);
        }

        protected virtual void WindowResized(int width, int height)
        {
            GUI.SetArea(0, 0, width, height);
        }

        protected virtual void FocusChanged(bool inFocus)
        {
            
        }

        internal GUIPanel CreatePanel(int x, int y, int width, int height)
        {
            GUIPanel newPanel = new GUIPanel();
            Internal_InitializeGUIPanel(mCachedPtr, newPanel);
            newPanel.Initialize();
            newPanel.SetArea(x, y, width, height);

            return newPanel;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern EditorWindow Internal_CreateOrGetInstance(string ns, string typeName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_InitializeGUIPanel(IntPtr nativeInstance, GUIPanel panel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetWidth(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetHeight(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_HasFocus(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ScreenToWindowPos(IntPtr nativeInstance, Vector2I position, out Vector2I windowPos);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_WindowToScreenPos(IntPtr nativeInstance, Vector2I position, out Vector2I screenPos);
    }
}
