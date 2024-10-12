cmake_minimum_required(VERSION 3.16)

project(dtls_pair_chat VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(appdtls_pair_chat
    src/main.cpp
)

qt_add_qml_module(appdtls_pair_chat
    URI dtls_pair_chat
    VERSION 1.0
    SOURCES
        include/ChatMessagesModel.h
        include/ConnectionHandler.h
        include/ConnectionSettings.h
        include/Handshake.h
        include/HostInfo.h
        include/PasswordVerifier.h
        include/UdpMessage.h
        include/UdpConnection.h
        src/ChatMessagesModel.cpp
        src/ConnectionHandler.cpp
        src/ConnectionSettings.cpp
        src/Handshake.cpp
        src/HostInfo.cpp
        src/PasswordVerifier.cpp
        src/UdpMessage.cpp
        src/UdpConnection.cpp
    QML_FILES
        qml/ChatListDelegate.qml
        qml/ChatScreen.qml
        qml/ConnectionDialog.qml
        qml/LoginScreen.qml
        qml/Main.qml
        qml/TextFieldWithErrorLabel.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appdtls_pair_chat PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appdtls_pair_chat
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appdtls_pair_chat
    PRIVATE
    Qt6::Network
    Qt6::Qml
    Qt6::Quick
)

target_include_directories(appdtls_pair_chat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(GNUInstallDirs)
install(TARGETS appdtls_pair_chat
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
