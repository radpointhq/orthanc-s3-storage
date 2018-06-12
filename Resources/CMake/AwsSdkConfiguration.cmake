if (NOT USE_SYSTEM_AWS_SDK)
    message("Getting AWS SDK from the web...")
    SET(AWS_SDK_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-sdk-cpp-1.4.65)
    SET(AWS_SDK_INSTALL_DIR ${CMAKE_BINARY_DIR}/aws-sdk-cpp-1.4.65-install)
    SET(AWS_SDK_BINARY_DIR ${CMAKE_BINARY_DIR}/aws-sdk-cpp-1.4.65-build)
    SET(AWS_SDK_URL "https://github.com/aws/aws-sdk-cpp/archive/1.4.65.tar.gz")
    SET(AWS_SDK_MD5 "5f6a0c627b212a80ade15600c7124ddc")

    include(ExternalProject)

    set(EXTERNAL_CXX_FLAGS "-Wno-unused-private-field")
    set(EXTERNAL_C_FLAGS "")

    ExternalProject_Add(AWS_SDK
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
        -DBUILD_SHARED_LIBS=(NOT ${STATIC_BUILD})
        -DBUILD_ONLY=s3
    )

    set(AWS_CORE_LIBRARY ${AWS_SDK_INSTALL_DIR}/lib/libaws-cpp-sdk-core.a)
    set(AWS_S3_LIBRARY ${AWS_SDK_INSTALL_DIR}/lib/libaws-cpp-sdk-s3.a)

    include_directories(${AWS_SDK_INSTALL_DIR}/include)
    link_libraries(${AWS_CORE_LIBRARY} ${AWS_S3_LIBRARY})


else()
    message("Looking for local AWS SDK")

    #CMake
    SET(BUILD_ONLY "s3;dynamodb")
    SET(BUILD_SHARED_LIBS "OFF")
    message("AWS_SDK_SOURCES_DIR: ${AWS_SDK_SOURCES_DIR}")
    SET(aws-sdk-cpp_DIR "${AWS_SDK_SOURCES_DIR}")

    #TODO: build?

    # Locate the aws sdk for c++ package.
    find_package(aws-sdk-cpp)
    add_definitions(-DUSE_IMPORT_EXPORT) #TODO: only for shared

    find_library(AWS_CORE_LIBRARY libaws-cpp-sdk-core.dylib.a)
    find_library(AWS_S3_LIBRARY libaws-cpp-sdk-s3.a)

    find_package(aws-sdk-cpp)
    add_definitions(-DUSE_IMPORT_EXPORT)

    #find_library(AWS_CORE_LIBRARY libaws-cpp-sdk-core)
    find_library(AWS_S3_LIBRARY libaws-cpp-sdk-s3)

    #include_directories(${AWS_CORE_INCLUDES} ${AWS_S3_INCLUDES})
    link_libraries(${AWS_CORE_LIBRARY} ${AWS_S3_LIBRARY})

    if (NOT ${AWS_S3_LIBRARY} OR NOT ${AWS_CORE_LIBRARY})
        message(FATAL_ERROR "Unable to find AWS libraries")
    endif()
endif()
