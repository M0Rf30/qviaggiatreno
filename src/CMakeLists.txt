# Libreria con la logica di analisi delle pagine (senza dipendenze dalla GUI
# dell'applicazione), condivisa tra l'eseguibile e i test.
qt_add_library(qviaggiatreno_core STATIC
    parser_viaggiatreno_base.cpp parser_viaggiatreno_base.h
    parser_viaggiatreno_lista.h
    parser_viaggiatreno_stazione.cpp parser_viaggiatreno_stazione.h
    parser_viaggiatreno_treno.cpp parser_viaggiatreno_treno.h
    parser_trenord.cpp parser_trenord.h
    utils.cpp utils.h
)
target_include_directories(qviaggiatreno_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(qviaggiatreno_core PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Xml
)

qt_add_executable(qviaggiatreno WIN32 MACOSX_BUNDLE
    main.cpp
    config.cpp config.h
    dialogo_configurazione.cpp dialogo_configurazione.h dlgconfigurazione.ui
    dialogo_nomestazione.cpp dialogo_nomestazione.h dlgnomestazione.ui
    download_trenord.cpp download_trenord.h
    download_viaggiatreno.cpp download_viaggiatreno.h
    items.cpp items.h
    qledindicator.cpp qledindicator.h
    qviaggiatreno.cpp qviaggiatreno.h
    qviaggiatrenoslots.cpp
    qviaggiatrenoslots_schede.cpp
    schedaavvisitrenord.cpp schedaavvisitrenord.h schedaavvisitrenord_widget.cpp wgtavvisitrenord.ui
    schedalistatreni.cpp schedalistatreni.h schedalistatreni_widget.cpp
    schedastazione.cpp schedastazione.h schedastazione_widget.cpp wgtstazione.ui
    schedatreno.cpp schedatreno.h schedatreno_widget.cpp
    schedaviaggiatreno.cpp schedaviaggiatreno.h
    application.qrc
)
target_compile_definitions(qviaggiatreno PRIVATE QVT_VERSION="${PROJECT_VERSION}")
target_link_libraries(qviaggiatreno PRIVATE
    qviaggiatreno_core
    Qt6::Network
)

install(TARGETS qviaggiatreno
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
