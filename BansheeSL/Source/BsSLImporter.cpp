//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSLImporter.h"
#include "BsDataStream.h"
#include "BsFileSystem.h"
#include "BsSLFXCompiler.h"

namespace BansheeEngine
{
	SLImporter::SLImporter()
		:SpecificImporter()
	{

	}

	SLImporter::~SLImporter()
	{

	}

	bool SLImporter::isExtensionSupported(const WString& ext) const
	{
		WString lowerCaseExt = ext;
		StringUtil::toLowerCase(lowerCaseExt);

		return lowerCaseExt == L"bsl";
	}

	bool SLImporter::isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const
	{
		return true; // Plain-text so I don't even check for magic number
	}

	ResourcePtr SLImporter::import(const Path& filePath, ConstImportOptionsPtr importOptions)
	{
		DataStreamPtr stream = FileSystem::openFile(filePath);
		String source = stream->getAsString();

		BSLFXCompileResult result = BSLFXCompiler::compile(source);

		if (result.shader != nullptr)
			result.shader->setName(filePath.getWFilename(false));
		else
		{
			String file;
			if (result.errorFile.empty())
				file = filePath.toString();
			else
				file = result.errorFile;

			LOGERR("Error while parsing shader FX code \"" + file + "\":\n" + result.errorMessage + ". Location: " +
				toString(result.errorLine) + " (" + toString(result.errorColumn) + ")");
		}

		return result.shader;
	}
}