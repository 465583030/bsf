//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsBlendState.h"
#include "BsRenderStateManager.h"

namespace BansheeEngine
{
	template<> struct RTTIPlainType<BLEND_STATE_DESC>
	{	
		enum { id = TID_BLEND_STATE_DESC }; enum { hasDynamicSize = 1 };

		static void toMemory(const BLEND_STATE_DESC& data, char* memory)
		{ 
			UINT32 size = getDynamicSize(data);

			memcpy(memory, &size, sizeof(UINT32));
			memory += sizeof(UINT32);
			size -= sizeof(UINT32);
			memcpy(memory, &data, size); 
		}

		static UINT32 fromMemory(BLEND_STATE_DESC& data, char* memory)
		{ 
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32)); 
			memory += sizeof(UINT32);

			UINT32 dataSize = size - sizeof(UINT32);
			memcpy((void*)&data, memory, dataSize); 

			return size;
		}

		static UINT32 getDynamicSize(const BLEND_STATE_DESC& data)	
		{ 
			UINT64 dataSize = sizeof(data) + sizeof(UINT32);

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<UINT32>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (UINT32)dataSize;
		}	
	}; 

	class BS_CORE_EXPORT BlendStateRTTI : public RTTIType<BlendState, IReflectable, BlendStateRTTI>
	{
	private:
		BLEND_STATE_DESC& getData(BlendState* obj) { return obj->mData; }
		void setData(BlendState* obj, BLEND_STATE_DESC& val) 
		{ 
			obj->mRTTIData = val;
		} 

	public:
		BlendStateRTTI()
		{
			addPlainField("mData", 0, &BlendStateRTTI::getData, &BlendStateRTTI::setData);
		}

		virtual void onDeserializationEnded(IReflectable* obj)
		{
			BlendState* blendState = static_cast<BlendState*>(obj);
			if(!blendState->mRTTIData.empty())
			{
				BLEND_STATE_DESC desc = any_cast<BLEND_STATE_DESC>(blendState->mRTTIData);

				blendState->initialize(desc);
			}

		}

		virtual const String& getRTTIName()
		{
			static String name = "BlendState";
			return name;
		}

		virtual UINT32 getRTTIId()
		{
			return TID_BlendState;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject()
		{
			return RenderStateManager::instance().createEmptyBlendState();
		}
	};
}