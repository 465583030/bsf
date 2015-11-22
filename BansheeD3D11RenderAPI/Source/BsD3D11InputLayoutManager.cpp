#include "BsD3D11InputLayoutManager.h"
#include "BsD3D11Mappings.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "BsD3D11GpuProgram.h"
#include "BsHardwareBufferManager.h"
#include "BsRenderStats.h"
#include "BsDebug.h"

namespace BansheeEngine
{
	size_t D3D11InputLayoutManager::HashFunc::operator()
		(const D3D11InputLayoutManager::VertexDeclarationKey &key) const
	{
		size_t hash = 0;
		hash_combine(hash, key.vertxDeclId);
		hash_combine(hash, key.vertexProgramId);

		return hash;
	}

	bool D3D11InputLayoutManager::EqualFunc::operator()
		(const D3D11InputLayoutManager::VertexDeclarationKey &a, const D3D11InputLayoutManager::VertexDeclarationKey &b) const
		
	{
		if (a.vertxDeclId != b.vertxDeclId)
			return false;

		if(a.vertexProgramId != b.vertexProgramId)
			return false;

		return true;
	}

	D3D11InputLayoutManager::D3D11InputLayoutManager()
		:mLastUsedCounter(0), mWarningShown(false)
	{

	}

	D3D11InputLayoutManager::~D3D11InputLayoutManager()
	{
		while(mInputLayoutMap.begin() != mInputLayoutMap.end())
		{
			auto firstElem = mInputLayoutMap.begin();

			SAFE_RELEASE(firstElem->second->inputLayout);
			bs_delete(firstElem->second);

			mInputLayoutMap.erase(firstElem);
			BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_InputLayout);
		}
	}

	ID3D11InputLayout* D3D11InputLayoutManager::retrieveInputLayout(const SPtr<VertexDeclarationCore>& vertexShaderDecl, const SPtr<VertexDeclarationCore>& vertexBufferDecl, D3D11GpuProgramCore& vertexProgram)
	{
		VertexDeclarationKey pair;
		pair.vertxDeclId = vertexBufferDecl->getId();
		pair.vertexProgramId = vertexProgram.getProgramId();

		auto iterFind = mInputLayoutMap.find(pair);
		if(iterFind == mInputLayoutMap.end())
		{
			if(mInputLayoutMap.size() >= DECLARATION_BUFFER_SIZE)
				removeLeastUsed(); // Prune so the buffer doesn't just infinitely grow

			addNewInputLayout(vertexShaderDecl, vertexBufferDecl, vertexProgram);

			iterFind = mInputLayoutMap.find(pair);

			if(iterFind == mInputLayoutMap.end()) // We failed to create input layout
				return nullptr;
		}

		iterFind->second->lastUsedIdx = ++mLastUsedCounter;
		return iterFind->second->inputLayout;
	}

	void D3D11InputLayoutManager::addNewInputLayout(const SPtr<VertexDeclarationCore>& vertexShaderDecl, const SPtr<VertexDeclarationCore>& vertexBufferDecl, D3D11GpuProgramCore& vertexProgram)
	{
		if (!vertexBufferDecl->isCompatible(vertexShaderDecl))
			return; // Error was already reported, so just quit here

		const VertexDeclarationProperties& declProps = vertexBufferDecl->getProperties();

		UINT32 numElements = declProps.getElementCount();
		D3D11_INPUT_ELEMENT_DESC* declElements = bs_newN<D3D11_INPUT_ELEMENT_DESC>(numElements);
		ZeroMemory(declElements, sizeof(D3D11_INPUT_ELEMENT_DESC) * numElements);

		unsigned int idx = 0;
		for (auto iter = declProps.getElements().begin(); iter != declProps.getElements().end(); ++iter)
		{
			declElements[idx].SemanticName			= D3D11Mappings::get(iter->getSemantic());
			declElements[idx].SemanticIndex			= iter->getSemanticIdx();
			declElements[idx].Format				= D3D11Mappings::get(iter->getType());
			declElements[idx].InputSlot				= iter->getStreamIdx();
			declElements[idx].AlignedByteOffset		= static_cast<WORD>(iter->getOffset());
			declElements[idx].InputSlotClass		= D3D11_INPUT_PER_VERTEX_DATA;
			declElements[idx].InstanceDataStepRate	= 0;

			idx++;
		}

		D3D11RenderAPI* d3d11rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = d3d11rs->getPrimaryDevice();

		const HLSLMicroCode& microcode = vertexProgram.getMicroCode();

		InputLayoutEntry* newEntry = bs_new<InputLayoutEntry>();
		newEntry->lastUsedIdx = ++mLastUsedCounter;
		newEntry->inputLayout = nullptr; 
		HRESULT hr = device.getD3D11Device()->CreateInputLayout( 
			declElements, 
			numElements, 
			&microcode[0], 
			microcode.size(),
			&newEntry->inputLayout);

		bs_deleteN(declElements, numElements);

		if (FAILED(hr)|| device.hasError())
			BS_EXCEPT(RenderingAPIException, "Unable to set D3D11 vertex declaration" + device.getErrorDescription());

		// Create key and add to the layout map
		VertexDeclarationKey pair;
		pair.vertxDeclId = vertexBufferDecl->getId();
		pair.vertexProgramId = vertexProgram.getProgramId();

		mInputLayoutMap[pair] = newEntry;

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_InputLayout);
	}

	void D3D11InputLayoutManager::removeLeastUsed()
	{
		if(!mWarningShown)
		{
			LOGWRN("Input layout buffer is full, pruning last " + toString(NUM_ELEMENTS_TO_PRUNE) + " elements. This is probably okay unless you are creating a massive amount of input layouts" \
				" as they will get re-created every frame. In that case you should increase the layout buffer size. This warning won't be shown again.");

			mWarningShown = true;
		}

		Map<UINT32, VertexDeclarationKey> leastFrequentlyUsedMap;

		for(auto iter = mInputLayoutMap.begin(); iter != mInputLayoutMap.end(); ++iter)
			leastFrequentlyUsedMap[iter->second->lastUsedIdx] = iter->first;

		UINT32 elemsRemoved = 0;
		for(auto iter = leastFrequentlyUsedMap.begin(); iter != leastFrequentlyUsedMap.end(); ++iter)
		{
			auto inputLayoutIter = mInputLayoutMap.find(iter->second);

			SAFE_RELEASE(inputLayoutIter->second->inputLayout);
			bs_delete(inputLayoutIter->second);

			mInputLayoutMap.erase(inputLayoutIter);
			BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_InputLayout);

			elemsRemoved++;
			if(elemsRemoved >= NUM_ELEMENTS_TO_PRUNE)
				break;
		}
	}
}