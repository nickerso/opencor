PROJECT(CellMLToolsPlugin)

# Retrieve some CellML API and LLVM settings

RETRIEVE_CELLML_API_SETTINGS()
RETRIEVE_LLVM_SETTINGS()

# Add the plugin

ADD_PLUGIN(CellMLTools
    SOURCES
        ../../guiinterface.cpp
        ../../i18ninterface.cpp
        ../../filetypeinterface.cpp
        ../../plugin.cpp
        ../../plugininfo.cpp
        ../../pluginmanager.cpp

        src/cellmltoolsplugin.cpp
    HEADERS_MOC
        ../../plugin.h
        ../../pluginmanager.h

        src/cellmltoolsplugin.h
    DEFINITIONS
        ${LLVM_DEFINITIONS}
    INCLUDE_DIRS
        src
    PLUGINS
        CellMLAPI
        CellMLSupport
        Compiler
        Core
        CoreSolver
        ${LLVM_PLUGIN}
    PLUGIN_BINARIES
        ${LLVM_PLUGIN_BINARY}
    QT_LIBRARIES
        QtCore
        QtGui
        QtNetwork
        QtWidgets
        QtXml
        QtXmlPatterns
    EXTERNAL_BINARIES
        ${CELLML_API_EXTERNAL_BINARIES}
    TESTS
        tests
)
