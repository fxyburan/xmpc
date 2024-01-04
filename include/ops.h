//
// Created by Xinyu Feng on 2024/1/4.
//

#ifndef XMPC_OPS_H_
#define XMPC_OPS_H_

#include "type.h"
#include "data.h"
#include "prf.h"
#include "config.h"

namespace xmpc {
class ops {
 public:
  static void share(uint64_t tid, MPCData &mpc_data, Party &p0, Party &p1, Party &p2);

  static void zero_sharing(Party &p0, Party &p1, Party &p2);

  static double reconstruct(uint64_t tid, Party a, Party b, uint8_t target);

  static void add(uint64_t id_x, uint64_t id_y, uint64_t id_result, Party &p0, Party &p1, Party &p2);

  static void mul(uint64_t id_x, uint64_t id_y, uint64_t id_result, Party &p0, Party &p1, Party &p2);
};
}

#endif //XMPC_OPS_H_
