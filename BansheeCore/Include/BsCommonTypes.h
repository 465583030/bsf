//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

namespace BansheeEngine 
{
	/** @addtogroup Utility-Core
	 *  @{
	 */

	/**	Factors used when blending new pixels with existing pixels. */
    enum BlendFactor
    {
		BF_ONE, /**< Use a value of one for all pixel components. */
		BF_ZERO, /**< Use a value of zero for all pixel components. */
		BF_DEST_COLOR, /**< Use the existing pixel value. */
		BF_SOURCE_COLOR, /**< Use the newly generated pixel value. */
		BF_INV_DEST_COLOR, /**< Use the inverse of the existing value. */
		BF_INV_SOURCE_COLOR, /**< Use the inverse of the newly generated pixel value. */
		BF_DEST_ALPHA, /**< Use the existing alpha value. */
		BF_SOURCE_ALPHA, /**< Use the newly generated alpha value. */
		BF_INV_DEST_ALPHA, /**< Use the inverse of the existing alpha value. */
		BF_INV_SOURCE_ALPHA /**< Use the inverse of the newly generated alpha value. */
    };

	/**	Operations that determines how are blending factors combined. */
	enum BlendOperation
	{
		BO_ADD, /**< Blend factors are added together. */
		BO_SUBTRACT, /**< Blend factors are subtracted in "srcFactor - dstFactor" order. */
		BO_REVERSE_SUBTRACT, /**< Blend factors are subtracted in "dstFactor - srcFactor" order. */
		BO_MIN, /**< Minimum of the two factors is chosen. */
		BO_MAX /**< Maximum of the two factors is chosen. */
	};

	/**	Comparison functions used for the depth/stencil buffer. */
    enum CompareFunction
    {
		CMPF_ALWAYS_FAIL, /**< Operation will always fail. */
		CMPF_ALWAYS_PASS, /**< Operation will always pass. */
		CMPF_LESS, /**< Operation will pass if the new value is less than existing value. */
        CMPF_LESS_EQUAL, /**< Operation will pass if the new value is less or equal than existing value. */
        CMPF_EQUAL, /**< Operation will pass if the new value is equal to the existing value. */
        CMPF_NOT_EQUAL, /**< Operation will pass if the new value is not equal to the existing value. */
        CMPF_GREATER_EQUAL, /**< Operation will pass if the new value greater or equal than the existing value. */
        CMPF_GREATER /**< Operation will pass if the new value greater than the existing value. */
    };

	/**
	 * Types of texture addressing modes that determine what happens when texture coordinates are outside of the valid range.
	 */
    enum TextureAddressingMode
    {
		TAM_WRAP, /**< Coordinates wrap back to the valid range. */
		TAM_MIRROR, /**< Coordinates flip every time the size of the valid range is passed. */
		TAM_CLAMP, /**< Coordinates are clamped within the valid range. */
		TAM_BORDER /**< Coordinates outside of the valid range will return a separately set border color. */
    };

	/**	Types of available filtering situations. */
    enum FilterType
    {
		FT_MIN, /**< The filter used when shrinking a texture. */
        FT_MAG, /**< The filter used when magnifying a texture. */
        FT_MIP /**< The filter used when filtering between mipmaps. */
    };

	/**	Filtering options for textures. */
    enum FilterOptions
    {
		FO_NONE = 0, /**< Use no filtering. Only relevant for mipmap filtering. */
		FO_POINT = 1, /**< Filter using the nearest found pixel. Most basic filtering. */
		FO_LINEAR = 2, /**< Average a 2x2 pixel area, signifies bilinear filtering for texture, trilinear for mipmaps. */
		FO_ANISOTROPIC = 3, /**< More advanced filtering that improves quality when viewing textures at a steep angle */
		FO_USE_COMPARISON = 4 /**< Specifies that the sampled values will be compared against existing sampled data. Should be OR-ed with other filtering options. */
    };

	/**	Types of frame buffers. */
	enum FrameBufferType
	{
		FBT_COLOR = 0x1,
		FBT_DEPTH = 0x2,
		FBT_STENCIL = 0x4
	};

	/**
	 * Types of culling that determine how (and if) hardware discards faces with certain winding order. Winding order can
	 * be used for determining front or back facing polygons by checking the order of its vertices from the render 
	 * perspective.
	 */
    enum CullingMode
    {
		CULL_NONE = 0, /**< Hardware performs no culling and renders both sides. */
		CULL_CLOCKWISE = 1, /**< Hardware culls faces that have a clockwise vertex ordering. */
        CULL_COUNTERCLOCKWISE = 2 /**< Hardware culls faces that have a counter-clockwise vertex ordering. */
    };

	/**	Polygon mode to use when rasterizing. */
    enum PolygonMode
    {
		PM_WIREFRAME = 1, /**< Render as wireframe showing only polygon outlines. */
        PM_SOLID = 2 /**< Render as solid showing whole polygons. */
    };

	/**	Types of action that can happen on the stencil buffer. */
	enum StencilOperation
	{
		SOP_KEEP, /**< Leave the stencil buffer unchanged. */
		SOP_ZERO, /**< Set the stencil value to zero. */
		SOP_REPLACE, /**< Replace the stencil value with the reference value. */
		SOP_INCREMENT, /**< Increase the stencil value by 1, clamping at the maximum value. */
		SOP_DECREMENT, /**< Decrease the stencil value by 1, clamping at 0. */
		SOP_INCREMENT_WRAP, /**< Increase the stencil value by 1, wrapping back to 0 when incrementing past the maximum value. */
		SOP_DECREMENT_WRAP, /**< Decrease the stencil value by 1, wrapping when decrementing 0. */
		SOP_INVERT /**< Invert the bits of the stencil buffer. */
	};

	/** These values represent a hint to the driver when locking a hardware buffer. */
	enum GpuLockOptions
	{
		/**
		 * Allows you to write to the buffer. Can cause a CPU-GPU sync point so avoid using it often (i.e. every frame) as 
		 * that might limit your performance significantly.
		 */
        GBL_READ_WRITE,
		/**
		 * Allows you to write to the buffer. Tells the driver to completely discard the contents of the buffer you are 
		 * writing to. The driver will (most likely) internally allocate another buffer with same specifications (which is
		 * fairly fast) and you will avoid CPU-GPU stalls.
		 */
		GBL_WRITE_ONLY_DISCARD,
		/**  Allows you to read from a buffer. Be aware that reading is usually a very slow operation. */
		GBL_READ_ONLY,
		/**
		 * Allows you to write to the buffer. Guarantees the driver that you will not be updating any part of the buffer 
		 * that is currently used. This will also avoid CPU-GPU stalls, without requiring you to discard the entire buffer.
		 * However it is hard to guarantee when GPU has finished using a buffer.
		 */
        GBL_WRITE_ONLY_NO_OVERWRITE,
		/** Allows you to both read and write to a buffer. */
		GBL_WRITE_ONLY	
	};

	/**
	 * Values that represent hardware buffer usage. These usually determine in what type of memory is buffer placed in,
	 * however that depends on rendering API.
	 */
	enum GpuBufferUsage 
	{
		/** 
		 * Signifies that you don't plan on modifying the buffer often (or at all)	after creation. Modifying such buffer 
		 * will involve a larger performance hit.
		 */
        GBU_STATIC = 1,
		/** Signifies that you will modify this buffer fairly often. */
		GBU_DYNAMIC = 2
	};

	/** Types of generic GPU buffers that may be attached to GPU programs. */
	enum GpuBufferType
	{
		/** 
		 * Buffer containing an array of structures. Structure parameters can usually be easily accessed from within the
		 * GPU program.
		 */
		GBT_STRUCTURED,
		/** Buffer containing raw bytes. It is up to the user to interpret the data. */
		GBT_RAW,
		/**
		 * Special type of buffer allowing you to specify arguments for draw operations inside the buffer instead of 
		 * providing them directly. Useful when you want to control drawing directly from GPU.
		 */
		GBT_INDIRECTARGUMENT,
		/** A stack-like buffer that allows you to add or remove elements to/from the buffer from within the GPU program. */
		GBT_APPENDCONSUME
	};

	/** Different types of GPU views that control how GPU sees a hardware buffer. */
	enum GpuViewUsage
	{
		/** Buffer is seen as a default shader resource, used primarily for reading. (e.g. a texture for sampling) */
		GVU_DEFAULT = 0x01,
		/** Buffer is seen as a render target that color pixels will be written to after pixel shader stage. */
		GVU_RENDERTARGET = 0x02,
		/** Buffer is seen as a depth stencil target that depth and stencil information is written to. */
		GVU_DEPTHSTENCIL = 0x04,
		/** Buffer that allows you to write to any part of it from within a GPU program. */
		GVU_RANDOMWRITE = 0x08
	};

	/** Type of parameter block usages. Signifies how often will parameter blocks be changed. */
	enum GpuParamBlockUsage
	{
		GPBU_STATIC, /**< Buffer will be rarely, if ever, updated. */
		GPBU_DYNAMIC /**< Buffer will be updated often (e.g. every frame). */
	};

	/** Type of a parameter in a GPU program. */
	enum GpuParamType
	{
		GPT_DATA, /**< Raw data type like float, Vector3, Color, etc. */
		GPT_TEXTURE, /**< Texture type (2D, 3D, cube, etc.) */
		GPT_BUFFER, /**< Data buffer (raw, structured, etc.) */
		GPT_SAMPLER /**< Sampler type (2D, 3D, cube, etc.) */
	};

	/**	Type of GPU data parameters that can be used as inputs to a GPU program. */
	enum GpuParamDataType
	{
		GPDT_FLOAT1 = 1,
		GPDT_FLOAT2 = 2,
		GPDT_FLOAT3 = 3,
		GPDT_FLOAT4 = 4,
		GPDT_MATRIX_2X2 = 11,
		GPDT_MATRIX_2X3 = 12,
		GPDT_MATRIX_2X4 = 13,
		GPDT_MATRIX_3X2 = 14,
		GPDT_MATRIX_3X3 = 15,
		GPDT_MATRIX_3X4 = 16,
		GPDT_MATRIX_4X2 = 17,
		GPDT_MATRIX_4X3 = 18,
		GPDT_MATRIX_4X4 = 19,
		GPDT_INT1 = 20,
		GPDT_INT2 = 21,
		GPDT_INT3 = 22,
		GPDT_INT4 = 23,
		GPDT_BOOL = 24,
		GPDT_STRUCT = 25,
		GPDT_COLOR = 26, // Same as GPDT_FLOAT4, but can be used to better deduce usage
		GPDT_COUNT = 27, // Keep at end before GPDT_UNKNOWN
		GPDT_UNKNOWN = 0xffff
	};

	/**	Contains data about a type used for GPU data parameters. */
	struct GpuParamDataTypeInfo
	{
		UINT32 baseTypeSize;
		UINT32 size;
		UINT32 alignment;
		UINT32 numRows;
		UINT32 numColumns;
	};

	/**	Contains a lookup table for various information of all types used for data GPU parameters. Sizes are in bytes. */
	struct GpuDataParamInfos
	{
		GpuDataParamInfos()
		{
			memset(lookup, 0, sizeof(lookup));

			lookup[(UINT32)GPDT_FLOAT1] = { 4, 4, 4, 1, 1 };
			lookup[(UINT32)GPDT_FLOAT2] = { 4, 8, 8, 1, 2 };
			lookup[(UINT32)GPDT_FLOAT3] = { 4, 16, 16, 1, 3 };
			lookup[(UINT32)GPDT_FLOAT4] = { 4, 16, 16, 1, 4 };
			lookup[(UINT32)GPDT_MATRIX_2X2] = { 4, 16, 8, 2, 2 };
			lookup[(UINT32)GPDT_MATRIX_2X3] = { 4, 32, 16, 2, 3 };
			lookup[(UINT32)GPDT_MATRIX_2X4] = { 4, 32, 16, 2, 4 };
			lookup[(UINT32)GPDT_MATRIX_3X2] = { 4, 24, 8, 3, 2 };
			lookup[(UINT32)GPDT_MATRIX_3X3] = { 4, 48, 16, 3, 3 };
			lookup[(UINT32)GPDT_MATRIX_3X4] = { 4, 48, 16, 3, 4 };
			lookup[(UINT32)GPDT_MATRIX_4X2] = { 4, 32, 8, 4, 2 };
			lookup[(UINT32)GPDT_MATRIX_4X3] = { 4, 64, 16, 4, 3 };
			lookup[(UINT32)GPDT_MATRIX_4X4] = { 4, 64, 16, 4, 4 };
			lookup[(UINT32)GPDT_INT1] = { 4, 4, 4, 1, 1 };
			lookup[(UINT32)GPDT_INT2] = { 4, 8, 8, 1, 2 };
			lookup[(UINT32)GPDT_INT3] = { 4, 12, 16, 1, 3 };
			lookup[(UINT32)GPDT_INT4] = { 4, 16, 16, 1, 4 };
			lookup[(UINT32)GPDT_BOOL] = { 4, 4, 4, 1, 1 };
		}

		GpuParamDataTypeInfo lookup[GPDT_COUNT];
	};

	/**	Type of GPU object parameters that can be used as inputs to a GPU program. */
	enum GpuParamObjectType
	{
		GPOT_SAMPLER1D = 1,
		GPOT_SAMPLER2D = 2,
		GPOT_SAMPLER3D = 3,
		GPOT_SAMPLERCUBE = 4,
		GPOT_SAMPLER2DMS = 5,
		GPOT_TEXTURE1D = 11,
		GPOT_TEXTURE2D = 12,
		GPOT_TEXTURE3D = 13,
		GPOT_TEXTURECUBE = 14,
		GPOT_TEXTURE2DMS = 15,
		GPOT_BYTE_BUFFER = 32,
		GPOT_STRUCTURED_BUFFER = 33,
		GPOT_RWTYPED_BUFFER = 41,
		GPOT_RWBYTE_BUFFER = 42,
		GPOT_RWSTRUCTURED_BUFFER = 43,
		GPOT_RWSTRUCTURED_BUFFER_WITH_COUNTER = 44,
		GPOT_RWAPPEND_BUFFER = 45,
		GPOT_RWCONSUME_BUFFER = 46,
		GPOT_UNKNOWN = 0xffff
	};

	/** These values represent a hint to the driver when writing to a GPU buffer. */
	enum class BufferWriteType
	{
		/**
		 * Default flag with least restrictions. Can cause a CPU-GPU sync point so avoid using it often (i.e. every frame)
		 * as that might limit your performance significantly.
		 */
		Normal,
		/**
		 * Tells the driver to completely discard the contents of the buffer you are writing to. The driver will (most
		 * likely) internally allocate another buffer with same specifications (which is fairly fast) and you will avoid 
		 * CPU-GPU stalls. 
		 */
		Discard,
		/**
		 * Guarantees the driver that you will not be updating any part of the buffer that is currently used. This will 
		 * also avoid CPU-GPU stalls, without requiring you to discard the entire buffer. However it is hard to guarantee 
		 * when GPU has finished using a buffer.
		 */
		NoOverwrite
	};

	/**
	 * Suggested queue priority numbers used for sorting objects in the render queue. Objects with higher priority will
	 * be renderer sooner.
	 */
	enum class QueuePriority
	{
		Opaque = 100000,
		Transparent = 90000,
		Skybox = 80000,
		Overlay = 70000
	};

	/** Type of sorting to perform on an object when added to a render queue. */
	enum class QueueSortType
	{
		FrontToBack, /**< All objects with the same priority will be rendered front to back based on their center. */
		BackToFront, /**< All objects with the same priority will be rendered back to front based on their center. */
		None /**< Objects will not be sorted and will be processed in the order they were added to the queue. */
	};

	/**	Flags that may be assigned to a shader that let the renderer know how to interpret the shader. */
	enum class ShaderFlags
	{
		Transparent = 0x1 /**< Signifies that the shader is rendering a transparent object. */
	};

	/**	Enum that defines possible window border styles. */
	enum class WindowBorder
	{
		Normal,
		None,
		Fixed
	};

	/**	Texture addressing mode, per component. */
	struct UVWAddressingMode
	{
		UVWAddressingMode()
			:u(TAM_WRAP), v(TAM_WRAP), w(TAM_WRAP)
		{ }

		bool operator==(const UVWAddressingMode& rhs) const
		{
			return u == rhs.u && v == rhs.v && w == rhs.w;
		}

		TextureAddressingMode u, v, w;
	};
    
	/**	References a subset of surfaces within a texture. */
	struct TextureSurface
	{
		TextureSurface(UINT32 mipLevel = 0, UINT32 numMipLevels = 1, 
			UINT32 arraySlice = 0, UINT32 numArraySlices = 1)
			:mipLevel(mipLevel), numMipLevels(numMipLevels), 
			arraySlice(arraySlice), numArraySlices(numArraySlices)
		{ }

		UINT32 mipLevel;
		UINT32 numMipLevels;
		UINT32 arraySlice;
		UINT32 numArraySlices;
	};

	/** Helper class for syncing dirty data from sim CoreObject to core CoreObject and other way around. */
	class CoreSyncData
	{
	public:
		CoreSyncData()
			:data(nullptr), size(0)
		{ }

		CoreSyncData(UINT8* data, UINT32 size)
			:data(data), size(size)
		{ }

		/** Gets the internal data and checks the data is of valid size. */
		template<class T>
		const T& getData() const
		{
			assert(sizeof(T) == size);

			return *(T*)data;
		}

		/**	Returns a pointer to internal data buffer. */
		UINT8* getBuffer() const { return data; }

		/**	Returns the size of the internal data buffer. */
		UINT32 getBufferSize() const { return size; }

	private:
		UINT8* data;
		UINT32 size;
	};

	typedef Map<String, String> NameValuePairList;

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(TextureSurface);
	/** @endcond */

	/** @} */
}