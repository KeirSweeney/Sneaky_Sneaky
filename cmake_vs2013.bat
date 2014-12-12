@pushd %~dp0
"%PROGRAMFILES(x86)%\CMake\bin\cmake.exe" -E make_directory build
"%PROGRAMFILES(x86)%\CMake\bin\cmake.exe" -E chdir build cmake -G "Visual Studio 12 2013 Win64" ..
@popd
@pause