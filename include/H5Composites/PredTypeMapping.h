/**
 * @file PredTypeMapping.h
 * @author Jon Burr
 * @brief Preprocessor definitions for all of the defined H5 predicate types and their corresponding C++ type
 * @version 0.0.0
 * @date 2021-12-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef H5COMPOSITES_PREDTYPEMAPPING_H
#define H5COMPOSITES_PREDTYPEMAPPING_H

#include <bitset>
#include "H5Cpp.h"

/**
 * @brief Map C++ types to their corresponding H5::DataType
 */
#define H5COMPOSITES_CTYPETOH5DTYPE \
    ((char               , NATIVE_CHAR)) \
    ((signed char        , NATIVE_SCHAR)) \
    ((unsigned char      , NATIVE_UCHAR)) \
    ((short              , NATIVE_SHORT)) \
    ((unsigned short     , NATIVE_USHORT)) \
    ((int                , NATIVE_INT)) \
    ((unsigned int       , NATIVE_UINT)) \
    ((long               , NATIVE_LONG)) \
    ((unsigned long      , NATIVE_ULONG)) \
    ((long long          , NATIVE_LLONG)) \
    ((unsigned long long , NATIVE_ULLONG)) \
    ((float              , NATIVE_FLOAT)) \
    ((double             , NATIVE_DOUBLE)) \
    ((long double        , NATIVE_LDOUBLE)) \
    ((std::bitset<8>     , NATIVE_B8)) \
    ((std::bitset<16>    , NATIVE_B16)) \
    ((std::bitset<32>    , NATIVE_B32)) \
    ((std::bitset<64>    , NATIVE_B64)) \
    ((bool               , NATIVE_HBOOL))

/**
 * @brief Map H5::DataTypes to their corresponding C++ type
 */
    #define H5COMPOSITES_H5DTYPETOCTYPE \
    ((STD_I8BE        , int8_t)) \
    ((STD_I8LE        , int8_t)) \
    ((STD_I16BE       , int16_t)) \
    ((STD_I16LE       , int16_t)) \
    ((STD_I32BE       , int32_t)) \
    ((STD_I32LE       , int32_t)) \
    ((STD_I64BE       , int64_t)) \
    ((STD_I64LE       , int64_t)) \
    ((STD_U8BE        , uint8_t)) \
    ((STD_U8LE        , uint8_t)) \
    ((STD_U16BE       , uint16_t)) \
    ((STD_U16LE       , uint16_t)) \
    ((STD_U32BE       , uint32_t)) \
    ((STD_U32LE       , uint32_t)) \
    ((STD_U64BE       , uint64_t)) \
    ((STD_U64LE       , uint64_t)) \
    ((STD_B8BE        , std::bitset<8>)) \
    ((STD_B8LE        , std::bitset<8>)) \
    ((STD_B16BE       , std::bitset<16>)) \
    ((STD_B16LE       , std::bitset<16>)) \
    ((STD_B32BE       , std::bitset<32>)) \
    ((STD_B32LE       , std::bitset<32>)) \
    ((STD_B64BE       , std::bitset<64>)) \
    ((STD_B64LE       , std::bitset<64>)) \
    ((C_S1            , char)) \
    ((FORTRAN_S1      , char)) \
    ((IEEE_F32BE      , float)) \
    ((IEEE_F32LE      , float)) \
    ((IEEE_F64BE      , double)) \
    ((IEEE_F64LE      , double)) \
    ((UNIX_D32BE      , float)) \
    ((UNIX_D32LE      , float)) \
    ((UNIX_D64BE      , double)) \
    ((UNIX_D64LE      , double)) \
    ((INTEL_I8        , int8_t)) \
    ((INTEL_I16       , int16_t)) \
    ((INTEL_I32       , int32_t)) \
    ((INTEL_I64       , int64_t)) \
    ((INTEL_U8        , uint8_t)) \
    ((INTEL_U16       , uint16_t)) \
    ((INTEL_U32       , uint32_t)) \
    ((INTEL_U64       , uint64_t)) \
    ((INTEL_B8        , std::bitset<8>)) \
    ((INTEL_B16       , std::bitset<16>)) \
    ((INTEL_B32       , std::bitset<32>)) \
    ((INTEL_B64       , std::bitset<64>)) \
    ((INTEL_F32       , float)) \
    ((INTEL_F64       , double)) \
    ((ALPHA_I8        , int8_t)) \
    ((ALPHA_I16       , int16_t)) \
    ((ALPHA_I32       , int32_t)) \
    ((ALPHA_I64       , int64_t)) \
    ((ALPHA_U8        , uint8_t)) \
    ((ALPHA_U16       , uint16_t)) \
    ((ALPHA_U32       , uint32_t)) \
    ((ALPHA_U64       , uint64_t)) \
    ((ALPHA_B8        , std::bitset<8>)) \
    ((ALPHA_B16       , std::bitset<16>)) \
    ((ALPHA_B32       , std::bitset<32>)) \
    ((ALPHA_B64       , std::bitset<64>)) \
    ((ALPHA_F32       , float)) \
    ((ALPHA_F64       , double)) \
    ((MIPS_I8         , int8_t)) \
    ((MIPS_I16        , int16_t)) \
    ((MIPS_I32        , int32_t)) \
    ((MIPS_I64        , int64_t)) \
    ((MIPS_U8         , uint8_t)) \
    ((MIPS_U16        , uint16_t)) \
    ((MIPS_U32        , uint32_t)) \
    ((MIPS_U64        , uint64_t)) \
    ((MIPS_B8         , std::bitset<8>)) \
    ((MIPS_B16        , std::bitset<16>)) \
    ((MIPS_B32        , std::bitset<32>)) \
    ((MIPS_B64        , std::bitset<64>)) \
    ((MIPS_F32        , float)) \
    ((MIPS_F64        , double)) \
    ((NATIVE_CHAR     , char)) \
    ((NATIVE_SCHAR    , signed char)) \
    ((NATIVE_UCHAR    , unsigned char)) \
    ((NATIVE_SHORT    , short)) \
    ((NATIVE_USHORT   , unsigned short)) \
    ((NATIVE_INT      , int)) \
    ((NATIVE_UINT     , unsigned int)) \
    ((NATIVE_LONG     , long)) \
    ((NATIVE_ULONG    , unsigned long)) \
    ((NATIVE_LLONG    , long long)) \
    ((NATIVE_ULLONG   , unsigned long long)) \
    ((NATIVE_FLOAT    , float)) \
    ((NATIVE_DOUBLE   , double)) \
    ((NATIVE_LDOUBLE  , long double)) \
    ((NATIVE_B8       , std::bitset<8>)) \
    ((NATIVE_B16      , std::bitset<16>)) \
    ((NATIVE_B32      , std::bitset<32>)) \
    ((NATIVE_B64      , std::bitset<64>)) \
    ((NATIVE_HSIZE    , hsize_t)) \
    ((NATIVE_HSSIZE   , hssize_t)) \
    ((NATIVE_HERR     , herr_t)) \
    ((NATIVE_HBOOL    , bool)) \
    ((NATIVE_INT8     , int8_t)) \
    ((NATIVE_UINT8    , uint8_t)) \
    ((NATIVE_INT16    , int16_t)) \
    ((NATIVE_UINT16   , uint16_t)) \
    ((NATIVE_INT32    , int32_t)) \
    ((NATIVE_UINT32   , uint32_t)) \
    ((NATIVE_INT64    , int64_t)) \
    ((NATIVE_UINT64   , uint64_t))

#endif //> !H5COMPOSITES_PREDTYPEMAPPING_H