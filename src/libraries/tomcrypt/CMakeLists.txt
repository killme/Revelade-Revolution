
project(tomcrypt C)

set(TOMCRYPT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/headers
    CACHE INTERNAL "LibTomcrypt include directories."
)

include_directories(
    ${TOMMATH_INCLUDE_DIRS}
    ${TOMCRYPT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(TOMCRYPT_LIBRARY tomcrypt CACHE INTERNAL "libtomcrypt library.")

tig_add_definitions_for(${TOMCRYPT_LIBRARY})

add_definitions(
    -Wall -Wsign-compare -W -Wshadow -Wno-unused-parameter
    
    -DLTC_SOURCE -DLTC_NO_ASM
    
    # additional warnings (newer GCC 3.4 and higher) TODO: disable this on incompatible compilers
    "-Wmissing-prototypes -Wsystem-headers -Wdeclaration-after-statement -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wpointer-arith"
)

IF(NOT ${CMAKE_BUILD_TYPE} EQUAL "DEBUG")
    add_definitions(-funroll-loops -fomit-frame-pointer)
endif()

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/headers/tomcrypt_config.h.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/src/headers/tomcrypt_config.h)

add_library(${TOMCRYPT_LIBRARY} STATIC
    # RSA
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/rsa/rsa_decrypt_key.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/rsa/rsa_encrypt_key.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/rsa/rsa_export.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/rsa/rsa_exptmod.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/rsa/rsa_free.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/rsa/rsa_import.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/rsa/rsa_make_key.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/rsa/rsa_sign_hash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/rsa/rsa_verify_hash.c
    
    # Math
    #${CMAKE_CURRENT_SOURCE_DIR}/src/math/fp/ltc_ecc_fp_mulmod.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/math/gmp_desc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/math/ltm_desc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/math/multi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/math/rand_prime.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/math/tfm_desc.c
    
    # Misc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/burn_stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/error_to_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/pk_get_oid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/zeromem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/pkcs5/pkcs_5_1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/pkcs5/pkcs_5_2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_argchk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_cipher_descriptor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_cipher_is_valid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_find_cipher.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_find_cipher_any.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_find_cipher_id.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_find_hash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_find_hash_any.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_find_hash_id.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_find_hash_oid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_find_prng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_fsa.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_hash_descriptor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_hash_is_valid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_ltc_mp_descriptor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_prng_descriptor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_prng_is_valid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_register_cipher.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_register_hash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_register_prng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_unregister_cipher.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_unregister_hash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crypt/crypt_unregister_prng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/base64/base64_decode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/base64/base64_encode.c
    
    # PRNG
    ${CMAKE_CURRENT_SOURCE_DIR}/src/prngs/fortuna.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/prngs/rc4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/prngs/rng_get_bytes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/prngs/rng_make_prng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/prngs/sober128.c
    #${CMAKE_CURRENT_SOURCE_DIR}/src/prngs/sober128tab.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/prngs/sprng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/prngs/yarrow.c
    
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/pkcs1/pkcs_1_i2osp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/pkcs1/pkcs_1_mgf1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/pkcs1/pkcs_1_oaep_decode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/pkcs1/pkcs_1_oaep_encode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/pkcs1/pkcs_1_os2ip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/pkcs1/pkcs_1_pss_decode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/pkcs1/pkcs_1_pss_encode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/pkcs1/pkcs_1_v1_5_decode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/pkcs1/pkcs_1_v1_5_encode.c

    ${CMAKE_CURRENT_SOURCE_DIR}/src/hashes/helper/hash_file.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hashes/helper/hash_filehandle.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hashes/helper/hash_memory.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hashes/helper/hash_memory_multi.c
    
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hashes/sha1.c
    
    # ASN1
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/sequence/der_encode_subject_public_key_info.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/sequence/der_encode_sequence_multi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/sequence/der_decode_sequence_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/sequence/der_length_sequence.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/sequence/der_encode_sequence_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/sequence/der_sequence_free.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/sequence/der_decode_sequence_multi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/sequence/der_decode_subject_public_key_info.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/sequence/der_decode_sequence_flexi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/bit/der_encode_raw_bit_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/bit/der_encode_bit_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/bit/der_decode_bit_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/bit/der_length_bit_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/bit/der_decode_raw_bit_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/utctime/der_decode_utctime.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/utctime/der_encode_utctime.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/utctime/der_length_utctime.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/choice/der_decode_choice.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/printable_string/der_decode_printable_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/printable_string/der_encode_printable_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/printable_string/der_length_printable_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/short_integer/der_decode_short_integer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/short_integer/der_encode_short_integer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/short_integer/der_length_short_integer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/octet/der_length_octet_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/octet/der_decode_octet_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/octet/der_encode_octet_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/ia5/der_encode_ia5_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/ia5/der_length_ia5_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/ia5/der_decode_ia5_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/set/der_encode_setof.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/set/der_encode_set.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/utf8/der_encode_utf8_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/utf8/der_decode_utf8_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/utf8/der_length_utf8_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/boolean/der_length_boolean.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/boolean/der_decode_boolean.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/boolean/der_encode_boolean.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/integer/der_encode_integer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/integer/der_length_integer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/integer/der_decode_integer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/object_identifier/der_encode_object_identifier.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/object_identifier/der_length_object_identifier.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pk/asn1/der/object_identifier/der_decode_object_identifier.c)

target_link_libraries(${TOMCRYPT_LIBRARY} ${TOMMATH_LIBRARIES})
tig_install(${TOMCRYPT_LIBRARY})

set(TOMCRYPT_LIBRARIES
    ${TOMCRYPT_LIBRARY}
    ${TOMMATH_LIBRARIES}
    CACHE INTERNAL "Tomcrypt libraries.")