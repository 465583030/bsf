#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsRect2I.h"
#include "BsVector2I.h"

namespace BansheeEngine
{
	class BS_SCR_BE_EXPORT ScriptCursor : public ScriptObject <ScriptCursor>
	{
	public:
		SCRIPT_OBJ(BansheeEngineAssemblyName, "BansheeEngine", "Cursor")

	private:
		static void internal_getScreenPosition(Vector2I* value);
		static void internal_setScreenPosition(Vector2I value);
		static void internal_hide();
		static void internal_show();
		static void internal_clipToRect(Rect2I value);
		static void internal_clipDisable();
		static void internal_setCursorStr(MonoString* name);
		static void internal_setCursor(CursorType cursor);
		static void internal_setCursorIconStr(MonoString* name, MonoObject* iconData, Vector2I hotspot);
		static void internal_setCursorIcon(CursorType cursor, MonoObject* iconData, Vector2I hotspot);
		static void internal_clearCursorIconStr(MonoString* name);
		static void internal_clearCursorIcon(CursorType cursor);

		ScriptCursor(MonoObject* instance);
	};
}