if(APPLE)
    add_definitions(-DN_UNDF=0x0)
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
        "tools/mac/dump_syms/dump_syms_tool.mm"
        "common/mac/dump_syms.mm"
        "common/dwarf/bytereader.cc"
        "common/dwarf/dwarf2reader.cc"
        "common/dwarf/dwarf2diehandler.cc"
        "common/stabs_reader.cc"
        "common/stabs_to_module.cc"
        "common/dwarf_cu_to_module.cc"
        "common/dwarf_cfi_to_module.cc"
        "common/mac/arch_utilities.cc"
        "common/mac/file_id.cc"
        "common/module.cc"
        "common/mac/macho_reader.cc"
        "common/language.cc"
        "common/md5.cc"
        "common/mac/macho_id.cc"
        "common/dwarf_line_to_module.cc"
        "common/mac/macho_utilities.cc"
        "common/mac/macho_walker.cc"
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

add_library(breakpad STATIC ${SRC_LIST})

if(WIN32)
    get_filename_component(VS_DIR [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\12.0\\Setup\\VS;ProductDir] REALPATH CACHE)
    include_directories("${VS_DIR}/DIA SDK/include")
endif()

add_executable(dump_syms ${DUMP_SRC_LIST})

if(APPLE)
    find_library(CORE_FOUNDATION CoreFoundation)
    target_link_libraries(breakpad ${CORE_FOUNDATION})
    find_library(FOUNDATION Foundation)
    target_link_libraries(dump_syms ${FOUNDATION})
elseif(WIN32)
    target_link_libraries(dump_syms "ImageHlp.lib" "${VS_DIR}/DIA SDK/lib/amd64/diaguids.lib")
endif()
