include_directories (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	)

option (WITH_DBUS_LOADERS "Build D-Bus loaders on UNIX" OFF)

if (WITH_DBUS_LOADERS)
	add_definitions (-DWITH_DBUS_LOADERS)
endif ()

option (WITH_QWT "Enable support for Qwt (for QML PlotItem, for example)" ON)
if (WITH_QWT)
	add_definitions (-DWITH_QWT)
endif ()

add_definitions (-DBOOST_PROGRAM_OPTIONS_DYN_LINK)
add_definitions (${QT_DEFINITIONS})

option (WITH_DOCS "Enable building documentation (requires Doxygen)" OFF)
option (WITH_DOCS_INSTALL "Install generated documentation (if WITH_DOCS is set)" OFF)
if (WITH_DOCS)
	find_package (Doxygen REQUIRED)

	set (DOXYDIR "${CMAKE_CURRENT_SOURCE_DIR}/../../doc/doxygen/core/")
	set (DOXYFILE "${DOXYDIR}/Doxyfile")

	add_custom_target (doc_core ALL
		"sed" "-i" "s/^PROJECT_NUMBER.*/PROJECT_NUMBER = '${LEECHCRAFT_VERSION}'/" "${DOXYFILE}"
		COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE}"
		COMMAND "mv" "-f" "${DOXYDIR}/out" "${CMAKE_CURRENT_BINARY_DIR}/out"
		WORKING_DIRECTORY "${DOXYDIR}"
	)
	if (WITH_DOCS_INSTALL)
		install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/out/html" DESTINATION "share/doc/leechcraft-core-${LEECHCRAFT_VERSION}")
	endif ()
endif ()

option (WITH_X11 "Enable X11 support on Linux" ON)
if (WITH_X11 AND NOT APPLE AND NOT USE_QT5)
	find_package (X11)

	if (X11_FOUND)
		add_definitions (-DHAVE_X11)
		set (ADDITIONAL_LIBS ${X11_X11_LIB})
	endif ()
endif ()

set (SRCS
	mainwindow.cpp
	commonjobadder.cpp
	core.cpp
	main.cpp
	pluginmanager.cpp
	xmlsettingsmanager.cpp
	pluginmanagerdialog.cpp
	iconthemeengine.cpp
	childactioneventfilter.cpp
	tabmanager.cpp
	authenticationdialog.cpp
	sslerrorsdialog.cpp
	storagebackend.cpp
	sqlstoragebackend.cpp
	debugmessagehandler.cpp
	application.cpp
	handlerchoicedialog.cpp
	shortcutmanager.cpp
	keysequencer.cpp
	networkaccessmanager.cpp
	coreproxy.cpp
	tagsmanager.cpp
	tagsviewer.cpp
	aboutdialog.cpp
	startupwizard.cpp
	localsockethandler.cpp
	wizardtypechoicepage.cpp
	newtabmenumanager.cpp
	plugintreebuilder.cpp
	coreinstanceobject.cpp
	settingstab.cpp
	separatetabbar.cpp
	separatetabwidget.cpp
	mwproxy.cpp
	winwarndialog.cpp
	coreplugin2manager.cpp
	dockmanager.cpp
	acceptlangwidget.cpp
	entitymanager.cpp
	colorthemeengine.cpp
	rootwindowsmanager.cpp
	docktoolbarmanager.cpp
	mainwindowmenumanager.cpp
	sslerrorshandler.cpp
	loaders/ipluginloader.cpp
	loaders/sopluginloader.cpp
	)

if (WITH_DBUS_LOADERS)
	set (SRCS
		${SRCS}
		loaders/dbuspluginloader.cpp
		loaders/infoproxy.cpp
		loaders/dbus/marshalling.cpp
		loaders/dbus/coreproxyserverwrapper.cpp
		loaders/dbus/coreproxyproxy.cpp
	)
endif ()

set (FORMS
	commonjobadder.ui
	leechcraft.ui
	pluginmanagerdialog.ui
	authenticationdialog.ui
	sslerrorsdialog.ui
	handlerchoicedialog.ui
	shortcutmanager.ui
	keysequencer.ui
	tagsviewer.ui
	aboutdialog.ui
	wizardtypechoicepage.ui
	settingstab.ui
	acceptlangwidget.ui
	)
set (RESOURCES mainresources.qrc)

set (UTILITY_SRCS)
foreach (ITEM IN LISTS UTIL_SRCS XSD_SRCS)
	list (APPEND UTILITY_SRCS "../${ITEM}")
endforeach ()

set (UTILITY_FORMS)
foreach (ITEM IN LISTS UTIL_FORMS XSD_FORMS)
	list (APPEND UTILITY_FORMS "../${ITEM}")
endforeach ()

CreateTrs ("" "ru_RU;de;uk_UA;it;ar;eo;es;tr;tt;en;be;fr;pl" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("" "en;ru_RU" "${SRCS};${UTILITY_SRCS}" "${FORMS};${UTILITY_FORMS}" "coresettings.xml")

if (USE_QT5)
	QT5_WRAP_UI (UIS_H ${FORMS})
	QT5_ADD_RESOURCES (RCC ${RESOURCES})
else ()
	QT4_WRAP_UI (UIS_H ${FORMS})
	QT4_ADD_RESOURCES (RCC ${RESOURCES})
endif ()

set (WIN_RESOURCES)
if (WIN32)
	set (WIN_RESOURCES ../../../tools/win32/leechcraft.rc)
endif ()

if (USE_QT5)
	set (LC_EXEC_SUFFIX "-qt5")
else ()
	set (LC_EXEC_SUFFIX "")
endif ()

add_executable (leechcraft${LC_EXEC_SUFFIX} WIN32
	../config.h
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCC}
	${WIN_RESOURCES}
	)

target_link_libraries (leechcraft${LC_EXEC_SUFFIX}
		${Boost_PROGRAM_OPTIONS_LIBRARY}
		${LEECHCRAFT_LIBRARIES}
		${ADDITIONAL_LIBS}
		)

install (TARGETS leechcraft${LC_EXEC_SUFFIX} DESTINATION ${LC_BINDIR})
install (FILES coresettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft${LC_EXEC_SUFFIX} Gui Network Widgets Xml Quick Qml Script Concurrent Svg Sql)

if (WITH_DBUS_LOADERS)
	add_subdirectory (loaders/dbus)
	FindQtLibs (leechcraft${LC_EXEC_SUFFIX} DBus)
endif ()
