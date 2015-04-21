SneakySneaky
============

Final assignment for Games Creation Processes

Resources
---------
* Lino Board: http://linoit.com/groups/Game%20Creation%5BTeam%20name%5D
* Urho3D Documentation: http://urho3d.github.io/documentation/HEAD/

Controls
--------

* `Left Click` Move
* `Q` Rotate Left
* `E` Rotate Right
* `T` Throw Object
* `↹ Tab` Show Inventory
* `[space]` Skip Intro / Advance Level

Your Friendly Guide to Developer Mode
-------------------------------------

Tap `` ` `` (the key to the left of 1) to enable developer mode.  
This allows you to access debug rendering as well as various cheats.

### Keys
* `1` Primary Debug Rendering
* `2` Physics Debug Rendering
* `3` Navigation Debug Rendering
* `0` Toggle Depth Testing for Debug Rendering
* `W` Peek Over Room Walls
* `R` Reload the Current Level
* `U` Unlock all Doors in the Current Level
* `D` Toggle Profiler.
* `C` Force a Crash.
* `N` Advance to the Next Level.

`↑` `↑` `↓` `↓` `←` `→` `←` `→` `B` `A`

Commit Guidelines
-----------------
* **DO NOT PUSH COMMITS TO THIS REPOSITORY**

* Source assets should not be committed, we need to find another way to store them (I'm currently using Dropbox).

* Commits should encompass a single logical change, and have a meaningful commit message. Try and avoid letting them get too overfull, but also don't be tempted to commit every time you save the file - generally everything should be in a working state.

Getting Started (Windows - Visual Studio)
-----------------------------------------
### Requirements
* [SourceTree](http://www.sourcetreeapp.com/)
* [Visual Studio 2013](http://www.visualstudio.com/en-us/downloads/download-visual-studio-vs#DownloadFamilies_2)
* [CMake 3.0.x](http://www.cmake.org/download/)

### Building
1. Clone *this repository* into a directory on your computer.
2. Run the `cmake_vs2013.bat` file.
3. In the `build/` directory, open `SneakySneaky.sln`.
4. Set the build configuration to either `Debug` or `RelWithDebInfo`.
5. Build / Debug with Visual Studio as normal.

### Notes
* The first build (and if you re-run CMake) will take a very long time.
* Do not edit any project options in Visual Studio - changes will be overwritten when they are re-generated.
* As an exception, you *can* create new files using Visual Studio, just make sure they are in the `src/` directory.

Getting Started (Mac OS X - Qt Creator)
---------------------------------------
### Requirements
* [SourceTree](http://www.sourcetreeapp.com/)
* [Qt Creator](https://www.qt.io/download-open-source/)
* [CMake 3.0.x](http://www.cmake.org/download/)

### Building
1. Clone *this repository* into a directory on your computer.
2. Open Qt Creator, `File -> Open New Project...`, select the CMakeLists.txt from the repository.
3. Leave the text box empty, click `Run CMake`, then `Done`.
4. Build / Run with Qt Creator as normal.

### Notes
* The first build will take a very long time.
* New source files need to be created outside of Qt Creator, they will be detected next time the project is built.

Importing Models
----------------
After the first build is successful, a number of command-line tools will be created in `Urho3D/Bin/`. The most useful of these is `AssetImporter`. To import a `.obj` model without any cruft, use the following incantation (replacing the relevant paths):
```
Urho3D/Bin/AssetImporter model source/model.obj ./data/Models/Output.mdl -o -h -nm -nt -ns -nf
```
