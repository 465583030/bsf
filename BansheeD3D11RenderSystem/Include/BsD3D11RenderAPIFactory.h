#pragma once

#include <string>
#include "BsRenderAPIFactory.h"
#include "BsRenderAPIManager.h"
#include "BsD3D11RenderAPI.h"

namespace BansheeEngine
{
	const String SystemName = "BansheeD3D11RenderSystem";

	/**
	 * @brief	Handles creation of the DX11 render system.
	 */
	class D3D11RenderAPIFactory : public RenderAPIFactory
	{
	public:
		/**
		 * @copydoc	RenderSystemFactory::create
		 */
		virtual void create();

		/**
		 * @copydoc	RenderSystemFactory::name
		 */
		virtual const String& name() const { return SystemName; }

	private:

		/**
		 * @brief	Registers the factory with the render system manager when constructed.
		 */
		class InitOnStart
		{
		public:
			InitOnStart() 
			{ 
				static RenderAPIFactoryPtr newFactory;
				if(newFactory == nullptr)
				{
					newFactory = bs_shared_ptr<D3D11RenderAPIFactory>();
					RenderAPIManager::instance().registerFactory(newFactory);
				}
			}
		};

		static InitOnStart initOnStart; // Makes sure factory is registered on program start
	};
}