if(CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_CONFIGURATION_TYPES "RelWithDebInfo" CACHE STRING "" FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)
endif()

project(TheTraitor)
include(ExternalProject)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_INSTALL_RPATH ".")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Gm- /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gm- /MP")
endif()

aux_source_directory(src SRC_LIST)
add_executable(${PROJECT_NAME} WIN32 ${SRC_LIST})

if(UNIX)
    set(URHO3D_VERSION_SUFFIX ".0")
    set(URHO3D_LIB_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}Urho3D${URHO3D_VERSION_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX})
else()
    set(URHO3D_VERSION_SUFFIX "")
    set(URHO3D_LIB_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}Urho3D${URHO3D_VERSION_SUFFIX}$<$<CONFIG:Debug>:_d>${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

externalproject_add(Urho3D
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Source
    CMAKE_ARGS -DURHO3D_64BIT=1 -DURHO3D_ANGELSCRIPT=0 -DURHO3D_NETWORK=0 -DURHO3D_URHO2D=0 -DURHO3D_TOOLS=1 -DURHO3D_MINIDUMPS=0 -DURHO3D_FILEWATCHER=0 -DURHO3D_OPENGL=1 -DURHO3D_LIB_TYPE=SHARED -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}
    BUILD_IN_SOURCE 0
    INSTALL_COMMAND ${CMAKE_COMMAND} -DDUMP_SYMS=$<TARGET_FILE:dump_syms> -DBINARY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Lib/${URHO3D_LIB_NAME} -P ${CMAKE_CURRENT_SOURCE_DIR}/DumpSymbols.cmake
    TEST_COMMAND ""
)

# Yet another hack.
add_definitions(-DURHO3D_PROFILING -DURHO3D_LOGGING)

externalproject_get_property(Urho3D BINARY_DIR)
include_directories(${BINARY_DIR}/Engine)

file(GLOB children ${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Source/Engine/*)
foreach(child ${children})
    if(IS_DIRECTORY ${child})
        include_directories(${child})
    endif()
endforeach()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Source/ThirdParty/SDL/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Source/ThirdParty/Bullet/src)

add_dependencies(${PROJECT_NAME} Urho3D)

if(1)
    add_definitions(-DGOOGLE_BREAKPAD)
    add_subdirectory(Breakpad)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Breakpad)
    add_dependencies(${PROJECT_NAME} Breakpad)
    target_link_libraries(${PROJECT_NAME} Breakpad)
endif()

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -DDUMP_SYMS=$<TARGET_FILE:dump_syms> -DBINARY_PATH=$<TARGET_FILE:${PROJECT_NAME}> -P ${CMAKE_CURRENT_SOURCE_DIR}/DumpSymbols.cmake
)

if(UNIX)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Lib/${URHO3D_LIB_NAME})
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Lib/${URHO3D_LIB_NAME} $<TARGET_FILE_DIR:${PROJECT_NAME}>/${URHO3D_LIB_NAME}
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Bin/AssetImporter${CMAKE_EXECUTABLE_SUFFIX} $<TARGET_FILE_DIR:${PROJECT_NAME}>/AssetImporter${CMAKE_EXECUTABLE_SUFFIX}
    )
else()
    target_link_libraries(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Lib/${CMAKE_SHARED_LIBRARY_PREFIX}Urho3D${URHO3D_VERSION_SUFFIX}$<$<CONFIG:Debug>:_d>.lib)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Bin/${URHO3D_LIB_NAME} $<TARGET_FILE_DIR:${PROJECT_NAME}>/${URHO3D_LIB_NAME}
        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Bin/AssetImporter${CMAKE_EXECUTABLE_SUFFIX} $<TARGET_FILE_DIR:${PROJECT_NAME}>/AssetImporter${CMAKE_EXECUTABLE_SUFFIX}
    )
endif()
