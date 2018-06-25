if (STATIC_BUILD OR NOT USE_SYSTEM_OPENSSL)
  # WARNING - We had to repack the upstream ".tar.gz" file to a ZIP
  # file, as the upstream distribution ships symbolic links that are
  # not always properly handled when uncompressing on Windows.

  SET(OPENSSL_SOURCES_DIR ${CMAKE_BINARY_DIR}/openssl-1.0.2d)
  SET(OPENSSL_URL "http://www.orthanc-server.com/downloads/third-party/openssl-1.0.2d.zip")
  SET(OPENSSL_MD5 "4b2ac15fc6db17f3dadc54482d3eee85")

  if (IS_DIRECTORY "${OPENSSL_SOURCES_DIR}")
    set(FirstRun OFF)
  else()
    set(FirstRun ON)
  endif()

  DownloadPackage(${OPENSSL_MD5} ${OPENSSL_URL} "${OPENSSL_SOURCES_DIR}")

  add_definitions(
    -DOPENSSL_THREADS
    -DOPENSSL_IA32_SSE2
    -DOPENSSL_NO_ASM
    -DOPENSSL_NO_DYNAMIC_ENGINE
    -DNO_WINDOWS_BRAINDEATH

    -DOPENSSL_NO_BF 
    -DOPENSSL_NO_CAMELLIA
    -DOPENSSL_NO_CAST 
    -DOPENSSL_NO_EC_NISTP_64_GCC_128
    -DOPENSSL_NO_GMP
    -DOPENSSL_NO_GOST
    -DOPENSSL_NO_HW
    -DOPENSSL_NO_JPAKE
    -DOPENSSL_NO_IDEA
    -DOPENSSL_NO_KRB5 
    -DOPENSSL_NO_MD2 
    -DOPENSSL_NO_MDC2 
    -DOPENSSL_NO_MD4
    -DOPENSSL_NO_RC2 
    -DOPENSSL_NO_RC4 
    -DOPENSSL_NO_RC5 
    -DOPENSSL_NO_RFC3779
    -DOPENSSL_NO_SCTP
    -DOPENSSL_NO_STORE
    -DOPENSSL_NO_SEED
    -DOPENSSL_NO_WHIRLPOOL
    -DOPENSSL_NO_RIPEMD
    )

  include_directories(
    ${OPENSSL_SOURCES_DIR}
    ${OPENSSL_SOURCES_DIR}/crypto
    ${OPENSSL_SOURCES_DIR}/crypto/asn1
    ${OPENSSL_SOURCES_DIR}/crypto/modes
    ${OPENSSL_SOURCES_DIR}/crypto/evp
    ${OPENSSL_SOURCES_DIR}/include
    )

  set(OPENSSL_SOURCES_SUBDIRS
    ${OPENSSL_SOURCES_DIR}/crypto
    ${OPENSSL_SOURCES_DIR}/crypto/aes
    ${OPENSSL_SOURCES_DIR}/crypto/asn1
    ${OPENSSL_SOURCES_DIR}/crypto/bio
    ${OPENSSL_SOURCES_DIR}/crypto/bn
    ${OPENSSL_SOURCES_DIR}/crypto/buffer
    ${OPENSSL_SOURCES_DIR}/crypto/cmac
    ${OPENSSL_SOURCES_DIR}/crypto/cms
    ${OPENSSL_SOURCES_DIR}/crypto/comp
    ${OPENSSL_SOURCES_DIR}/crypto/conf
    ${OPENSSL_SOURCES_DIR}/crypto/des
    ${OPENSSL_SOURCES_DIR}/crypto/dh
    ${OPENSSL_SOURCES_DIR}/crypto/dsa
    ${OPENSSL_SOURCES_DIR}/crypto/dso
    ${OPENSSL_SOURCES_DIR}/crypto/engine
    ${OPENSSL_SOURCES_DIR}/crypto/err
    ${OPENSSL_SOURCES_DIR}/crypto/evp
    ${OPENSSL_SOURCES_DIR}/crypto/hmac
    ${OPENSSL_SOURCES_DIR}/crypto/lhash
    ${OPENSSL_SOURCES_DIR}/crypto/md5
    ${OPENSSL_SOURCES_DIR}/crypto/modes
    ${OPENSSL_SOURCES_DIR}/crypto/objects
    ${OPENSSL_SOURCES_DIR}/crypto/ocsp
    ${OPENSSL_SOURCES_DIR}/crypto/pem
    ${OPENSSL_SOURCES_DIR}/crypto/pkcs12
    ${OPENSSL_SOURCES_DIR}/crypto/pkcs7
    ${OPENSSL_SOURCES_DIR}/crypto/pqueue
    ${OPENSSL_SOURCES_DIR}/crypto/rand
    ${OPENSSL_SOURCES_DIR}/crypto/rsa
    ${OPENSSL_SOURCES_DIR}/crypto/sha
    ${OPENSSL_SOURCES_DIR}/crypto/srp
    ${OPENSSL_SOURCES_DIR}/crypto/stack
    ${OPENSSL_SOURCES_DIR}/crypto/ts
    ${OPENSSL_SOURCES_DIR}/crypto/txt_db
    ${OPENSSL_SOURCES_DIR}/crypto/ui
    ${OPENSSL_SOURCES_DIR}/crypto/x509
    ${OPENSSL_SOURCES_DIR}/crypto/x509v3
    ${OPENSSL_SOURCES_DIR}/ssl
    )

  if (ENABLE_PKCS11)
    list(APPEND OPENSSL_SOURCES_SUBDIRS
      # EC, ECDH and ECDSA are necessary for PKCS11
      ${OPENSSL_SOURCES_DIR}/crypto/ec
      ${OPENSSL_SOURCES_DIR}/crypto/ecdh
      ${OPENSSL_SOURCES_DIR}/crypto/ecdsa
      )
  else()
    add_definitions(
      -DOPENSSL_NO_EC
      -DOPENSSL_NO_ECDH
      -DOPENSSL_NO_ECDSA
      )
  endif()

  foreach(d ${OPENSSL_SOURCES_SUBDIRS})
    AUX_SOURCE_DIRECTORY(${d} OPENSSL_SOURCES)
  endforeach()

  list(REMOVE_ITEM OPENSSL_SOURCES
    ${OPENSSL_SOURCES_DIR}/crypto/LPdir_unix.c
    ${OPENSSL_SOURCES_DIR}/crypto/LPdir_vms.c
    ${OPENSSL_SOURCES_DIR}/crypto/LPdir_win.c
    ${OPENSSL_SOURCES_DIR}/crypto/LPdir_win32.c
    ${OPENSSL_SOURCES_DIR}/crypto/LPdir_wince.c
    ${OPENSSL_SOURCES_DIR}/crypto/armcap.c
    ${OPENSSL_SOURCES_DIR}/crypto/bf/bfs.cpp
    ${OPENSSL_SOURCES_DIR}/crypto/bio/bss_rtcp.c
    ${OPENSSL_SOURCES_DIR}/crypto/bn/exp.c
    ${OPENSSL_SOURCES_DIR}/crypto/conf/cnf_save.c
    ${OPENSSL_SOURCES_DIR}/crypto/conf/test.c
    ${OPENSSL_SOURCES_DIR}/crypto/des/des.c
    ${OPENSSL_SOURCES_DIR}/crypto/des/des3s.cpp
    ${OPENSSL_SOURCES_DIR}/crypto/des/des_opts.c
    ${OPENSSL_SOURCES_DIR}/crypto/des/dess.cpp
    ${OPENSSL_SOURCES_DIR}/crypto/des/read_pwd.c
    ${OPENSSL_SOURCES_DIR}/crypto/des/speed.c
    ${OPENSSL_SOURCES_DIR}/crypto/evp/e_dsa.c
    ${OPENSSL_SOURCES_DIR}/crypto/evp/m_ripemd.c
    ${OPENSSL_SOURCES_DIR}/crypto/lhash/lh_test.c
    ${OPENSSL_SOURCES_DIR}/crypto/md5/md5s.cpp
    ${OPENSSL_SOURCES_DIR}/crypto/pkcs7/bio_ber.c
    ${OPENSSL_SOURCES_DIR}/crypto/pkcs7/pk7_enc.c
    ${OPENSSL_SOURCES_DIR}/crypto/ppccap.c
    ${OPENSSL_SOURCES_DIR}/crypto/rand/randtest.c
    ${OPENSSL_SOURCES_DIR}/crypto/s390xcap.c
    ${OPENSSL_SOURCES_DIR}/crypto/sparcv9cap.c
    ${OPENSSL_SOURCES_DIR}/crypto/x509v3/tabtest.c
    ${OPENSSL_SOURCES_DIR}/crypto/x509v3/v3conf.c
    ${OPENSSL_SOURCES_DIR}/ssl/ssl_task.c
    ${OPENSSL_SOURCES_DIR}/crypto/LPdir_nyi.c
    ${OPENSSL_SOURCES_DIR}/crypto/aes/aes_x86core.c
    ${OPENSSL_SOURCES_DIR}/crypto/bio/bss_dgram.c
    ${OPENSSL_SOURCES_DIR}/crypto/bn/bntest.c
    ${OPENSSL_SOURCES_DIR}/crypto/bn/expspeed.c
    ${OPENSSL_SOURCES_DIR}/crypto/bn/exptest.c
    ${OPENSSL_SOURCES_DIR}/crypto/engine/enginetest.c
    ${OPENSSL_SOURCES_DIR}/crypto/evp/evp_test.c
    ${OPENSSL_SOURCES_DIR}/crypto/hmac/hmactest.c
    ${OPENSSL_SOURCES_DIR}/crypto/md5/md5.c
    ${OPENSSL_SOURCES_DIR}/crypto/md5/md5test.c
    ${OPENSSL_SOURCES_DIR}/crypto/o_dir_test.c
    ${OPENSSL_SOURCES_DIR}/crypto/pkcs7/dec.c
    ${OPENSSL_SOURCES_DIR}/crypto/pkcs7/enc.c
    ${OPENSSL_SOURCES_DIR}/crypto/pkcs7/sign.c
    ${OPENSSL_SOURCES_DIR}/crypto/pkcs7/verify.c
    ${OPENSSL_SOURCES_DIR}/crypto/rsa/rsa_test.c
    ${OPENSSL_SOURCES_DIR}/crypto/sha/sha.c
    ${OPENSSL_SOURCES_DIR}/crypto/sha/sha1.c
    ${OPENSSL_SOURCES_DIR}/crypto/sha/sha1t.c
    ${OPENSSL_SOURCES_DIR}/crypto/sha/sha1test.c
    ${OPENSSL_SOURCES_DIR}/crypto/sha/sha256t.c
    ${OPENSSL_SOURCES_DIR}/crypto/sha/sha512t.c
    ${OPENSSL_SOURCES_DIR}/crypto/sha/shatest.c
    ${OPENSSL_SOURCES_DIR}/crypto/srp/srptest.c

    ${OPENSSL_SOURCES_DIR}/crypto/bn/divtest.c
    ${OPENSSL_SOURCES_DIR}/crypto/bn/bnspeed.c
    ${OPENSSL_SOURCES_DIR}/crypto/des/destest.c
    ${OPENSSL_SOURCES_DIR}/crypto/dh/p192.c
    ${OPENSSL_SOURCES_DIR}/crypto/dh/p512.c
    ${OPENSSL_SOURCES_DIR}/crypto/dh/p1024.c
    ${OPENSSL_SOURCES_DIR}/crypto/des/rpw.c
    ${OPENSSL_SOURCES_DIR}/ssl/ssltest.c
    ${OPENSSL_SOURCES_DIR}/crypto/dsa/dsagen.c
    ${OPENSSL_SOURCES_DIR}/crypto/dsa/dsatest.c
    ${OPENSSL_SOURCES_DIR}/crypto/dh/dhtest.c
    ${OPENSSL_SOURCES_DIR}/crypto/pqueue/pq_test.c
    ${OPENSSL_SOURCES_DIR}/crypto/des/ncbc_enc.c

    ${OPENSSL_SOURCES_DIR}/crypto/evp/evp_extra_test.c
    ${OPENSSL_SOURCES_DIR}/crypto/evp/verify_extra_test.c
    ${OPENSSL_SOURCES_DIR}/crypto/x509/verify_extra_test.c
    ${OPENSSL_SOURCES_DIR}/crypto/x509v3/v3prin.c
    ${OPENSSL_SOURCES_DIR}/crypto/x509v3/v3nametest.c
    ${OPENSSL_SOURCES_DIR}/crypto/constant_time_test.c
    ${OPENSSL_SOURCES_DIR}/crypto/ec/ecp_nistz256_table.c

    ${OPENSSL_SOURCES_DIR}/ssl/heartbeat_test.c
    )


  if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set_source_files_properties(
      ${OPENSSL_SOURCES}
      PROPERTIES COMPILE_DEFINITIONS
      "OPENSSL_SYSNAME_WIN32;SO_WIN32;WIN32_LEAN_AND_MEAN;L_ENDIAN")
  endif()

  source_group(ThirdParty\\OpenSSL REGULAR_EXPRESSION ${OPENSSL_SOURCES_DIR}/.*)

else()
  include(FindOpenSSL)

  if (NOT ${OPENSSL_FOUND})
    message(FATAL_ERROR "Unable to find OpenSSL")
  endif()

  include_directories(${OPENSSL_INCLUDE_DIR})
  link_libraries(${OPENSSL_LIBRARIES})
endif()
