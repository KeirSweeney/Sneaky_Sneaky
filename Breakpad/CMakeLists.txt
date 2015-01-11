if(APPLE)
    set(SRC_LIST
        "client/mac/handler/breakpad_nlist_64.cc"
        "client/mac/handler/dynamic_images.cc"
        "client/mac/handler/exception_handler.cc"
        "client/mac/handler/minidump_generator.cc"
        "common/mac/string_utilities.cc"
        "client/minidump_file_writer.cc"
        "common/mac/MachIPC.mm"
        "common/string_conversion.cc"
        "common/convert_UTF.c"
        "client/mac/crash_generation/crash_generation_client.cc"
        "common/mac/file_id.cc"
        "common/md5.cc"
        "common/mac/macho_id.cc"
        "common/mac/macho_utilities.cc"
        "common/mac/macho_walker.cc"
        "common/mac/bootstrap_compat.cc"
    )
    set(DUMP_SRC_LIST
        ""
    )
elseif(UNIX) 
    set(SRC_LIST "")
    set(DUMP_SRC_LIST "")
elseif(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
    set(SRC_LIST
        "client/windows/handler/exception_handler.cc"
        "client/windows/crash_generation/crash_generation_client.cc"
        "common/windows/guid_string.cc"
    )
    set(DUMP_SRC_LIST
        "tools/windows/dump_syms/dump_syms.cc"
        "common/windows/pdb_source_line_writer.cc"
        "common/windows/omap.cc"
        "common/windows/dia_util.cc"
        "common/windows/guid_string.cc"
        "common/windows/string_utils.cc"
    )
endif()

include_directories(".")
add_library(Breakpad STATIC ${SRC_LIST})

if(WIN32)
    get_filename_component(VS_DIR [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\12.0\\Setup\\VS;ProductDir] REALPATH CACHE)
    include_directories("${VS_DIR}/DIA SDK/include")
endif()

add_executable(BreakpadDump ${DUMP_SRC_LIST})

if(APPLE)
    find_library(CORE_FOUNDATION CoreFoundation)
    target_link_libraries(Breakpad ${CORE_FOUNDATION})
elseif(WIN32)
    target_link_libraries(BreakpadDump "ImageHlp.lib" "${VS_DIR}/DIA SDK/lib/amd64/diaguids.lib")
endif()
