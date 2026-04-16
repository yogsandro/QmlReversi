cmake_minimum_required(VERSION 3.16)
project(QmlOthello
	VERSION 0.1
	DESCRIPTION "Othello Qml view" 
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Declare imported target
add_library(libothello SHARED IMPORTED)

# Path to the .so on disk (adjust)
set_target_properties(libothello PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libothello.so"
    INTERFACE_INCLUDE_DIRECTORIES "path to othello include dir"
)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_standard_project_setup()

qt_add_executable(${PROJECT_NAME} 
	QmlOthello.cpp
	GameModel.h
	GameModel.cpp
)

# Add the QML file(s) to resources
qt_add_qml_module(${PROJECT_NAME} 
    URI OthelloQmlApp
    VERSION 1.0
    QML_FILES
        main.qml
)


target_link_libraries(${PROJECT_NAME} 
    PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 libothello
)

# Create a simple configuration header
configure_file(config.h.in config.h)


# Include the configuration header in the build
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")


# ----- Deploy target using linuxdeployqt -----

# Output directory for staged bundle
set(LINUXDEPLOY_DIR "${CMAKE_BINARY_DIR}/linuxdeploy")
set(LINUXDEPLOY_APPIMAGE "")

# Ensure output dir exists
file(MAKE_DIRECTORY "${LINUXDEPLOY_DIR}")

# Path to the built binary
set(APP_BIN "$<TARGET_FILE:${PROJECT_NAME}>")

# Create a desktop file (optional but recommended for linuxdeployqt)
set(DESKTOP_FILE "${LINUXDEPLOY_DIR}/SimpleQmlApp.desktop")
file(WRITE "${DESKTOP_FILE}"
"[Desktop Entry]\n"
"Type=Application\n"
"Name=SimpleQmlApp\n"
"Exec=${PROJECT_NAME}\n"
"Icon=app\n"
"Categories=Utility;\n"
)

# Optionally add an icon file copy if you have one (png/svg)
# file(COPY ${CMAKE_SOURCE_DIR}/icon.png DESTINATION ${LINUXDEPLOY_DIR})

# The linuxdeployqt command will be run after build.
add_custom_command(
    OUTPUT "${LINUXDEPLOY_DIR}/.linuxdeploy_done"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${LINUXDEPLOY_DIR}/usr"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${PROJECT_NAME}> "${LINUXDEPLOY_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DESKTOP_FILE}" "${LINUXDEPLOY_DIR}/"
    # If you have an icon: COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/icon.png ${LINUXDEPLOY_DIR}/
    # Run linuxdeployqt to gather runtime libs/plugins/qml imports.
    COMMAND linuxdeployqt "${LINUXDEPLOY_DIR}/${PROJECT_NAME}" -appimage -qmldir=${CMAKE_SOURCE_DIR} --output-dir="${LINUXDEPLOY_DIR}"  # -appimage creates an AppImage
    # Mark done file
    COMMAND ${CMAKE_COMMAND} -E touch "${LINUXDEPLOY_DIR}/.linuxdeploy_done"
    DEPENDS ${PROJECT_NAME}
    COMMENT "Running linuxdeployqt to bundle Qt and QML into ${LINUXDEPLOY_DIR}"
    VERBATIM
)

add_custom_target(deploy-linux
    DEPENDS "${LINUXDEPLOY_DIR}/.linuxdeploy_done"
)