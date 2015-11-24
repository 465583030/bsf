#include "BsFileSystem.h"
#include "BsException.h"
#include "BsDataStream.h"
#include "BsDebug.h"
#include <windows.h>

namespace BansheeEngine
{
	void win32_handleError(DWORD error, const WString& path)
	{
		switch (error)
		{
		case ERROR_FILE_NOT_FOUND:
			LOGERR("File at path: \"" + toString(path) + "\" not found.");
			break;
		case ERROR_PATH_NOT_FOUND:
		case ERROR_BAD_NETPATH:
		case ERROR_CANT_RESOLVE_FILENAME:
		case ERROR_INVALID_DRIVE:
			LOGERR("Path \"" + toString(path) + "\" not found.");
			break;
		case ERROR_ACCESS_DENIED:
			LOGERR("Access to path \"" + toString(path) + "\" denied.");
			break;
		case ERROR_ALREADY_EXISTS:
		case ERROR_FILE_EXISTS:
			LOGERR("File/folder at path \"" + toString(path) + "\" already exists.");
			break;
		case ERROR_INVALID_NAME:
		case ERROR_DIRECTORY:
		case ERROR_FILENAME_EXCED_RANGE:
		case ERROR_BAD_PATHNAME:
			LOGERR("Invalid path string: \"" + toString(path) + "\".");
			break;
		case ERROR_FILE_READ_ONLY:
			LOGERR("File at path \"" + toString(path) + "\" is read only.");
			break;
		case ERROR_CANNOT_MAKE:
			LOGERR("Cannot create file/folder at path: \"" + toString(path) + "\".");
			break;
		case ERROR_DIR_NOT_EMPTY:
			LOGERR("Directory at path \"" + toString(path) + "\" not empty.");
			break;
		case ERROR_WRITE_FAULT:
			LOGERR("Error while writing a file at path \"" + toString(path) + "\".");
			break;
		case ERROR_READ_FAULT:
			LOGERR("Error while reading a file at path \"" + toString(path) + "\".");
			break;
		case ERROR_SHARING_VIOLATION:
			LOGERR("Sharing violation at path \"" + toString(path) + "\".");
			break;
		case ERROR_LOCK_VIOLATION:
			LOGERR("Lock violation at path \"" + toString(path) + "\".");
			break;
		case ERROR_HANDLE_EOF:
			LOGERR("End of file reached for file at path \"" + toString(path) + "\".");
			break;
		case ERROR_HANDLE_DISK_FULL:
		case ERROR_DISK_FULL:
			LOGERR("Disk full.");
			break;
		case ERROR_NEGATIVE_SEEK:
			LOGERR("Negative seek.");
			break;
		default:
			LOGERR("Undefined file system exception: " + toString((UINT32)error));
			break;
		}
	}

	WString win32_getCurrentDirectory()
	{
		DWORD len = GetCurrentDirectoryW(0, NULL);
		if (len > 0)
		{
			wchar_t* buffer = (wchar_t*)bs_alloc(len * sizeof(wchar_t));

			DWORD n = GetCurrentDirectoryW(len, buffer);
			if (n > 0 && n <= len)
			{
				WString result(buffer);
				if (result[result.size() - 1] != '\\')
					result.append(L"\\");

				bs_free(buffer);
				return result;
			}

			bs_free(buffer);
		}

		return StringUtil::WBLANK;
	}

	WString win32_getTempDirectory()
	{
		DWORD len = GetTempPathW(0, NULL);
		if (len > 0)
		{
			wchar_t* buffer = (wchar_t*)bs_alloc(len * sizeof(wchar_t));

			DWORD n = GetTempPathW(len, buffer);
			if (n > 0 && n <= len)
			{
				WString result(buffer);
				if (result[result.size() - 1] != '\\')
					result.append(L"\\");

				bs_free(buffer);
				return result;
			}

			bs_free(buffer);
		}

		return StringUtil::WBLANK;
	}

	bool win32_pathExists(const WString& path)
	{
		DWORD attr = GetFileAttributesW(path.c_str());
		if (attr == 0xFFFFFFFF)
		{
			switch (GetLastError())
			{
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
			case ERROR_NOT_READY:
			case ERROR_INVALID_DRIVE:
				return false;
			default:
				win32_handleError(GetLastError(), path);
			}
		}
		return true;
	}

	bool win32_isDirectory(const WString& path)
	{
		DWORD attr = GetFileAttributesW(path.c_str());
		if (attr == 0xFFFFFFFF)
			win32_handleError(GetLastError(), path);

		return (attr & FILE_ATTRIBUTE_DIRECTORY) != FALSE;
	}

	bool win32_isDevice(const WString& path)
	{
		WString ucPath = path;
		StringUtil::toUpperCase(ucPath);

		return
			ucPath.compare(0, 4, L"\\\\.\\") == 0 ||
			ucPath.compare(L"CON") == 0 ||
			ucPath.compare(L"PRN") == 0 ||
			ucPath.compare(L"AUX") == 0 ||
			ucPath.compare(L"NUL") == 0 ||
			ucPath.compare(L"LPT1") == 0 ||
			ucPath.compare(L"LPT2") == 0 ||
			ucPath.compare(L"LPT3") == 0 ||
			ucPath.compare(L"LPT4") == 0 ||
			ucPath.compare(L"LPT5") == 0 ||
			ucPath.compare(L"LPT6") == 0 ||
			ucPath.compare(L"LPT7") == 0 ||
			ucPath.compare(L"LPT8") == 0 ||
			ucPath.compare(L"LPT9") == 0 ||
			ucPath.compare(L"COM1") == 0 ||
			ucPath.compare(L"COM2") == 0 ||
			ucPath.compare(L"COM3") == 0 ||
			ucPath.compare(L"COM4") == 0 ||
			ucPath.compare(L"COM5") == 0 ||
			ucPath.compare(L"COM6") == 0 ||
			ucPath.compare(L"COM7") == 0 ||
			ucPath.compare(L"COM8") == 0 ||
			ucPath.compare(L"COM9") == 0;
	}

	bool win32_isFile(const WString& path)
	{
		return !win32_isDirectory(path) && !win32_isDevice(path);
	}

	bool win32_createFile(const WString& path)
	{
		HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, 0, CREATE_NEW, 0, 0);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			return true;
		}
		else if (GetLastError() == ERROR_FILE_EXISTS)
			return false;
		else
			win32_handleError(GetLastError(), path);

		return false;
	}


	bool win32_createDirectory(const WString& path)
	{
		if (win32_pathExists(path) && win32_isDirectory(path))
			return false;

		if (CreateDirectoryW(path.c_str(), 0) == FALSE)
			win32_handleError(GetLastError(), path);

		return true;
	}

	void win32_remove(const WString& path)
	{
		if (win32_isDirectory(path))
		{
			if (RemoveDirectoryW(path.c_str()) == 0)
				win32_handleError(GetLastError(), path);
		}
		else
		{
			if (DeleteFileW(path.c_str()) == 0)
				win32_handleError(GetLastError(), path);
		}
	}

	void win32_copyFile(const WString& from, const WString& to)
	{
		if (CopyFileW(from.c_str(), to.c_str(), FALSE) == FALSE)
			win32_handleError(GetLastError(), from);
	}

	void win32_rename(const WString& oldPath, const WString& newPath)
	{
		if (MoveFileW(oldPath.c_str(), newPath.c_str()) == 0)
			win32_handleError(GetLastError(), oldPath);
	}

	UINT64 win32_getFileSize(const WString& path)
	{
		WIN32_FILE_ATTRIBUTE_DATA attrData;
		if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &attrData) == FALSE)
			win32_handleError(GetLastError(), path);

		LARGE_INTEGER li;
		li.LowPart = attrData.nFileSizeLow;
		li.HighPart = attrData.nFileSizeHigh;
		return (UINT64)li.QuadPart;
	}

	std::time_t win32_getLastModifiedTime(const WString& path)
	{
		WIN32_FILE_ATTRIBUTE_DATA fad;
		if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &fad) == 0)
			win32_handleError(GetLastError(), path);

		ULARGE_INTEGER ull;
		ull.LowPart = fad.ftLastWriteTime.dwLowDateTime;
		ull.HighPart = fad.ftLastWriteTime.dwHighDateTime;

		return (std::time_t) ((ull.QuadPart / 10000000ULL) - 11644473600ULL);
	}

	DataStreamPtr FileSystem::openFile(const Path& fullPath, bool readOnly)
	{
		WString pathWString = fullPath.toWString();
		const wchar_t* pathString = pathWString.c_str();

		if (!win32_pathExists(pathString) || !win32_isFile(pathString))
		{
			LOGWRN("Attempting to open a file that doesn't exist: " + fullPath.toString());
			return nullptr;
		}

		UINT64 fileSize = getFileSize(fullPath);

		// Always open in binary mode
		// Also, always include reading
		std::ios::openmode mode = std::ios::in | std::ios::binary;
		std::shared_ptr<std::istream> baseStream = 0;
		std::shared_ptr<std::ifstream> roStream = 0;
		std::shared_ptr<std::fstream> rwStream = 0;

		if (!readOnly)
		{
			mode |= std::ios::out;
			rwStream = bs_shared_ptr_new<std::fstream>();
			rwStream->open(pathString, mode);
			baseStream = rwStream;
		}
		else
		{
			roStream = bs_shared_ptr_new<std::ifstream>();
			roStream->open(pathString, mode);
			baseStream = roStream;
		}

		// Should check ensure open succeeded, in case fail for some reason.
		if (baseStream->fail())
		{
			LOGWRN("Cannot open file: " + fullPath.toString());
			return nullptr;
		}

		/// Construct return stream, tell it to delete on destroy
		FileDataStream* stream = 0;
		if (rwStream)
		{
			// use the writeable stream 
			stream = bs_new<FileDataStream>(rwStream, (size_t)fileSize, true);
		}
		else
		{
			// read-only stream
			stream = bs_new<FileDataStream>(roStream, (size_t)fileSize, true);
		}

		return bs_shared_ptr<FileDataStream>(stream);
	}

	DataStreamPtr FileSystem::createAndOpenFile(const Path& fullPath)
	{
		// Always open in binary mode
		// Also, always include reading
		std::ios::openmode mode = std::ios::out | std::ios::binary;
		std::shared_ptr<std::fstream> rwStream = bs_shared_ptr_new<std::fstream>();
		rwStream->open(fullPath.toWString().c_str(), mode);

		// Should check ensure open succeeded, in case fail for some reason.
		if (rwStream->fail())
			BS_EXCEPT(FileNotFoundException, "Cannot open file: " + fullPath.toString());

		/// Construct return stream, tell it to delete on destroy
		return bs_shared_ptr_new<FileDataStream>(rwStream, 0, true);
	}

	UINT64 FileSystem::getFileSize(const Path& fullPath)
	{
		return win32_getFileSize(fullPath.toWString());
	}

	void FileSystem::remove(const Path& fullPath, bool recursively)
	{
		WString fullPathStr = fullPath.toWString();

		if (!FileSystem::exists(fullPath))
			return;

		if (recursively)
		{
			Vector<Path> files;
			Vector<Path> directories;

			getChildren(fullPath, files, directories);

			for (auto& file : files)
				remove(file, false);

			for (auto& dir : directories)
				remove(dir, true);
		}

		win32_remove(fullPathStr);
	}

	void FileSystem::move(const Path& oldPath, const Path& newPath, bool overwriteExisting)
	{
		WString oldPathStr = oldPath.toWString();
		WString newPathStr = newPath.toWString();

		if (win32_pathExists(newPathStr))
		{
			if (overwriteExisting)
				win32_remove(newPathStr);
			else
			{
				LOGWRN("Move operation failed because another file already exists at the new path: \"" + toString(newPathStr) + "\"");
				return;
			}
		}

		win32_rename(oldPathStr, newPathStr);
	}

	void FileSystem::copy(const Path& oldPath, const Path& newPath, bool overwriteExisting)
	{
		Stack<std::tuple<Path, Path>> todo;
		todo.push(std::make_tuple(oldPath, newPath));

		while (!todo.empty())
		{
			auto current = todo.top();
			todo.pop();

			Path sourcePath = std::get<0>(current);
			WString sourcePathStr = sourcePath.toWString();
			if (!win32_pathExists(sourcePathStr))
				continue;

			bool srcIsFile = win32_isFile(sourcePathStr);

			Path destinationPath = std::get<1>(current);
			WString destPathStr = destinationPath.toWString();

			bool destExists = win32_pathExists(destPathStr);
			if (destExists)
			{
				if (win32_isFile(destPathStr))
				{
					if (overwriteExisting)
						win32_remove(destPathStr);
					else
					{
						LOGWRN("Copy operation failed because another file already exists at the new path: \"" + toString(destPathStr) + "\"");
						return;
					}
				}
			}

			bool destIsFile = !destinationPath.getWExtension().empty();

			if (!srcIsFile && destIsFile)
			{
				LOGWRN("Cannot copy a source folder to a destination file.");
				return;
			}
			else if (srcIsFile && !destIsFile)
			{
				Path destinationFilePath = destinationPath;
				destinationFilePath.append(sourcePath.getWTail());

				win32_copyFile(sourcePathStr, destinationFilePath.toWString());
			}
			else if (srcIsFile && destIsFile)
			{
				win32_copyFile(sourcePathStr, destPathStr);
			}
			else if (!srcIsFile && !destIsFile)
			{
				if (!destExists)
					win32_createDirectory(destPathStr);

				Vector<Path> files;
				Vector<Path> directories;
				getChildren(destinationPath, files, directories);

				for (auto& file : files)
				{
					Path fileDestPath = destinationPath;
					fileDestPath.append(file.getWTail());

					todo.push(std::make_tuple(file, fileDestPath));
				}

				for (auto& dir : directories)
				{
					Path dirDestPath = destinationPath;
					dirDestPath.append(dir.getWTail());

					todo.push(std::make_tuple(dir, dirDestPath));
				}
			}
		}
	}

	bool FileSystem::exists(const Path& fullPath)
	{
		return win32_pathExists(fullPath.toWString());
	}

	bool FileSystem::isFile(const Path& fullPath)
	{
		WString pathStr = fullPath.toWString();

		return win32_pathExists(pathStr) && win32_isFile(pathStr);
	}

	bool FileSystem::isDirectory(const Path& fullPath)
	{
		WString pathStr = fullPath.toWString();

		return win32_pathExists(pathStr) && win32_isDirectory(pathStr);
	}

	void FileSystem::createDir(const Path& fullPath)
	{
		Path parentPath = fullPath;
		while (!exists(parentPath) && parentPath.getNumDirectories() > 0)
		{
			parentPath = parentPath.getParent();
		}

		for (UINT32 i = parentPath.getNumDirectories(); i < fullPath.getNumDirectories(); i++)
		{
			parentPath.append(fullPath[i]);
			win32_createDirectory(parentPath.toWString());
		}

		if (fullPath.isFile())
			win32_createDirectory(fullPath.toWString());
	}

	void FileSystem::getChildren(const Path& dirPath, Vector<Path>& files, Vector<Path>& directories)
	{
		if (dirPath.isFile())
			return;

		WString findPath = dirPath.toWString();
		findPath.append(L"*");

		WIN32_FIND_DATAW findData;
		HANDLE fileHandle = FindFirstFileW(findPath.c_str(), &findData);

		bool lastFailed = false;
		WString tempName;
		do
		{
			if (lastFailed || fileHandle == INVALID_HANDLE_VALUE)
			{
				if (GetLastError() == ERROR_NO_MORE_FILES)
					break;
				else
					win32_handleError(GetLastError(), findPath);
			}
			else
			{
				tempName = findData.cFileName;

				if (tempName != L"." && tempName != L"..")
				{
					Path fullPath = dirPath;
					if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
						directories.push_back(fullPath.append(tempName + L"/"));
					else
						files.push_back(fullPath.append(tempName));
				}
			}

			lastFailed = FindNextFileW(fileHandle, &findData) == FALSE;
		} while (true);

		FindClose(fileHandle);
	}

	bool FileSystem::iterate(const Path& dirPath, std::function<bool(const Path&)> fileCallback,
		std::function<bool(const Path&)> dirCallback, bool recursive)
	{
		if (dirPath.isFile())
			return true;

		WString findPath = dirPath.toWString();
		findPath.append(L"*");

		WIN32_FIND_DATAW findData;
		HANDLE fileHandle = FindFirstFileW(findPath.c_str(), &findData);

		bool lastFailed = false;
		WString tempName;
		do
		{
			if (lastFailed || fileHandle == INVALID_HANDLE_VALUE)
			{
				if (GetLastError() == ERROR_NO_MORE_FILES)
					break;
				else
					win32_handleError(GetLastError(), findPath);
			}
			else
			{
				tempName = findData.cFileName;

				if (tempName != L"." && tempName != L"..")
				{
					Path fullPath = dirPath;
					if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
					{
						Path childDir = fullPath.append(tempName + L"/");
						if (dirCallback != nullptr)
						{
							if (!dirCallback(childDir))
							{
								FindClose(fileHandle);
								return false;
							}
						}

						if (recursive)
						{
							if (!iterate(childDir, fileCallback, dirCallback, recursive))
							{
								FindClose(fileHandle);
								return false;
							}
						}
					}
					else
					{
						Path filePath = fullPath.append(tempName);
						if (fileCallback != nullptr)
						{
							if (!fileCallback(filePath))
							{
								FindClose(fileHandle);
								return false;
							}
						}
					}
				}
			}

			lastFailed = FindNextFileW(fileHandle, &findData) == FALSE;
		} while (true);

		FindClose(fileHandle);
		return true;
	}

	std::time_t FileSystem::getLastModifiedTime(const Path& fullPath)
	{
		return win32_getLastModifiedTime(fullPath.toWString().c_str());
	}

	Path FileSystem::getWorkingDirectoryPath()
	{
		return Path(win32_getCurrentDirectory());
	}

	Path FileSystem::getTempDirectoryPath()
	{
		return Path(win32_getTempDirectory());
	}
}