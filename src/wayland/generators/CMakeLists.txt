find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)
if (NOT WAYLAND_SCANNER_EXECUTABLE)
    message(FATAL_ERROR "Failed to find wayland-scanner")
endif()

macro(generate_wayland_protocol proto_xml)
    set(proto_src "${CMAKE_CURRENT_SOURCE_DIR}/${proto_xml}")
    get_filename_component(proto_base "${proto_xml}" NAME_WE)

    set(gen_h "${CMAKE_CURRENT_BINARY_DIR}/${proto_base}-client-protocol.h")
    set(gen_c "${CMAKE_CURRENT_BINARY_DIR}/${proto_base}-client-protocol.c")

    execute_process(
        COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header  "${proto_src}" "${gen_h}"
        COMMAND ${WAYLAND_SCANNER_EXECUTABLE} private-code   "${proto_src}" "${gen_c}"
        RESULT_VARIABLE _wayland_scan_result
    )
    if (NOT _wayland_scan_result EQUAL 0)
        message(FATAL_ERROR "wayland-scanner failed on ${proto_xml}")
    endif()

    set_source_files_properties(${gen_h} ${gen_c} PROPERTIES GENERATED TRUE)
    target_include_directories(awin PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
endmacro()

generate_wayland_protocol("wayland.xml")
generate_wayland_protocol("viewporter.xml")
generate_wayland_protocol("xdg-shell.xml")
generate_wayland_protocol("idle-inhibit-unstable-v1.xml")
generate_wayland_protocol("relative-pointer-unstable-v1.xml")
generate_wayland_protocol("fractional-scale-v1.xml")
generate_wayland_protocol("xdg-activation-v1.xml")
generate_wayland_protocol("xdg-decoration-unstable-v1.xml")

target_sources(awin PRIVATE "wayland.c")