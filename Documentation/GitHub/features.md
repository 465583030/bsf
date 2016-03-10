# Features

Here you will find a complete list of all currently available features. Banshee is current in development and more features are being added frequently.

## Editor
* __Asset management__
  * Simple drag and drop import for many popular formats
  * Automatic reimport of externally modified assets (e.g. modify a shader, see changes in editor right away)
  * Asset modifications immediately reflected in-game (resource hot-swap)
  * Version control friendly format
* __Powerful object inspector__
  * Exposes script object properties for artists/designers
  * Automatically generated GUI for custom classes
  * Customize visible elements via attributes or create GUI manually
* __Level creation__
  * Simple drag and drop interface
  * Traditional set of tools (Move/Scale/Rotate/Select, etc.)
  * Interface for creating custom 2D and 3D tools
* __Prefab system__
  * Save parts or entire levels as prefabs so they may be re-used later
  * Separate larger levels into smaller prefabs for easier loading
  * Reduce conflicts when multiple people are working on the same level
  * Customize individual prefab instances without breaking the prefab link
  * Supports nesting and complex hierarchies to ensure maintaining complex levels is easy
* __Play in editor__
  * Compile all scripts within editor
  * Scripts and data transparently reloaded after compilation so changes may be tested immediately
  * Pause and frame-step to better test and debug your game
  * Analyze and modify scene while playing
* __Fully extensible__
  * Specialized scripting API only for editor extensions
  * Easy to use without needing to know about engine internals
  * Extend almost anything. Create:
	* Custom editor windows
	* Custom object inspectors
	* Custom 2D/3D tools
	* Code for automating common tasks
* __Game publishing__
  * Build a game ready for distribution from within editor
  * One click build process, just choose a platform and go
  * Automatically detects required resources
  * Automatically packages and outputs an executable
* __Customizable frontend__
  * Dockable layout and floating windows
  * Custom GUI skin & localization support
   
## Core
* __Quality design__
  * Modern code using C++14
  * Clean layered design
  * Fully documented
  * Modular & plugin based
  * Minimal third-party dependencies
  * Multiplatform ready
* __Renderer__
  * DX9, DX11 and OpenGL 4.3 render systems
  * Multi-threaded rendering
  * Powerful material system
    * BansheeFX language for material definitions
    * Shader parsing for HLSL9, HLSL11 and GLSL
* __Asset pipeline__
  * Asynchronous resource loading
  * Extensible importer system
  * Available importer plugins for:
    * FBX, OBJ, DAE meshes
    * PNG, PSD, BMP, JPG, ... images
    * OTF, TTF fonts
    * HLSL9, HLSL11, GLSL shaders
* __GUI system__
  * Unicode text rendering and input
  * Easy to use layout based system
  * Many common GUI controls
  * Fully skinnable
  * Automatic batching for fast rendering
  * Supports texture atlases
  * Supports arbitrary 3D transformations
  * Localization support (string tables)
* __Input__
  * Mouse/Keyboard/Gamepad support
  * Provides both raw and OS input
  * Virtual input with built-in key mapping
  * Virtual axes for analog input devices
* __Physics__
  * Implemented using NVIDIA PhysX
  * Multi-threaded for best performance
  * Abstract plugin interface extensible for other physics implementations (e.g. Havok, Bullet)
  * Supported features
    * Colliders (Box, Sphere, Capsule, Mesh)
    * Triggers
    * Rigidbody
    * Character controller
    * Joints (Fixed, Distance, Hinge, Spherical, Slider, D6)
	* Scene queries
	* Collision filtering
	* Discrete or continuous collision detection
* __Scripting__
  * C# 5.0
  * Separate high level engine API
  * Integrated runtime for maximum performance
  * Full access to .NET framework
  * Integration with Visual Studio
  * Automatic serialization
	* Works with custom components, resources or arbitrary types
	* Save/load data with no additional code
	* Handles complex types (e.g. array, list, dictionary) and references
	* Fast and small memory footprint
* __Other__
  * CPU & GPU profiler
  * Advanced run-time type information for C++ code
    * Iterate over class fields, safely cast objects, clone objects, detect base types
	* Find references to specific objects (e.g. all resources used in a scene)
	* Serialize/deserialize with no additional code and with automatic versioning
	* Generate diffs
  * Utility library
    * Math
	* File system
    * Events
	* Thread pool
    * Task scheduler
    * Logging
	* Debug drawing
	* Crash reporting
	* Unit testing
	* Custom memory allocators