//
// Created by Xinyu Feng on 2024/1/4.
//

#ifndef XMPC_PRF_H
#define XMPC_PRF_H

#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include "prng.h"

#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
/** GCC-compatible compiler, targeting x86/x86-64 **/
#include <wmmintrin.h>
#include <emmintrin.h>
#else
#include "sse2neon.h"
#endif

const __m128i BIT1 = _mm_setr_epi8(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i BIT2 = _mm_setr_epi8(2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i BIT4 = _mm_setr_epi8(4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i BIT8 = _mm_setr_epi8(8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i BIT16 = _mm_setr_epi8(16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i BIT32 = _mm_setr_epi8(32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i BIT64 = _mm_setr_epi8(64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i BIT128 = _mm_setr_epi8(128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

#define EXPAND_ASSIST(v1, v2, v3, v4, shuff_const, aes_const)                    \
    v2 = _mm_aeskeygenassist_si128(v4,aes_const);                           \
    v3 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(v3),              \
                                         _mm_castsi128_ps(v1), 16));        \
    v1 = _mm_xor_si128(v1,v3);                                              \
    v3 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(v3),              \
                                         _mm_castsi128_ps(v1), 140));       \
    v1 = _mm_xor_si128(v1,v3);                                              \
    v2 = _mm_shuffle_epi32(v2,shuff_const);                                 \
    v1 = _mm_xor_si128(v1,v2)

namespace xmpc::prf {

class AESObject {
 private:
  //AES variables
  __m128i pseudoRandomString[256]{0};
  __m128i tempSecComp[256]{0};
  unsigned long rCounter = -1;

  small_mpc_t AESKey[16]{0};
  __m128i rKey[16]{0};

  //Extraction variables
  __m128i randomBitNumber{0};
  uint8_t randomBitCounter = 0;
  __m128i random8BitNumber{0};
  uint8_t random8BitCounter = 0;
//        __m128i random128BitNumber {0};

 public:
  AESObject() {
  }

  AESObject(const signed_mpc_t &key) {

    char *common_aes_key = (char *) &key;

    {
      __m128i x0, x1, x2;
      //block *kp = (block *)&aesKey;
      rKey[0] = x0 = _mm_loadu_si128((__m128i *) common_aes_key);
      x2 = _mm_setzero_si128();
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 1);
      rKey[1] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 2);
      rKey[2] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 4);
      rKey[3] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 8);
      rKey[4] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 16);
      rKey[5] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 32);
      rKey[6] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 64);
      rKey[7] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 128);
      rKey[8] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 27);
      rKey[9] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 54);
      rKey[10] = x0;
    }

  }

  AESObject(char *filename, unsigned int pid = -1) {
    std::ifstream f(filename);
    std::string str{std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
    f.close();
    int len = str.length();

    strncpy((char *) AESKey, str.c_str(), 16);
    //KEY EXPANSION
    {
      __m128i x0, x1, x2;

      rKey[0] = x0 = _mm_loadu_si128((__m128i *) AESKey);
      x2 = _mm_setzero_si128();
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 1);
      rKey[1] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 2);
      rKey[2] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 4);
      rKey[3] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 8);
      rKey[4] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 16);
      rKey[5] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 32);
      rKey[6] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 64);
      rKey[7] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 128);
      rKey[8] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 27);
      rKey[9] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 54);
      rKey[10] = x0;
    }
    //Set initial counter
    rCounter = (((unsigned long) pid) << 32) - 1;
  }

  void InitializeWithFile(char *filename, unsigned int pid = -1) {
    std::ifstream f(filename);
    std::string str{std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
    f.close();
    int len = str.length();

    strncpy((char *) AESKey, str.c_str(), 16);
    //KEY EXPANSION
    {
      __m128i x0, x1, x2;

      rKey[0] = x0 = _mm_loadu_si128((__m128i *) AESKey);
      x2 = _mm_setzero_si128();
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 1);
      rKey[1] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 2);
      rKey[2] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 4);
      rKey[3] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 8);
      rKey[4] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 16);
      rKey[5] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 32);
      rKey[6] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 64);
      rKey[7] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 128);
      rKey[8] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 27);
      rKey[9] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 54);
      rKey[10] = x0;
    }
    //Set initial counter
    rCounter = (((unsigned long) pid) << 32) - 1;
  }

  void InitializeWithSeed(char *seed, unsigned int pid = -1) {
    strncpy((char *) AESKey, seed, 16);
    //KEY EXPANSION
    {
      __m128i x0, x1, x2;

      rKey[0] = x0 = _mm_loadu_si128((__m128i *) AESKey);
      x2 = _mm_setzero_si128();
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 1);
      rKey[1] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 2);
      rKey[2] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 4);
      rKey[3] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 8);
      rKey[4] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 16);
      rKey[5] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 32);
      rKey[6] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 64);
      rKey[7] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 128);
      rKey[8] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 27);
      rKey[9] = x0;
      EXPAND_ASSIST(x0, x1, x2, x0, 255, 54);
      rKey[10] = x0;
    }
    //Set initial counter
    rCounter = (((unsigned long) pid) << 32) - 1;
  }

  void AES_ecb_encrypt_chunk_in_out(__m128i *in, __m128i *out, unsigned nblks) {

    int numberOfLoops = nblks / 8;
    int blocksPipeLined = numberOfLoops * 8;
    int remainingEncrypts = nblks - blocksPipeLined;

    unsigned j, rnds = 10;
    const __m128i *sched = ((__m128i *) (rKey));
    for (int i = 0; i < numberOfLoops; i++) {
      out[0 + i * 8] = _mm_xor_si128(in[0 + i * 8], sched[0]);
      out[1 + i * 8] = _mm_xor_si128(in[1 + i * 8], sched[0]);
      out[2 + i * 8] = _mm_xor_si128(in[2 + i * 8], sched[0]);
      out[3 + i * 8] = _mm_xor_si128(in[3 + i * 8], sched[0]);
      out[4 + i * 8] = _mm_xor_si128(in[4 + i * 8], sched[0]);
      out[5 + i * 8] = _mm_xor_si128(in[5 + i * 8], sched[0]);
      out[6 + i * 8] = _mm_xor_si128(in[6 + i * 8], sched[0]);
      out[7 + i * 8] = _mm_xor_si128(in[7 + i * 8], sched[0]);

      for (j = 1; j < rnds; ++j) {
        out[0 + i * 8] = _mm_aesenc_si128(out[0 + i * 8], sched[j]);
        out[1 + i * 8] = _mm_aesenc_si128(out[1 + i * 8], sched[j]);
        out[2 + i * 8] = _mm_aesenc_si128(out[2 + i * 8], sched[j]);
        out[3 + i * 8] = _mm_aesenc_si128(out[3 + i * 8], sched[j]);
        out[4 + i * 8] = _mm_aesenc_si128(out[4 + i * 8], sched[j]);
        out[5 + i * 8] = _mm_aesenc_si128(out[5 + i * 8], sched[j]);
        out[6 + i * 8] = _mm_aesenc_si128(out[6 + i * 8], sched[j]);
        out[7 + i * 8] = _mm_aesenc_si128(out[7 + i * 8], sched[j]);
      }
      out[0 + i * 8] = _mm_aesenclast_si128(out[0 + i * 8], sched[j]);
      out[1 + i * 8] = _mm_aesenclast_si128(out[1 + i * 8], sched[j]);
      out[2 + i * 8] = _mm_aesenclast_si128(out[2 + i * 8], sched[j]);
      out[3 + i * 8] = _mm_aesenclast_si128(out[3 + i * 8], sched[j]);
      out[4 + i * 8] = _mm_aesenclast_si128(out[4 + i * 8], sched[j]);
      out[5 + i * 8] = _mm_aesenclast_si128(out[5 + i * 8], sched[j]);
      out[6 + i * 8] = _mm_aesenclast_si128(out[6 + i * 8], sched[j]);
      out[7 + i * 8] = _mm_aesenclast_si128(out[7 + i * 8], sched[j]);
    }
    for (int i = blocksPipeLined; i < blocksPipeLined + remainingEncrypts; ++i) {
      out[i] = _mm_xor_si128(in[i], sched[0]);
    }

    for (j = 1; j < rnds; ++j) {
      for (int i = blocksPipeLined; i < blocksPipeLined + remainingEncrypts; ++i) {
        out[i] = _mm_aesenc_si128(out[i], sched[j]);
      }

    }

    for (int i = blocksPipeLined; i < blocksPipeLined + remainingEncrypts; ++i) {
      out[i] = _mm_aesenclast_si128(out[i], sched[j]);
    }

  }

  void AES_ecb_encrypt_chunk_in_out(__m128i *in, __m128i *out) {

    unsigned j, rnds = 10;
    const __m128i *sched = ((__m128i *) (rKey));
    int i = 0;
    out[0 + i * 8] = _mm_xor_si128(in[0 + i * 8], sched[0]);

    for (j = 1; j < rnds; ++j) {
      out[0 + i * 8] = _mm_aesenc_si128(out[0 + i * 8], sched[j]);
    }
    out[0 + i * 8] = _mm_aesenclast_si128(out[0 + i * 8], sched[j]);

  }

  __m128i newRandomNumber() {
    //        return  _mm_setzero_si128();
    rCounter++;
    if (rCounter % 256 == 0)//generate more random seeds
    {
      //    std::cout << "COUNTR : " << rCounter << std::endl;

      for (int i = 0; i < 256; i++) {
        tempSecComp[i] = _mm_set1_epi32(rCounter + i);
      }

      AES_ecb_encrypt_chunk_in_out(tempSecComp, pseudoRandomString, 256);
    }
    return pseudoRandomString[rCounter & 0xff];
  }

  __m128i newRandomNumber(const unsigned long &plaintext) {

    tempSecComp[0] = _mm_set1_epi32(plaintext);
    AES_ecb_encrypt_chunk_in_out(tempSecComp, pseudoRandomString);

    return pseudoRandomString[0];
  }

  //Randomness functions
  mpc_t get128Bits() {
    mpc_t ret;
    ret = (mpc_t) (newRandomNumber());
    return ret;
  }

  signed_mpc_t PRF_AES_CTR(const unsigned long &key) {
    signed_mpc_t ret;
    ret = (signed_mpc_t) newRandomNumber(key);
    ret >>= 64;
    return ret;
  }

  small_mpc_t get8Bits() {
    small_mpc_t ret;

    if (random8BitCounter == 0) {
      random8BitNumber = newRandomNumber();
    }

    uint8_t *temp = (uint8_t *) &random8BitNumber;
    ret = (small_mpc_t) temp[random8BitCounter];

    random8BitCounter++;
    if (random8BitCounter == 16) {
      random8BitCounter = 0;
    }

    return ret;

  }

  small_mpc_t getBit() {
    small_mpc_t ret;
    __m128i temp;

    if (randomBitCounter == 0) {
      randomBitNumber = newRandomNumber();
    }

    int x = randomBitCounter % 8;
    switch (x) {
      case 0 :temp = _mm_and_si128(randomBitNumber, BIT1);
        break;
      case 1 :temp = _mm_and_si128(randomBitNumber, BIT2);
        break;
      case 2 :temp = _mm_and_si128(randomBitNumber, BIT4);
        break;
      case 3 :temp = _mm_and_si128(randomBitNumber, BIT8);
        break;
      case 4 :temp = _mm_and_si128(randomBitNumber, BIT16);
        break;
      case 5 :temp = _mm_and_si128(randomBitNumber, BIT32);
        break;
      case 6 :temp = _mm_and_si128(randomBitNumber, BIT64);
        break;
      case 7 :temp = _mm_and_si128(randomBitNumber, BIT128);
        break;
    }
    uint8_t *val = (uint8_t *) &temp;
    ret = (val[0] >> x);

    randomBitCounter++;
    if (randomBitCounter % 8 == 0) {
      randomBitNumber = _mm_srli_si128(randomBitNumber, 1);
    }

    if (randomBitCounter == 128) {
      randomBitCounter = 0;
    }

    return ret;

  }

  //Other randomness functions
  small_mpc_t randModPrime() {
    small_mpc_t ret;

    do {
      ret = get8Bits();
    } while (ret >= bnd);

    return (ret % sField);

  }

  small_mpc_t randNonZeroModPrime() {
    small_mpc_t ret;
    do {
      ret = randModPrime();

    } while (ret == 0);

    return ret;

  }

  mpc_t randModuloOdd() {
    mpc_t ret;
    do {
      ret = get128Bits();
    } while (ret == MINUS_ONE);
    return ret;

  }

  small_mpc_t AES_random(int i) {
    small_mpc_t ret;
    do {
      ret = get8Bits();
    } while (ret >= ((256 / i) * i));

    return (ret % i);
  }

  void AES_random_shuffle(std::vector<small_mpc_t> &vec, size_t begin_offset, size_t end_offset) {
    auto it = vec.begin();
    auto first = it + begin_offset;
    auto last = it + end_offset;
    auto n = last - first;

    for (auto i = n - 1; i > 0; --i) {
      using std::swap;
      swap(first[i], first[AES_random(i + 1)]);
    }
  }

};

}

#endif //XMPC_PRF_H
