set(USE_SYSTEM_AWS_SDK OFF CACHE BOOL "Use the system version of AWS SDK")

if (NOT USE_SYSTEM_AWS_SDK)
    message("Getting AWS SDK from the web...")
    SET(AWS_SDK_VERSION "")
    SET(AWS_SDK_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-sdk-cpp-${AWS_SDK_VERSION})
    SET(AWS_SDK_BINARY_DIR ${CMAKE_BINARY_DIR}/aws-sdk-cpp-${AWS_SDK_VERSION}-build)
    SET(AWS_SDK_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/orthanc/aws)

    #SET(AWS_SDK_URL "https://github.com/aws/aws-sdk-cpp/archive/1.4.70.tar.gz")
    #SET(AWS_SDK_MD5 "d41d8cd98f00b204e9800998ecf8427e")
    SET(AWS_SDK_GIT_REPO "https://github.com/aws/aws-sdk-cpp.git")
    SET(AWS_SDK_GIT_TAG "${AWS_SDK_VERSION}")

    #file(MAKE_DIRECTORY ${AWS_SDK_BINARY_DIR})
    
    # There is an issue with static build because aws libs 
    # are adding libcurl libz and libssl from the system
    # A solution would be to eitherr add here the sources directly
    # or make aws use the same libs as Orthanc
    SET(AWS_SDK_SHARED "ON")
    SET(AWS_CORE_LIBRARY ${AWS_SDK_INSTALL_DIR}/lib/libaws-cpp-sdk-core${CMAKE_SHARED_LIBRARY_SUFFIX})
    SET(AWS_S3_LIBRARY ${AWS_SDK_INSTALL_DIR}/lib/libaws-cpp-sdk-s3${CMAKE_SHARED_LIBRARY_SUFFIX})
    SET(AWS_TRANSFER ${AWS_SDK_INSTALL_DIR}/lib/libaws-cpp-sdk-transfer${CMAKE_SHARED_LIBRARY_SUFFIX})

    include(ExternalProject)

    set(EXTERNAL_CXX_FLAGS "-Wno-unused-private-field")
    set(EXTERNAL_C_FLAGS "")

    ExternalProject_Add(aws-cpp-sdk
        SOURCE_DIR ${AWS_SDK_SOURCES_DIR}
        BINARY_DIR ${AWS_SDK_BINARY_DIR}
        #URL ${AWS_SDK_URL}
        #URL_HASH "MD5=${AWS_SDK_MD5}"
        GIT_REPOSITORY "${AWS_SDK_GIT_REPO}"
        GIT_TAG "${AWS_SDK_GIT_TAG}"
        CMAKE_ARGS
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${AWS_SDK_INSTALL_DIR}
        -DCMAKE_CXX_FLAGS=${EXTERNAL_CXX_FLAGS}
        -DCMAKE_C_FLAGS=${EXTERNAL_C_FLAGS}
        -DBUILD_SHARED_LIBS=${AWS_SDK_SHARED}
        -DBUILD_ONLY=transfer;s3
        -DAUTORUN_UNIT_TESTS=OFF
        #GIT_PROGRESS "ON"
        #TODO enable tests again once the issue is fixed and the pull request is accepted:
        #https://github.com/aws/aws-sdk-cpp/issues/1010
        #https://github.com/aws/aws-sdk-cpp/pull/1033
        #https://github.com/ploki/aws-sdk-cpp/commit/c5aa6ec225cb9f77793654e5334ee49faa6e1ae2
        #-DSIMPLE_INSTALL="ON"
        #-DNO_HTTP_CLIENT="ON"
        #-DNO_ENCRYPTION="ON"
        #-DFORCE_CURL="ON"
    )

    include_directories(${AWS_SDK_INSTALL_DIR}/include)

    link_libraries(${AWS_S3_LIBRARY})
    link_libraries(${AWS_CORE_LIBRARY})
    link_libraries(${AWS_TRANSFER})

else()
    message("Looking for local AWS SDK: ${AWS_SDK_BINARY_DIR}")

    # Locate the aws sdk for c++ package.
    find_package(aws-sdk-cpp REQUIRED
        HINTS ${AWS_SDK_BINARY_DIR}
        NO_DEFAULT_PATH
        )

    add_definitions(-DUSE_IMPORT_EXPORT)

    link_libraries(aws-cpp-sdk-core)
    link_libraries(aws-cpp-sdk-s3)
    link_libraries(aws-cpp-sdk-transfer)

endif()
