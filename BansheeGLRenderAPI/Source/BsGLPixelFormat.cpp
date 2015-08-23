#include "BsGLPixelFormat.h"
#include "BsRenderAPI.h"
#include "BsBitwise.h"
#include "BsDebug.h"

namespace BansheeEngine  
{
    GLenum GLPixelUtil::getGLOriginFormat(PixelFormat mFormat)
    {
        switch(mFormat)
        {
			case PF_R8:
				return GL_RED;
			case PF_R8G8:
				return GL_RG;
            case PF_R8G8B8:
                return GL_RGB;
            case PF_B8G8R8:
                return GL_BGR;
			case PF_R8G8B8X8:
			case PF_R8G8B8A8:
			case PF_A8R8G8B8:
			case PF_X8R8G8B8:
                return GL_RGBA;
            case PF_B8G8R8A8:
			case PF_B8G8R8X8:
			case PF_A8B8G8R8:
			case PF_X8B8G8R8:
                return GL_BGRA;
			case PF_FLOAT16_R:
                return GL_RED;
			case PF_FLOAT16_RG:
				return GL_RG;
            case PF_FLOAT16_RGB:
                return GL_RGB;
            case PF_FLOAT16_RGBA:
                return GL_RGBA;
			case PF_FLOAT32_R:
                return GL_RED;
			case PF_FLOAT32_RG:
				return GL_RG;
            case PF_FLOAT32_RGB:
                return GL_RGB;
            case PF_FLOAT32_RGBA:
                return GL_RGBA;
			case PF_FLOAT_R11G11B10:
				return GL_RGB;
			case PF_UNORM_R10G10B10A2:
				return GL_RGBA;
            case PF_BC1:
            case PF_BC1a:
            case PF_BC3:
			case PF_BC7:
				return GL_RGBA;
			case PF_BC4:
				return GL_RED;
			case PF_BC5:
				return GL_RG;
			case PF_BC6H:
				return GL_RGB;
            default:
                return 0;
        }
    }

    GLenum GLPixelUtil::getGLOriginDataType(PixelFormat format)
    {
        switch(format)
        {
			case PF_R8:
            case PF_R8G8B8:
            case PF_B8G8R8:
			case PF_R8G8:
				return GL_UNSIGNED_BYTE;
			case PF_X8B8G8R8:
			case PF_A8B8G8R8:
			case PF_X8R8G8B8:
            case PF_A8R8G8B8:
				return GL_UNSIGNED_INT_8_8_8_8;
            case PF_B8G8R8A8:
			case PF_B8G8R8X8:
			case PF_R8G8B8A8:
			case PF_R8G8B8X8:
				return GL_UNSIGNED_INT_8_8_8_8_REV;
			case PF_FLOAT16_R:
			case PF_FLOAT16_RG:
            case PF_FLOAT16_RGB:
            case PF_FLOAT16_RGBA:
                return GL_HALF_FLOAT;
			case PF_FLOAT32_R:
			case PF_FLOAT32_RG:
            case PF_FLOAT32_RGB:
            case PF_FLOAT32_RGBA:
                return GL_FLOAT;
			case PF_FLOAT_R11G11B10:
				return GL_UNSIGNED_INT_10F_11F_11F_REV;
			case PF_UNORM_R10G10B10A2:
				return GL_UNSIGNED_INT_2_10_10_10_REV;
            default:
                return 0;
        }
    }

	GLenum GLPixelUtil::getBaseFormatFromCompressedInternalFormat(GLenum internalFormat)
	{
		 switch(internalFormat) 
		 {
		 case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
		 case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		 case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
		 case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
		 case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
		 case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		 case GL_COMPRESSED_RGBA_BPTC_UNORM:
		 case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
			 return GL_RGBA;
		 case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		 case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
		 case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
		 case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
			 return GL_RGB;
		 case GL_COMPRESSED_RED_RGTC1:
		 case GL_COMPRESSED_SIGNED_RED_RGTC1:
			 return GL_RED;
		 case GL_COMPRESSED_RG_RGTC2:
		 case GL_COMPRESSED_SIGNED_RG_RGTC2:
			 return GL_RG;
		 }

		 return GL_RGBA;
	}
    
    GLenum GLPixelUtil::getGLInternalFormat(PixelFormat mFormat, bool hwGamma)
    {
        switch(mFormat) {
            case PF_R8:
                return GL_R8;
			case PF_R8G8:
				return GL_RG8;
            case PF_R8G8B8:
            case PF_B8G8R8:
			case PF_B8G8R8X8:
			case PF_R8G8B8X8:
				if (hwGamma)
					return GL_SRGB8;
				else
					return GL_RGB8;
            case PF_B8G8R8A8:
			case PF_R8G8B8A8:
				if (hwGamma)
					return GL_SRGB8_ALPHA8;
				else
					return GL_RGBA8;
			case PF_FLOAT16_R:
				return GL_R16F;
            case PF_FLOAT16_RGB:
                return GL_RGB16F;
			case PF_FLOAT16_RG: 
				return GL_RG16F;
            case PF_FLOAT16_RGBA:
                return GL_RGBA16F;
			case PF_FLOAT32_R:
				return GL_R32F;
			case PF_FLOAT32_RG:
				return GL_RG32F;
            case PF_FLOAT32_RGB:
                return GL_RGB32F;
            case PF_FLOAT32_RGBA:
                return GL_RGBA32F;
			case PF_BC1a:
			case PF_BC1:
				if (hwGamma)
					return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
				else
					return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            case PF_BC2:
				if (hwGamma)
					return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
				else
	                return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            case PF_BC3:
				if (hwGamma)
					return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
				else
	                return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			case PF_BC4:
				return GL_COMPRESSED_RED_RGTC1;
			case PF_BC5:
				return GL_COMPRESSED_RG_RGTC2;
			case PF_BC6H:
				return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
			case PF_BC7:
				if (hwGamma)
					return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
				else
					return GL_COMPRESSED_RGBA_BPTC_UNORM;
			case PF_D16:
				return GL_DEPTH_COMPONENT16;
			case PF_D32:
				return GL_DEPTH_COMPONENT32F;
			case PF_D24S8:
				return GL_DEPTH24_STENCIL8;
			case PF_D32_S8X24:
				return GL_DEPTH32F_STENCIL8;
			case PF_FLOAT_R11G11B10:
				return GL_R11F_G11F_B10F;
			case PF_UNORM_R10G10B10A2:
				return GL_RGB10_A2;
            default:
                return GL_NONE;
        }
    }

    GLenum GLPixelUtil::getClosestGLInternalFormat(PixelFormat mFormat, bool hwGamma)
    {
        GLenum format = getGLInternalFormat(mFormat, hwGamma);
        if(format == GL_NONE)
		{
			if (hwGamma)
				return GL_SRGB8;
			else
				return GL_RGBA8;
		}
        else
            return format;
    }

	GLenum GLPixelUtil::getDepthStencilTypeFromFormat(PixelFormat mFormat)
	{
		switch(mFormat)
		{
		case PF_D32_S8X24:
			return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		case PF_D24S8:
			return GL_UNSIGNED_INT_24_8;
		case PF_D32:
			return GL_FLOAT;
		case PF_D16:
			return GL_UNSIGNED_SHORT;
		}

		BS_EXCEPT(InvalidParametersException, "Invalid depth stencil format");
	}
		
	PixelFormat GLPixelUtil::getClosestEngineFormat(GLenum fmt)
	{
		switch(fmt) 
		{
		case GL_R8:
			return PF_R8;
		case GL_RG8:
			return PF_R8G8;
		case GL_RGB8:
		case GL_SRGB8:
			return PF_R8G8B8X8;
		case GL_RGBA8:
		case GL_SRGB8_ALPHA8:
			return PF_R8G8B8A8;
		case GL_R16F:
			return PF_FLOAT16_R;
		case GL_RG16F:
			return PF_FLOAT16_RG;
		case GL_RG32F:
			return PF_FLOAT32_RG;
		case GL_R32F:
			return PF_FLOAT32_R;
		case GL_RGB16F:
			return PF_FLOAT16_RGB;
		case GL_RGBA16F:
			return PF_FLOAT16_RGBA;
		case GL_RGB32F:
			return PF_FLOAT32_RGB;
		case GL_RGBA32F:
			return PF_FLOAT32_RGBA;
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
			return PF_BC1;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
			return PF_BC2;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
			return PF_BC3;
		case GL_COMPRESSED_RED_RGTC1:
		case GL_COMPRESSED_SIGNED_RED_RGTC1:
			return PF_BC4;
		case GL_COMPRESSED_RG_RGTC2:
		case GL_COMPRESSED_SIGNED_RG_RGTC2:
			return PF_BC5;
		case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
		case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
			return PF_BC6H;
		case GL_COMPRESSED_RGBA_BPTC_UNORM:
		case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
			return PF_BC7;
		case GL_DEPTH_COMPONENT16:
			return PF_D16;
		case GL_DEPTH_COMPONENT32F:
			return PF_D32;
		case GL_DEPTH24_STENCIL8:
			return PF_D24S8;
		case GL_DEPTH32F_STENCIL8:
			return PF_D32_S8X24;
		case GL_RGB10_A2:
			return PF_UNORM_R10G10B10A2;
		case GL_R11F_G11F_B10F:
			return PF_FLOAT_R11G11B10;
		default:
			return PF_R8G8B8A8;
		};
	}

	PixelFormat GLPixelUtil::getClosestValidFormat(PixelFormat fmt)
	{
		switch(fmt) 
		{
		case PF_R8:
			return PF_R8;
		case PF_R8G8:
			return PF_R8G8;
		case PF_R8G8B8:
		case PF_B8G8R8:
		case PF_B8G8R8X8:
		case PF_R8G8B8X8:
			return PF_R8G8B8X8;
		case PF_B8G8R8A8:
		case PF_R8G8B8A8:
			return PF_R8G8B8A8;
		case PF_FLOAT16_R:
			return PF_FLOAT16_R;
		case PF_FLOAT16_RGB:
			return PF_FLOAT16_RGB;
		case PF_FLOAT16_RG: 
			return PF_FLOAT16_RG;
		case PF_FLOAT16_RGBA:
			return PF_FLOAT16_RGBA;
		case PF_FLOAT32_R:
			return PF_FLOAT32_R;
		case PF_FLOAT32_RG:
			return PF_FLOAT32_RG;
		case PF_FLOAT32_RGB:
			return PF_FLOAT32_RGB;
		case PF_FLOAT32_RGBA:
			return PF_FLOAT32_RGBA;
		case PF_BC1:
			return PF_BC1;
		case PF_BC1a:
			return PF_BC1;
		case PF_BC3:
			return PF_BC3;
		case PF_BC4:
			return PF_BC4;
		case PF_BC5:
			return PF_BC5;
		case PF_BC6H:
			return PF_BC6H;
		case PF_BC7:
			return PF_BC7;
		case PF_D16:
			return PF_D16;
		case PF_D32:
			return PF_D32;
		case PF_D24S8:
			return PF_D24S8;
		case PF_D32_S8X24:
			return PF_D32_S8X24;
		case PF_UNORM_R10G10B10A2:
			return PF_UNORM_R10G10B10A2;
		case PF_FLOAT_R11G11B10:
			return PF_FLOAT_R11G11B10;
		default:
			return PF_UNKNOWN;
		}
	}
};
