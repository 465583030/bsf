//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsResource.h"
#include "BsHardwareBuffer.h"
#include "BsPixelUtil.h"
#include "BsTextureView.h"

namespace BansheeEngine 
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**	Flags that describe how is a texture used. */
    enum TextureUsage
    {
		TU_STATIC = GBU_STATIC, /**< A regular texture that is not often or ever updated from the CPU. */
		TU_DYNAMIC = GBU_DYNAMIC, /**< A regular texture that is often updated by the CPU. */
		TU_RENDERTARGET = 0x200, /**< Texture that can be rendered to by the GPU. */
		TU_DEPTHSTENCIL = 0x400, /**< Texture used as a depth/stencil buffer by the GPU. */
		TU_LOADSTORE = 0x800, /**< Texture that allows load/store operations from the GPU program. */
		TU_CPUCACHED = 0x1000, /**< Ensures all texture data will also be cached in system memory. */
		TU_DEFAULT = TU_STATIC
    };

	/**	Available texture types. */
    enum TextureType
    {
		TEX_TYPE_1D = 1, /**< One dimensional texture. Just a row of pixels. */
		TEX_TYPE_2D = 2, /**< Two dimensional texture. */
		TEX_TYPE_3D = 3, /**< Three dimensional texture. */
		TEX_TYPE_CUBE_MAP = 4 /**< Texture consisting out of six 2D textures describing an inside of a cube. Allows special sampling. */
    };

	/**	Texture mipmap options. */
	enum TextureMipmap
	{
		MIP_UNLIMITED = 0x7FFFFFFF /**< Create all mip maps down to 1x1. */
	};

	/** Properties of a Texture. Shared between sim and core thread versions of a Texture. */
	class BS_CORE_EXPORT TextureProperties
	{
	public:
		TextureProperties();
		TextureProperties(TextureType textureType, UINT32 width, UINT32 height, UINT32 depth, UINT32 numMipmaps,
			PixelFormat format, int usage, bool hwGamma, UINT32 multisampleCount);

		/**	Gets the type of texture. */
        TextureType getTextureType() const { return mTextureType; }

        /**
		 * Gets the number of mipmaps to be used for this texture. This number excludes the top level map (which is always
		 * assumed to be present).
         */
        UINT32 getNumMipmaps() const {return mNumMipmaps;}

		/** Gets whether this texture will be set up so that on sampling it, hardware gamma correction is applied. */
		bool isHardwareGammaEnabled() const { return mHwGamma; }

		/**	Gets the number of samples used for multisampling (0 if multisampling is not used). */
		UINT32 getMultisampleCount() const { return mMultisampleCount; }

        /**	Returns the height of the texture.  */
        UINT32 getHeight() const { return mHeight; }

        /**	Returns the width of the texture. */
        UINT32 getWidth() const { return mWidth; }

        /**	Returns the depth of the texture (only applicable for 3D textures). */
        UINT32 getDepth() const { return mDepth; }

        /**	Returns texture usage (TextureUsage) of this texture. */
        int getUsage() const { return mUsage; }

		/**	Returns the pixel format for the texture surface. */
		PixelFormat getFormat() const { return mFormat; }

        /**	Returns true if the texture has an alpha layer. */
        bool hasAlpha() const;

        /**	Return the number of faces this texture has. */
        UINT32 getNumFaces() const;

		/**
		 * Maps a sub-resource index to an exact face and mip level. Sub-resource indexes are used when reading or writing
		 * to the resource.
		 * 			
		 * @note	
		 * Sub-resource index is only valid for the instance it was created on. You cannot use a sub-resource index from a
		 * different texture and expect to get valid result. Modifying the resource so the number of sub-resources changes
		 * invalidates all sub-resource indexes.
		 */
		void mapFromSubresourceIdx(UINT32 subresourceIdx, UINT32& face, UINT32& mip) const;

		/**
		 * Map a face and a mip level to a sub-resource index you can use for updating or reading a specific sub-resource.
		 * 			
		 * @note	
		 * Generated sub-resource index is only valid for the instance it was created on. Modifying the resource so the 
		 * number of sub-resources changes, invalidates all sub-resource indexes.
		 */
		UINT32 mapToSubresourceIdx(UINT32 face, UINT32 mip) const;

		/**
		 * Allocates a buffer you may use for storage when reading or writing a sub-resource. You need to allocate such a 
		 * buffer if you are calling readSubresource().
		 *
		 * You can retrieve a sub-resource index by calling mapToSubresourceIdx().
		 * 			
		 * @note	Thread safe.
		 */
		PixelDataPtr allocateSubresourceBuffer(UINT32 subresourceIdx) const;

	protected:
		friend class TextureRTTI;

		UINT32 mHeight;
		UINT32 mWidth;
		UINT32 mDepth;

		UINT32 mNumMipmaps;
		bool mHwGamma;
		UINT32 mMultisampleCount;

		TextureType mTextureType;
		PixelFormat mFormat;
		int mUsage;
	};

	/** @cond INTERNAL */

	/**
	 * Core thread version of a Texture.
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT TextureCore : public CoreObjectCore
	{
	public:
		TextureCore(TextureType textureType, UINT32 width, UINT32 height, UINT32 depth, UINT32 numMipmaps,
			PixelFormat format, int usage, bool hwGamma, UINT32 multisampleCount, const PixelDataPtr& initData);
		virtual ~TextureCore() {}


		/** @copydoc CoreObjectCore::initialize */
		virtual void initialize() override;

		/**
		 * Updates a part of the texture with the provided data.
		 *
		 * @param[in]	subresourceIdx		Index of the subresource to update, if the texture has more than one.
		 * @param[in]	data				Data to update the texture with.
		 * @param[in]	discardEntireBuffer When true the existing contents of the resource you are updating will be 
		 *									discarded. This can make the operation faster. Resources with certain buffer 
		 *									types might require this flag to be in a specific state otherwise the operation 
		 *									will fail.
		 */
		virtual void writeSubresource(UINT32 subresourceIdx, const PixelData& data, bool discardEntireBuffer);

		/**
		 * Reads a part of the current resource into the provided @p data parameter.
		 * 			Data buffer needs to be pre-allocated.
		 *
		 * @param[in]	subresourceIdx		Index of the subresource to update, if the texture has more than one.
		 * @param[out]	data				Buffer that will receive the data. Should be allocated with 
		 *									allocateSubresourceBuffer() to ensure it is of valid type and size.
		 */
		virtual void readSubresource(UINT32 subresourceIdx, PixelData& data);

		/**
		 * Locks the buffer for reading or writing.
		 *
		 * @param[in]	options 	Options for controlling what you may do with the locked data.
		 * @param[in]	mipLevel	(optional) Mipmap level to lock.
		 * @param[in]	face		(optional) Texture face to lock.					
		 * @return					Pointer to the buffer data. Only valid until you call unlock().
		 * 			
		 * @note	
		 * If you are just reading or writing one block of data use readData()/writeData() methods as they can be much faster
		 * in certain situations.
		 */
		PixelData lock(GpuLockOptions options, UINT32 mipLevel = 0, UINT32 face = 0);

		/** 
		 * Unlocks a previously locked buffer. After the buffer is unlocked, any data returned by lock becomes invalid. 
		 *
		 * @see	lock()
		 */
		void unlock();

		/**
		 * Copies the contents a subresource in this texture to another texture. Texture format and size of the subresource
		 * must match.
		 *
		 * You are allowed to copy from a multisampled to non-multisampled surface, which will resolve the multisampled
		 * surface before copying.
		 *
		 * @param[in]	srcSubresourceIdx	Index of the subresource to copy from.
		 * @param[in]	destSubresourceIdx	Index of the subresource to copy to.
		 * @param[in]	target				Texture that contains the destination subresource.
		 */
		void copy(UINT32 srcSubresourceIdx, UINT32 destSubresourceIdx, const SPtr<TextureCore>& target);

		/**
		 * Reads data from the texture buffer into the provided buffer.
		 * 		  
		 * @param[out]	dest		Previously allocated buffer to read data into.
		 * @param[in]	mipLevel	(optional) Mipmap level to read from.
		 * @param[in]	face		(optional) Texture face to read from.
		 */
		virtual void readData(PixelData& dest, UINT32 mipLevel = 0, UINT32 face = 0) = 0;

		/**
		 * Writes data from the provided buffer into the texture buffer.
		 * 		  
		 * @param[in]	dest				Buffer to retrieve the data from.
		 * @param[in]	mipLevel			(optional) Mipmap level to write into.
		 * @param[in]	face				(optional) Texture face to write into.
		 * @param[in]	discardWholeBuffer	(optional) If true any existing texture data will be discard. This can improve 
		 *									performance of the write operation.
		 */
		virtual void writeData(const PixelData& src, UINT32 mipLevel = 0, UINT32 face = 0, bool discardWholeBuffer = false) = 0;

		/**
		 * Returns true if the texture can be bound to a shader.
		 *
		 * @note	This is only false for some rare special cases (e.g. AA render texture in DX9). Internal method.
		 */
		virtual bool isBindableAsShaderResource() const { return true; }

		/**	Returns properties that contain information about the texture. */
		const TextureProperties& getProperties() const { return mProperties; }

		/************************************************************************/
		/* 								TEXTURE VIEW                      		*/
		/************************************************************************/

		/**
		 * Requests a texture view for the specified mip and array ranges. Returns an existing view of one for the specified
		 * ranges already exists, otherwise creates a new one. You must release all views by calling releaseView() when done.
		 *
		 * @note	Core thread only.
		 */
		static TextureViewPtr requestView(const SPtr<TextureCore>& texture, UINT32 mostDetailMip, UINT32 numMips,
			UINT32 firstArraySlice, UINT32 numArraySlices, GpuViewUsage usage);

		/**
		 * Releases the view. View won't actually get destroyed until all references to it are released.
		 *
		 * @note	Core thread only.
		 */
		static void releaseView(const TextureViewPtr& view);

	protected:
		/** @copydoc lock */
		virtual PixelData lockImpl(GpuLockOptions options, UINT32 mipLevel = 0, UINT32 face = 0) = 0;

		/** @copydoc unlock */
		virtual void unlockImpl() = 0;

		/** @copydoc copy */
		virtual void copyImpl(UINT32 srcFace, UINT32 srcMipLevel, UINT32 destFace, UINT32 destMipLevel, 
			const SPtr<TextureCore>& target) = 0;

		/************************************************************************/
		/* 								TEXTURE VIEW                      		*/
		/************************************************************************/

		/**	Creates a new empty/undefined texture view. */
		virtual TextureViewPtr createView(const SPtr<TextureCore>& texture, const TEXTURE_VIEW_DESC& desc);

		/**
		 * Releases all internal texture view references. Views won't get destroyed if there are external references still 
		 * held.
		 */
		void clearBufferViews();

		/** Holds a single texture view with a usage reference count. */
		struct TextureViewReference
		{
			TextureViewReference(TextureViewPtr _view)
				:view(_view), refCount(0)
			{ }

			TextureViewPtr view;
			UINT32 refCount;
		};

		UnorderedMap<TEXTURE_VIEW_DESC, TextureViewReference*, TextureView::HashFunction, TextureView::EqualFunction> mTextureViews;
		TextureProperties mProperties;
		PixelDataPtr mInitData;
	};

	/** @endcond */

	/**
	 * Abstract class representing a texture. Specific render systems have their own Texture implementations. Internally
	 * represented as one or more surfaces with pixels in a certain number of dimensions, backed by a hardware buffer.
	 *
	 * @note	Sim thread.
	 */
    class BS_CORE_EXPORT Texture : public Resource
    {
    public:
		/**
		 * Updates the texture with new data. The actual write will be queued for later execution on the core thread. 
		 * Provided data buffer will be locked until the operation completes.
		 *
		 * @param[in]	accessor	Accessor to queue the operation on.
		 * @return					Async operation object you can use to track operation completion.
		 *
		 * @see		TextureCore::writeSubresource
		 */
		AsyncOp writeSubresource(CoreAccessor& accessor, UINT32 subresourceIdx, const PixelDataPtr& data, 
			bool discardEntireBuffer);

		/**
		 * Reads internal texture data to the provided previously allocated buffer. The read is queued for execution on the
		 * core thread and not executed immediately. Provided data buffer will be locked until the operation completes.
		 *
		 * @param[in]	accessor	Accessor to queue the operation on.
		 * @return					Async operation object you can use to track operation completion.
		 *
		 * @see		TextureCore::readSubresource
		 */
		AsyncOp readSubresource(CoreAccessor& accessor, UINT32 subresourceIdx, const PixelDataPtr& data);

		/**
		 * Reads data from the cached system memory texture buffer into the provided buffer. 
		 * 		  
		 * @param[out]	dest		Previously allocated buffer to read data into.
		 * @param[in]	mipLevel	(optional) Mipmap level to read from.
		 * @param[in]	face		(optional) Texture face to read from.
		 *
		 * @note	
		 * The data read is the cached texture data. Any data written to the texture from the GPU or core thread will not 
		 * be reflected in this data. Use readSubresource() if you require those changes.
		 * @note
		 * The texture must have been created with TU_CPUCACHED usage otherwise this method will not return any data.
		 */
		void readData(PixelData& dest, UINT32 mipLevel = 0, UINT32 face = 0);

		/**	Returns properties that contain information about the texture. */
		const TextureProperties& getProperties() const { return mProperties; }

		/**	Retrieves a core implementation of a texture usable only from the core thread. */
		SPtr<TextureCore> getCore() const;

		/************************************************************************/
		/* 								STATICS		                     		*/
		/************************************************************************/

		/**
		 * Creates a new empty texture.
		 *
		 * @param[in]	texType				Type of the texture.
		 * @param[in]	width				Width of the texture in pixels.
		 * @param[in]	height				Height of the texture in pixels.
		 * @param[in]	depth				Depth of the texture in pixels (Must be 1 for 2D textures).
		 * @param[in]	numMips				Number of mip-maps the texture has. This number excludes the full resolution map.
		 * @param[in]	format				Format of the pixels.
		 * @param[in]	usage				Describes how we plan on using the texture in the pipeline.
		 * @param[in]	hwGammaCorrection	If true the texture data is assumed to have been gamma corrected and will be
		 *									converted back to linear space when sampled on GPU.
		 * @param[in]	multisampleCount	If higher than 1, texture containing multiple samples per pixel is created.
		 */
		static HTexture create(TextureType texType, UINT32 width, UINT32 height, UINT32 depth,
			int numMips, PixelFormat format, int usage = TU_DEFAULT,
			bool hwGammaCorrection = false, UINT32 multisampleCount = 0);


		/**
		 * Creates a new empty texture.
		 *
		 * @param[in]	texType				Type of the texture.
		 * @param[in]	width				Width of the texture in pixels.
		 * @param[in]	height				Height of the texture in pixels.
		 * @param[in]	numMips				Number of mip-maps the texture has. This number excludes the full resolution map.
		 * @param[in]	format				Format of the pixels.
		 * @param[in]	usage				Describes planned texture use.
		 * @param[in]	hwGammaCorrection	If true the texture data is assumed to have been gamma corrected and will be
		 *									converted back to linear space when sampled on GPU.
		 * @param[in]	multisampleCount	If higher than 1, texture containing multiple samples per pixel is created.
		 */
		static HTexture create(TextureType texType, UINT32 width, UINT32 height, int numMips,
			PixelFormat format, int usage = TU_DEFAULT,
			bool hwGammaCorrection = false, UINT32 multisampleCount = 0);

		/**
		 * Creates a new 2D or 3D texture initialized using the provided pixel data. Texture will not have any mipmaps.
		 *
		 * @param[in]	pixelData			Data to initialize the texture width.
		 * @param[in]	usage				Describes planned texture use.
		 * @param[in]	hwGammaCorrection	If true the texture data is assumed to have been gamma corrected and will be
		 *									converted back to linear space when sampled on GPU.
		 */
		static HTexture create(const PixelDataPtr& pixelData, int usage = TU_DEFAULT, bool hwGammaCorrection = false);

		/**
		 * @copydoc	create(TextureType, UINT32, UINT32, UINT32, int, PixelFormat, int, bool, UINT32)
		 *
		 * @note	Internal method. Creates a texture pointer without a handle. Use create() for normal usage.
		 */
		static TexturePtr _createPtr(TextureType texType, UINT32 width, UINT32 height, UINT32 depth,
			int num_mips, PixelFormat format, int usage = TU_DEFAULT,
			bool hwGammaCorrection = false, UINT32 multisampleCount = 0);

		/**
		 * @copydoc	create(TextureType, UINT32, UINT32, int, PixelFormat, int, bool, UINT32)
		 *
		 * @note	Internal method. Creates a texture pointer without a handle. Use create() for normal usage.
		 */
		static TexturePtr _createPtr(TextureType texType, UINT32 width, UINT32 height, int num_mips,
			PixelFormat format, int usage = TU_DEFAULT, bool hwGammaCorrection = false, UINT32 multisampleCount = 0);

		/**
		 * @copydoc	create(const PixelDataPtr&, int, bool)
		 *
		 * @note	Internal method. Creates a texture pointer without a handle. Use create() for normal usage.
		 */
		static TexturePtr _createPtr(const PixelDataPtr& pixelData, int usage = TU_DEFAULT, bool hwGammaCorrection = false);

    protected:
		friend class TextureManager;

		Texture(TextureType textureType, UINT32 width, UINT32 height, UINT32 depth, UINT32 numMipmaps,
			PixelFormat format, int usage, bool hwGamma, UINT32 multisampleCount);

		Texture(const PixelDataPtr& pixelData, int usage, bool hwGamma);

		/** @copydoc Resource::initialize */
		void initialize() override;

		/** @copydoc CoreObject::createCore */
		SPtr<CoreObjectCore> createCore() const override;

		/** @copydoc Resource::calculateSize */
		UINT32 calculateSize() const;

		/**
		 * Creates buffers used for caching of CPU texture data.
		 *
		 * @note	Make sure to initialize all texture properties before calling this.
		 */
		void createCPUBuffers();

		/**	Updates the cached CPU buffers with new data. */
		void updateCPUBuffers(UINT32 subresourceIdx, const PixelData& data);

	protected:
		Vector<PixelDataPtr> mCPUSubresourceData;
		TextureProperties mProperties;
		mutable PixelDataPtr mInitData;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		Texture(); // Serialization only

		friend class TextureRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
    };

	/** @} */
}