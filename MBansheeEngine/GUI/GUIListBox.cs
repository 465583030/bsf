﻿using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    public sealed class GUIListBox : GUIElement
    {
        public delegate void OnSelectionChangedDelegate(int index);

        public event OnSelectionChangedDelegate OnSelectionChanged;

        public GUIListBox(LocString[] elements, string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, elements, style, options);
        }

        public GUIListBox(LocString[] elements, params GUIOption[] options)
        {
            Internal_CreateInstance(this, elements, "", options);
        }

        public void SetElements(LocString[] elements)
        {
            Internal_SetElements(mCachedPtr, elements);
        }

        public void SetTint(Color color)
        {
            Internal_SetTint(mCachedPtr, color);
        }

        private void DoOnSelectionChanged(int index)
        {
            if (OnSelectionChanged != null)
                OnSelectionChanged(index);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUIListBox instance, LocString[] elements, string style, params GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetElements(IntPtr nativeInstance, LocString[] elements);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTint(IntPtr nativeInstance, Color color);
    }
}
