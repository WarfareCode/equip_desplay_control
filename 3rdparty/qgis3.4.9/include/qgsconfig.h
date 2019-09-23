
// QGSCONFIG.H

#ifndef QGSCONFIG_H
#define QGSCONFIG_H

// Version must be specified according to
// <int>.<int>.<int>-<any text>.
// or else upgrading old project file will not work
// reliably.
#define VERSION "3.4.9-Madeira"

//used in vim src/core/qgis.cpp
//The way below should work but it resolves to a number like 0110 which the compiler treats as octal I think
//because debuggin it out shows the decimal number 72 which results in incorrect version status.
//As a short term fix I (Tim) am defining the version in top level cmake. It would be good to
//reinstate this more generic approach below at some point though
//#define VERSION_INT 349
#define VERSION_INT 30409
#define ABISYM(x) x ## 30409
//used in main.cpp and anywhere else where the release name is needed
#define RELEASE_NAME "Madeira"

#define QGIS_PLUGIN_SUBDIR "plugins"
#define QGIS_DATA_SUBDIR "."
#define QGIS_LIBEXEC_SUBDIR "."
#define QGIS_LIB_SUBDIR "lib"
#define QGIS_QML_SUBDIR "qml"
#define CMAKE_INSTALL_PREFIX "D:/Fb/work/qgis_vs2017_qt5.12.4/qgis-ltr-3.4.9/install-vs2017"
#define CMAKE_SOURCE_DIR "D:/Fb/work/qgis_vs2017_qt5.12.4/qgis-ltr-3.4.9"

#define QGIS_SERVER_MODULE_SUBDIR "server"

#define QSCINTILLA_VERSION_STR "2.11.2"

#if defined( __APPLE__ )
//used by Mac to find system or bundle resources relative to amount of bundling
#define QGIS_MACAPP_BUNDLE 
#endif

#define QT_PLUGINS_DIR "C:/Qt/Qt5.12.4/5.12.4/msvc2017_64/plugins"
#define OSG_PLUGINS_PATH ""

/* #undef USING_NMAKE */

/* #undef USING_NINJA */

#define HAVE_GUI

#define HAVE_POSTGRESQL

/* #undef HAVE_ORACLE */

/* #undef HAVE_OSGEARTHQT */

/* #undef SERVER_SKIP_ECW */

/* #undef HAVE_SERVER_PYTHON_PLUGINS */

#define HAVE_OPENCL

/* #undef ENABLE_MODELTEST */

/* #undef HAVE_3D */

#define USE_THREAD_LOCAL

/* #undef QGISDEBUG */

/* #undef HAVE_QUICK */

#endif

