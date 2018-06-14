set(USE_SYSTEM_AWS_SDK OFF CACHE BOOL "Use the system version of AWS SDK")

if (NOT USE_SYSTEM_AWS_SDK)
    message("Getting AWS SDK from the web...")
    SET(AWS_SDK_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-sdk-cpp-1.4.65)
    SET(AWS_SDK_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
    SET(AWS_SDK_BINARY_DIR ${CMAKE_BINARY_DIR}/aws-sdk-cpp-1.4.65-build)
    SET(AWS_SDK_URL "https://github.com/aws/aws-sdk-cpp/archive/1.4.65.tar.gz")
    SET(AWS_SDK_MD5 "5f6a0c627b212a80ade15600c7124ddc")

    if (${STATIC_BUILD})
        SET(AWS_SDK_SHARED "OFF")
        set(AWS_CORE_LIBRARY ${AWS_SDK_INSTALL_DIR}/lib/libaws-cpp-sdk-core.a)
        set(AWS_S3_LIBRARY ${AWS_SDK_INSTALL_DIR}/lib/libaws-cpp-sdk-s3.a)
    else (${STATIC_BUILD})
        SET(AWS_SDK_SHARED "ON")
        set(AWS_CORE_LIBRARY ${AWS_SDK_INSTALL_DIR}/lib/libaws-cpp-sdk-core.${CMAKE_SHARED_LIBRARY_SUFFIX})
        set(AWS_S3_LIBRARY ${AWS_SDK_INSTALL_DIR}/lib/libaws-cpp-sdk-s3.${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif (${STATIC_BUILD})

    include(ExternalProject)

    set(EXTERNAL_CXX_FLAGS "-Wno-unused-private-field")
    set(EXTERNAL_C_FLAGS "")

    ExternalProject_Add(aws-cpp-sdk-s3
        SOURCE_DIR ${AWS_SDK_SOURCES_DIR}
        BINARY_DIR ${AWS_SDK_BINARY_DIR}
        URL ${AWS_SDK_URL}
        URL_HASH "MD5=${AWS_SDK_MD5}"
        #GIT_REPOSITORY "git@github.com:aws/aws-sdk-cpp.git"
        #GIT_TAG "1.4.65"
        CMAKE_ARGS
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${AWS_SDK_INSTALL_DIR}
        -DCMAKE_CXX_FLAGS=${EXTERNAL_CXX_FLAGS}
        -DCMAKE_C_FLAGS=${EXTERNAL_C_FLAGS}
        -DBUILD_SHARED_LIBS=${AWS_SDK_SHARED}
        -DBUILD_ONLY=s3
    )


    include_directories(${AWS_SDK_INSTALL_DIR}/include)
    link_libraries(${AWS_CORE_LIBRARY} ${AWS_S3_LIBRARY})


else()
    message("Looking for local AWS SDK...")

    # Locate the aws sdk for c++ package.
    find_package(aws-sdk-cpp REQUIRED
        HINTS ${AWS_SDK_BINARY_DIR}
        NO_DEFAULT_PATH
        )

    add_definitions(-DUSE_IMPORT_EXPORT)

    link_libraries(aws-cpp-sdk-s3)

endif()
