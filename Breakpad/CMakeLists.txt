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
elseif(UNIX) 
    set(SRC_LIST "")
else()
    set(SRC_LIST "")
endif()

include_directories(".")

add_library(Breakpad STATIC ${SRC_LIST})

if(APPLE)
    find_library(CORE_FOUNDATION CoreFoundation)
    target_link_libraries(Breakpad ${CORE_FOUNDATION})
endif()
