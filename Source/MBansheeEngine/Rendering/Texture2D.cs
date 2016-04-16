﻿//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Rendering
     *  @{
     */

    /// <summary>
    /// A two dimensional texture.
    /// </summary>
    public sealed class Texture2D : Texture
    {
        /// <summary>
        /// Constructor for the internal use by the runtime.
        /// </summary>
        private Texture2D()
        { }

        /// <summary>
        /// Creates a new blank 2D texture.
        /// </summary>
        /// <param name="width">Width of the texture in pixels.</param>
        /// <param name="height">Height of the texture in pixels.</param>
        /// <param name="format">Format of the pixels.</param>
        /// <param name="usage">Describes planned texture use.</param>
        /// <param name="numSamples">If higher than 1, texture containing multiple samples per pixel is created.</param>
        /// <param name="hasMipmaps">Should the texture allocate memory for the entire mip-map chain or only the top level.
        ///                          </param>
        /// <param name="gammaCorrection">If true the texture data is assumed to have be gamma corrected and will be
        ///                               converted back to linear space when sampled on GPU, and converted to gamma space
        ///                               before being written by the GPU.</param>
        public Texture2D(int width, int height, PixelFormat format = PixelFormat.R8G8B8A8, TextureUsage usage = TextureUsage.Default, 
            int numSamples = 1, bool hasMipmaps = false, bool gammaCorrection = false)
        {
            Internal_CreateInstance(this, format, width, height, usage, numSamples, hasMipmaps, gammaCorrection);
        }

        /// <summary>
        /// Returns pixels for the specified mip level. Pixels will be read from system memory, which means the texture has
        /// to be created with <see cref="TextureUsage.CPUCached"/>. If the texture was updated from the GPU the pixels
        /// retrieved from this method will not reflect that, and you should use <see cref="GetGPUPixels"/> instead.
        /// </summary>
        /// <param name="mipLevel">Mip level to retrieve pixels for. Top level (0) is the highest quality.</param>
        /// <returns>A set of pixels for the specified mip level.</returns>
        public PixelData GetPixels(int mipLevel = 0)
        {
            return Internal_GetPixels(mCachedPtr, mipLevel);
        }

        /// <summary>
        /// Sets pixels for the specified mip level.
        /// </summary>
        /// <param name="data">Pixels to assign to the specified mip level. Pixel data must match the mip level size
        ///                    and texture pixel format.</param>
        /// <param name="mipLevel">Mip level to set pixels for. Top level (0) is the highest quality.</param>
        public void SetPixels(PixelData data, int mipLevel = 0)
        {
            Internal_SetPixels(mCachedPtr, data, mipLevel);
        }

        /// <summary>
        /// Sets pixels for the specified mip level.
        /// </summary>
        /// <param name="data">Pixels to assign to the specified mip level. Size of the array must match texture width
        ///                    multiplied by height. Data is expected to be laid out row by row. Pixels will be
        ///                    automatically converted to the valid pixel format.</param>
        /// <param name="mipLevel">Mip level to set pixels for. Top level (0) is the highest quality.</param>
        public void SetPixels(Color[] data, int mipLevel = 0)
        {
            if (data == null || data.Length != (Width*Height))
            {
                int size = data == null ? 0 : data.Length;
                Debug.LogError("SetPixels called with incorrect size: " + size);
            }

            Internal_SetPixelsArray(mCachedPtr, data, mipLevel);
        }

        /// <summary>
        /// Reads texture pixels directly from the GPU. This is similar to <see cref="GetPixels"/> but the texture doesn't
        /// need to be created with <see cref="TextureUsage.CPUCached"/>, and the data will contain any updates performed by
        /// the GPU. This method can be potentially slow as it introduces a CPU-GPU synchronization point. Additionally
        /// this method is asynchronous which means the data is not available immediately.
        /// </summary>
        /// <param name="mipLevel">Mip level to retrieve pixels for. Top level (0) is the highest quality.</param>
        /// <returns><see cref="AsyncOp"/> object that will contain <see cref="PixelData"/> object when the operation 
        ///          completes.</returns>
        public AsyncOp GetGPUPixels(int mipLevel = 0)
        {
            return Internal_GetGPUPixels(mCachedPtr, mipLevel);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(Texture2D instance, PixelFormat format, int width, 
            int height, TextureUsage usage, int numSamples, bool hasMipmaps, bool gammaCorrection);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern PixelData Internal_GetPixels(IntPtr thisPtr, int mipLevel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern AsyncOp Internal_GetGPUPixels(IntPtr thisPtr, int mipLevel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPixels(IntPtr thisPtr, PixelData data, int mipLevel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPixelsArray(IntPtr thisPtr, Color[] data, int mipLevel);
    }

    /** @} */
}
