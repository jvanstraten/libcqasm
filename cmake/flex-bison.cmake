cmake_minimum_required(VERSION 2.8.2)

find_package(BISON ${BISON_VERSION_REQUIRED})

if(NOT BISON_FOUND)

    message(WARNING "bison ${BISON_VERSION_REQUIRED} not found on your system. trying to build from source...")

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/bison-download.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/bison-download/CMakeLists.txt"
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bison-download"
    )
    if(result)
        message(FATAL_ERROR "download step for bison ${BISON_VERSION_REQUIRED} failed: ${result}")
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bison-download"
    )
    if(result)
        message(FATAL_ERROR "download step for bison ${BISON_VERSION_REQUIRED} failed: ${result}")
    endif()

    execute_process(
        COMMAND ./configure --prefix "${CMAKE_CURRENT_BINARY_DIR}/bison-install"
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bison-build"
    )
    if(result)
        message(FATAL_ERROR "configure step for bison ${BISON_VERSION_REQUIRED} failed: ${result}")
    endif()

    execute_process(
        COMMAND make -j8
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bison-build"
    )
    if(result)
        message(FATAL_ERROR "build step for bison ${BISON_VERSION_REQUIRED} failed: ${result}")
    endif()

    execute_process(
        COMMAND make install
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bison-build"
    )
    if(result)
        message(FATAL_ERROR "install step for bison ${BISON_VERSION_REQUIRED} failed: ${result}")
    endif()

    set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/bison-install")
    find_package(
        BISON ${BISON_VERSION_REQUIRED} EXACT
        REQUIRED
    )

endif()

# find_package(FLEX ${FLEX_VERSION_REQUIRED})

if(NOT FLEX_FOUND)

    message(WARNING "flex ${FLEX_VERSION_REQUIRED} not found on your system. trying to build from source...")

    set(GETTEXT_VERSION_REQUIRED 0.19)

#     find_package(Gettext ${GETTEXT_VERSION_REQUIRED})
    if(NOT GETTEXT_FOUND)

        message(WARNING "you also don't have gettext. this will take a while...")

        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gettext-download.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/gettext-download/CMakeLists.txt"
        )

        execute_process(
            COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
            RESULT_VARIABLE result
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gettext-download"
        )
        if(result)
            message(FATAL_ERROR "download step for gettext failed: ${result}")
        endif()

        execute_process(
            COMMAND ${CMAKE_COMMAND} --build .
            RESULT_VARIABLE result
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gettext-download"
        )
        if(result)
            message(FATAL_ERROR "download step for gettext failed: ${result}")
        endif()

        execute_process(
            COMMAND ./configure --prefix "${CMAKE_CURRENT_BINARY_DIR}/gettext-install"
            RESULT_VARIABLE result
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gettext-build"
        )
        if(result)
            message(FATAL_ERROR "configure step for gettext failed: ${result}")
        endif()

        execute_process(
            COMMAND make -j8
            RESULT_VARIABLE result
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gettext-build"
        )
        if(result)
            message(FATAL_ERROR "build step for gettext failed: ${result}")
        endif()

        execute_process(
            COMMAND make install
            RESULT_VARIABLE result
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gettext-build"
        )
        if(result)
            message(FATAL_ERROR "install step for gettext failed: ${result}")
        endif()

        set(ENV{PATH} "$ENV{PATH}:${CMAKE_CURRENT_BINARY_DIR}/gettext-install/bin")

    endif()

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/flex-download.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/flex-download/CMakeLists.txt"
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/flex-download"
    )
    if(result)
        message(FATAL_ERROR "download step for flex ${FLEX_VERSION_REQUIRED} failed: ${result}")
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/flex-download"
    )
    if(result)
        message(FATAL_ERROR "download step for flex ${FLEX_VERSION_REQUIRED} failed: ${result}")
    endif()

    execute_process(
        COMMAND "./autogen.sh"
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/flex-build"
    )
    if(result)
        message(FATAL_ERROR "configure step for flex ${FLEX_VERSION_REQUIRED} failed: ${result}")
    endif()

    execute_process(
        COMMAND ./configure --prefix "${CMAKE_CURRENT_BINARY_DIR}/flex-install"
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/flex-build"
    )
    if(result)
        message(FATAL_ERROR "configure step for flex ${FLEX_VERSION_REQUIRED} failed: ${result}")
    endif()

    execute_process(
        COMMAND make -j8
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/flex-build"
    )
    if(result)
        message(FATAL_ERROR "build step for flex ${FLEX_VERSION_REQUIRED} failed: ${result}")
    endif()

    execute_process(
        COMMAND make install
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/flex-build"
    )
    if(result)
        message(FATAL_ERROR "install step for flex ${FLEX_VERSION_REQUIRED} failed: ${result}")
    endif()

    set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/flex-install")
    find_package(
        FLEX ${FLEX_VERSION_REQUIRED} EXACT
        REQUIRED
    )

endif()
