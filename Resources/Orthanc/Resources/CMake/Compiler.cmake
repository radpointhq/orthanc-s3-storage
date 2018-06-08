# This file sets all the compiler-related flags

if (CMAKE_CROSSCOMPILING OR
    "${CMAKE_SYSTEM_VERSION}" STREQUAL "LinuxStandardBase")
  # Cross-compilation necessarily implies standalone and static build
  SET(STATIC_BUILD ON)
  SET(STANDALONE_BUILD ON)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-long-long")

  # --std=c99 makes libcurl not to compile
  # -pedantic gives a lot of warnings on OpenSSL 
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -Wno-variadic-macros")

  if (CMAKE_CROSSCOMPILING)
    # http://stackoverflow.com/a/3543845/881731
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -I<CMAKE_CURRENT_SOURCE_DIR> <SOURCE> <OBJECT>")
  endif()

elseif (MSVC)
  # Use static runtime under Visual Studio
  # http://www.cmake.org/Wiki/CMake_FAQ#Dynamic_Replace
  # http://stackoverflow.com/a/6510446
  foreach(flag_var
    CMAKE_C_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE 
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL 
    CMAKE_CXX_FLAGS_MINSIZEREL 
    CMAKE_C_FLAGS_RELWITHDEBINFO 
    CMAKE_CXX_FLAGS_RELWITHDEBINFO) 
    string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
  endforeach(flag_var)

  # Add /Zm256 compiler option to Visual Studio to fix PCH errors
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm256")

  add_definitions(
    -D_CRT_SECURE_NO_WARNINGS=1
    -D_CRT_SECURE_NO_DEPRECATE=1
    )

  if (MSVC_VERSION LESS 1600)
    # Starting with Visual Studio >= 2010 (i.e. macro _MSC_VER >=
    # 1600), Microsoft ships a standard-compliant <stdint.h>
    # header. For earlier versions of Visual Studio, give access to a
    # compatibility header.
    # http://stackoverflow.com/a/70630/881731
    # https://en.wikibooks.org/wiki/C_Programming/C_Reference/stdint.h#External_links
    include_directories(${ORTHANC_ROOT}/Resources/ThirdParty/VisualStudio)
  endif()

  link_libraries(netapi32)
endif()


if (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD" OR
    ${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")
  # In FreeBSD/OpenBSD, the "/usr/local/" folder contains the ports and need to be imported
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I/usr/local/include")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib")
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L/usr/local/lib")
endif()


if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR
    ${CMAKE_SYSTEM_NAME} STREQUAL "kFreeBSD" OR
    ${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD" OR
    ${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")

  if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")
    # The "--no-undefined" linker flag makes the shared libraries
    # (plugins ModalityWorklists and ServeFolders) fail to compile on OpenBSD
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
  endif()

  if (NOT DEFINED ENABLE_PLUGINS_VERSION_SCRIPT OR 
      ENABLE_PLUGINS_VERSION_SCRIPT)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script=${ORTHANC_ROOT}/Plugins/Samples/Common/VersionScript.map")
  endif()

  # Remove the "-rdynamic" option
  # http://www.mail-archive.com/cmake@cmake.org/msg08837.html
  set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
  link_libraries(pthread)

  if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")
    link_libraries(rt)
  endif()

  if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD" AND
      NOT ${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")
    link_libraries(dl)
  endif()

  if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD" AND
      NOT ${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")
    # The "--as-needed" linker flag is not available on FreeBSD and OpenBSD
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--as-needed")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
  endif()

  if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD" AND
      NOT ${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")
    # FreeBSD/OpenBSD have just one single interface for file
    # handling, which is 64bit clean, so there is no need to define macro
    # for LFS (Large File Support).
    # https://ohse.de/uwe/articles/lfs.html
    add_definitions(
      -D_LARGEFILE64_SOURCE=1 
      -D_FILE_OFFSET_BITS=64
      )
  endif()

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  if (MSVC)
    message("MSVC compiler version = " ${MSVC_VERSION} "\n")
    # Starting Visual Studio 2013 (version 1800), it is not possible
    # to target Windows XP anymore
    if (MSVC_VERSION LESS 1800)
      add_definitions(
        -DWINVER=0x0501
        -D_WIN32_WINNT=0x0501
        )
    endif()
  else()
    add_definitions(
      -DWINVER=0x0501
      -D_WIN32_WINNT=0x0501
      )
  endif()

  add_definitions(
    -D_CRT_SECURE_NO_WARNINGS=1
    )
  link_libraries(rpcrt4 ws2_32)

  if (CMAKE_COMPILER_IS_GNUCXX)
    # Some additional C/C++ compiler flags for MinGW
    SET(MINGW_NO_WARNINGS "-Wno-unused-function -Wno-unused-variable")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MINGW_NO_WARNINGS} -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MINGW_NO_WARNINGS}")

    # This is a patch for MinGW64
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--allow-multiple-definition -static-libgcc -static-libstdc++")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--allow-multiple-definition -static-libgcc -static-libstdc++")

    CHECK_LIBRARY_EXISTS(winpthread pthread_create "" HAVE_WIN_PTHREAD)
    if (HAVE_WIN_PTHREAD)
      # This line is necessary to compile with recent versions of MinGW,
      # otherwise "libwinpthread-1.dll" is not statically linked.
      SET(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic")
      add_definitions(-DHAVE_WIN_PTHREAD=1)
    else()
      add_definitions(-DHAVE_WIN_PTHREAD=0)
    endif()
  endif()

elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -exported_symbols_list ${ORTHANC_ROOT}/Plugins/Samples/Common/ExportedSymbols.list")

  add_definitions(
    -D_XOPEN_SOURCE=1
    )
  link_libraries(iconv)

elseif (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  message("Building using Emscripten (for WebAssembly or asm.js targets)")

  # The BINARYEN_TRAP_MODE specifies what to do when divisions per
  # zero (and similar conditions like integer overflows) are
  # encountered: The "clamp" mode avoids throwing errors, as they
  # cannot be properly catched by "try {} catch (...)" constructions.
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s EXTRA_EXPORTED_RUNTIME_METHODS='[\"ccall\", \"cwrap\"]' -s BINARYEN_TRAP_MODE='\"clamp\"'")

else()
  message(FATAL_ERROR "Support your platform here")
endif()


if (DEFINED ENABLE_PROFILING AND ENABLE_PROFILING)
  if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -pg")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
  else()
    message(FATAL_ERROR "Don't know how to enable profiling on your configuration")
  endif()
endif()


if (CMAKE_COMPILER_IS_GNUCXX)
  # "When creating a static library using binutils (ar) and there
  # exist a duplicate object name (e.g. a/Foo.cpp.o, b/Foo.cpp.o), the
  # resulting static library can end up having only one of the
  # duplicate objects. [...] This bug only happens if there are many
  # objects." The trick consists in replacing the "r" argument
  # ("replace") provided to "ar" (as used in CMake < 3.1) by the "q"
  # argument ("quick append"). This is because of the fact that CMake
  # will invoke "ar" several times with several batches of ".o"
  # objects, and using "r" would overwrite symbols defined in
  # preceding batches. https://cmake.org/Bug/view.php?id=14874
  set(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> <LINK_FLAGS> q <TARGET> <OBJECTS>")
endif()


if (STATIC_BUILD)
  add_definitions(-DORTHANC_STATIC=1)
else()
  add_definitions(-DORTHANC_STATIC=0)
endif()
