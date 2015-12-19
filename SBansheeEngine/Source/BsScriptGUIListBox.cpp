#include "BsScriptGUIListBox.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoManager.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUILayout.h"
#include "BsGUIListBox.h"
#include "BsGUIOptions.h"
#include "BsScriptGUIElementStyle.h"
#include "BsScriptGUILayout.h"
#include "BsScriptHString.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptGUIListBox::OnSelectionChangedThunkDef ScriptGUIListBox::onSelectionChangedThunk;

	ScriptGUIListBox::ScriptGUIListBox(MonoObject* instance, GUIListBox* listBox)
		:TScriptGUIElement(instance, listBox)
	{

	}

	void ScriptGUIListBox::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIListBox::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetElements", &ScriptGUIListBox::internal_setElements);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUIListBox::internal_setTint);
		metaData.scriptClass->addInternalCall("Internal_SelectElement", &ScriptGUIListBox::internal_selectElement);
		metaData.scriptClass->addInternalCall("Internal_DeselectElement", &ScriptGUIListBox::internal_deselectElement);
		metaData.scriptClass->addInternalCall("Internal_GetElementStates", &ScriptGUIListBox::internal_getElementStates);
		metaData.scriptClass->addInternalCall("Internal_SetElementStates", &ScriptGUIListBox::internal_setElementStates);

		onSelectionChangedThunk = (OnSelectionChangedThunkDef)metaData.scriptClass->getMethod("DoOnSelectionChanged", 1)->getThunk();
	}

	void ScriptGUIListBox::internal_createInstance(MonoObject* instance, MonoArray* elements, bool multiselect,
		MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		UINT32 optionsArrayLen = (UINT32)mono_array_length(guiOptions);
		for(UINT32 i = 0; i < optionsArrayLen; i++)
			options.addOption(mono_array_get(guiOptions, GUIOption, i));

		UINT32 elementsArrayLen = (UINT32)mono_array_length(elements);
		Vector<HString> nativeElements;
		for(UINT32 i = 0; i < elementsArrayLen; i++)
		{
			MonoObject* stringManaged = (MonoObject*)mono_array_get(elements, MonoObject*, i);

			if(stringManaged == nullptr)
				nativeElements.push_back(HString::dummy());
			else
			{
				ScriptHString* textScript = ScriptHString::toNative(stringManaged);
				nativeElements.push_back(textScript->getInternalValue());
			}
		}

		GUIListBox* guiListBox = GUIListBox::create(nativeElements, multiselect, options, toString(MonoUtil::monoToWString(style)));
		guiListBox->onSelectionToggled.connect(std::bind(&ScriptGUIListBox::onSelectionChanged, instance, _1, _2));

		ScriptGUIListBox* nativeInstance = new (bs_alloc<ScriptGUIListBox>()) ScriptGUIListBox(instance, guiListBox);
	}

	void ScriptGUIListBox::internal_setElements(ScriptGUIListBox* nativeInstance, MonoArray* elements)
	{
		UINT32 elementsArrayLen = (UINT32)mono_array_length(elements);
		Vector<HString> nativeElements;
		for(UINT32 i = 0; i < elementsArrayLen; i++)
		{
			MonoObject* stringManaged = (MonoObject*)mono_array_get(elements, MonoObject*, i);

			if(stringManaged == nullptr)
				nativeElements.push_back(HString::dummy());
			else
			{
				ScriptHString* textScript = ScriptHString::toNative(stringManaged);
				nativeElements.push_back(textScript->getInternalValue());
			}
		}

		GUIListBox* listBox = (GUIListBox*)nativeInstance->getGUIElement();
		listBox->setElements(nativeElements);
	}

	void ScriptGUIListBox::internal_setTint(ScriptGUIListBox* nativeInstance, Color* color)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->getGUIElement();
		listBox->setTint(*color);
	}

	void ScriptGUIListBox::internal_selectElement(ScriptGUIListBox* nativeInstance, int idx)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->getGUIElement();
		listBox->selectElement(idx);
	}

	void ScriptGUIListBox::internal_deselectElement(ScriptGUIListBox* nativeInstance, int idx)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->getGUIElement();
		listBox->deselectElement(idx);
	}

	MonoArray* ScriptGUIListBox::internal_getElementStates(ScriptGUIListBox* nativeInstance)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->getGUIElement();
		const Vector<bool>& states = listBox->getElementStates();

		UINT32 numElements = (UINT32)states.size();
		ScriptArray outStates = ScriptArray::create<bool>(numElements);

		for (UINT32 i = 0; i < numElements; i++)
			outStates.set(i, states[i]);

		return outStates.getInternal();
	}

	void ScriptGUIListBox::internal_setElementStates(ScriptGUIListBox* nativeInstance, MonoArray* monoStates)
	{
		if (monoStates == nullptr)
			return;

		ScriptArray inStates(monoStates);
		UINT32 numElements = inStates.size();

		Vector<bool> states(numElements);
		for (UINT32 i = 0; i < numElements; i++)
			states[i] = inStates.get<bool>(i);

		GUIListBox* listBox = (GUIListBox*)nativeInstance->getGUIElement();
		listBox->setElementStates(states);
	}

	void ScriptGUIListBox::onSelectionChanged(MonoObject* instance, UINT32 index, bool enabled)
	{
		MonoUtil::invokeThunk(onSelectionChangedThunk, instance, index);
	}
}