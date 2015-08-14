#pragma once

#include "BsEditorPrerequisites.h"
#include "BsEditorWidgetLayout.h"
#include "BsDockManagerLayout.h"
#include "BsRTTIType.h"

namespace BansheeEngine
{
	class EditorWidgetLayoutRTTI : public RTTIType<EditorWidgetLayout, IReflectable, EditorWidgetLayoutRTTI>
	{
	private:
		EditorWidgetLayout::Entry& getEntry(EditorWidgetLayout* obj, UINT32 idx) { return obj->mEntries[idx]; }
		void setEntry(EditorWidgetLayout* obj, UINT32 idx, EditorWidgetLayout::Entry& val) { obj->mEntries[idx] = val; } 
		UINT32 getEntriesArraySize(EditorWidgetLayout* obj) { return (UINT32)obj->mEntries.size(); }
		void setEntriesArraySize(EditorWidgetLayout* obj, UINT32 size) { obj->mEntries.resize(size); }

		DockManagerLayoutPtr getDockLayout(EditorWidgetLayout* obj) { return obj->mDockLayout; }
		void setDockLayout(EditorWidgetLayout* obj, DockManagerLayoutPtr val) { obj->mDockLayout = val; }

		bool& getIsMainWindowMaximized(EditorWidgetLayout* obj) { return obj->mMaximized; }
		void setIsMainWindowMaximized(EditorWidgetLayout* obj, bool& val) { obj->mMaximized = val; }

	public:
		EditorWidgetLayoutRTTI()
		{
			addPlainArrayField("mRootEntry", 0, &EditorWidgetLayoutRTTI::getEntry, &EditorWidgetLayoutRTTI::getEntriesArraySize, 
				&EditorWidgetLayoutRTTI::setEntry, &EditorWidgetLayoutRTTI::setEntriesArraySize);

			addReflectablePtrField("mDockLayout", 1, &EditorWidgetLayoutRTTI::getDockLayout, &EditorWidgetLayoutRTTI::setDockLayout);
			addPlainField("mMaximized", 2, &EditorWidgetLayoutRTTI::getIsMainWindowMaximized, &EditorWidgetLayoutRTTI::setIsMainWindowMaximized);
		}

		virtual const String& getRTTIName()
		{
			static String name = "EditorWidgetLayout";
			return name;
		}

		virtual UINT32 getRTTIId()
		{
			return TID_EditorWidgetLayout;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject()
		{
			return bs_shared_ptr_new<EditorWidgetLayout>(EditorWidgetLayout::PrivatelyConstruct());
		}
	};
}

namespace BansheeEngine
{
	template<> struct RTTIPlainType<BansheeEngine::EditorWidgetLayout::Entry>
	{	
		enum { id = BansheeEngine::TID_EditorWidgetLayoutEntry }; enum { hasDynamicSize = 1 };

		static void toMemory(const BansheeEngine::EditorWidgetLayout::Entry& data, char* memory)
		{ 
			UINT32 size = 0;
			char* memoryStart = memory;
			memory += sizeof(UINT32);
			size += sizeof(UINT32);

			memory = rttiWriteElem(data.widgetNames, memory, size);
			memory = rttiWriteElem(data.isDocked, memory, size);
			memory = rttiWriteElem(data.x, memory, size);
			memory = rttiWriteElem(data.y, memory, size);
			memory = rttiWriteElem(data.width, memory, size);
			memory = rttiWriteElem(data.height, memory, size);

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		static UINT32 fromMemory(BansheeEngine::EditorWidgetLayout::Entry& data, char* memory)
		{ 
			UINT32 size = 0;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			memory = rttiReadElem(data.widgetNames, memory);
			memory = rttiReadElem(data.isDocked, memory);
			memory = rttiReadElem(data.x, memory);
			memory = rttiReadElem(data.y, memory);
			memory = rttiReadElem(data.width, memory);
			memory = rttiReadElem(data.height, memory);

			return size;
		}

		static UINT32 getDynamicSize(const BansheeEngine::EditorWidgetLayout::Entry& data)	
		{ 
			UINT64 dataSize = sizeof(UINT32) + rttiGetElemSize(data.widgetNames) + rttiGetElemSize(data.isDocked) + 
				rttiGetElemSize(data.x) + rttiGetElemSize(data.y) + rttiGetElemSize(data.width) + rttiGetElemSize(data.height);

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<UINT32>::max())
			{
				__string_throwDataOverflowException();
			}
#endif

			return (UINT32)dataSize;
		}	
	}; 
}