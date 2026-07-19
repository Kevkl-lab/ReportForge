cmake_minimum_required(VERSION 3.20)
project(ReportForge VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure compiler flags for C++23 and warnings
if(MSVC)
    add_compile_options(/W4 /std:c++23)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -std=c++2b) # clang/gcc
endif()

# Fallback prefix path for the local Qt 6.9.1 installation on this system
if(EXISTS "/Users/kevinklein/Qt6.9.1/6.9.1/macos")
    list(APPEND CMAKE_PREFIX_PATH "/Users/kevinklein/Qt6.9.1/6.9.1/macos")
endif()

# Stub out WrapOpenGL to avoid linking legacy AGL framework on macOS
if(APPLE)
    add_library(WrapOpenGL::WrapOpenGL INTERFACE IMPORTED)
    set_target_properties(WrapOpenGL::WrapOpenGL PROPERTIES
        INTERFACE_LINK_LIBRARIES "-framework OpenGL;-framework AppKit"
    )
    set(WrapOpenGL_FOUND TRUE)
endif()

# Find Qt 6 components
find_package(Qt6 COMPONENTS Core Gui Widgets Sql Pdf PdfWidgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Sources list grouped by architectural layers
set(SOURCES
    # Main entry
    src/main.cpp

    # Core & Interfaces
    src/core/enums.h
    src/core/observer.h

    # Models
    src/models/project.h
    src/models/finding.h
    src/models/template.h
    src/models/timeline.h
    src/models/evidence.h

    # Database
    src/database/db_manager.h
    src/database/db_manager.cpp
    src/database/repository.h
    src/database/project_repository.h
    src/database/project_repository.cpp
    src/database/finding_repository.h
    src/database/finding_repository.cpp
    src/database/template_repository.h
    src/database/template_repository.cpp
    src/database/timeline_repository.h
    src/database/timeline_repository.cpp

    # Services
    src/services/pdf_generator.h
    src/services/pdf_generator.cpp
    src/services/import_export.h
    src/services/import_export.cpp
    src/services/cvss_calculator.h
    src/services/cvss_calculator.cpp

    # Dialogs
    src/dialogs/project_dialog.h
    src/dialogs/project_dialog.cpp
    src/dialogs/finding_dialog.h
    src/dialogs/finding_dialog.cpp
    src/dialogs/template_dialog.h
    src/dialogs/template_dialog.cpp

    # Widgets
    src/widgets/sidebar.h
    src/widgets/sidebar.cpp
    src/widgets/dashboard_view.h
    src/widgets/dashboard_view.cpp
    src/widgets/projects_view.h
    src/widgets/projects_view.cpp
    src/widgets/project_detail_view.h
    src/widgets/project_detail_view.cpp
    src/widgets/findings_view.h
    src/widgets/findings_view.cpp
    src/widgets/finding_detail_view.h
    src/widgets/finding_detail_view.cpp
    src/widgets/templates_view.h
    src/widgets/templates_view.cpp
    src/widgets/drag_drop_area.h
    src/widgets/drag_drop_area.cpp
    src/widgets/markdown_editor.h
    src/widgets/markdown_editor.cpp

    # Utils
    src/utils/file_utils.h
    src/utils/file_utils.cpp
)

add_executable(ReportForge ${SOURCES})

target_link_libraries(ReportForge PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Pdf
    Qt6::PdfWidgets
)

# Output directories config
set_target_properties(ReportForge PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(APPLE)
    set_target_properties(ReportForge PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "ReportForge"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.reportforge.app"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        INSTALL_RPATH "@executable_path/../Frameworks"
    )
endif()

# ----------------------------------------------------
# INSTALLATION & COMPONENT DEPLOYMENT
# ----------------------------------------------------
include(GNUInstallDirs)

install(TARGETS ReportForge
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Qt 6 modern deployment scripting (Bundles Qt libraries and plugins into the package)
if(COMMAND qt_generate_deploy_app_script)
    qt_generate_deploy_app_script(
        TARGET ReportForge
        OUTPUT_SCRIPT deploy_script.cmake
    )
    install(SCRIPT ${deploy_script.cmake})
endif()

# ----------------------------------------------------
# CPACK INSTALLER CONFIGURATION
# ----------------------------------------------------
set(CPACK_PACKAGE_NAME "ReportForge")
set(CPACK_PACKAGE_VENDOR "Kevkl-lab")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ReportForge - Professional Pentest Report Generator")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "ReportForge")

if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "ReportForge")
    set(CPACK_SYSTEM_NAME "macOS-AppleSilicon")
elseif(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_PACKAGE_NAME "ReportForge")
    set(CPACK_NSIS_DISPLAY_NAME "ReportForge")
    set(CPACK_NSIS_HELP_LINK "https://github.com/Kevkl-lab/ReportForge")
    # Add Start Menu shortcut
    set(CPACK_NSIS_MENU_LINKS "bin/ReportForge.exe" "ReportForge")
    # Add Desktop shortcut and ensure it is removed on uninstall
    set(CPACK_NSIS_CREATE_ICONS_EXTRA "  CreateShortCut '$DESKTOP\\\\ReportForge.lnk' '$INSTDIR\\\\bin\\\\ReportForge.exe'")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA "  Delete '$DESKTOP\\\\ReportForge.lnk'")
else()
    set(CPACK_GENERATOR "DEB;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kevkl-lab <support@reportforge.local>")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif()

include(CPack)

