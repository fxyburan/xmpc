//
// Created by Xinyu Feng on 2024/1/4.
//

#ifndef XMPC_PRNG_H_
#define XMPC_PRNG_H_

#include <fstream>
#include <memory>
#include <openssl/rand.h>
#include "type.h"

static std::string seed;

namespace xmpc {
class AESObject {
 public:
  AESObject() {
    seed = "1234567890";
  }

  explicit AESObject(const signed_mpc_t &key) {
    seed = (char *) &key;
  }

  explicit AESObject(char *filename, unsigned int pid = -1) {
    std::ifstream seed_file(filename);
    seed = std::string{std::istreambuf_iterator<char>(seed_file), std::istreambuf_iterator<char>()};
    seed_file.close();
  }

  static mpc_t GetRandomBits() {
    unsigned char s_buf_out[20] = {0};
    RAND_seed(seed.c_str(), (int) seed.length());
    RAND_bytes(s_buf_out, 16);

    mpc_t res = 0;
    for (int i = 0; i < 16; ++i) {
      res = (res << 8) + s_buf_out[i];
    }
    return res;
  }

  static mpc_t PRF(const std::string &key) {
    unsigned char s_buf_out[20] = {0};
    RAND_seed(key.c_str(), (int) key.length());
    RAND_bytes(s_buf_out, 16);

    mpc_t res = 0;
    for (int i = 0; i < 16; ++i) {
      res = (res << 8) + s_buf_out[i];
    }
    return res;
  }

  static small_mpc_t Get8Bits() {
    RAND_seed(seed.c_str(), (int) seed.length());

    unsigned char s_buf_out[1];
    RAND_bytes(s_buf_out, 1);

    return (small_mpc_t) s_buf_out[0];
  }
};

}

#endif //XMPC_PRNG_H_
