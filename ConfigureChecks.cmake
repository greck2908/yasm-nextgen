INCLUDE(CheckCXXSourceCompiles)
INCLUDE(TestCXXAcceptsFlag)
INCLUDE(CMakeBackwardCompatibilityCXX)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckIncludeFile)
INCLUDE(CheckSymbolExists)

FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

INCLUDE(YasmCXXTR1)

#FIND_PROGRAM(XMLTO NAMES xmlto)
#IF (XMLTO)
#    SET(BUILD_MAN ON)
#ENDIF (XMLTO)

# Platform-specific include files (POSIX, Win32)
CHECK_INCLUDE_FILE(locale.h HAVE_LOCALE_H)
CHECK_INCLUDE_FILE(libgen.h HAVE_LIBGEN_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILE(direct.h HAVE_DIRECT_H)

CHECK_SYMBOL_EXISTS(abort "stdlib.h" HAVE_ABORT)

CHECK_FUNCTION_EXISTS(getcwd HAVE_GETCWD)

CONFIGURE_FILE(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

ADD_DEFINITIONS(-DHAVE_CONFIG_H)
INCLUDE(FindPythonInterp)
#INCLUDE(FindPythonLibs)

#IF (PYTHONINTERP_FOUND)
#    EXEC_PROGRAM("${PYTHON_EXECUTABLE}"
#                 ARGS "${yasm_SOURCE_DIR}/CMake/have_pyrex.py"
#                 RETURN_VALUE HAVE_PYREX)
#ENDIF (PYTHONINTERP_FOUND)

FIND_PROGRAM(RE2C_EXECUTABLE NAMES re2c)

IF (CMAKE_COMPILER_IS_GNUCXX)
    CHECK_CXX_ACCEPTS_FLAG(-pipe CXX_ACCEPTS_PIPE)
    CHECK_CXX_ACCEPTS_FLAG(-ansi CXX_ACCEPTS_ANSI)
    CHECK_CXX_ACCEPTS_FLAG(-pedantic CXX_ACCEPTS_PEDANTIC)
    CHECK_CXX_ACCEPTS_FLAG(-Wall CXX_ACCEPTS_WALL)
    CHECK_CXX_ACCEPTS_FLAG(-Wextra CXX_ACCEPTS_WEXTRA)
    CHECK_CXX_ACCEPTS_FLAG(-Weffc++ CXX_ACCEPTS_WEFFCPP)
    CHECK_CXX_ACCEPTS_FLAG(-Wstrict-null-sentinel CXX_ACCEPTS_WSTRICTNULL)
    CHECK_CXX_ACCEPTS_FLAG(-Woverloaded-virtual CXX_ACCEPTS_WOVERVIRTUAL)
    CHECK_CXX_ACCEPTS_FLAG(-Wno-unused-parameter CXX_ACCEPTS_WNOUNUSEDPARAM)

    IF (CXX_ACCEPTS_PIPE)
        ADD_DEFINITIONS(-pipe)
    ENDIF (CXX_ACCEPTS_PIPE)

    IF (CXX_ACCEPTS_ANSI)
        ADD_DEFINITIONS(-ansi)
    ENDIF (CXX_ACCEPTS_ANSI)

    IF (CXX_ACCEPTS_PEDANTIC)
        ADD_DEFINITIONS(-pedantic)
    ENDIF (CXX_ACCEPTS_PEDANTIC)

    IF (CXX_ACCEPTS_WALL)
        ADD_DEFINITIONS(-Wall)
    ENDIF (CXX_ACCEPTS_WALL)

    IF (CXX_ACCEPTS_WEXTRA)
        ADD_DEFINITIONS(-Wextra)
    ENDIF (CXX_ACCEPTS_WEXTRA)

    #IF (CXX_ACCEPTS_WEFFCPP)
    #    ADD_DEFINITIONS(-Weffc++)
    #ENDIF (CXX_ACCEPTS_WEFFCPP)

    IF (CXX_ACCEPTS_WSTRICTNULL)
        ADD_DEFINITIONS(-Wstrict-null-sentinel)
    ENDIF (CXX_ACCEPTS_WSTRICTNULL)

    IF (CXX_ACCEPTS_WOVERVIRTUAL)
        ADD_DEFINITIONS(-Woverloaded-virtual)
    ENDIF (CXX_ACCEPTS_WOVERVIRTUAL)

    IF (CXX_ACCEPTS_WNOUNUSEDPARAM)
        ADD_DEFINITIONS(-Wno-unused-parameter)
    ENDIF (CXX_ACCEPTS_WNOUNUSEDPARAM)
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

# Disable some annoying Visual Studio warnings
IF (MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_WARNINGS)
ENDIF (MSVC)
