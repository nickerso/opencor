PROJECT(OpenCOR)

# Minimum required version of CMake

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# Policies that we want to use

IF(POLICY CMP0054)
    CMAKE_POLICY(SET CMP0054 NEW)
ENDIF()

# Some in-house CMake functions/macros

INCLUDE(${CMAKE_SOURCE_DIR}/cmake/common.cmake)

# Our options

OPTION(ENABLE_SAMPLES "Enable the sample plugins to be built" OFF)
OPTION(ENABLE_TESTS "Enable the tests to be built" OFF)

OPTION(USE_PREBUILT_LLVM_PLUGIN "Use the pre-built version of the LLVM plugin" ON)
OPTION(USE_PREBUILT_QSCINTILLA_PLUGIN "Use the pre-built version of the QScintilla plugin" ON)
OPTION(USE_PREBUILT_QWT_PLUGIN "Use the pre-built version of the Qwt plugin" ON)
OPTION(USE_PREBUILT_SUNDIALS_PLUGIN "Use the pre-built version of the SUNDIALS plugin" ON)

# Initialise the project

INITIALISE_PROJECT()

# Version/snapshot of OpenCOR

SET(SNAPSHOT ON)

SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 4)
SET(VERSION_PATCH 0)

TRY_RUN(DATE_RUN DATE_COMPILE
        ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/cmake/date.c
        RUN_OUTPUT_VARIABLE DATE)

IF(SNAPSHOT)
    SET(VERSION ${DATE})
ELSE()
    SET(VERSION ${VERSION_MAJOR}.${VERSION_MINOR})

    IF(NOT ${VERSION_PATCH} EQUAL 0)
        SET(VERSION ${VERSION}.${VERSION_PATCH})
    ENDIF()
ENDIF()

FILE(WRITE ${CMAKE_BINARY_DIR}/versiondate.txt "${VERSION}\n${DATE}")

# Files that make up the GUI version of OpenCOR

SET(SOURCES
    src/checkforupdateswindow.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/pluginswindow.cpp
    src/preferenceswindow.cpp
    src/splashscreenwindow.cpp

    src/misc/cliapplication.cpp
    src/misc/cliutils.cpp
    src/misc/guiutils.cpp

    src/plugins/guiinterface.cpp
    src/plugins/i18ninterface.cpp
    src/plugins/plugin.cpp
    src/plugins/plugininfo.cpp
    src/plugins/pluginmanager.cpp
)

SET(HEADERS_MOC
    src/checkforupdateswindow.h
    src/mainwindow.h
    src/pluginswindow.h
    src/preferenceswindow.h
    src/splashscreenwindow.h

    src/misc/cliutils.h

    src/plugins/plugin.h
    src/plugins/pluginmanager.h
)

SET(UIS
    src/checkforupdateswindow.ui
    src/mainwindow.ui
    src/pluginswindow.ui
    src/preferenceswindow.ui
    src/splashscreenwindow.ui
)

SET(RESOURCES
    res/common.qrc
    res/ui.qrc
)

# Files that make up the CLI version of OpenCOR (Windows specific)

IF(WIN32)
    SET(WINDOWS_CLI_SOURCES
        src/misc/cliapplication.cpp
        src/misc/cliutils.cpp

        src/plugins/plugin.cpp
        src/plugins/plugininfo.cpp
        src/plugins/pluginmanager.cpp

        src/windows/main.cpp
    )

    SET(WINDOWS_CLI_HEADERS_MOC
        src/misc/cliutils.h

        src/plugins/plugin.h
        src/plugins/pluginmanager.h
    )

    SET(WINDOWS_CLI_RESOURCES
        res/common.qrc
    )
ENDIF()

# Various include directories

INCLUDE_DIRECTORIES(
    src/misc
    src/plugins
    src/plugins/miscellaneous/Core/src
)

# Update the translation (.ts) files and generate the language (.qm) files
# that will later on be embedded in the OpenCOR executable as resources

UPDATE_LANGUAGE_FILES(${PROJECT_NAME} FALSE)

# Third-party library that must be directly embedded in the GUI version of
# OpenCOR

INCLUDE(src/3rdparty/QtSingleApplication/QtSingleApplication.cmake)

# Set the application icon, but only for Windows and OS X, since in the case of
# Linux, it's done through the use of app_icon (see res/ui.qrc) in
# src/mainwindow.ui

IF(WIN32)
    LIST(APPEND SOURCES ${CMAKE_SOURCE_DIR}/res/${PROJECT_NAME}.rc)
ELSEIF(APPLE)
    SET(IconFile ${PROJECT_NAME}.icns)

    SET(MACOSX_BUNDLE_ICON_FILE ${IconFile})

    SET_SOURCE_FILES_PROPERTIES(res/${IconFile} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    LIST(APPEND SOURCES res/${IconFile})
ENDIF()

# Check whether tests are required and, if so, 'reset' our list of tests and set
# the destination tests directory

IF(ENABLE_TESTS)
    # 'Reset' our list of tests

    SET(TESTS_LIST_FILENAME ${PROJECT_BUILD_DIR}/tests.txt)

    FILE(WRITE ${TESTS_LIST_FILENAME})

    # Destination tests directory
    # Note: DEST_TESTS_DIR isn't only used here, but also in our ADD_PLUGIN()
    #       macro...

    IF(APPLE)
        SET(DEST_TESTS_DIR ${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/MacOS)
    ELSE()
        SET(DEST_TESTS_DIR ${PROJECT_BUILD_DIR}/bin)
    ENDIF()
ENDIF()

# Check whether tests are required and, if so, build our main test program

IF(ENABLE_TESTS)
    # Build our main test program

    SET(RUNTESTS_NAME runtests)

    QT5_ADD_RESOURCES(RUNTESTS_SOURCES_RCS src/tests/res/tests.qrc)

    ADD_EXECUTABLE(${RUNTESTS_NAME}
        src/tests/src/main.cpp

        ${RUNTESTS_SOURCES_RCS}
    )

    TARGET_LINK_LIBRARIES(${RUNTESTS_NAME}
        Qt5::Core
    )

    # Copy our main test program to our tests directory

    SET(MAIN_TEST_FILENAME ${RUNTESTS_NAME}${CMAKE_EXECUTABLE_SUFFIX})

    ADD_CUSTOM_COMMAND(TARGET ${RUNTESTS_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/${MAIN_TEST_FILENAME}
                                                        ${DEST_TESTS_DIR}/${MAIN_TEST_FILENAME})

    # Clean up our test program, but only if we are on OS X

    IF(APPLE)
        OS_X_CLEAN_UP_FILE_WITH_QT_LIBRARIES(${RUNTESTS_NAME} ${DEST_TESTS_DIR} ${MAIN_TEST_FILENAME} QtCore)
    ENDIF()
ENDIF()

# Build the OpenCOR plugins
# Note: the build order must be such that plugins needed by others are built
#       first...

IF(ENABLE_SAMPLES)
    SET(SAMPLE_PLUGINS
        sample/Sample
        sample/SampleTools
        sample/SampleView
        sample/SampleWindow
    )
ELSE()
    SET(SAMPLE_PLUGINS)
ENDIF()

SET(PLUGINS
    # Plugins that have no dependencies

    api/CellMLAPI

    dataStore/CoreDataStore

    miscellaneous/Core

    solver/CoreSolver

    thirdParty/LLVM
    thirdParty/QScintilla
    thirdParty/Qwt
    thirdParty/SUNDIALS

    # Plugins that build on top of some third-party libraries

    miscellaneous/Compiler

    # Plugins that provide support for some third-party libraries

    support/CellMLSupport
    support/QScintillaSupport

    # Plugins that extend some libraries (incl. third-party ones)

    widget/Editor
    widget/EditorList
    widget/Viewer

    # Plugins that provide some generic features

    editing/CoreEditing

    # Plugins that have dependencies on others

    editing/CoreCellMLEditing

    # Selectable plugins

    dataStore/CSVDataStore

    editing/CellMLAnnotationView
    editing/CellMLTextView
    editing/RawCellMLView
    editing/RawView

    miscellaneous/HelpWindow

    organisation/CellMLModelRepositoryWindow
    organisation/FileBrowserWindow
    organisation/FileOrganiserWindow

    simulation/SingleCellView

    solver/CVODESolver
    solver/ForwardEulerSolver
    solver/FourthOrderRungeKuttaSolver
    solver/HeunSolver
    solver/IDASolver
    solver/KINSOLSolver
    solver/SecondOrderRungeKuttaSolver

    tool/CellMLTools

    # Sample plugins

    ${SAMPLE_PLUGINS}
)

FOREACH(PLUGIN ${PLUGINS})
    SET(PLUGIN_DIR src/plugins/${PLUGIN})

    ADD_SUBDIRECTORY(${PLUGIN_DIR})
    # Note: this will process the CMakeLists.txt file of the plugin resulting in
    #       PLUGIN_INCLUDE_DIRS being set, hence we can use it below to update
    #       our include directories

    FOREACH(PLUGIN_INCLUDE_DIR ${PLUGIN_INCLUDE_DIRS})
        INCLUDE_DIRECTORIES(${PLUGIN_DIR}/${PLUGIN_INCLUDE_DIR})
    ENDFOREACH()

    INCLUDE_DIRECTORIES(build/${PLUGIN_DIR})
ENDFOREACH()

# Build the GUI version of OpenCOR

QT5_WRAP_CPP(SOURCES_MOC ${HEADERS_MOC})
QT5_WRAP_UI(SOURCES_UIS ${UIS})
QT5_ADD_RESOURCES(SOURCES_RCS ${RESOURCES})

ADD_EXECUTABLE(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
    ${SOURCES}
    ${SOURCES_MOC}
    ${SOURCES_UIS}
    ${SOURCES_RCS}
)

IF(WIN32)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        ${Qt5Core_QTMAIN_LIBRARIES}
    )
ENDIF()

FOREACH(REQUIRED_QT_MODULE ${REQUIRED_QT_MODULES})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        Qt5::${REQUIRED_QT_MODULE}
    )
ENDFOREACH()

# Build the CLI version of OpenCOR (Windows specific)

IF(WIN32)
    QT5_WRAP_CPP(WINDOWS_CLI_SOURCES_MOC ${WINDOWS_CLI_HEADERS_MOC})
    QT5_ADD_RESOURCES(WINDOWS_CLI_SOURCES_RCS ${WINDOWS_CLI_RESOURCES})

    SET(WINDOWS_CLI_PROJECT_NAME ${PROJECT_NAME}.com)

    ADD_EXECUTABLE(${WINDOWS_CLI_PROJECT_NAME}
        ${WINDOWS_CLI_SOURCES}
        ${WINDOWS_CLI_SOURCES_MOC}
        ${WINDOWS_CLI_SOURCES_RCS}
    )

    TARGET_LINK_LIBRARIES(${WINDOWS_CLI_PROJECT_NAME}
        Qt5::Core
        Qt5::Xml
        Qt5::XmlPatterns
    )
ENDIF()

# Additional compiler and linker settings

ADD_DEFINITIONS(-D${PROJECT_NAME}_MAIN)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    LINK_FLAGS "${LINK_FLAGS_PROPERTIES}"
)

IF(WIN32)
    SET_TARGET_PROPERTIES(${WINDOWS_CLI_PROJECT_NAME} PROPERTIES
        LINK_FLAGS "${LINK_FLAGS_PROPERTIES}"
    )
ENDIF()

# Some post-processing specific stuff

IF(APPLE)
    # Configure and use our own Info.plist file
    # Note: the reason for using our own Info.plist file is that it contains
    #       some information about associating .cellml files to OpenCOR,
    #       something that can't be done using CMake (or so it seems)...

    SET(InfoPlistFile ${CMAKE_BINARY_DIR}/Info.plist)

    EXECUTE_PROCESS(COMMAND date "+%Y"
                    OUTPUT_VARIABLE YEAR
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/distrib/osx/Info.plist.in ${InfoPlistFile})

    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${InfoPlistFile}
    )

    # Note: ideally, what follows would be done using macdeployqt, but it has
    #       regularly been causing us problems, so instead we have decided to do
    #       everything ourselves...

    # Clean up the OpenCOR executable

    SET(QT_LIBRARIES
        QtCLucene
        QtConcurrent
        QtCore
        QtGui
        QtHelp
        QtMacExtras
        QtMultimedia
        QtMultimediaWidgets
        QtNetwork
        QtOpenGL
        QtPositioning
        QtPrintSupport
        QtQml
        QtQuick
        QtSensors
        QtSql
        QtSvg
        QtWebChannel
        QtWebKit
        QtWebKitWidgets
        QtWidgets
        QtXml
        QtXmlPatterns
    )

    OS_X_CLEAN_UP_FILE_WITH_QT_LIBRARIES(${PROJECT_NAME} ${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/MacOS ${PROJECT_NAME} ${QT_LIBRARIES})

    # Deploy our Qt libraries

    OS_X_DEPLOY_QT_LIBRARIES(${QT_LIBRARIES})

    IF(ENABLE_TESTS)
        OS_X_DEPLOY_QT_LIBRARIES(QtTest)
    ENDIF()

    # Deploy some plugins

    OS_X_DEPLOY_QT_PLUGIN(imageformats qjpeg)
    OS_X_DEPLOY_QT_PLUGIN(platforms qcocoa)
    OS_X_DEPLOY_QT_PLUGIN(printsupport cocoaprintersupport)
    OS_X_DEPLOY_QT_PLUGIN(sqldrivers qsqlite)
ELSE()
    # Copy the GUI version of the OpenCOR executable to the build/bin folder

    COPY_FILE_TO_BUILD_DIR(${PROJECT_NAME} ${PROJECT_BUILD_DIR} bin
                           ${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX})

    IF(WIN32)
        # Copy the CLI version of the OpenCOR executable to both the build and
        # build/bin folders
        # Note: the idea is that, on Windows, we can test both the CLI and the
        #       GUI versions of OpenCOR from either build or build/bin...

        COPY_FILE_TO_BUILD_DIR(${WINDOWS_CLI_PROJECT_NAME} ${PROJECT_BUILD_DIR} .
                               ${WINDOWS_CLI_PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} ${PROJECT_NAME}.com)
        COPY_FILE_TO_BUILD_DIR(${WINDOWS_CLI_PROJECT_NAME} ${PROJECT_BUILD_DIR} bin
                               ${WINDOWS_CLI_PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} ${PROJECT_NAME}.com)

        # Additional files required by OpenCOR
        # Note: these files may or not be needed in order to test OpenCOR
        #       locally. It all depends on the way the user's computer is set
        #       up. So, by copying them over, we are sure that the 32-bit/64-bit
        #       release/debug version of OpenCOR will work fine...

        SET(MSVC_DIRNAME "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/redist")

        IF(NOT RELEASE_MODE)
            SET(MSVC_DIRNAME "${MSVC_DIRNAME}/Debug_NonRedist")
        ENDIF()

        IF(${ARCHITECTURE} EQUAL 32)
            SET(MSVC_DIRNAME "${MSVC_DIRNAME}/x86")
        ELSE()
            SET(MSVC_DIRNAME "${MSVC_DIRNAME}/x64")
        ENDIF()

        IF(RELEASE_MODE)
            SET(ADDITIONAL_FILES
                ${MSVC_DIRNAME}/Microsoft.VC120.CRT/msvcp120.dll
                ${MSVC_DIRNAME}/Microsoft.VC120.CRT/msvcr120.dll
            )
        ELSE()
            SET(ADDITIONAL_FILES
                ${MSVC_DIRNAME}/Microsoft.VC120.DebugCRT/msvcp120d.dll
                ${MSVC_DIRNAME}/Microsoft.VC120.DebugCRT/msvcr120d.dll
            )
        ENDIF()

        IF(${ARCHITECTURE} EQUAL 32)
            RETRIEVE_BINARY_FILE(distrib/${DISTRIB_DIR} libeay32.dll 84fe64c29404e73eb03a950fdc2a0b6314f05b32)
            RETRIEVE_BINARY_FILE(distrib/${DISTRIB_DIR} ssleay32.dll 310275ff1a16404a0a5d15a8491a45bc88592830)
        ELSE()
            RETRIEVE_BINARY_FILE(distrib/${DISTRIB_DIR} libeay32.dll 2cf5db76a32190f7fe97efc4285419cab638c43b)
            RETRIEVE_BINARY_FILE(distrib/${DISTRIB_DIR} ssleay32.dll 592fa12875358a93c14befc841c42c3c8568a400)
        ENDIF()

        LIST(APPEND ADDITIONAL_FILES
            ${CMAKE_SOURCE_DIR}/distrib/${DISTRIB_DIR}/libeay32.dll
            ${CMAKE_SOURCE_DIR}/distrib/${DISTRIB_DIR}/ssleay32.dll
        )

        FOREACH(ADDITIONAL_FILE ${ADDITIONAL_FILES})
            GET_FILENAME_COMPONENT(DIRNAME ${ADDITIONAL_FILE} DIRECTORY)
            GET_FILENAME_COMPONENT(FILENAME ${ADDITIONAL_FILE} NAME)

            COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${DIRNAME} . ${FILENAME})
            COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${DIRNAME} bin ${FILENAME})
        ENDFOREACH()
    ENDIF()
ENDIF()

# Package OpenCOR

SET(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
SET(CPACK_PACKAGE_CONTACT "Team OpenCOR (team@opencor.ws)")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A cross-platform CellML-based modelling environment")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/distrib/readMe.txt")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
SET(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME}" "${PROJECT_NAME}")
SET(CPACK_PACKAGE_VENDOR "Physiome Project")
SET(CPACK_PACKAGE_VERSION "${VERSION}")

IF(SNAPSHOT)
    SET(CPACK_PACKAGE_VERSION_PATCH "")
ELSE()
    SET(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
ENDIF()

IF(WIN32)
    # Select NSIS, if available, and ZIP as the packagers on Windows

    IF(EXISTS $ENV{PROGRAMFILES}/NSIS/makensis.exe)
        SET(NSIS_FOUND TRUE)
        SET(NSIS_GENERATOR NSIS)
    ELSE()
        SET(NSIS_FOUND FALSE)
        SET(NSIS_GENERATOR)
    ENDIF()

    SET(CPACK_GENERATOR ${NSIS_GENERATOR} ZIP)

    SET(CPACK_SYSTEM_NAME "Windows${ARCHITECTURE}")

    SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")

    # Specify the default installation directory

    IF(NSIS_FOUND)
        IF(${ARCHITECTURE} EQUAL 32)
            SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
        ELSE()
            SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
        ENDIF()

        SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${PROJECT_NAME}.exe")
    ENDIF()

    # Install both the GUI and CLI versions of OpenCOR

    INSTALL(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
    INSTALL(FILES ${PROJECT_BUILD_DIR}/bin/${PROJECT_NAME}.com
            DESTINATION bin)

    # Additional files required by OpenCOR

    INSTALL(FILES ${ADDITIONAL_FILES}
            DESTINATION bin)

    # Qt libraries required by OpenCOR

    WINDOWS_DEPLOY_QT_LIBRARIES(Qt5CLucene Qt5Core Qt5Gui Qt5Help Qt5Multimedia Qt5MultimediaWidgets Qt5Network Qt5OpenGL Qt5Positioning Qt5PrintSupport Qt5Qml Qt5Quick Qt5Sensors Qt5Sql Qt5Svg Qt5WebChannel Qt5WebKit Qt5WebKitWidgets Qt5Widgets Qt5Xml Qt5XmlPatterns)
    WINDOWS_DEPLOY_QT_LIBRARIES(icudt53 icuin53 icuuc53)

    IF(ENABLE_TESTS)
        WINDOWS_DEPLOY_QT_LIBRARIES(Qt5Test)
    ENDIF()

    # Qt plugins required by OpenCOR

    WINDOWS_DEPLOY_QT_PLUGIN(imageformats qjpeg)
    WINDOWS_DEPLOY_QT_PLUGIN(platforms qwindows)
    WINDOWS_DEPLOY_QT_PLUGIN(printsupport windowsprintersupport)
    WINDOWS_DEPLOY_QT_PLUGIN(sqldrivers qsqlite)

    # Batch and VBScript files to run OpenCOR (useful when downloading a ZIPped
    # version of OpenCOR)

    SET(BatFile ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bat)
    SET(VbsFile ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.vbs)

    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/distrib/windows/application.bat.in ${BatFile})
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/distrib/windows/application.vbs.in ${VbsFile})

    INSTALL(FILES ${BatFile}
            DESTINATION .)
    INSTALL(FILES ${VbsFile}
            DESTINATION .)

    # File type association
    # Note: the calls to SHChangeNotify are to ensure that Windows refreshes
    #       file icons (so that it is clear to the user that an extension has
    #       been (un)registered...

    IF(NSIS_FOUND)
        SET(CPACK_NSIS_DEFINES "!include ${CMAKE_SOURCE_DIR}\\\\distrib\\\\windows\\\\FileAssociation.nsh")

        SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
            \\\${RegisterExtension} \\\"\\\$INSTDIR\\\\bin\\\\${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}\\\" \\\".cellml\\\" \\\"CellML File\\\"
            System::Call \\\"Shell32::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)\\\"
        ")
        SET(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
            \\\${UnregisterExtension} \\\".cellml\\\" \\\"CellML File\\\"
            System::Call \\\"Shell32::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)\\\"
        ")
    ENDIF()
ELSEIF(APPLE)
    # Select PackageMaker and ZIP as the packagers on OS X
    # Note: this is actually done within the package script, so that the ZIP
    #       package can be done 'properly', i.e. we don't have the full tree
    #       structure in the ZIP file...

    SET(CPACK_SYSTEM_NAME "OSX")

    SET(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/distrib/osx/welcome.txt")
    SET(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/distrib/readMe.txt")
    SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")

    SET(CPACK_SET_DESTDIR TRUE)

    INSTALL(TARGETS ${PROJECT_NAME} BUNDLE
            DESTINATION .)

    # Shell script to run OpenCOR (useful when downloading a ZIPped version of
    # OpenCOR)

    SET(SHELL_FILENAME ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.sh)

    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/distrib/osx/application.sh.in ${SHELL_FILENAME})

    INSTALL(FILES ${SHELL_FILENAME}
            RENAME ${PROJECT_NAME}
            DESTINATION .
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
ELSE()
    # Select TGZ as the packager on Linux

    SET(CPACK_GENERATOR TGZ)

    SET(CPACK_SYSTEM_NAME "Linux${ARCHITECTURE}")

    # OpenCOR itself

    INSTALL(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

    # Qt libraries required by OpenCOR
    # Note: unlike for Windows and OS X, we don't have a macro for deploying our
    #       Qt libraries. Indeed, on Linux, the version of a file is appended to
    #       its suffix, so unless making the macro unnecessarily cumbersome to
    #       use, it's better to do things ourselves...

    FOREACH(QT_LIBRARY CLucene Core DBus Gui Help Network OpenGL Positioning PrintSupport Qml Quick Sensors Sql Svg WebChannel WebKit WebKitWidgets Widgets Xml XmlPatterns)
        INSTALL(FILES ${QT_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}Qt${QT_VERSION_MAJOR}${QT_LIBRARY}${CMAKE_SHARED_LIBRARY_SUFFIX}.${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}
                DESTINATION lib
                RENAME ${CMAKE_SHARED_LIBRARY_PREFIX}Qt${QT_VERSION_MAJOR}${QT_LIBRARY}${CMAKE_SHARED_LIBRARY_SUFFIX}.${QT_VERSION_MAJOR})
    ENDFOREACH()

    FOREACH(QT_FILE icudata icui18n icuuc)
        INSTALL(FILES ${QT_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${QT_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}.53.1
                DESTINATION lib
                RENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${QT_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}.53)
    ENDFOREACH()

    # Qt plugins required by OpenCOR

    LINUX_DEPLOY_QT_PLUGIN(imageformats qjpeg)
    LINUX_DEPLOY_QT_PLUGIN(platforms qxcb)
    LINUX_DEPLOY_QT_PLUGIN(printsupport cupsprintersupport)
    LINUX_DEPLOY_QT_PLUGIN(sqldrivers qsqlite)

    # Shell script to run OpenCOR

    SET(ShellFile ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.sh)

    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/distrib/linux/application.sh.in ${ShellFile})

    INSTALL(FILES ${ShellFile} RENAME ${PROJECT_NAME}
            DESTINATION .
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
ENDIF()

# Specify the package file name, depending on whether we are generating a
# snapshot version of OpenCOR

IF(SNAPSHOT)
    SET(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${DATE}-${CPACK_SYSTEM_NAME})
ELSE()
    SET(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${VERSION_MAJOR}-${VERSION_MINOR})

    IF(NOT ${VERSION_PATCH} EQUAL 0)
        SET(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME}-${VERSION_PATCH})
    ENDIF()

    SET(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME}-${CPACK_SYSTEM_NAME})
ENDIF()

# Notice and license files

SET(FILES
    LICENSE.txt
    NOTICE.txt
)

FOREACH(FILE ${FILES})
    INSTALL(FILES ${CMAKE_SOURCE_DIR}/${FILE}
            DESTINATION .
            PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
ENDFOREACH()

# Some sample CellML files

SET(MODEL_FILES
    hodgkin_huxley_squid_axon_model_1952.cellml
    noble_model_1962.cellml
    van_der_pol_model_1928.cellml
)

FOREACH(MODEL_FILE ${MODEL_FILES})
    INSTALL(FILES ${CMAKE_SOURCE_DIR}/models/${MODEL_FILE}
            DESTINATION models
            PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
ENDFOREACH()

# Some user-defined format files

SET(FORMAT_FILES
    C.xml
    F77.xml
    MATLAB.xml
    Python.xml
    README.txt
)

FOREACH(FORMAT_FILE ${FORMAT_FILES})
    INSTALL(FILES ${CMAKE_SOURCE_DIR}/formats/${FORMAT_FILE}
            DESTINATION formats
            PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
ENDFOREACH()

INCLUDE(CPack)
