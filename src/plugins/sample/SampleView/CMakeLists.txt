PROJECT(SampleViewPlugin)

# Add the plugin

ADD_PLUGIN(SampleView
    SOURCES
        ../../i18ninterface.cpp
        ../../plugininfo.cpp
        ../../viewinterface.cpp

        src/sampleviewplugin.cpp
        src/sampleviewwidget.cpp
    HEADERS_MOC
        src/sampleviewplugin.h
        src/sampleviewwidget.h
    UIS
        src/sampleviewwidget.ui
    INCLUDE_DIRS
        src
    PLUGINS
        Core
    QT_MODULES
        Widgets
        XmlPatterns
    QT_LIBRARIES
        QtCore
        QtGui
        QtNetwork
        QtWidgets
        QtXmlPatterns
)
