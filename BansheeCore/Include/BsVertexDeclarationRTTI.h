//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsVertexBuffer.h"
#include "BsHardwareBufferManager.h"

namespace BansheeEngine
{
	class VertexDeclarationRTTI : public RTTIType<VertexDeclaration, IReflectable, VertexDeclarationRTTI>
	{
	private:
		VertexElement& getElement(VertexDeclaration* obj, UINT32 idx)
		{
			auto iter = obj->mElementList.begin();
			for(UINT32 i = 0; i < idx; i++)
				++iter;

			return *iter;
		}

		void setElement(VertexDeclaration* obj, UINT32 idx, VertexElement& data)
		{
			auto iter = obj->mElementList.begin();
			for(UINT32 i = 0; i < idx; i++)
				++iter;

			*iter = data;
		}

		UINT32 getElementArraySize(VertexDeclaration* obj)
		{
			return (UINT32)obj->mElementList.size();
		}

		void setElementArraySize(VertexDeclaration* obj, UINT32 size)
		{
			for(size_t i = obj->mElementList.size(); i < size; i++)
				obj->mElementList.push_back(VertexElement());
		}

	public:
		VertexDeclarationRTTI()
		{
			addPlainArrayField("mElementList", 0, &VertexDeclarationRTTI::getElement, &VertexDeclarationRTTI::getElementArraySize, 
				&VertexDeclarationRTTI::setElement, &VertexDeclarationRTTI::setElementArraySize);
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject() 
		{
			return HardwareBufferManager::instance().createVertexDeclaration(VertexDeclaration::VertexElementList());
		}

		virtual const String& getRTTIName() 
		{
			static String name = "VertexDeclaration";
			throw name;
		}

		virtual UINT32 getRTTIId() 
		{
			return TID_VertexDeclaration;
		}
	};
}