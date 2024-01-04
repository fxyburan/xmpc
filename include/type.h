//
// Created by Xinyu Feng on 2024/1/4.
//

#ifndef CRYPTO_SRC_COMMON_TYPE_H_
#define CRYPTO_SRC_COMMON_TYPE_H_

#include <bitset>
#include <vector>

typedef unsigned __int128 uint128_t;
typedef __int128 int128_t;

typedef unsigned char ui8;
typedef unsigned short int ui16;
typedef unsigned int ui32;
typedef unsigned long int ui64;
typedef uint128_t ui128;
typedef signed char i8;
typedef short int i16;
typedef int i32;
typedef long int i64;
typedef int128_t i128;
typedef std::bitset<128> bmpc;

#define HIGH_PRECISION
#ifdef HIGH_PRECISION
typedef i128 mpc_t;
typedef ui8 small_mpc_t;
typedef i128 signed_mpc_t;
typedef i8 bit_t;
#define sField  137
#define bnd ((256/sField)*sField)
const mpc_t MINUS_ONE = (signed_mpc_t) -1;
const i64 MINUS_ONE_64 = (i64) -1;

#define BIT_SIZE 128
#define BIT_SIZE_64 64
const mpc_t power_64 = (signed_mpc_t) 1 << (BIT_SIZE_64);
#define signn(x) ((x<0)? -1:1)
#endif

#ifndef HIGH_PRECISION
typedef ui64 mpc_t;
typedef ui8 small_mpc_t;
typedef i64 signed_mpc_t;
typedef i8 bit_t;
#endif

#endif //CRYPTO_SRC_COMMON_TYPE_H_
