project(SneakySneaky)
include(ExternalProject)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_INSTALL_RPATH ".")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

aux_source_directory(src SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

externalproject_add(
    Urho3D
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Source
    CMAKE_ARGS -DURHO3D_64BIT=1 -DURHO3D_ANGELSCRIPT=0 -DURHO3D_NETWORK=0 -DURHO3D_URHO2D=0 -DURHO3D_TOOLS=1 -DURHO3D_MINIDUMPS=0 -DURHO3D_FILEWATCHER=0 -DURHO3D_OPENGL=1 -DURHO3D_LIB_TYPE=SHARED -DCMAKE_BUILD_TYPE=RelWithDebInfo
    BUILD_IN_SOURCE 0
    INSTALL_COMMAND ""
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

if(APPLE)
    set(URHO3D_VERSION_SUFFIX ".0")
else()
    set(URHO3D_VERSION_SUFFIX "")
endif()
set(URHO3D_LIB_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}Urho3D${URHO3D_VERSION_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX})

target_link_libraries(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Lib/${URHO3D_LIB_NAME})
add_dependencies(${PROJECT_NAME} Urho3D)

if(UNIX)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Lib/${URHO3D_LIB_NAME} $<TARGET_FILE_DIR:${PROJECT_NAME}>/${URHO3D_LIB_NAME}
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
    )
else()
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/Lib/${URHO3D_LIB_NAME} $<TARGET_FILE_DIR:${PROJECT_NAME}>/${URHO3D_LIB_NAME}
        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
    )
endif()
