﻿using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    public sealed class GUIToggleGroup : ScriptObject
    {
        public GUIToggleGroup()
        {
            Internal_CreateInstance(this);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUIToggleGroup instance);
    }
}
