#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsModule.h"

namespace BansheeEngine
{
	/**
	 * @brief	Helper class for DX11 rendering.
	 */
	class D3D11RenderUtility : public Module<D3D11RenderUtility>
	{
	public:
		D3D11RenderUtility(D3D11Device* device);
		~D3D11RenderUtility();

		/**
		 * @brief	Draws a quad that clears the current viewport. This is supposed to emulate functionality
		 *			available with other APIs like DX9 and OpenGL where you can clear only a part of the render target.
		 *			(DX11 API only provides a way to clear the entire render target).
		 *
		 * @param	buffers			Combination of one or more elements of FrameBufferType
		 *							denoting which buffers are to be cleared.
		 * @param	color			(optional) The color to clear the color buffer with, if enabled.
		 * @param	depth			(optional) The value to initialize the depth buffer with, if enabled.
		 * @param	stencil			(optional) The value to initialize the stencil buffer with, if enabled.
		 */
		void drawClearQuad(UINT32 clearBuffers, const Color& color, float depth, UINT16 stencil);

	protected:
		/**
		 * @brief	Initializes resources needed for drawing the clear quad. Should be called one time at start-up.
		 */
		void initClearQuadResources();

		D3D11Device* mDevice;

		ID3D11Buffer* mClearQuadIB;
		ID3D11Buffer* mClearQuadVB;
		ID3D11InputLayout* mClearQuadIL;
		ID3D11VertexShader* mClearQuadVS;
		ID3D11PixelShader* mClearQuadPS;

		SPtr<BlendStateCore> mClearQuadBlendStateYesC;
		SPtr<BlendStateCore> mClearQuadBlendStateNoC;
		SPtr<RasterizerStateCore> mClearQuadRasterizerState;
		SPtr<DepthStencilStateCore> mClearQuadDSStateNoD_NoS;
		SPtr<DepthStencilStateCore> mClearQuadDSStateYesD_NoS;
		SPtr<DepthStencilStateCore> mClearQuadDSStateYesD_YesS;
		SPtr<DepthStencilStateCore> mClearQuadDSStateNoD_YesS;
	};
}