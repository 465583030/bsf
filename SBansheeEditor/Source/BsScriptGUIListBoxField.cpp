#include "BsScriptGUIListBoxField.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsGUIListBoxField.h"
#include "BsGUIOptions.h"
#include "BsGUIContent.h"
#include "BsScriptHString.h"
#include "BsScriptGUIContent.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptGUIListBoxField::OnSelectionChangedThunkDef ScriptGUIListBoxField::onSelectionChangedThunk;

	ScriptGUIListBoxField::ScriptGUIListBoxField(MonoObject* instance, GUIListBoxField* listBoxField)
		:TScriptGUIElement(instance, listBoxField)
	{

	}

	void ScriptGUIListBoxField::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIListBoxField::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetElements", &ScriptGUIListBoxField::internal_setElements);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUIListBoxField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUIListBoxField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUIListBoxField::internal_setTint);
		metaData.scriptClass->addInternalCall("Internal_SelectElement", &ScriptGUIListBoxField::internal_selectElement);
		metaData.scriptClass->addInternalCall("Internal_DeselectElement", &ScriptGUIListBoxField::internal_deselectElement);
		metaData.scriptClass->addInternalCall("Internal_GetElementStates", &ScriptGUIListBoxField::internal_getElementStates);
		metaData.scriptClass->addInternalCall("Internal_SetElementStates", &ScriptGUIListBoxField::internal_setElementStates);

		onSelectionChangedThunk = (OnSelectionChangedThunkDef)metaData.scriptClass->getMethod("DoOnSelectionChanged", 1)->getThunk();
	}

	void ScriptGUIListBoxField::internal_createInstance(MonoObject* instance, MonoArray* elements, bool multiselect, MonoObject* title, 
		UINT32 titleWidth, MonoString* style, MonoArray* guiOptions, bool withTitle)
	{
		GUIOptions options;

		UINT32 arrayLen = (UINT32)mono_array_length(guiOptions);
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(mono_array_get(guiOptions, GUIOption, i));

		String styleName = toString(MonoUtil::monoToWString(style));

		UINT32 elementsArrayLen = (UINT32)mono_array_length(elements);
		Vector<HString> nativeElements;
		for (UINT32 i = 0; i < elementsArrayLen; i++)
		{
			MonoObject* stringManaged = (MonoObject*)mono_array_get(elements, MonoObject*, i);

			if (stringManaged == nullptr)
				nativeElements.push_back(HString::dummy());
			else
			{
				ScriptHString* textScript = ScriptHString::toNative(stringManaged);
				nativeElements.push_back(textScript->getInternalValue());
			}
		}

		GUIListBoxField* guiField = nullptr;
		if (withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			guiField = GUIListBoxField::create(nativeElements, multiselect, nativeContent, titleWidth, options, styleName);
		}
		else
		{
			guiField = GUIListBoxField::create(nativeElements, multiselect, options, styleName);
		}

		guiField->onSelectionChanged.connect(std::bind(&ScriptGUIListBoxField::onSelectionChanged, instance, _1));

		ScriptGUIListBoxField* nativeInstance = new (bs_alloc<ScriptGUIListBoxField>()) ScriptGUIListBoxField(instance, guiField);
	}

	UINT32 ScriptGUIListBoxField::internal_getValue(ScriptGUIListBoxField* nativeInstance)
	{
		GUIListBoxField* field = static_cast<GUIListBoxField*>(nativeInstance->getGUIElement());

		const Vector<bool>& states = field->getElementStates();
		for (UINT32 i = 0; i < (UINT32)states.size(); i++)
		{
			if (states[i])
				return i;
		}

		return UINT_MAX;
	}

	void ScriptGUIListBoxField::internal_setValue(ScriptGUIListBoxField* nativeInstance, UINT32 index)
	{
		GUIListBoxField* field = static_cast<GUIListBoxField*>(nativeInstance->getGUIElement());
		Vector<bool> states = field->getElementStates();

		UINT32 numElements = (UINT32)states.size();
		if (index >= numElements)
			return;

		for (UINT32 i = 0; i < numElements; i++)
		{
			if (states[i])
				states[i] = true;
			else
				states[i] = false;
		}

		field->setElementStates(states);
	}

	void ScriptGUIListBoxField::internal_setElements(ScriptGUIListBoxField* nativeInstance, MonoArray* elements)
	{
		UINT32 elementsArrayLen = (UINT32)mono_array_length(elements);
		Vector<HString> nativeElements;
		for (UINT32 i = 0; i < elementsArrayLen; i++)
		{
			MonoObject* stringManaged = (MonoObject*)mono_array_get(elements, MonoObject*, i);

			if (stringManaged == nullptr)
				nativeElements.push_back(HString::dummy());
			else
			{
				ScriptHString* textScript = ScriptHString::toNative(stringManaged);
				nativeElements.push_back(textScript->getInternalValue());
			}
		}

		GUIListBoxField* field = static_cast<GUIListBoxField*>(nativeInstance->getGUIElement());
		field->setElements(nativeElements);
	}

	void ScriptGUIListBoxField::internal_selectElement(ScriptGUIListBoxField* nativeInstance, int idx)
	{
		GUIListBoxField* listBox = (GUIListBoxField*)nativeInstance->getGUIElement();
		listBox->selectElement(idx);
	}

	void ScriptGUIListBoxField::internal_deselectElement(ScriptGUIListBoxField* nativeInstance, int idx)
	{
		GUIListBoxField* listBox = (GUIListBoxField*)nativeInstance->getGUIElement();
		listBox->deselectElement(idx);
	}

	MonoArray* ScriptGUIListBoxField::internal_getElementStates(ScriptGUIListBoxField* nativeInstance)
	{
		GUIListBoxField* listBox = (GUIListBoxField*)nativeInstance->getGUIElement();
		const Vector<bool>& states = listBox->getElementStates();

		UINT32 numElements = (UINT32)states.size();
		ScriptArray outStates = ScriptArray::create<bool>(numElements);

		for (UINT32 i = 0; i < numElements; i++)
			outStates.set(i, states[i]);

		return outStates.getInternal();
	}

	void ScriptGUIListBoxField::internal_setElementStates(ScriptGUIListBoxField* nativeInstance, MonoArray* monoStates)
	{
		if (monoStates == nullptr)
			return;

		ScriptArray inStates(monoStates);
		UINT32 numElements = inStates.size();

		Vector<bool> states(numElements);
		for (UINT32 i = 0; i < numElements; i++)
			states[i] = inStates.get<bool>(i);

		GUIListBoxField* listBox = (GUIListBoxField*)nativeInstance->getGUIElement();
		listBox->setElementStates(states);
	}

	void ScriptGUIListBoxField::internal_setTint(ScriptGUIListBoxField* nativeInstance, Color color)
	{
		GUIListBoxField* field = (GUIListBoxField*)nativeInstance->getGUIElement();
		field->setTint(color);
	}

	void ScriptGUIListBoxField::onSelectionChanged(MonoObject* instance, UINT32 newIndex)
	{
		MonoUtil::invokeThunk(onSelectionChangedThunk, instance, newIndex);
	}
}