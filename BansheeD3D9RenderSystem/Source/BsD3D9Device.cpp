#include "BsD3D9Device.h"
#include "BsD3D9DeviceManager.h"
#include "BsD3D9Driver.h"
#include "BsD3D9RenderSystem.h"
#include "BsD3D9ResourceManager.h"
#include "BsD3D9RenderWindow.h"
#include "BsHardwareBufferManager.h"
#include "BsException.h"

namespace BansheeEngine
{
	HWND D3D9Device::msSharedFocusWindow = 0;

	D3D9Device::D3D9Device(D3D9DeviceManager* deviceManager, UINT adapterNumber, HMONITOR hMonitor, 
		D3DDEVTYPE devType, DWORD behaviorFlags)
	{
		mpDeviceManager	= deviceManager;
		mpDevice = nullptr;		
		mAdapterNumber = adapterNumber;
		mMonitor = hMonitor;
		mDeviceType = devType;
		mFocusWindow = 0;
		mBehaviorFlags = behaviorFlags;	
		mD3D9DeviceCapsValid = false;
		mDeviceLost = false;
		mPresentationParamsCount = 0;
		mPresentationParams	= nullptr;
		memset(&mD3D9DeviceCaps, 0, sizeof(mD3D9DeviceCaps));
		memset(&mCreationParams, 0, sizeof(mCreationParams));		
	}

	D3D9Device::~D3D9Device()
	{

	}

	D3D9Device::RenderWindowToResorucesIterator D3D9Device::getRenderWindowIterator(const D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.find(renderWindow);

		if (it == mMapRenderWindowToResoruces.end())
			BS_EXCEPT(RenderingAPIException, "Render window was not attached to this device.");

		return it;
	}

	void D3D9Device::attachRenderWindow(const D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.find(renderWindow);

		if (it == mMapRenderWindowToResoruces.end())
		{
			RenderWindowResources* renderWindowResources = bs_new<RenderWindowResources>();

			memset(renderWindowResources, 0, sizeof(RenderWindowResources));						
			renderWindowResources->adapterOrdinalInGroupIndex = 0;					
			renderWindowResources->acquired = false;
			mMapRenderWindowToResoruces[renderWindow] = renderWindowResources;			
		}

		updateRenderWindowsIndices();
	}

	void D3D9Device::detachRenderWindow(const D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.find(renderWindow);

		if (it != mMapRenderWindowToResoruces.end())
		{		
			// The focus window in which the d3d9 device created on is detached.
			// resources must be acquired again.
			if (mFocusWindow == renderWindow->_getWindowHandle())
			{
				mFocusWindow = 0;				
			}

			// Case this is the shared focus window.
			if (renderWindow->_getWindowHandle() == msSharedFocusWindow)			
				setSharedWindowHandle(0);		
			
			RenderWindowResources* renderWindowResources = it->second;

			releaseRenderWindowResources(renderWindowResources);

			if(renderWindowResources != nullptr)
				bs_delete(renderWindowResources);
			
			mMapRenderWindowToResoruces.erase(it);		
		}

		updateRenderWindowsIndices();
	}

	bool D3D9Device::acquire()
	{	
		updatePresentationParameters();

		bool resetDevice = false;
			
		// Create device if need to.
		if (mpDevice == nullptr)
		{			
			createD3D9Device();
		}
		else
		{
			RenderWindowToResorucesIterator itPrimary = getRenderWindowIterator(getPrimaryWindow());

			if (itPrimary->second->swapChain != nullptr)
			{
				D3DPRESENT_PARAMETERS currentPresentParams;
				HRESULT hr;

				hr = itPrimary->second->swapChain->GetPresentParameters(&currentPresentParams);
				if (FAILED(hr))
				{
					BS_EXCEPT(RenderingAPIException, "GetPresentParameters failed");
				}
				
				// Desired parameters are different then current parameters.
				// Possible scenario is that primary window resized and in the mean while another
				// window attached to this device.
				if (memcmp(&currentPresentParams, &mPresentationParams[0], sizeof(D3DPRESENT_PARAMETERS)) != 0)
				{
					resetDevice = true;					
				}				
			}

			// Make sure depth stencil is set to valid surface. It is going to be
			// grabbed by the primary window using the GetDepthStencilSurface method.
			if (resetDevice == false)
			{
				mpDevice->SetDepthStencilSurface(itPrimary->second->depthBuffer);
			}
			
		}

		// Reset device will update all render window resources.
		if (resetDevice)
		{
			reset();
		}
		else // No need to reset -> just acquire resources.
		{
			// Update resources of each window.
			RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();

			while (it != mMapRenderWindowToResoruces.end())
			{
				acquireRenderWindowResources(it);
				++it;
			}
		}
									
		return true;
	}

	void D3D9Device::release()
	{
		if (mpDevice != nullptr)
		{
			D3D9RenderSystem* renderSystem = static_cast<D3D9RenderSystem*>(BansheeEngine::RenderSystem::instancePtr());

			RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();

			while (it != mMapRenderWindowToResoruces.end())
			{
				RenderWindowResources* renderWindowResources = it->second;

				releaseRenderWindowResources(renderWindowResources);
				++it;
			}

			releaseD3D9Device();
		}				
	}

	bool D3D9Device::acquire(const D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);
		
		acquireRenderWindowResources(it);

		return true;
	}

	void D3D9Device::notifyDeviceLost()
	{
		// Case this device is already in lost state.
		if (mDeviceLost)
			return;

		// Case we just moved from valid state to lost state.
		mDeviceLost = true;	

		D3D9RenderSystem* renderSystem = static_cast<D3D9RenderSystem*>(BansheeEngine::RenderSystem::instancePtr());
		renderSystem->notifyOnDeviceLost(this);
	}	

	IDirect3DSurface9* D3D9Device::getDepthBuffer(const D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);		

		return it->second->depthBuffer;
	}

	IDirect3DSurface9* D3D9Device::getBackBuffer(const D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);
	
		return it->second->backBuffer;		
	}

	void D3D9Device::setAdapterOrdinalIndex(const D3D9RenderWindow* renderWindow, UINT32 adapterOrdinalInGroupIndex)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);

		it->second->adapterOrdinalInGroupIndex = adapterOrdinalInGroupIndex;

		updateRenderWindowsIndices();
	}
	
	void D3D9Device::destroy()
	{	
		// Lock access to rendering device.
		D3D9RenderSystem::getResourceManager()->lockDeviceAccess();
		
		release();
		
		RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();

		if (it != mMapRenderWindowToResoruces.end())
		{	
			if (it->first->_getWindowHandle() == msSharedFocusWindow)
				setSharedWindowHandle(0);

			if(it->second != nullptr)
				bs_delete(it->second);

			++it;
		}
		mMapRenderWindowToResoruces.clear();
		
		// Reset dynamic attributes.		
		mFocusWindow = 0;		
		mD3D9DeviceCapsValid = false;

		if(mPresentationParams != nullptr)
			bs_deleteN(mPresentationParams, mPresentationParamsCount);

		mPresentationParamsCount = 0;

		// Notify the device manager on this instance destruction.	
		mpDeviceManager->notifyOnDeviceDestroy(this);

		// UnLock access to rendering device.
		D3D9RenderSystem::getResourceManager()->unlockDeviceAccess();
	}	

	bool D3D9Device::isDeviceLost()
	{		
		HRESULT hr;

		hr = mpDevice->TestCooperativeLevel();

		if (hr == D3DERR_DEVICELOST ||
			hr == D3DERR_DEVICENOTRESET)
		{
			return true;
		}
		
		return false;
	}

	bool D3D9Device::reset()
	{
		HRESULT hr;

		// Check that device is in valid state for reset.
		hr = mpDevice->TestCooperativeLevel();
		if (hr == D3DERR_DEVICELOST ||
			hr == D3DERR_DRIVERINTERNALERROR)
		{
			return false;
		}

		// Lock access to rendering device.
		D3D9RenderSystem::getResourceManager()->lockDeviceAccess();
					
		D3D9RenderSystem* renderSystem = static_cast<D3D9RenderSystem*>(BansheeEngine::RenderSystem::instancePtr());

		// Inform all resources that device lost.
		D3D9RenderSystem::getResourceManager()->notifyOnDeviceLost(mpDevice);

		// Notify all listener before device is rested
		renderSystem->notifyOnDeviceLost(this);

		updatePresentationParameters();

		RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();

		while (it != mMapRenderWindowToResoruces.end())
		{
			RenderWindowResources* renderWindowResources = it->second;

			releaseRenderWindowResources(renderWindowResources);
			++it;
		}

		clearDeviceStreams();

		// Reset the device using the presentation parameters.
		hr = mpDevice->Reset(mPresentationParams);
	
		if (hr == D3DERR_DEVICELOST)
		{
			// UnLock access to rendering device.
			D3D9RenderSystem::getResourceManager()->unlockDeviceAccess();

			// Don't continue
			return false;
		}
		else if (FAILED(hr))
		{
			BS_EXCEPT(RenderingAPIException, "Cannot reset device!");
		}

		mDeviceLost = false;

		// Update resources of each window.
		it = mMapRenderWindowToResoruces.begin();

		while (it != mMapRenderWindowToResoruces.end())
		{
			acquireRenderWindowResources(it);
			++it;
		}		

		D3D9Device* pCurActiveDevice = mpDeviceManager->getActiveDevice();

		mpDeviceManager->setActiveDevice(this);

		// Inform all resources that device has been reset.
		D3D9RenderSystem::getResourceManager()->notifyOnDeviceReset(mpDevice);

		mpDeviceManager->setActiveDevice(pCurActiveDevice);
		
		renderSystem->notifyOnDeviceReset(this);

		// UnLock access to rendering device.
		D3D9RenderSystem::getResourceManager()->unlockDeviceAccess();
	
		return true;
	}

	bool D3D9Device::isAutoDepthStencil() const
	{
		const D3DPRESENT_PARAMETERS& primaryPresentationParams = mPresentationParams[0];
		
		// Check if auto depth stencil can be used.
		for (unsigned int i = 1; i < mPresentationParamsCount; i++)
		{			
			// disable AutoDepthStencil if these parameters are not all the same.
			if(primaryPresentationParams.BackBufferHeight != mPresentationParams[i].BackBufferHeight || 
				primaryPresentationParams.BackBufferWidth != mPresentationParams[i].BackBufferWidth	|| 
				primaryPresentationParams.BackBufferFormat != mPresentationParams[i].BackBufferFormat || 
				primaryPresentationParams.AutoDepthStencilFormat != mPresentationParams[i].AutoDepthStencilFormat ||
				primaryPresentationParams.MultiSampleQuality != mPresentationParams[i].MultiSampleQuality ||
				primaryPresentationParams.MultiSampleType != mPresentationParams[i].MultiSampleType)
			{
				return false;
			}
		}

		return true;
	}

	const D3DCAPS9& D3D9Device::getD3D9DeviceCaps() const
	{
		if (mD3D9DeviceCapsValid == false)
		{
			BS_EXCEPT(RenderingAPIException, "Device caps are invalid!");
		}

		return mD3D9DeviceCaps;
	}

	D3DFORMAT D3D9Device::getBackBufferFormat() const
	{		
		if (mPresentationParams == NULL || mPresentationParamsCount == 0)
		{
			BS_EXCEPT(RenderingAPIException, "Presentation parameters are invalid!");
		}

		return mPresentationParams[0].BackBufferFormat;
	}

	D3DFORMAT D3D9Device::getDepthStencilFormat() const
	{		
		if (mPresentationParams == NULL || mPresentationParamsCount == 0)
		{
			BS_EXCEPT(RenderingAPIException, "Presentation parameters are invalid!");
		}

		return mPresentationParams[0].AutoDepthStencilFormat;
	}

	IDirect3DDevice9* D3D9Device::getD3D9Device() const
	{
		return mpDevice;
	}

	void D3D9Device::updatePresentationParameters()
	{		
		// Clear old presentation parameters.
		if(mPresentationParams != nullptr)
			bs_deleteN(mPresentationParams, mPresentationParamsCount);

		mPresentationParamsCount = 0;		

		if (mMapRenderWindowToResoruces.size() > 0)
		{
			mPresentationParams = bs_newN<D3DPRESENT_PARAMETERS>((UINT32)mMapRenderWindowToResoruces.size());

			RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();

			while (it != mMapRenderWindowToResoruces.end())
			{
				const D3D9RenderWindow* renderWindow = it->first;
				RenderWindowResources* renderWindowResources = it->second;

				// Ask the render window to build it's own parameters.
				renderWindow->_buildPresentParameters(&renderWindowResources->presentParameters);
				
				// Update shared focus window handle.
				if (renderWindow->isFullScreen() && renderWindowResources->presentParametersIndex == 0 && msSharedFocusWindow == NULL)
					setSharedWindowHandle(renderWindow->_getWindowHandle());					

				// This is the primary window or a full screen window that is part of multi head device.
				if (renderWindowResources->presentParametersIndex == 0 || renderWindow->isFullScreen())
				{
					mPresentationParams[renderWindowResources->presentParametersIndex] = renderWindowResources->presentParameters;
					mPresentationParamsCount++;
				}
															
				++it;
			}
		}

		// Case we have to cancel auto depth stencil.
		if (isMultihead() && isAutoDepthStencil() == false)
		{
			for(unsigned int i = 0; i < mPresentationParamsCount; i++)
			{
				mPresentationParams[i].EnableAutoDepthStencil = false;
			}
		}
	}	

	UINT D3D9Device::getAdapterNumber() const
	{
		return mAdapterNumber;
	}

	D3DDEVTYPE D3D9Device::getDeviceType() const
	{
		return mDeviceType;
	}

	bool D3D9Device::isMultihead() const
	{
		for (auto& resourceData : mMapRenderWindowToResoruces)
		{
			RenderWindowResources* renderWindowResources = resourceData.second;

			if (renderWindowResources->adapterOrdinalInGroupIndex > 0 && resourceData.first->isFullScreen())
				return true;
		}

		return false;
	}

	void D3D9Device::clearDeviceStreams()
	{
		D3D9RenderSystem* renderSystem = static_cast<D3D9RenderSystem*>(BansheeEngine::RenderSystem::instancePtr());

		// Set all texture units to nothing to release texture surfaces
		for (DWORD stage = 0; stage < mD3D9DeviceCaps.MaxSimultaneousTextures; ++stage)
		{
			DWORD dwCurValue = D3DTOP_FORCE_DWORD;
			HRESULT hr;

			hr = mpDevice->SetTexture(stage, nullptr);
			if( hr != S_OK )
			{
				String str = "Unable to disable texture '" + toString((unsigned int)stage) + "' in D3D9";
				BS_EXCEPT(RenderingAPIException, str);
			}
		
			mpDevice->GetTextureStageState(static_cast<DWORD>(stage), D3DTSS_COLOROP, &dwCurValue);

			if (dwCurValue != D3DTOP_DISABLE)
			{
				hr = mpDevice->SetTextureStageState(static_cast<DWORD>(stage), D3DTSS_COLOROP, D3DTOP_DISABLE);
				if( hr != S_OK )
				{
					String str = "Unable to disable texture '" + toString((unsigned)stage) + "' in D3D9";
					BS_EXCEPT(RenderingAPIException, str);
				}
			}			
		
			// set stage desc. to defaults
			renderSystem->mTexStageDesc[stage].pTex = 0;
			renderSystem->mTexStageDesc[stage].coordIndex = 0;
			renderSystem->mTexStageDesc[stage].texType = D3D9Mappings::D3D_TEX_TYPE_NORMAL;
		}

		// Unbind any vertex streams to avoid memory leaks				
		for (unsigned int i = 0; i < mD3D9DeviceCaps.MaxStreams; ++i)
		{
			mpDevice->SetStreamSource(i, nullptr, 0, 0);
		}
	}

	void D3D9Device::createD3D9Device()
	{		
		// Update focus window.
		const D3D9RenderWindow* primaryRenderWindow = getPrimaryWindow();

		// Case we have to share the same focus window.
		if (msSharedFocusWindow != NULL)
			mFocusWindow = msSharedFocusWindow;
		else
			mFocusWindow = primaryRenderWindow->_getWindowHandle();		

		IDirect3D9* pD3D9 = D3D9RenderSystem::getDirect3D9();
		HRESULT hr;

		if (isMultihead())
		{
			mBehaviorFlags |= D3DCREATE_ADAPTERGROUP_DEVICE;
		}		
		else
		{
			mBehaviorFlags &= ~D3DCREATE_ADAPTERGROUP_DEVICE;
		}

		// Try to create the device with hardware vertex processing. 
		mBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		hr = pD3D9->CreateDevice(mAdapterNumber, mDeviceType, mFocusWindow,
			mBehaviorFlags, mPresentationParams, &mpDevice);

		if (FAILED(hr))
		{
			// Try a second time, may fail the first time due to back buffer count,
			// which will be corrected down to 1 by the runtime
			hr = pD3D9->CreateDevice(mAdapterNumber, mDeviceType, mFocusWindow,
				mBehaviorFlags, mPresentationParams, &mpDevice);
		}

		// Case hardware vertex processing failed.
		if (FAILED(hr))
		{
			// Try to create the device with mixed vertex processing.
			mBehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
			mBehaviorFlags |= D3DCREATE_MIXED_VERTEXPROCESSING;

			hr = pD3D9->CreateDevice(mAdapterNumber, mDeviceType, mFocusWindow,
				mBehaviorFlags, mPresentationParams, &mpDevice);
		}

		if (FAILED(hr))
		{
			// Try to create the device with software vertex processing.
			mBehaviorFlags &= ~D3DCREATE_MIXED_VERTEXPROCESSING;
			mBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
			hr = pD3D9->CreateDevice(mAdapterNumber, mDeviceType, mFocusWindow,
				mBehaviorFlags, mPresentationParams, &mpDevice);
		}

		if (FAILED(hr))
		{
			// Try reference device
			mDeviceType = D3DDEVTYPE_REF;
			hr = pD3D9->CreateDevice(mAdapterNumber, mDeviceType, mFocusWindow,
				mBehaviorFlags, mPresentationParams, &mpDevice);

			if (FAILED(hr))
			{
				BS_EXCEPT(RenderingAPIException, "Cannot create device!");
			}
		}

		// Get current device caps.
		hr = mpDevice->GetDeviceCaps(&mD3D9DeviceCaps);
		if (FAILED(hr))
		{
			BS_EXCEPT(RenderingAPIException, "Cannot get device caps!");
		}

		// Get current creation parameters caps.
		hr = mpDevice->GetCreationParameters(&mCreationParams);
		if (FAILED(hr) )
		{
			BS_EXCEPT(RenderingAPIException, "Error Get Creation Parameters");
		}

		mD3D9DeviceCapsValid = true;

		// Lock access to rendering device.
		D3D9RenderSystem::getResourceManager()->lockDeviceAccess();

		D3D9Device* pCurActiveDevice = mpDeviceManager->getActiveDevice();

		mpDeviceManager->setActiveDevice(this);

		// Inform all resources that new device created.
		D3D9RenderSystem::getResourceManager()->notifyOnDeviceCreate(mpDevice);

		mpDeviceManager->setActiveDevice(pCurActiveDevice);

		// UnLock access to rendering device.
		D3D9RenderSystem::getResourceManager()->unlockDeviceAccess();
	}

	void D3D9Device::releaseD3D9Device()
	{
		if (mpDevice != nullptr)
		{
			// Lock access to rendering device.
			D3D9RenderSystem::getResourceManager()->lockDeviceAccess();

			D3D9Device* pCurActiveDevice = mpDeviceManager->getActiveDevice();

			mpDeviceManager->setActiveDevice(this);

			// Inform all resources that device is going to be destroyed.
			D3D9RenderSystem::getResourceManager()->notifyOnDeviceDestroy(mpDevice);

			mpDeviceManager->setActiveDevice(pCurActiveDevice);
			
			// Release device.
			SAFE_RELEASE(mpDevice);	
			
			// UnLock access to rendering device.
			D3D9RenderSystem::getResourceManager()->unlockDeviceAccess();
		}
	}

	void D3D9Device::releaseRenderWindowResources(RenderWindowResources* renderWindowResources)
	{
		SAFE_RELEASE(renderWindowResources->backBuffer);
		SAFE_RELEASE(renderWindowResources->depthBuffer);
		SAFE_RELEASE(renderWindowResources->swapChain);
		renderWindowResources->acquired = false;
	}

	void D3D9Device::invalidate(const D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);

		it->second->acquired = false;		
	}

	bool D3D9Device::validate(D3D9RenderWindow* renderWindow)
	{
		// Validate that the render window should run on this device.
		if (!validateDisplayMonitor(renderWindow))
			return false;
		
		// Validate that this device created on the correct target focus window handle		
		validateFocusWindow();		
		
		// Validate that the render window dimensions matches to back buffer dimensions.
		validateBackBufferSize(renderWindow);

		// Validate that this device is in valid rendering state.
		if (!validateDeviceState(renderWindow))
			return false;

		return true;
	}

	void D3D9Device::validateFocusWindow()
	{
		// Focus window changed -> device should be re-acquired.
		if ((msSharedFocusWindow != NULL && mCreationParams.hFocusWindow != msSharedFocusWindow) ||
			(msSharedFocusWindow == NULL && mCreationParams.hFocusWindow != getPrimaryWindow()->_getWindowHandle()))
		{
			// Lock access to rendering device.
			D3D9RenderSystem::getResourceManager()->lockDeviceAccess();

			release();
			acquire();

			// UnLock access to rendering device.
			D3D9RenderSystem::getResourceManager()->unlockDeviceAccess();
		}
	}

	bool D3D9Device::validateDeviceState(const D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);		
		RenderWindowResources* renderWindowResources =  it->second;
		HRESULT hr;

		hr = mpDevice->TestCooperativeLevel();	

		// Case device is not valid for rendering. 
		if (FAILED(hr))
		{					
			// device lost, and we can't reset
			// can't do anything about it here, wait until we get 
			// D3DERR_DEVICENOTRESET; rendering calls will silently fail until 
			// then (except Present, but we ignore device lost there too)
			if (hr == D3DERR_DEVICELOST)
			{						
				releaseRenderWindowResources(renderWindowResources);
				notifyDeviceLost();							
				return false;
			}

			// device lost, and we can reset
			else if (hr == D3DERR_DEVICENOTRESET)
			{					
				bool deviceRestored = reset();

				// Device was not restored yet.
				if (deviceRestored == false)
				{
					// Wait a while
					Sleep(50);					
					return false;
				}																								
			}						
		}

		// Render window resources explicitly invalidated. (Resize or window mode switch) 
		if (renderWindowResources->acquired == false)
		{
			if (getPrimaryWindow() == renderWindow)
			{
				bool deviceRestored = reset();

				// Device was not restored yet.
				if (deviceRestored == false)
				{
					// Wait a while
					Sleep(50);					
					return false;
				}	
			}
			else
			{
				acquire(renderWindow);
			}
		}

		return true;
	}
		
	void D3D9Device::validateBackBufferSize(const D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);
		RenderWindowResources*	renderWindowResources = it->second;
	
		// Case size has been changed.
		if (renderWindow->getWidth() != renderWindowResources->presentParameters.BackBufferWidth ||
			renderWindow->getHeight() != renderWindowResources->presentParameters.BackBufferHeight)
		{			
			if (renderWindow->getWidth() > 0)
				renderWindowResources->presentParameters.BackBufferWidth = renderWindow->getWidth();

			if (renderWindow->getHeight() > 0)
				renderWindowResources->presentParameters.BackBufferHeight = renderWindow->getHeight();

			invalidate(renderWindow);
		}				
	}

	bool D3D9Device::validateDisplayMonitor(D3D9RenderWindow* renderWindow)
	{
		// Ignore full screen since it doesn't really move and it is possible 
		// that it created using multi-head adapter so for a subordinate the
		// native monitor handle and this device handle will be different.
		if (renderWindow->isFullScreen())
			return true;

		HMONITOR	hRenderWindowMonitor = NULL;

		// Find the monitor this render window belongs to.
		hRenderWindowMonitor = MonitorFromWindow(renderWindow->_getWindowHandle(), MONITOR_DEFAULTTONULL);

		// This window doesn't intersect with any of the display monitor
		if (hRenderWindowMonitor == NULL)		
			return false;		
		

		// Case this window changed monitor.
		if (hRenderWindowMonitor != mMonitor)
		{	
			// Lock access to rendering device.
			D3D9RenderSystem::getResourceManager()->lockDeviceAccess();

			mpDeviceManager->linkRenderWindow(renderWindow);

			// UnLock access to rendering device.
			D3D9RenderSystem::getResourceManager()->unlockDeviceAccess();

			return false;
		}

		return true;
	}

	void D3D9Device::present(const D3D9RenderWindow* renderWindow)
	{		
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);
		RenderWindowResources*	renderWindowResources = it->second;				

		// Skip present while current device state is invalid.
		if (mDeviceLost || 
			renderWindowResources->acquired == false || 
			isDeviceLost())		
			return;		


		HRESULT hr;

		if (isMultihead())
		{
			// Only the master will call present method results in synchronized
			// buffer swap for the rest of the implicit swap chain.
			if (getPrimaryWindow() == renderWindow)
				hr = mpDevice->Present( NULL, NULL, NULL, NULL );
			else
				hr = S_OK;
		}
		else
		{
			hr = renderWindowResources->swapChain->Present(NULL, NULL, NULL, NULL, 0);			
		}


		if(D3DERR_DEVICELOST == hr)
		{
			releaseRenderWindowResources(renderWindowResources);
			notifyDeviceLost();
		}
		else if( FAILED(hr) )
		{
			BS_EXCEPT(RenderingAPIException, "Error Presenting surfaces");
		}
	}

	void D3D9Device::acquireRenderWindowResources(RenderWindowToResorucesIterator it)
	{
		RenderWindowResources*	renderWindowResources = it->second;
		const D3D9RenderWindow*	renderWindow = it->first;			
		
		releaseRenderWindowResources(renderWindowResources);

		// Create additional swap chain
		if (isSwapChainWindow(renderWindow) && !isMultihead())
		{
			// Create swap chain
			HRESULT hr = mpDevice->CreateAdditionalSwapChain(&renderWindowResources->presentParameters, 
				&renderWindowResources->swapChain);

			if (FAILED(hr))
			{
				// Try a second time, may fail the first time due to back buffer count,
				// which will be corrected by the runtime
				hr = mpDevice->CreateAdditionalSwapChain(&renderWindowResources->presentParameters, 
					&renderWindowResources->swapChain);
			}

			if (FAILED(hr))
			{
				BS_EXCEPT(RenderingAPIException, "Unable to create an additional swap chain");
			}
		}
		else
		{
			// The swap chain is already created by the device
			HRESULT hr = mpDevice->GetSwapChain(renderWindowResources->presentParametersIndex, 
				&renderWindowResources->swapChain);
			if (FAILED(hr)) 
			{
				BS_EXCEPT(RenderingAPIException, "Unable to get the swap chain");
			}
		}

		// Store references to buffers for convenience		
		renderWindowResources->swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, 
			&renderWindowResources->backBuffer);

		// Additional swap chains need their own depth buffer
		// to support resizing them
		if (renderWindow->_isDepthBuffered()) 
		{
			// if multihead is enabled, depth buffer can be created automatically for 
			// all the adapters. if multihead is not enabled, depth buffer is just
			// created for the main swap chain
			if (isMultihead() && isAutoDepthStencil() || 
			    isMultihead() == false && isSwapChainWindow(renderWindow) == false)
			{
				mpDevice->GetDepthStencilSurface(&renderWindowResources->depthBuffer);
			}
			else
			{
				UINT32 targetWidth  = renderWindow->getWidth();
				UINT32 targetHeight = renderWindow->getHeight();

				if (targetWidth == 0)
					targetWidth = 1;

				if (targetHeight == 0)
					targetHeight = 1;

				HRESULT hr = mpDevice->CreateDepthStencilSurface(
					targetWidth, targetHeight,
					renderWindowResources->presentParameters.AutoDepthStencilFormat,
					renderWindowResources->presentParameters.MultiSampleType,
					renderWindowResources->presentParameters.MultiSampleQuality, 
					(renderWindowResources->presentParameters.Flags & D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL),
					&renderWindowResources->depthBuffer, NULL
					);

				if (FAILED(hr)) 
				{
					BS_EXCEPT(RenderingAPIException, "Unable to create a depth buffer for the swap chain");
				}

				if (isSwapChainWindow(renderWindow) == false)
				{
					mpDevice->SetDepthStencilSurface(renderWindowResources->depthBuffer);
				}
			}
		}

		renderWindowResources->acquired = true; 
	}

	bool D3D9Device::isSwapChainWindow(const D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);
		
		if (it->second->presentParametersIndex == 0 || renderWindow->isFullScreen())			
			return false;
			
		return true;
	}

	const D3D9RenderWindow* D3D9Device::getPrimaryWindow()
	{		
		RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();
	
		while (it != mMapRenderWindowToResoruces.end() && it->second->presentParametersIndex != 0)				
			++it;		

		assert(it != mMapRenderWindowToResoruces.end());

		return it->first;
	}

	void D3D9Device::setSharedWindowHandle(HWND hSharedHWND)
	{
		if (hSharedHWND != msSharedFocusWindow)					
			msSharedFocusWindow = hSharedHWND;					
	}

	void D3D9Device::updateRenderWindowsIndices()
	{
		// Update present parameters index attribute per render window.
		if (isMultihead())
		{
			RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();

			// Multi head device case -  
			// Present parameter index is based on adapter ordinal in group index.
			while (it != mMapRenderWindowToResoruces.end())			
			{
				it->second->presentParametersIndex = it->second->adapterOrdinalInGroupIndex;
				++it;
			}
		}
		else
		{
			// Single head device case - 
			// Just assign index in incremental order - 
			// NOTE: It suppose to cover both cases that possible here:
			// 1. Single full screen window - only one allowed per device (this is not multi-head case).
			// 2. Multiple window mode windows.

			UINT32 nextPresParamIndex = 0;

			RenderWindowToResorucesIterator it;
			const D3D9RenderWindow* deviceFocusWindow = NULL;

			// In case a d3d9 device exists - try to keep the present parameters order
			// so that the window that the device is focused on will stay the same and we
			// will avoid device re-creation.
			if (mpDevice != NULL)
			{
				it = mMapRenderWindowToResoruces.begin();
				while (it != mMapRenderWindowToResoruces.end())			
				{
					if (it->first->_getWindowHandle() == mCreationParams.hFocusWindow)
					{
						deviceFocusWindow = it->first;
						it->second->presentParametersIndex = nextPresParamIndex;
						++nextPresParamIndex;
						break;
					}					
					++it;
				}
			}
			
		

			it = mMapRenderWindowToResoruces.begin();
			while (it != mMapRenderWindowToResoruces.end())			
			{
				if (it->first != deviceFocusWindow)
				{
					it->second->presentParametersIndex = nextPresParamIndex;
					++nextPresParamIndex;
				}								
				++it;
			}
		}
	}

	void D3D9Device::copyContentsToMemory(const D3D9RenderWindow* renderWindow, 
		const PixelData &dst, RenderTarget::FrameBuffer buffer)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);
		RenderWindowResources* resources = it->second;
		bool swapChain = isSwapChainWindow(renderWindow);

		if ((dst.getLeft() < 0) || (dst.getRight() > renderWindow->getWidth()) ||
			(dst.getTop() < 0) || (dst.getBottom() > renderWindow->getHeight()) ||
			(dst.getFront() != 0) || (dst.getBack() != 1))
		{
			BS_EXCEPT(InvalidParametersException, "Invalid box.");
		}

		HRESULT hr;
		IDirect3DSurface9* pSurf = NULL;
		IDirect3DSurface9* pTempSurf = NULL;
		D3DSURFACE_DESC desc;
		D3DLOCKED_RECT lockedRect;


		if (buffer == RenderTarget::FB_AUTO)
		{
			//buffer = mIsFullScreen? FB_FRONT : FB_BACK;
			buffer = RenderTarget::FB_FRONT;
		}

		if (buffer == RenderTarget::FB_FRONT)
		{
			D3DDISPLAYMODE dm;

			if (FAILED(hr = mpDevice->GetDisplayMode(0, &dm)))
			{
				BS_EXCEPT(RenderingAPIException, "Can't get display mode: TODO PORT NO ERROR"); // TODO PORT - Translate HR to error
			}

			desc.Width = dm.Width;
			desc.Height = dm.Height;
			desc.Format = D3DFMT_A8R8G8B8;
			if (FAILED(hr = mpDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height,
				desc.Format,
				D3DPOOL_SYSTEMMEM,
				&pTempSurf,
				0)))
			{
				BS_EXCEPT(RenderingAPIException, "Can't create offscreen buffer: TODO PORT NO ERROR"); // TODO PORT - Translate HR to error
			}

			if (FAILED(hr = swapChain ? resources->swapChain->GetFrontBufferData(pTempSurf) :
				mpDevice->GetFrontBufferData(0, pTempSurf)))
			{
				SAFE_RELEASE(pTempSurf);
				BS_EXCEPT(RenderingAPIException, "Can't get front buffer: TODO PORT NO ERROR"); // TODO PORT - Translate HR to error
			}

			if(renderWindow->isFullScreen())
			{
				if ((dst.getLeft() == 0) && (dst.getRight() == renderWindow->getWidth()) && (dst.getTop() == 0) && (dst.getBottom() == renderWindow->getHeight()))
				{
					hr = pTempSurf->LockRect(&lockedRect, 0, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
				}
				else
				{
					RECT rect;

					rect.left = static_cast<LONG>(dst.getLeft());
					rect.right = static_cast<LONG>(dst.getRight());
					rect.top = static_cast<LONG>(dst.getTop());
					rect.bottom = static_cast<LONG>(dst.getBottom());

					hr = pTempSurf->LockRect(&lockedRect, &rect, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
				}
				if (FAILED(hr))
				{
					SAFE_RELEASE(pTempSurf);
					BS_EXCEPT(RenderingAPIException, "Can't lock rect:  TODO PORT NO ERROR"); // TODO PORT - Translate HR to error
				} 
			}
			else
			{
				RECT srcRect;
				//GetClientRect(mHWnd, &srcRect);
				srcRect.left = static_cast<LONG>(dst.getLeft());
				srcRect.top = static_cast<LONG>(dst.getTop());
				srcRect.right = static_cast<LONG>(dst.getRight());
				srcRect.bottom = static_cast<LONG>(dst.getBottom());
				POINT point;
				point.x = srcRect.left;
				point.y = srcRect.top;
				ClientToScreen(renderWindow->_getWindowHandle(), &point);
				srcRect.top = point.y;
				srcRect.left = point.x;
				srcRect.bottom += point.y;
				srcRect.right += point.x;

				desc.Width = srcRect.right - srcRect.left;
				desc.Height = srcRect.bottom - srcRect.top;

				if (FAILED(hr = pTempSurf->LockRect(&lockedRect, &srcRect, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK)))
				{
					SAFE_RELEASE(pTempSurf);
					BS_EXCEPT(RenderingAPIException, "Can't lock rect:  TODO PORT NO ERROR"); // TODO PORT - Translate HR to error
				} 
			}
		}
		else
		{
			SAFE_RELEASE(pSurf);
			if(FAILED(hr = swapChain? resources->swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pSurf) :
				mpDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurf)))
			{
				BS_EXCEPT(RenderingAPIException, "Can't get back buffer:  TODO PORT NO ERROR"); // TODO PORT - Translate HR to error
			}

			if(FAILED(hr = pSurf->GetDesc(&desc)))
			{
				BS_EXCEPT(RenderingAPIException, "Can't get description:  TODO PORT NO ERROR"); // TODO PORT - Translate HR to error
			}

			if (FAILED(hr = mpDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height,
				desc.Format,
				D3DPOOL_SYSTEMMEM,
				&pTempSurf,
				0)))
			{
				BS_EXCEPT(RenderingAPIException, "Can't create offscreen surface:  TODO PORT NO ERROR"); // TODO PORT - Translate HR to error
			}

			if (desc.MultiSampleType == D3DMULTISAMPLE_NONE)
			{
				if (FAILED(hr = mpDevice->GetRenderTargetData(pSurf, pTempSurf)))
				{
					SAFE_RELEASE(pTempSurf);
					BS_EXCEPT(RenderingAPIException, "Can't get render target data:  TODO PORT NO ERROR"); // TODO PORT - Translate HR to error
				}
			}
			else
			{
				IDirect3DSurface9* pStretchSurf = 0;

				if (FAILED(hr = mpDevice->CreateRenderTarget(desc.Width, desc.Height,
					desc.Format,
					D3DMULTISAMPLE_NONE,
					0,
					false,
					&pStretchSurf,
					0)))
				{
					SAFE_RELEASE(pTempSurf);
					BS_EXCEPT(RenderingAPIException, "Can't create render target:  TODO PORT NO ERROR"); // TODO PORT - Translate HR to error
				}

				if (FAILED(hr = mpDevice->StretchRect(pSurf, 0, pStretchSurf, 0, D3DTEXF_NONE)))
				{
					SAFE_RELEASE(pTempSurf);
					SAFE_RELEASE(pStretchSurf);
					BS_EXCEPT(RenderingAPIException, "Can't stretch rect:  TODO PORT NO ERROR"); // TODO PORT - Translate HR to error
				}
				if (FAILED(hr = mpDevice->GetRenderTargetData(pStretchSurf, pTempSurf)))
				{
					SAFE_RELEASE(pTempSurf);
					SAFE_RELEASE(pStretchSurf);
					BS_EXCEPT(RenderingAPIException, "Can't get render target data:  TODO PORT NO ERROR"); // TODO PORT - Translate HR to error
				}
				SAFE_RELEASE(pStretchSurf);
			}

			if ((dst.getLeft() == 0) && (dst.getRight() == renderWindow->getWidth()) && (dst.getTop() == 0) && (dst.getBottom() == renderWindow->getHeight()))
			{
				hr = pTempSurf->LockRect(&lockedRect, 0, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
			}
			else
			{
				RECT rect;

				rect.left = static_cast<LONG>(dst.getLeft());
				rect.right = static_cast<LONG>(dst.getRight());
				rect.top = static_cast<LONG>(dst.getTop());
				rect.bottom = static_cast<LONG>(dst.getBottom());

				hr = pTempSurf->LockRect(&lockedRect, &rect, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
			}
			if (FAILED(hr))
			{
				SAFE_RELEASE(pTempSurf);
				BS_EXCEPT(RenderingAPIException, "Can't lock rect:  TODO PORT NO ERROR"); // TODO PORT - Translate HR to error
			}
		}

		PixelFormat format = BansheeEngine::D3D9Mappings::_getPF(desc.Format);

		if (format == PF_UNKNOWN)
		{
			SAFE_RELEASE(pTempSurf);
			BS_EXCEPT(RenderingAPIException, "Unsupported format");
		}

		PixelData src(dst.getWidth(), dst.getHeight(), 1, format);
		src.setExternalBuffer((UINT8*)lockedRect.pBits);
		src.setRowPitch(lockedRect.Pitch / PixelUtil::getNumElemBytes(format));
		src.setSlicePitch(desc.Height * src.getRowPitch());

		PixelUtil::bulkPixelConversion(src, dst);

		SAFE_RELEASE(pTempSurf);
		SAFE_RELEASE(pSurf);
	}
}
