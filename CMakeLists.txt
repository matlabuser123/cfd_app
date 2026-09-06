cmake_minimum_required(VERSION 3.25)

project(CFDApp
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CFDAPP_BUILD_TESTS "Build tests" ON)
option(CFDAPP_BUILD_BENCHMARKS "Build benchmarks" ON)
option(CFDAPP_BUILD_GUI "Build optional Qt GUI" OFF)
option(CFDAPP_BUILD_CUDA "Build optional CUDA backend" OFF)

find_package(OpenMP QUIET)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    # Statically link the MinGW C/C++ runtime. Without this, built executables depend on
    # libstdc++-6.dll / libgcc_s_seh-1.dll / libwinpthread-1.dll resolved from PATH at load
    # time. On this machine (and potentially on end-user machines installing the packaged
    # ZIP), an unrelated MinGW toolchain earlier on PATH (e.g. Git for Windows' bundled
    # mingw64) can shadow those DLLs with an ABI-incompatible build, causing affected
    # executables to fail with STATUS_ENTRYPOINT_NOT_FOUND before main() runs. Static linking
    # removes the dependency entirely and makes builds reproducible regardless of PATH order.
    add_link_options(-static-libgcc -static-libstdc++ -static)
endif()

file(GLOB_RECURSE CFDAPP_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)
list(FILTER CFDAPP_SOURCES EXCLUDE REGEX "[/\\\\]gui[/\\\\]")

set(CFDAPP_CUDA_SOURCES)
if(CFDAPP_BUILD_CUDA)
    enable_language(CUDA)
    find_package(CUDAToolkit REQUIRED)
    list(APPEND CFDAPP_CUDA_SOURCES
        src/gpu/CUDAKernels.cu
        src/gpu/GPUFieldOperations.cu
        src/gpu/GPULinearAlgebra.cu
    )
endif()

add_library(cfd_core STATIC ${CFDAPP_SOURCES} ${CFDAPP_CUDA_SOURCES})
target_include_directories(cfd_core PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_compile_features(cfd_core PUBLIC cxx_std_20)
if(OpenMP_CXX_FOUND)
    target_link_libraries(cfd_core PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(cfd_core PUBLIC CFDAPP_HAS_OPENMP=1)
else()
    target_compile_definitions(cfd_core PUBLIC CFDAPP_HAS_OPENMP=0)
endif()
if(CFDAPP_BUILD_CUDA)
    target_link_libraries(cfd_core PUBLIC CUDA::cudart)
    target_compile_definitions(cfd_core PUBLIC CFDAPP_HAS_CUDA=1)
    set_target_properties(cfd_core PROPERTIES CUDA_SEPARABLE_COMPILATION ON CUDA_STANDARD 17 CUDA_STANDARD_REQUIRED ON)
else()
    target_compile_definitions(cfd_core PUBLIC CFDAPP_HAS_CUDA=0)
endif()

add_executable(cfdapp apps/cfdapp/main.cpp)
target_link_libraries(cfdapp PRIVATE cfd_core)

target_compile_definitions(cfdapp PRIVATE
    CFDAPP_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
    CFDAPP_VERSION="${PROJECT_VERSION}"
)

include(GNUInstallDirs)
install(TARGETS cfdapp RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY cases/ DESTINATION cases)
install(FILES README.md roadmap.md docs/developer-guide.md DESTINATION docs)

set(CPACK_PACKAGE_NAME "CFDApp")
set(CPACK_PACKAGE_VENDOR "CFDApp")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ computational fluid dynamics application")
set(CPACK_PACKAGE_FILE_NAME "CFDApp-${PROJECT_VERSION}-Windows-x64")
set(CPACK_GENERATOR "ZIP")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
set(CPACK_VERBATIM_VARIABLES ON)
include(CPack)

if(CFDAPP_BUILD_TESTS OR CFDAPP_BUILD_BENCHMARKS)
    enable_testing()
endif()

if(CFDAPP_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(CFDAPP_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(CFDAPP_BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    set(CMAKE_AUTOMOC ON)
    add_executable(cfd_gui
        apps/cfd_gui_main.cpp
        src/gui/CaseEditor.cpp
        src/gui/CFDController.cpp
        src/gui/MainWindow.cpp
        src/gui/SolverWorker.cpp
        src/gui/VTKResultView.cpp
    )
    target_include_directories(cfd_gui PRIVATE ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(cfd_gui PRIVATE cfd_core Qt6::Widgets)
    target_compile_features(cfd_gui PRIVATE cxx_std_20)
endif()
