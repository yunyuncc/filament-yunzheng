


set(TARGET filamentapp)
set(PUBLIC_HDR_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${PUBLIC_HDR_DIR})
set(PUBLIC_HDRS
        ${PUBLIC_HDR_DIR}/filamentapp/Config.h
        ${PUBLIC_HDR_DIR}/filamentapp/Cube.h
        ${PUBLIC_HDR_DIR}/filamentapp/FilamentApp.h
        ${PUBLIC_HDR_DIR}/filamentapp/IBL.h
        ${PUBLIC_HDR_DIR}/filamentapp/IcoSphere.h
        ${PUBLIC_HDR_DIR}/filamentapp/MeshAssimp.h
        ${PUBLIC_HDR_DIR}/filamentapp/NativeWindowHelper.h
        ${PUBLIC_HDR_DIR}/filamentapp/Sphere.h
)

set(SRCS
        ${CMAKE_CURRENT_LIST_DIR}/src/Cube.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/FilamentApp.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/IBL.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/IcoSphere.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/Image.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/MeshAssimp.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/Sphere.cpp
)
set(LIBS
        assimp
        filament
        filament-iblprefilter
        sdl2
        stb
        #math
        filamat
        utils
        #getopt
        imgui
        filagui
        image
        camutils
        geometry
)

if (APPLE)
    list(APPEND SRCS ${CMAKE_CURRENT_LIST_DIR}/src/NativeWindowHelperCocoa.mm)
    list(APPEND LIBS "-framework Cocoa -framework QuartzCore")
endif()

if (LINUX)
    list(APPEND SRCS ${CMAKE_CURRENT_LIST_DIR}/src/NativeWindowHelperLinux.cpp)
endif()

if (WIN32)
    list(APPEND SRCS ${CMAKE_CURRENT_LIST_DIR}/src/NativeWindowHelperWindows.cpp)
    list(APPEND LIBS sdl2main)
endif()



set(MATC_API_FLAGS )
if (FILAMENT_SUPPORTS_OPENGL)
    message("FILAMENT_SUPPORTS_OPENGL")
    add_definitions(-DFILAMENT_SUPPORTS_OPENGL)
    set(MATC_API_FLAGS ${MATC_API_FLAGS} -a opengl)
endif()
if (FILAMENT_SUPPORTS_METAL)
    message(FILAMENT_SUPPORTS_METAL)
    add_definitions(-DFILAMENT_SUPPORTS_METAL)
    set(MATC_API_FLAGS ${MATC_API_FLAGS} -a metal)
endif()
if (FILAMENT_SUPPORTS_VULKAN)
    message(FILAMENT_SUPPORTS_VULKAN)
    add_definitions(-DFILAMENT_DRIVER_SUPPORTS_VULKAN)
    set(MATC_API_FLAGS ${MATC_API_FLAGS} -a vulkan)
endif()
set(IS_MOBILE_TARGET FALSE)
# Target system.
if (IS_MOBILE_TARGET)
    set(MATC_TARGET mobile)
else()
    set(MATC_TARGET desktop)
endif()


# Disable optimizations and enable debug info (preserves names in SPIR-V)
if (FILAMENT_DISABLE_MATOPT)
    set(MATC_OPT_FLAGS -gd)
endif()
set(MATC_BASE_FLAGS ${MATC_API_FLAGS} -p ${MATC_TARGET} ${MATC_OPT_FLAGS})

message("MATC_BASE_FLAGS: ${MATC_BASE_FLAGS}")



set(MATERIAL_SRCS
${CMAKE_CURRENT_LIST_DIR}/materials/aiDefaultMat.mat
${CMAKE_CURRENT_LIST_DIR}/materials/aiDefaultTrans.mat
${CMAKE_CURRENT_LIST_DIR}/materials/depthVisualizer.mat
${CMAKE_CURRENT_LIST_DIR}/materials/transparentColor.mat
)

set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})
set(MATERIAL_DIR  "${GENERATION_ROOT}/generated/material")
set(RESOURCE_DIR  "${GENERATION_ROOT}/generated/resources")

file(MAKE_DIRECTORY ${MATERIAL_DIR})
file(MAKE_DIRECTORY ${RESOURCE_DIR})

set(RESOURCE_BINS)
foreach (mat_src ${MATERIAL_SRCS})
get_filename_component(localname "${mat_src}" NAME_WE)
get_filename_component(fullname "${mat_src}" ABSOLUTE)
set(output_path "${MATERIAL_DIR}/${localname}.filamat")
add_custom_command(
    OUTPUT ${output_path}
    COMMAND matc ${MATC_BASE_FLAGS} -o ${output_path} ${fullname}
    MAIN_DEPENDENCY ${mat_src}
    #DEPENDS matc
    COMMENT "Compiling material ${mat_src} to ${output_path}"
)
list(APPEND RESOURCE_BINS ${output_path})
endforeach()

get_resgen_vars(${RESOURCE_DIR} filamentapp)
message("RESGEN_HEADER :${RESGEN_HEADER}")
message("RESGEN_SOURCE :${RESGEN_SOURCE}")
message("RESGEN_FLAGS :${RESGEN_FLAGS}")
message("RESGEN_SOURCE_FLAGS :${RESGEN_SOURCE_FLAGS}")
message("RESGEN_OUTPUTS :${RESGEN_OUTPUTS}")

add_custom_command(
    OUTPUT ${RESGEN_OUTPUTS}
    COMMAND resgen ${RESGEN_FLAGS} ${RESOURCE_BINS}
    DEPENDS ${RESOURCE_BINS}
    COMMENT "Aggregating resources"
)
if (DEFINED RESGEN_SOURCE_FLAGS)
set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()
# CMake fails to invoke ar on Windows unless there is at least one C/C++ file in the library.
set(DUMMY_SRC "${RESOURCE_DIR}/dummy.c")
add_custom_command(OUTPUT ${DUMMY_SRC} COMMAND echo "//" > ${DUMMY_SRC})

add_library(filamentapp-resources ${DUMMY_SRC} ${RESGEN_SOURCE})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_link_libraries(${TARGET} PUBLIC ${LIBS} filamentapp-resources)

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
target_include_directories(${TARGET} PRIVATE ${GENERATION_ROOT})


# ==================================================================================================
# Compiler flags
# ==================================================================================================

if (MSVC)
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/fp:fast>)
else()
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:-ffast-math>)
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
endif()

# Multi-configuration generators, like Visual Studio or Xcode, place executable binaries in a
# sub-directory named after the configuration, like "Debug" or "Release".
# For these generators, in order to find assets, we must "walk" up an additional directory.
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (_isMultiConfig)
    target_compile_definitions(${TARGET} PRIVATE RELATIVE_ASSET_PATH="..")
else()
    target_compile_definitions(${TARGET} PRIVATE RELATIVE_ASSET_PATH=".")
endif()
target_compile_features(${TARGET} PRIVATE cxx_std_17)