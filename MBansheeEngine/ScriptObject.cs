﻿using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    public class ScriptObject
    {
        protected IntPtr mCachedPtr;

        ~ScriptObject()
        {
            Internal_ManagedInstanceDeleted(mCachedPtr);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ManagedInstanceDeleted(IntPtr nativeInstance);
    }
}
