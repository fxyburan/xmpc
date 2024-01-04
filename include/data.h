//
// Created by Xinyu Feng on 2024/1/4.
//

#ifndef XMPC_DATA_H_
#define XMPC_DATA_H_

#include <cstdint>
#include <unordered_map>
#include <iostream>
#include <cassert>

#include "prng.h"
#include "config.h"

namespace xmpc {
template<typename T>
mpc_t FloatToFixed(T a, ui64 precision) {
  return ((mpc_t) ((((signed_mpc_t) (a)) << precision)
      + (signed_mpc_t) (((a) - (signed_mpc_t) (a)) * (1L << precision))));
}

template<typename T>
double FixedToFloat(T a, int precision) {
  return ((double((signed_mpc_t) (a))) / (1L << precision));
}

template<typename T>
std::string I128ToString(T x) {
  if (!x) { return "0"; }
  std::string tmp;
  int len = 0;
  if (x < 0) {
    x = -x;
    tmp.append("-");
  }

  while (x) {
    tmp.append(std::to_string((i16) (x % 10)));
    x /= 10;
    ++len;
  }

  std::string res;
  if (tmp[0] == '-') {
    res.append("-");
    for (int i = len; i >= 1; --i) { res += tmp[i]; }
  } else { for (int i = len - 1; i >= 0; --i) { res += tmp[i]; }}
  return res;
}

template<typename T>
T StringToI128(const std::string &str) {
  __int128 x = 0, f = 1;
  for (auto ch : str) {
    if (ch < '0' || ch > '9') {
      if (ch == '-') { f = -1; }
    } else if (ch >= '0' && ch <= '9') { x = x * 10 + ch - '0'; }
  }
  return x * f;
}

class MPCData {
 public:
  explicit MPCData(double val) {
    shares_[0] = xmpc::AESObject::GetRandomBits();
    shares_[1] = xmpc::AESObject::GetRandomBits();
    shares_[2] = xmpc::FloatToFixed(val, PRECISION) - shares_[0] - shares_[1];
  }

  mpc_t GetShare(int idx) {
    assert(idx < 3 && idx >= 0);
    return shares_[idx];
  }
 private:
  mpc_t shares_[3]{};
};

class Party {
 public:
  explicit Party(uint8_t pid) : pid_(pid) {}
  uint32_t GetPid() const {
    return pid_;
  }

  void PrintShares(uint64_t tid) {
    std::cout << "[" << I128ToString(shares_.at(tid).first) << ", " << I128ToString(shares_.at(tid).second) << "]\n";
  }

  void InsertShares(uint64_t tid, mpc_t s1, mpc_t s2) {
    shares_.insert(std::make_pair(tid, std::make_pair(s1, s2)));
  }

  std::pair<mpc_t, mpc_t> GetShares(uint64_t tid) {
    return shares_.at(tid);
  }

  mpc_t GetAlpha() const {
    return alpha_;
  }
  void SetAlpha(mpc_t alpha) {
    Party::alpha_ = alpha;
  }

 private:
  uint8_t pid_;
  mpc_t alpha_{};
  std::unordered_map<uint64_t, std::pair<mpc_t, mpc_t>> shares_;
};
}

#endif //XMPC_DATA_H_
