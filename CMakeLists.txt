cmake_minimum_required(VERSION 3.20)
project(lib-pluma VERSION 0.3.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build types and warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Core library
add_library(pluma SHARED)
add_library(pluma::pluma ALIAS pluma)

target_include_directories(pluma
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Phase 1: Foundation Core
target_sources(pluma PRIVATE
    src/NodeId.cpp
    src/RevisionId.cpp
    src/PieceTable.cpp
    src/Command.cpp
    src/Transaction.cpp
    src/UndoManager.cpp
    
    # Phase 4: Style System
    src/Style/PropertyBag.cpp
    src/Style/StyleResolver.cpp
    src/Style/FormatRegistry.cpp

    # Phase 5: Typography
    src/Typography/DummyTypography.cpp

    # Phase 6 & 7: Layout
    src/Layout/LayoutEngine.cpp
    src/Layout/LayoutScheduler.cpp

    # Phase 8 & 9: Render
    src/Render/DisplayList.cpp
    src/Render/CairoRenderer.cpp

    # Phase 10: DOM
    src/DOM/DOMManager.cpp

    # Phase 11, 12 & 13: Editor, Input & Controller
    src/Editor/CaretResolver.cpp
    src/Input/InputRouter.cpp
    src/PlumaEditor.cpp

    # Phase 15: Document Services
    src/Services/ServiceManager.cpp
    src/Services/SearchIndexer.cpp
    src/Services/SpellCheckerService.cpp
    src/Services/SpellCheckAnalyzer.cpp

    # Phase 16: Accessibility
    src/A11y/A11yTreeBuilder.cpp

    # Phase 17: Plugins
    src/Plugins/MarkdownImporter.cpp
    src/Plugins/MarkdownExporter.cpp
    src/Plugins/PdfExporter.cpp
    src/Plugins/PlumaArchiveImporter.cpp
    src/Plugins/PlumaArchiveExporter.cpp

    # Phase 18: Diagnostics
    src/Diagnostics/Dumper.cpp
    src/Diagnostics/Profiler.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(LIBZIP REQUIRED libzip)
pkg_check_modules(NUSPELL REQUIRED nuspell)

include(FetchContent)
FetchContent_Declare(json
  URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

target_include_directories(pluma PUBLIC ${CAIRO_INCLUDE_DIRS} ${LIBZIP_INCLUDE_DIRS} ${NUSPELL_INCLUDE_DIRS})
target_link_libraries(pluma PUBLIC ${CAIRO_LIBRARIES} ${LIBZIP_LIBRARIES} ${NUSPELL_LIBRARIES} nlohmann_json::nlohmann_json)

# Testing
enable_testing()
add_subdirectory(tests)

# --- Installation & Packaging ---
include(GNUInstallDirs)

install(TARGETS pluma
    EXPORT plumaTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/pluma/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pluma
    COMPONENT Development
)

# CPack configuration for Debian packages (.deb)
set(CPACK_PACKAGE_NAME "libpluma")
set(CPACK_PACKAGE_VENDOR "hdrdevs")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A high-performance document composition engine")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "horaciodrs@gmail.com")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL Runtime Development)

set(CPACK_DEBIAN_RUNTIME_PACKAGE_NAME "libpluma")
set(CPACK_DEBIAN_DEVELOPMENT_PACKAGE_NAME "libpluma-dev")

set(CPACK_DEBIAN_RUNTIME_FILE_NAME "libpluma-${PROJECT_VERSION}.deb")
set(CPACK_DEBIAN_DEVELOPMENT_FILE_NAME "libpluma-dev-${PROJECT_VERSION}.deb")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "hdrdevs")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_DEPENDS "libcairo2, libzip5, libnuspell5")
set(CPACK_DEBIAN_DEVELOPMENT_PACKAGE_DEPENDS "libpluma (= ${PROJECT_VERSION}), libzip-dev, libnuspell-dev")
set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")

include(CPack)

# ==========================================
# Examples
# ==========================================
add_executable(demo_render examples/demo_render.cpp)
target_link_libraries(demo_render PRIVATE pluma ${CAIRO_LIBRARIES})
target_include_directories(demo_render PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
