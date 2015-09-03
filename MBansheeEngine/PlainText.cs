﻿using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /// <summary>
    /// A resource containing plain text data.
    /// </summary>
    public class PlainText : Resource
    {
        /// <summary>
        /// Constructor used internally by the runtime.
        /// </summary>
        private PlainText()
        { }

        /// <summary>
        /// Creates a new plain text resource.
        /// </summary>
        /// <param name="text">Textual data to store.</param>
        public PlainText(string text)
        {
            Internal_CreateInstance(this, text);
        }

        /// <summary>
        /// Textual data contained in the resource.
        /// </summary>
        public string Text
        {
            get { return Internal_GetText(mCachedPtr); }
            set { Internal_SetText(mCachedPtr, value); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(PlainText instance, string text);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetText(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetText(IntPtr thisPtr, string value);
    }
}
