PROJECT(QwtPlugin)

# Use our pre-built version unless instructed otherwise

IF(USE_PREBUILT_QWT_PLUGIN)
    # Retrieve the plugin's binary file(s)

    STRING(REPLACE "${${CMAKE_PROJECT_NAME}_SOURCE_DIR}/" "" RELATIVE_PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/bin/${DISTRIB_BINARY_DIR}")

    IF(WIN32)
        IF(${ARCHITECTURE} EQUAL 32)
            IF(RELEASE_MODE)
                RETRIEVE_BINARY_FILE(${RELATIVE_PROJECT_SOURCE_DIR} Qwt.dll 7e426df100999d92136534936efc94ca327cc355)
                RETRIEVE_BINARY_FILE(${RELATIVE_PROJECT_SOURCE_DIR} Qwt.lib f76d287001a5213be882ed4dc189039f60991472)
            ELSE()
                RETRIEVE_BINARY_FILE(${RELATIVE_PROJECT_SOURCE_DIR} Qwt.dll bce6c40b97da8ccbf16df358a63fcdab620af015)
                RETRIEVE_BINARY_FILE(${RELATIVE_PROJECT_SOURCE_DIR} Qwt.lib 6b7c5d4e2ef27da90db80a8a8e0f94b457e9287e)
            ENDIF()
        ELSE()
            IF(RELEASE_MODE)
                RETRIEVE_BINARY_FILE(${RELATIVE_PROJECT_SOURCE_DIR} Qwt.dll cd73e4c56d575ffaa59db6c8584f4089fc5a1fde)
                RETRIEVE_BINARY_FILE(${RELATIVE_PROJECT_SOURCE_DIR} Qwt.lib f2b1702f51e720e7ed38ceb85885fc7222e0b4cc)
            ELSE()
                RETRIEVE_BINARY_FILE(${RELATIVE_PROJECT_SOURCE_DIR} Qwt.dll 47125d0492a6ebdc7d8fdcaf1dd837e7988b3e83)
                RETRIEVE_BINARY_FILE(${RELATIVE_PROJECT_SOURCE_DIR} Qwt.lib 230cf96aa995cb2425dd0d6df9acbf766a3c47e1)
            ENDIF()
        ENDIF()
    ELSEIF(APPLE)
        RETRIEVE_BINARY_FILE(${RELATIVE_PROJECT_SOURCE_DIR} libQwt.dylib c141718049364a969e62e365b6da2ba79abdeccc)
    ELSE()
        IF(${ARCHITECTURE} EQUAL 32)
            RETRIEVE_BINARY_FILE(${RELATIVE_PROJECT_SOURCE_DIR} libQwt.so cefb301d50d048713740e4643f6ed95402269878)
        ELSE()
            RETRIEVE_BINARY_FILE(${RELATIVE_PROJECT_SOURCE_DIR} libQwt.so ca1d77f9e9d8e05d908bfea207724999bd9ccb18)
        ENDIF()
    ENDIF()

    # Add the plugin as a binary
    # Note: in the case of Windows, must refer to the distribution directory as
    #       opposed to the plugins directory since we need the .lib file and
    #       it's only there...

    IF(WIN32)
        SET(QWT_PLUGIN_BINARY ${PROJECT_SOURCE_DIR}/bin/${DISTRIB_BINARY_DIR}/${CMAKE_IMPORT_LIBRARY_PREFIX}Qwt${CMAKE_IMPORT_LIBRARY_SUFFIX} PARENT_SCOPE)
    ELSE()
        SET(QWT_PLUGIN_BINARY ${DEST_PLUGINS_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}Qwt${CMAKE_SHARED_LIBRARY_SUFFIX} PARENT_SCOPE)
    ENDIF()

    ADD_PLUGIN_BINARY(Qwt
        INCLUDE_DIRS
            src
            textengines/mathml
        QT_LIBRARIES
            QtConcurrent
            QtCore
            QtGui
            QtOpenGL
            QtPrintSupport
            QtSvg
            QtWidgets
            QtXml
    )
ELSE()
    # Add the plugin

    SET(QWT_PLUGIN Qwt PARENT_SCOPE)

    ADD_PLUGIN(Qwt
        THIRD_PARTY
        SOURCES
            ../../plugininfo.cpp

            src/qwt_abstract_legend.cpp
            src/qwt_abstract_scale.cpp
            src/qwt_abstract_scale_draw.cpp
            src/qwt_abstract_slider.cpp
            src/qwt_analog_clock.cpp
            src/qwt_arrow_button.cpp
            src/qwt_clipper.cpp
            src/qwt_color_map.cpp
            src/qwt_column_symbol.cpp
            src/qwt_compass.cpp
            src/qwt_compass_rose.cpp
            src/qwt_counter.cpp
            src/qwt_curve_fitter.cpp
            src/qwt_date.cpp
            src/qwt_date_scale_draw.cpp
            src/qwt_date_scale_engine.cpp
            src/qwt_dial.cpp
            src/qwt_dial_needle.cpp
            src/qwt_dyngrid_layout.cpp
            src/qwt_event_pattern.cpp
            src/qwt_graphic.cpp
            src/qwt_interval.cpp
            src/qwt_interval_symbol.cpp
            src/qwt_knob.cpp
            src/qwt_legend.cpp
            src/qwt_legend_data.cpp
            src/qwt_legend_label.cpp
            src/qwt_magnifier.cpp
            src/qwt_math.cpp
            src/qwt_matrix_raster_data.cpp
            src/qwt_null_paintdevice.cpp
            src/qwt_painter.cpp
            src/qwt_painter_command.cpp
            src/qwt_panner.cpp
            src/qwt_picker.cpp
            src/qwt_picker_machine.cpp
            src/qwt_pixel_matrix.cpp
            src/qwt_plot.cpp
            src/qwt_plot_abstract_barchart.cpp
            src/qwt_plot_axis.cpp
            src/qwt_plot_barchart.cpp
            src/qwt_plot_canvas.cpp
            src/qwt_plot_curve.cpp
            src/qwt_plot_dict.cpp
            src/qwt_plot_directpainter.cpp
            src/qwt_plot_glcanvas.cpp
            src/qwt_plot_grid.cpp
            src/qwt_plot_histogram.cpp
            src/qwt_plot_intervalcurve.cpp
            src/qwt_plot_item.cpp
            src/qwt_plot_layout.cpp
            src/qwt_plot_legenditem.cpp
            src/qwt_plot_magnifier.cpp
            src/qwt_plot_marker.cpp
            src/qwt_plot_multi_barchart.cpp
            src/qwt_plot_panner.cpp
            src/qwt_plot_picker.cpp
            src/qwt_plot_rasteritem.cpp
            src/qwt_plot_renderer.cpp
            src/qwt_plot_rescaler.cpp
            src/qwt_plot_scaleitem.cpp
            src/qwt_plot_seriesitem.cpp
            src/qwt_plot_shapeitem.cpp
            src/qwt_plot_spectrocurve.cpp
            src/qwt_plot_spectrogram.cpp
            src/qwt_plot_svgitem.cpp
            src/qwt_plot_textlabel.cpp
            src/qwt_plot_tradingcurve.cpp
            src/qwt_plot_xml.cpp
            src/qwt_plot_zoneitem.cpp
            src/qwt_plot_zoomer.cpp
            src/qwt_point_3d.cpp
            src/qwt_point_data.cpp
            src/qwt_point_mapper.cpp
            src/qwt_point_polar.cpp
            src/qwt_raster_data.cpp
            src/qwt_round_scale_draw.cpp
            src/qwt_sampling_thread.cpp
            src/qwt_scale_div.cpp
            src/qwt_scale_draw.cpp
            src/qwt_scale_engine.cpp
            src/qwt_scale_map.cpp
            src/qwt_scale_widget.cpp
            src/qwt_series_data.cpp
            src/qwt_slider.cpp
            src/qwt_spline.cpp
            src/qwt_symbol.cpp
            src/qwt_system_clock.cpp
            src/qwt_text.cpp
            src/qwt_text_engine.cpp
            src/qwt_text_label.cpp
            src/qwt_thermo.cpp
            src/qwt_transform.cpp
            src/qwt_wheel.cpp
            src/qwt_widget_overlay.cpp
            src/qwtplugin.cpp

            textengines/mathml/qwt_mathml_text_engine.cpp
            textengines/mathml/qwt_mml_document.cpp
            textengines/mathml/qwt_mml_entity_table.cpp
        HEADERS_MOC
            src/qwt_abstract_legend.h
            src/qwt_abstract_scale.h
            src/qwt_abstract_slider.h
            src/qwt_analog_clock.h
            src/qwt_compass.h
            src/qwt_counter.h
            src/qwt_dial.h
            src/qwt_dyngrid_layout.h
            src/qwt_knob.h
            src/qwt_legend.h
            src/qwt_legend_label.h
            src/qwt_magnifier.h
            src/qwt_panner.h
            src/qwt_picker.h
            src/qwt_plot.h
            src/qwt_plot_canvas.h
            src/qwt_plot_glcanvas.h
            src/qwt_plot_magnifier.h
            src/qwt_plot_panner.h
            src/qwt_plot_picker.h
            src/qwt_plot_renderer.h
            src/qwt_plot_zoomer.h
            src/qwt_sampling_thread.h
            src/qwt_scale_widget.h
            src/qwt_slider.h
            src/qwt_text_label.h
            src/qwt_thermo.h
            src/qwt_wheel.h
            src/qwtplugin.h
        INCLUDE_DIRS
            src
            textengines/mathml
        DEFINITIONS
            QWT_DLL
            QWT_MAKEDLL
        QT_MODULES
            Concurrent
            OpenGL
            PrintSupport
            Svg
            Widgets
            Xml
        QT_LIBRARIES
            QtConcurrent
            QtCore
            QtGui
            QtOpenGL
            QtPrintSupport
            QtSvg
            QtWidgets
            QtXml
    )
ENDIF()
