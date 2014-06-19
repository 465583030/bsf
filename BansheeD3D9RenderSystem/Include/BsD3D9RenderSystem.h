#pragma once

#include "BsD3D9Prerequisites.h"
#include "BsString.h"
#include "BsRenderSystem.h"
#include "BsRenderSystemCapabilities.h"
#include "BsD3D9Mappings.h"

namespace BansheeEngine 
{
	/**
	Implementation of DirectX9 as a rendering system.
	*/
	class BS_D3D9_EXPORT D3D9RenderSystem : public RenderSystem
	{
	public:
		// constructor
		D3D9RenderSystem(HINSTANCE hInstance);
		// destructor
		~D3D9RenderSystem();

		/**
		 * @copydoc RenderSystem::getName()
		 */
		const String& getName() const;

		/**
		 * @copydoc RenderSystem::getShadingLanguageName()
		 */
		const String& getShadingLanguageName() const;

		/**
		 * @copydoc RenderSystem::setRenderTarget()
		 */
		void setRenderTarget(RenderTargetPtr target);

		/**
		 * @copydoc RenderSystem::bindGpuProgram()
		 */
		void bindGpuProgram(HGpuProgram prg);

		/**
		 * @copydoc RenderSystem::unbindGpuProgram()
		 */
		void unbindGpuProgram(GpuProgramType gptype);

		/**
		 * @copydoc RenderSystem::bindGpuParams()
		 */
		void bindGpuParams(GpuProgramType gptype, GpuParamsPtr params);

		/**
		 * @copydoc RenderSystem::setVertexBuffers()
		 */
		void setVertexBuffers(UINT32 index, VertexBufferPtr* buffers, UINT32 numBuffers);

		/**
		 * @copydoc RenderSystem::setIndexBuffer()
		 */
		void setIndexBuffer(const IndexBufferPtr& buffer);

		/**
		 * @copydoc RenderSystem::setVertexDeclaration()
		 */
		void setVertexDeclaration(VertexDeclarationPtr vertexDeclaration);

		/**
		 * @copydoc RenderSystem::setDrawOperation()
		 */
		void setDrawOperation(DrawOperationType op);

		/**
		 * @copydoc RenderSystem::setSamplerState()
		 */
		void setTexture(GpuProgramType gptype, UINT16 unit, bool enabled, const TexturePtr &texPtr);

		/**
		 * @copydoc RenderSystem::setSamplerState()
		 */
		void setSamplerState(GpuProgramType gptype, UINT16 unit, const SamplerStatePtr& state);

		/**
		 * @copydoc RenderSystem::setBlendState()
		 */
		void setBlendState(const BlendStatePtr& blendState);

		/**
		 * @copydoc RenderSystem::setRasterizerState()
		 */
		void setRasterizerState(const RasterizerStatePtr& rasterizerState);

		/**
		 * @copydoc RenderSystem::setDepthStencilState()
		 */
		void setDepthStencilState(const DepthStencilStatePtr& depthStencilState, UINT32 stencilRefValue);

		/**
		 * @copydoc RenderSystem::setViewport()
		 */
		void setViewport(Viewport vp);

		/**
		 * @copydoc RenderSystem::beginFrame()
		 */
		void beginFrame();

		/**
		 * @copydoc RenderSystem::endFrame()
		 */
		void endFrame();		

		/**
		 * @copydoc RenderSystem::draw()
		 */
		void draw(UINT32 vertexOffset, UINT32 vertexCount);

		/**
		 * @copydoc RenderSystem::drawIndexed()
		 */
		void drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount);

        void setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom);

		/**
		 * @copydoc RenderSystem::clearRenderTarget()
		 */
		void clearRenderTarget(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0);

		/**
		 * @copydoc RenderSystem::clearViewport()
		 */
		void clearViewport(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0);

		void convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest, bool forGpuProgram = false);

        float getHorizontalTexelOffset();
        float getVerticalTexelOffset();
        float getMinimumDepthInputValue();
        float getMaximumDepthInputValue();
		VertexElementType getColorVertexElementType() const;

		/************************************************************************/
		/* 				Internal use by DX9 RenderSystem only                   */
		/************************************************************************/

		static D3D9ResourceManager* getResourceManager();
		static D3D9DeviceManager* getDeviceManager();
		static IDirect3D9* getDirect3D9();
		static UINT	getResourceCreationDeviceCount();
		static IDirect3DDevice9* getResourceCreationDevice(UINT index);
		static IDirect3DDevice9* getActiveD3D9Device();

		void determineMultisampleSettings(IDirect3DDevice9* d3d9Device, UINT32 multisampleCount, const String& multisampleHint, D3DFORMAT d3dPixelFormat,
			bool fullScreen, D3DMULTISAMPLE_TYPE *outMultisampleType, DWORD *outMultisampleQuality) const;

		void registerWindow(RenderWindow& renderWindow);

	private:
		static D3D9RenderSystem* msD3D9RenderSystem;

		IDirect3D9*	mpD3D;
		bool mIsFrameInProgress;
		bool mRestoreFrameOnReset;

		mutable D3D9DriverList* mDriverList;
		D3D9Driver* mActiveD3DDriver;

		HINSTANCE mhInstance;

		UINT32 mViewportLeft, mViewportTop, mViewportWidth, mViewportHeight;
		RECT mScissorRect;

		DrawOperationType mCurrentDrawOperation;

		/// structure holding texture unit settings for every stage
		struct sD3DTextureStageDesc
		{
			/// the type of the texture
			D3D9Mappings::D3DTexType texType;
			/// which texCoordIndex to use
			size_t coordIndex;
			/// texture 
			IDirect3DBaseTexture9 *pTex;
			/// vertex texture 
			IDirect3DBaseTexture9 *pVertexTex;
		};

		UINT32 mNumTexStages;
		sD3DTextureStageDesc* mTexStageDesc;

		D3D9DriverList* getDirect3DDrivers() const;
				
		// state management methods, very primitive !!!
		HRESULT __SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
		HRESULT __SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);
		HRESULT __SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);

		HRESULT __SetFloatRenderState(D3DRENDERSTATETYPE state, float value)
		{
			return __SetRenderState(state, *((LPDWORD)(&value)));
		}

		/// return anisotropy level
		DWORD _getCurrentAnisotropy(UINT32 unit);
		bool _checkMultiSampleQuality(D3DMULTISAMPLE_TYPE type, DWORD *outQuality, D3DFORMAT format, UINT adapterNum, D3DDEVTYPE deviceType, BOOL fullScreen);
		
        D3D9HLSLProgramFactory* mHLSLProgramFactory;
		D3D9ResourceManager* mResourceManager;
		D3D9DeviceManager* mDeviceManager;

        /// Internal method for populating the capabilities structure
		virtual RenderSystemCapabilities* createRenderSystemCapabilities() const;
		RenderSystemCapabilities* updateRenderSystemCapabilities(D3D9RenderWindow* renderWindow);

		/** See RenderSystem definition */
		virtual void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps);

        void convertVertexShaderCaps(RenderSystemCapabilities* rsc) const;
        void convertPixelShaderCaps(RenderSystemCapabilities* rsc) const;

	protected:
		// I know that's a lot of friends, but I'd rather have friend classes than exposing the needed methods
		// as public interface. 
		friend class D3D9Texture;
		friend class D3D9RenderWindow;
		friend class D3D9Device;
		friend class D3D9TextureManager;
		friend class D3D9DeviceManager;		
		friend class D3D9RenderWindowManager;

		void initialize_internal(AsyncOp& asyncOp);
		void destroy_internal();

		void setClipPlanesImpl(const PlaneList& clipPlanes);	

		String getErrorDescription( long errorNumber ) const;

		void setClipPlane (UINT16 index, float A, float B, float C, float D);
		void enableClipPlane (UINT16 index, bool enable);

		HINSTANCE getInstanceHandle() const { return mhInstance; }

		/**
		 * @brief	Returns the D3D9 specific mode used for drawing, depending on the
		 * 			currently set draw operation;
		 */
		D3DPRIMITIVETYPE getD3D9PrimitiveType() const;

        /** Check whether or not filtering is supported for the precise texture format requested
        with the given usage options.
        */
        bool checkTextureFilteringSupported(TextureType ttype, PixelFormat format, int usage);

		/************************************************************************/
		/* 							Sampler states                     			*/
		/************************************************************************/

		/** Sets the texture addressing mode for a texture unit.*/	
		void setTextureAddressingMode(UINT16 stage, const UVWAddressingMode& uvw);

		/** Sets a single filter for a given texture unit.
		@param unit The texture unit to set the filtering options for
		@param ftype The filter type
		@param filter The filter to be used
		*/
		void setTextureFiltering(UINT16 unit, FilterType ftype, FilterOptions filter);

		/** Sets the maximal anisotropy for the specified texture unit.*/
		void setTextureAnisotropy(UINT16 unit, unsigned int maxAnisotropy);

		/** Sets the texture border colour for a texture unit.*/
		void setTextureBorderColor(UINT16 stage, const Color& color);

		/** Sets the mipmap bias value for a given texture unit.
		@remarks
		This allows you to adjust the mipmap calculation up or down for a
		given texture unit. Negative values force a larger mipmap to be used, 
		positive values force a smaller mipmap to be used. Units are in numbers
		of levels, so +1 forces the mipmaps to one smaller level.
		@note Only does something if render system has capability RSC_MIPMAP_LOD_BIAS.
		*/
		void setTextureMipmapBias(UINT16 unit, float bias);

		/************************************************************************/
		/* 								Blend states                      		*/
		/************************************************************************/

		/** Sets the global blending factors for combining subsequent renders with the existing frame contents.
		The result of the blending operation is:</p>
		<p align="center">final = (texture * sourceFactor) + (pixel * destFactor)</p>
		Each of the factors is specified as one of a number of options, as specified in the SceneBlendFactor
		enumerated type.
		By changing the operation you can change addition between the source and destination pixels to a different operator.
		@param sourceFactor The source factor in the above calculation, i.e. multiplied by the texture colour components.
		@param destFactor The destination factor in the above calculation, i.e. multiplied by the pixel colour components.
		@param op The blend operation mode for combining pixels
		*/
		void setSceneBlending( BlendFactor sourceFactor, BlendFactor destFactor, BlendOperation op );

		/** Sets the global blending factors for combining subsequent renders with the existing frame contents.
		The result of the blending operation is:</p>
		<p align="center">final = (texture * sourceFactor) + (pixel * destFactor)</p>
		Each of the factors is specified as one of a number of options, as specified in the SceneBlendFactor
		enumerated type.
		@param sourceFactor The source factor in the above calculation, i.e. multiplied by the texture colour components.
		@param destFactor The destination factor in the above calculation, i.e. multiplied by the pixel colour components.
		@param sourceFactorAlpha The source factor in the above calculation for the alpha channel, i.e. multiplied by the texture alpha components.
		@param destFactorAlpha The destination factor in the above calculation for the alpha channel, i.e. multiplied by the pixel alpha components.
		@param op The blend operation mode for combining pixels
		@param alphaOp The blend operation mode for combining pixel alpha values
		*/
		void setSceneBlending( BlendFactor sourceFactor, BlendFactor destFactor, BlendFactor sourceFactorAlpha, 
			BlendFactor destFactorAlpha, BlendOperation op, BlendOperation alphaOp );

		/** Sets the global alpha rejection approach for future renders.
		By default images are rendered regardless of texture alpha. This method lets you change that.
		@param func The comparison function which must pass for a pixel to be written.
		@param val The value to compare each pixels alpha value to (0-255)
		*/
		void setAlphaTest(CompareFunction func, unsigned char value);

		/**
		 * @brief	Enable alpha coverage if supported.
		 */
		void setAlphaToCoverage(bool enabled);

		/** Sets whether or not colour buffer writing is enabled, and for which channels. 
		@remarks
		For some advanced effects, you may wish to turn off the writing of certain colour
		channels, or even all of the colour channels so that only the depth buffer is updated
		in a rendering pass. However, the chances are that you really want to use this option
		through the Material class.
		@param red, green, blue, alpha Whether writing is enabled for each of the 4 colour channels. */
		void setColorBufferWriteEnabled(bool red, bool green, bool blue, bool alpha);

		/************************************************************************/
		/* 								Rasterizer states                  		*/
		/************************************************************************/

		/** Sets the culling mode for the render system based on the 'vertex winding'.
		A typical way for the rendering engine to cull triangles is based on the
		'vertex winding' of triangles. Vertex winding refers to the direction in
		which the vertices are passed or indexed to in the rendering operation as viewed
		from the camera, and will wither be clockwise or anticlockwise (that's 'counterclockwise' for
		you Americans out there ;) The default is CULL_CLOCKWISE i.e. that only triangles whose vertices
		are passed/indexed in anticlockwise order are rendered - this is a common approach and is used in 3D studio models
		for example. You can alter this culling mode if you wish but it is not advised unless you know what you are doing.
		You may wish to use the CULL_NONE option for mesh data that you cull yourself where the vertex
		winding is uncertain.
		*/
		void setCullingMode(CullingMode mode);

		/** Sets how to rasterise triangles, as points, wireframe or solid polys. */
		void setPolygonMode(PolygonMode level);

		/** Sets the depth bias, NB you should use the Material version of this. 
		@remarks
		When polygons are coplanar, you can get problems with 'depth fighting' where
		the pixels from the two polys compete for the same screen pixel. This is particularly
		a problem for decals (polys attached to another surface to represent details such as
		bulletholes etc.).
		@par
		A way to combat this problem is to use a depth bias to adjust the depth buffer value
		used for the decal such that it is slightly higher than the true value, ensuring that
		the decal appears on top.
		@note
		The final bias value is a combination of a constant bias and a bias proportional
		to the maximum depth slope of the polygon being rendered. The final bias
		is constantBias + slopeScaleBias * maxslope. Slope scale biasing is
		generally preferable but is not available on older hardware.
		@param constantBias The constant bias value, expressed as a value in 
		homogeneous depth coordinates.
		@param slopeScaleBias The bias value which is factored by the maximum slope
		of the polygon, see the description above. This is not supported by all
		cards.

		*/
		void setDepthBias(float constantBias, float slopeScaleBias);

		/**
		* @brief	Scissor test allows you to 'mask off' rendering in all but a given rectangular area
		* 			identified by the rectangle set by setScissorRect().
		*/
		void setScissorTestEnable(bool enable);

		/**
		 * @brief	Only applies when rendering to a multisample render target.
		 * 			If disabled all of the samples will be taken from the center of the pixel,
		 * 			effectively making the image aliased. Default value is true where samples are
		 * 			picked randomly within the pixel.
		 */
		void setMultisampleAntialiasEnable(bool enable);

		/**
		 * @brief	Only applies when rendering to a non-multisample render target.
		 * 			If enabled, lines will be antialiased. Default state is disabled.
		 */
		void setAntialiasedLineEnable(bool enable);

		/************************************************************************/
		/* 						Depth stencil state                      		*/
		/************************************************************************/
		
		/** Sets the mode of operation for depth buffer tests from this point onwards.
		Sometimes you may wish to alter the behaviour of the depth buffer to achieve
		special effects. Because it's unlikely that you'll set these options for an entire frame,
		but rather use them to tweak settings between rendering objects, this is an internal
		method (indicated by the '_' prefix) which will be used by a SceneManager implementation
		rather than directly from the client application.
		If this method is never called the settings are automatically the same as the default parameters.
		@param depthTest If true, the depth buffer is tested for each pixel and the frame buffer is only updated
		if the depth function test succeeds. If false, no test is performed and pixels are always written.
		@param depthWrite If true, the depth buffer is updated with the depth of the new pixel if the depth test succeeds.
		If false, the depth buffer is left unchanged even if a new pixel is written.
		@param depthFunction Sets the function required for the depth test.
		*/
		void setDepthBufferParams(bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL);

		/** Sets whether or not the depth buffer check is performed before a pixel write.
		@param enabled If true, the depth buffer is tested for each pixel and the frame buffer is only updated
		if the depth function test succeeds. If false, no test is performed and pixels are always written.
		*/
		void setDepthBufferCheckEnabled(bool enabled = true);

		/** Sets whether or not the depth buffer is updated after a pixel write.
		@param enabled If true, the depth buffer is updated with the depth of the new pixel if the depth test succeeds.
		If false, the depth buffer is left unchanged even if a new pixel is written.
		*/
		void setDepthBufferWriteEnabled(bool enabled = true);

		/** Sets the comparison function for the depth buffer check.
		Advanced use only - allows you to choose the function applied to compare the depth values of
		new and existing pixels in the depth buffer. Only an issue if the deoth buffer check is enabled
		(see _setDepthBufferCheckEnabled)
		@param  func The comparison between the new depth and the existing depth which must return true
		for the new pixel to be written.
		*/
		void setDepthBufferFunction(CompareFunction func = CMPF_LESS_EQUAL);

		/** Turns stencil buffer checking on or off.
		@remarks
		Stencilling (masking off areas of the rendering target based on the stencil
		buffer) can be turned on or off using this method. By default, stencilling is
		disabled.
		*/
		void setStencilCheckEnabled(bool enabled);

		/** This method allows you to set stencil buffer operations in one call.
		@param stencilFailOp The action to perform when the stencil check fails
		@param depthFailOp The action to perform when the stencil check passes, but the
		depth buffer check still fails
		@param passOp The action to take when both the stencil and depth check pass.
		@param ccw If set to true, the stencil operations will be applied to counterclockwise
		faces. Otherwise they will be applied to clockwise faces.
		*/
		void setStencilBufferOperations(StencilOperation stencilFailOp = SOP_KEEP,
			StencilOperation depthFailOp = SOP_KEEP, StencilOperation passOp = SOP_KEEP,
			bool ccw = true);

		/**
		* @brief	Sets a stencil buffer comparison function. The result of this will cause one of 3 actions depending on whether the test fails,
		*		succeeds but with the depth buffer check still failing, or succeeds with the
		*		depth buffer check passing too.
		* @param ccw If set to true, the stencil operations will be applied to counterclockwise
		*		faces. Otherwise they will be applied to clockwise faces.
		 */
		void setStencilBufferFunc(CompareFunction func = CMPF_ALWAYS_PASS, bool ccw = true);

		/**
		* @brief	The bitmask applied to both the stencil value and the reference value
		*		before comparison
		 */
		void setStencilBufferReadMask(UINT32 mask = 0xFFFFFFFF);

		/**
		* @brief	The bitmask applied to the stencil value before writing it to the stencil buffer.
		 */
		void setStencilBufferWriteMask(UINT32 mask = 0xFFFFFFFF);

		/**
		 * @brief	Sets a reference values used for stencil buffer comparisons. 
		 * 			Actual comparison function and stencil operations are set by setting the DepthStencilState.
		 */
		void setStencilRefValue(UINT32 refValue);

		void clearArea(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0, const RectI& clearArea = RectI::EMPTY);

		/// Notify when a device has been lost.
		void notifyOnDeviceLost(D3D9Device* device);

		/// Notify when a device has been reset.
		void notifyOnDeviceReset(D3D9Device* device);
	};
}