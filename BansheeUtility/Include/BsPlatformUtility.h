#pragma once

namespace BansheeEngine
{
	struct MACAddress;

	/**
	 * @brief	Possible type of platform file dialogs.
	 */
	enum class FileDialogType
	{
		OpenFile = 0x0, 
		OpenFolder = 0x1, 
		Save = 0x2,
		Multiselect = 0x10000,
		TypeMask = 0xFFFF
	};

	/**
	 * @brief	Provides access to various operating system specific utility functions.
	 */
	class BS_UTILITY_EXPORT PlatformUtility
	{
	public:
		/**
		 * @brief	Terminates the current process.
		 * 			
		 * @param	force	True if the process should be forcefully terminated with no cleanup.
		 */
		static void terminate(bool force = false);

		/**
		 * @brief	Queries the internal system performance counter you can use for very precise time
		 * 			measurements. Value is in milliseconds.
		 *
		 * @note	Thread safe.
		 */
		static double queryPerformanceTimerMs();

		/**
		 * @brief	Adds a string to the clipboard.
		 *
		 * @note	Thread safe.
		 */
		static void copyToClipboard(const WString& string);

		/**
		 * @brief	Reads a string from the clipboard and returns it. If there is no
		 * 			string in the clipboard it returns an empty string.
		 *
		 * @note	Both wide and normal strings will be read, but normal strings will be converted to
		 * 			a wide string before returning.
		 *
		 *			Thread safe.
		 */
		static WString copyFromClipboard();

		/**
		 * @brief	Converts a keyboard key-code to a Unicode character.
		 *
		 * @note	Normally this will output a single character, but it can happen it outputs multiple 
		 *			in case a accent/diacritic character could not be combined with the virtual key into 
		 *			a single character.
		 */
		static WString keyCodeToUnicode(UINT32 keyCode);

		/**
		 * @brief	Populates the provided buffer with a MAC address of the first available
		 *			adapter, if one exists. If no adapters exist, returns false.
		 */
		static bool getMACAddress(MACAddress& address);

		/**
		 * @brief	Creates a new universally unique identifier and returns it as a string.
		 */
		static String generateUUID();

		/**
		 * @brief	Opens the provided file or folder using the default application for that file type, as specified
		 * 			by the operating system.
		 *
		 * @param	path	Absolute path to the file or folder to open.
		 */
		static void open(const Path& path);

		/**
		 * @brief	Displays a platform specific file/folder open/save dialog.
		 *
		 * @param	type		Type of dialog to open.
		 * @param	defaultPath	Initial path the dialog will be set to once opened.
		 * @param	filterList	Semi-colon separated list of file names or types to display in the dialog, e.g. "exe;txt;png".
		 *						Ignored if dialog is to display folders instead of files.
		 * @param	paths		Output list of selected file or folder paths (if any).
		 *
		 * @return	True if file was selected and false if selection was canceled.
		 */
		static bool openBrowseDialog(FileDialogType type, const Path& defaultPath, const WString& filterList,
			Vector<Path>& paths);
	};

	/**
	 * @brief Represents a MAC (ethernet) address.
	 */
	struct MACAddress
	{
		UINT8 value[6];
	};
}