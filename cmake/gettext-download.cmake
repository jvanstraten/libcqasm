cmake_minimum_required(VERSION 2.8.2)

project(gettext-download NONE)

include(ExternalProject)
ExternalProject_Add(
    gettext
    URL "https://ftp.gnu.org/gnu/gettext/gettext-${GETTEXT_VERSION_REQUIRED}.tar.gz"
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/gettext-build"
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/gettext-install"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ""
    TEST_COMMAND      ""
)
