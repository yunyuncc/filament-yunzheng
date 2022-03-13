cmake_minimum_required(VERSION 3.19)
project(filament-demo C CXX ASM)
set(CMAKE_CXX_STANDARD 17)

#set(FILAMENT_INSTALL_PREFIX "/Users/yunzheng/Desktop/code/filament-1.18.0/out/debug")
#set(FILAMENT_INSTALL_PREFIX "/Users/yun/Desktop/github.com/filament-1.20.0/out/debug")
set(FILAMENT_INSTALL_PREFIX "/Users/yun/Desktop/github.com/filament-1.18.0/out/debug")

add_executable(matc IMPORTED)
set_property(TARGET matc PROPERTY IMPORTED_LOCATION "${FILAMENT_INSTALL_PREFIX}/filament/bin/matc")

add_executable(resgen IMPORTED)
set_property(TARGET resgen PROPERTY IMPORTED_LOCATION "${FILAMENT_INSTALL_PREFIX}/filament/bin/resgen")


# By default, build with support for OpenGL on all platforms.
option(FILAMENT_SUPPORTS_OPENGL "Include the OpenGL backend" ON)
if (APPLE AND NOT WEBGL)
    option(FILAMENT_SUPPORTS_METAL "Include the Metal backend" ON)
else()
    option(FILAMENT_SUPPORTS_METAL "Include the Metal backend" OFF)
endif()
if (WIN32 OR WEBGL OR IOS)
    option(FILAMENT_SUPPORTS_VULKAN "Include the Vulkan backend" OFF)
else()
    option(FILAMENT_SUPPORTS_VULKAN "Include the Vulkan backend" ON)
endif()

if (CMAKE_BUILD_TYPE MATCHES Release)
    option(FILAMENT_DISABLE_MATOPT "Disable material optimizations" OFF)
else()
    option(FILAMENT_DISABLE_MATOPT "Disable material optimizations" ON)
endif()


# Sets the following variables: RESGEN_HEADER, RESGEN_SOURCE, RESGEN_FLAGS, RESGEN_SOURCE_FLAGS,
# and RESGEN_OUTPUTS. Please pass in an ARCHIVE_NAME that is unique to your project, otherwise the
# incbin directive will happily consume a blob from the wrong project without warnings or errors.
# Also be sure to include the ASM language in the CMake "project" directive for your project.
function(get_resgen_vars ARCHIVE_DIR ARCHIVE_NAME)
    set(OUTPUTS
        ${ARCHIVE_DIR}/${ARCHIVE_NAME}.bin
        ${ARCHIVE_DIR}/${ARCHIVE_NAME}.S
        ${ARCHIVE_DIR}/${ARCHIVE_NAME}.apple.S
        ${ARCHIVE_DIR}/${ARCHIVE_NAME}.h
    )
    if (IOS)
        set(ASM_ARCH_FLAG "-arch ${DIST_ARCH}")
    endif()
    if (APPLE)
        set(ASM_SUFFIX ".apple")
    endif()
    set(RESGEN_HEADER "${ARCHIVE_DIR}/${ARCHIVE_NAME}.h" PARENT_SCOPE)
    # Visual Studio makes it difficult to use assembly without using MASM. MASM doesn't support
    # the equivalent of .incbin, so on Windows we'll just tell resgen to output a C file.
    if (WEBGL OR WIN32 OR ANDROID_ON_WINDOWS)
        set(RESGEN_OUTPUTS "${OUTPUTS};${ARCHIVE_DIR}/${ARCHIVE_NAME}.c" PARENT_SCOPE)
        set(RESGEN_FLAGS -qcx ${ARCHIVE_DIR} -p ${ARCHIVE_NAME} PARENT_SCOPE)
        set(RESGEN_SOURCE "${ARCHIVE_DIR}/${ARCHIVE_NAME}.c" PARENT_SCOPE)
    else()
        set(RESGEN_OUTPUTS "${OUTPUTS}" PARENT_SCOPE)
        set(RESGEN_FLAGS -qx ${ARCHIVE_DIR} -p ${ARCHIVE_NAME} PARENT_SCOPE)
        set(RESGEN_SOURCE "${ARCHIVE_DIR}/${ARCHIVE_NAME}${ASM_SUFFIX}.S" PARENT_SCOPE)
        set(RESGEN_SOURCE_FLAGS "-I${ARCHIVE_DIR} ${ASM_ARCH_FLAG}" PARENT_SCOPE)
    endif()
endfunction()


set(SRC src/main.cpp)
include_directories(${FILAMENT_INSTALL_PREFIX}/filament/include)

##这里面都是filamentapp的依赖
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/third_party/libsdl2/tnt)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/third_party/imgui/tnt)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/filamentlibs/filagui)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/third_party/stb/tnt)

#不知道为什么assimp 的config.h 生成到这里了
include_directories(${CMAKE_CURRENT_BINARY_DIR}/third_party/libassimp/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/third_party/libassimp)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/third_party/libassimp/tnt)

set(TARGET demo)

set(FRAMEWORKS "-framework Cocoa -framework Metal -framework CoreVideo")
set(FILAMENT_LIBS 
    filament 
    backend 
    bluegl 
    bluevk 
    filabridge 
    filaflat 
    utils 
    geometry 
    smol-v 
    vkshaders 
    ibl 
    matdbg 
    filamat
)
add_subdirectory(filamentapp)

add_executable(${TARGET} ${SRC})
target_compile_features(${TARGET} PRIVATE cxx_std_17)

target_link_directories(${TARGET} PRIVATE ${FILAMENT_INSTALL_PREFIX}/filament/lib/x86_64)
target_link_libraries(${TARGET} PRIVATE ${FRAMEWORKS})
target_link_libraries(${TARGET} PRIVATE ${FILAMENT_LIBS})
target_link_libraries(${TARGET} PRIVATE filamentapp)
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/filamentapp/include)
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/filamentapp/generated)


