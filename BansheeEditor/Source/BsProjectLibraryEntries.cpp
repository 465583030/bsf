#include "BsProjectLibraryEntries.h"
#include "BsProjectLibraryEntriesRTTI.h"

namespace BansheeEngine
{
	ProjectLibraryEntries::ProjectLibraryEntries(const ProjectLibrary::DirectoryEntry& rootEntry)
		:mRootEntry(rootEntry)
	{ }

	ProjectLibraryEntries::ProjectLibraryEntries(const ConstructPrivately& dummy)
	{ }

	std::shared_ptr<ProjectLibraryEntries> ProjectLibraryEntries::create(const ProjectLibrary::DirectoryEntry& rootEntry)
	{
		return bs_shared_ptr<ProjectLibraryEntries>(rootEntry);
	}

	std::shared_ptr<ProjectLibraryEntries> ProjectLibraryEntries::createEmpty()
	{
		return bs_shared_ptr<ProjectLibraryEntries>(ConstructPrivately());
	}

	RTTITypeBase* ProjectLibraryEntries::getRTTIStatic()
	{
		return ProjectLibraryEntriesRTTI::instance();
	}

	RTTITypeBase* ProjectLibraryEntries::getRTTI() const
	{
		return ProjectLibraryEntries::getRTTIStatic();
	}
}