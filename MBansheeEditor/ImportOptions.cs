﻿using System;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Base class for all import options. Allows control over how is a specific resource type imported.
    /// </summary>
    public class ImportOptions : ScriptObject
    {

    }

    /// <summary>
    /// Provides options for controlling how is a texture resource imported.
    /// </summary>
    public class TextureImportOptions : ImportOptions
    {
        /// <summary>
        /// Creates new texture import options with default values.
        /// </summary>
        public TextureImportOptions()
        {
            Internal_CreateInstance(this);
        }

        /// <summary>
        /// Pixel format to import as.
        /// </summary>
        public PixelFormat Format
        {
            get { return Internal_GetPixelFormat(mCachedPtr); }
            set { Internal_SetPixelFormat(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines whether the imported texture will have mipmaps generated.
        /// </summary>
        public bool GenerateMipmaps
        {
            get { return Internal_GetGenerateMipmaps(mCachedPtr); }
            set { Internal_SetGenerateMipmaps(mCachedPtr, value); }
        }

        /// <summary>
        /// Maximum mipmap level to generate, if mipmap generation is enabled.
        /// </summary>
        public int MaxMipmapLevel
        {
            get { return Internal_GetMaxMipmapLevel(mCachedPtr); }
            set { Internal_SetMaxMipmapLevel(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines whether the texture data is also stored in main memory, available for fast CPU access.
        /// </summary>
        public bool CPUReadable
        {
            get { return Internal_GetCPUReadable(mCachedPtr); }
            set { Internal_SetCPUReadable(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines should the texture data be treated as if its in sRGB (gamma) space. Such texture will be converted by
        /// hardware to linear space before use on the GPU.
        /// </summary>
        public bool IsSRGB
        {
            get { return Internal_GetIsSRGB(mCachedPtr); }
            set { Internal_SetIsSRGB(mCachedPtr, value); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(TextureImportOptions instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern PixelFormat Internal_GetPixelFormat(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPixelFormat(IntPtr thisPtr, PixelFormat value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetGenerateMipmaps(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetGenerateMipmaps(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetMaxMipmapLevel(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMaxMipmapLevel(IntPtr thisPtr, int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetCPUReadable(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCPUReadable(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetIsSRGB(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetIsSRGB(IntPtr thisPtr, bool value);
    }

    /// <summary>
    /// Provides options for controlling how is a mesh resource imported.
    /// </summary>
    public class MeshImportOptions : ImportOptions
    {
        /// <summary>
        /// Creates new mesh import options with default values.
        /// </summary>
        public MeshImportOptions()
        {
            Internal_CreateInstance(this);
        }

        /// <summary>
        /// Determines whether the mesh data is also stored in main memory, available for fast CPU access. 
        /// </summary>
        public bool CPUReadable
        {
            get { return Internal_GetCPUReadable(mCachedPtr); }
            set { Internal_SetCPUReadable(mCachedPtr, value); }
        }

        /// <summary>
        /// Controls should mesh normals be imported if available.
        /// </summary>
        public bool ImportNormals
        {
            get { return Internal_GetImportNormals(mCachedPtr); }
            set { Internal_SetImportNormals(mCachedPtr, value); }
        }

        /// <summary>
        /// Controls should mesh tangents/bitangents be imported if available.
        /// </summary>
        public bool ImportTangents
        {
            get { return Internal_GetImportTangents(mCachedPtr); }
            set { Internal_SetImportTangents(mCachedPtr, value); }
        }

        /// <summary>
        /// Controls should mesh skin data like bone weights, indices and bind poses be imported if available.
        /// </summary>
        public bool ImportSkin
        {
            get { return Internal_GetImportSkin(mCachedPtr); }
            set { Internal_SetImportSkin(mCachedPtr, value); }
        }

        /// <summary>
        /// Controls should animation clips be imported if available.
        /// </summary>
        public bool ImportAnimation
        {
            get { return Internal_GetImportAnimation(mCachedPtr); }
            set { Internal_SetImportAnimation(mCachedPtr, value); }
        }

        /// <summary>
        /// Controls should mesh blend shapes be imported if available.
        /// </summary>
        public bool ImportBlendShapes
        {
            get { return Internal_GetImportBlendShapes(mCachedPtr); }
            set { Internal_SetImportBlendShapes(mCachedPtr, value); }
        }

        /// <summary>
        /// Uniformly scales the imported mesh by the specified value.
        /// </summary>
        public float Scale
        {
            get { return Internal_GetScale(mCachedPtr); }
            set { Internal_SetScale(mCachedPtr, value); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(MeshImportOptions instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetCPUReadable(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCPUReadable(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetImportNormals(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetImportNormals(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetImportTangents(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetImportTangents(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetImportSkin(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetImportSkin(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetImportAnimation(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetImportAnimation(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetImportBlendShapes(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetImportBlendShapes(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetScale(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetScale(IntPtr thisPtr, float value);
    }

    /// <summary>
    /// Import options that provide various options for controlling how is a font resource imported.
    /// </summary>
    public class FontImportOptions : ImportOptions
    {
        /// <summary>
        /// Creates new font import options with default values.
        /// </summary>
        public FontImportOptions()
        {
            Internal_CreateInstance(this);
        }

        /// <summary>
        /// Font sizes in points that are to be imported.
        /// </summary>
        public UInt32[] FontSizes
        {
            get { return Internal_GetFontSizes(mCachedPtr); }
            set { Internal_SetFontSizes(mCachedPtr, value); }
        }

        /// <summary>
        /// Dots per inch resolution to use when rendering the characters into the texture.
        /// </summary>
        public UInt32 DPI
        {
            get { return Internal_GetDPI(mCachedPtr); }
            set { Internal_SetDPI(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines should rendered characters be antialiased.
        /// </summary>
        public bool Antialiasing
        {
            get { return Internal_GetAntialiasing(mCachedPtr); }
            set { Internal_SetAntialiasing(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines character ranges to import from the font. Ranges are defined as unicode numbers.
        /// </summary>
        public CharRange[] CharRanges
        {
            get { return Internal_GetCharRanges(mCachedPtr); }
            set { Internal_SetCharRanges(mCachedPtr, value); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(FontImportOptions instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt32[] Internal_GetFontSizes(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetFontSizes(IntPtr thisPtr, UInt32[] value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt32 Internal_GetDPI(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetDPI(IntPtr thisPtr, UInt32 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetAntialiasing(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetAntialiasing(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern CharRange[] Internal_GetCharRanges(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCharRanges(IntPtr thisPtr, CharRange[] value);
    }

    /// <summary>
    /// Provides various options for controlling how is a script file imported.
    /// </summary>
    public class ScriptCodeImportOptions : ImportOptions
    {
        /// <summary>
        /// Creates new script import options with default values.
        /// </summary>
        public ScriptCodeImportOptions()
        {
            Internal_CreateInstance(this);
        }

        /// <summary>
        /// Determines whether the script is editor-only or a normal game script. Editor scripts are compiled in a separate
        /// assembly and may reference editor specific functionality, but are not available in the final game code.
        /// </summary>
        public bool EditorScript
        {
            get { return Internal_IsEditorScript(mCachedPtr); }
            set { Internal_SetEditorScript(mCachedPtr, value); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(ScriptCodeImportOptions instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsEditorScript(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetEditorScript(IntPtr thisPtr, bool value);
    }
}
