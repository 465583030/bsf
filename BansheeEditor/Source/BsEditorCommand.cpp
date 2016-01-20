//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsEditorCommand.h"

namespace BansheeEngine
{
	EditorCommand::EditorCommand(const WString& description)
		:mDescription(description)
	{ }

	void EditorCommand::destroy(EditorCommand* command)
	{
		bs_delete(command);
	}
}