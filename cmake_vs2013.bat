@pushd %~dp0
"%PROGRAMFILES(x86)%\CMake\bin\cmake.exe" -E make_directory build
"%PROGRAMFILES(x86)%\CMake\bin\cmake.exe" -E chdir build cmake -G "Visual Studio 12 2013 Win64" .. -DCMAKE_CONFIGURATION_TYPES=RelWithDebInfo -DCMAKE_BUILD_TYPE=RelWithDebInfo
@popd
@pause