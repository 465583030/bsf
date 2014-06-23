#include "BsD3D9VertexDeclaration.h"
#include "BsD3D9Mappings.h"
#include "BsException.h"
#include "BsD3D9RenderSystem.h"
#include "BsD3D9ResourceManager.h"
#include "BsRenderStats.h"

namespace BansheeEngine 
{
	D3D9VertexDeclaration::D3D9VertexDeclaration(const VertexDeclaration::VertexElementList& elements)
		:VertexDeclaration(elements)
    { }

    D3D9VertexDeclaration::~D3D9VertexDeclaration()
    {    }

	void D3D9VertexDeclaration::destroy_internal()
	{
		releaseDeclaration();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_VertexDeclaration);
		VertexDeclaration::destroy_internal();
	}

	void D3D9VertexDeclaration::notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device)
	{
		
	}

	void D3D9VertexDeclaration::notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device)
	{
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		auto it = mMapDeviceToDeclaration.find(d3d9Device);

		if (it != mMapDeviceToDeclaration.end())	
		{
			SAFE_RELEASE(it->second);	
			mMapDeviceToDeclaration.erase(it);
		}
	}

    IDirect3DVertexDeclaration9* D3D9VertexDeclaration::getD3DVertexDeclaration()
    {
		IDirect3DDevice9* pCurDevice   = D3D9RenderSystem::getActiveD3D9Device();
		auto it = mMapDeviceToDeclaration.find(pCurDevice);
		IDirect3DVertexDeclaration9* lpVertDecl = NULL;

		// Case we have to create the declaration for this device.
		if (it == mMapDeviceToDeclaration.end() || it->second == NULL)
		{
			D3DVERTEXELEMENT9* d3delems = bs_newN<D3DVERTEXELEMENT9, PoolAlloc>((UINT32)(mElementList.size() + 1));

			VertexElementList::const_iterator i, iend;
			unsigned int idx;
			iend = mElementList.end();
			for (idx = 0, i = mElementList.begin(); i != iend; ++i, ++idx)
			{
				d3delems[idx].Method = D3DDECLMETHOD_DEFAULT;
				d3delems[idx].Offset = static_cast<WORD>(i->getOffset());
				d3delems[idx].Stream = i->getStreamIdx();
				d3delems[idx].Type = D3D9Mappings::get(i->getType());
				d3delems[idx].Usage = D3D9Mappings::get(i->getSemantic());

				if (i->getSemantic() == VES_COLOR)
				{
					d3delems[idx].UsageIndex = 0;
				}
				else
				{
					d3delems[idx].UsageIndex = static_cast<BYTE>(i->getSemanticIdx());
				}
			}
			// Add terminator
			d3delems[idx].Stream = 0xff;
			d3delems[idx].Offset = 0;
			d3delems[idx].Type = D3DDECLTYPE_UNUSED;
			d3delems[idx].Method = 0;
			d3delems[idx].Usage = 0;
			d3delems[idx].UsageIndex = 0;

			
			HRESULT hr = pCurDevice->CreateVertexDeclaration(d3delems, &lpVertDecl);

			if (FAILED(hr))
			{
				BS_EXCEPT(InternalErrorException, "Cannot create D3D9 vertex declaration: ");
			}

			bs_deleteN<PoolAlloc>(d3delems, (UINT32)(mElementList.size() + 1));

			mMapDeviceToDeclaration[pCurDevice] = lpVertDecl;

			BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_VertexDeclaration);
		}

		// Declaration already exits.
		else
		{
			lpVertDecl = mMapDeviceToDeclaration[pCurDevice];
		}
		
        return lpVertDecl;
    }

	void D3D9VertexDeclaration::releaseDeclaration()
	{
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		auto it = mMapDeviceToDeclaration.begin();

		while (it != mMapDeviceToDeclaration.end())
		{
			SAFE_RELEASE(it->second);
			++it;
		}	
		mMapDeviceToDeclaration.clear();
	}
}

