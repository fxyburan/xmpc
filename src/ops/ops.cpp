//
// Created by Xinyu Feng on 2024/1/4.
//

#include "ops.h"

void xmpc::ops::share(uint64_t tid, MPCData &mpc_data, Party &p0, Party &p1, Party &p2) {
  auto s_0 = mpc_data.GetShare(0);
  auto s_1 = mpc_data.GetShare(1);
  auto s_2 = mpc_data.GetShare(2);

  p0.InsertShares(tid, s_0, s_1);
  p1.InsertShares(tid, s_1, s_2);
  p2.InsertShares(tid, s_2, s_0);
}

void xmpc::ops::zero_sharing(Party &p0, Party &p1, Party &p2) {
  xmpc::prf::AESObject aes_object;
  auto p0_s0 = aes_object.PRF_AES_CTR(0);
  auto p0_s1 = aes_object.PRF_AES_CTR(1);
  p0.SetAlpha(p0_s0 - p0_s1);

  auto p1_s0 = aes_object.PRF_AES_CTR(1);
  auto p1_s1 = aes_object.PRF_AES_CTR(2);
  p1.SetAlpha(p1_s0 - p1_s1);

  auto p2_s0 = aes_object.PRF_AES_CTR(2);
  auto p2_s1 = aes_object.PRF_AES_CTR(0);
  p2.SetAlpha(p2_s0 - p2_s1);
}

double xmpc::ops::reconstruct(uint64_t tid, Party a, Party b, uint8_t target) {
  mpc_t x, y, z;
  if (b.GetPid() - a.GetPid() == 1 || a.GetPid() - b.GetPid() == 2) {
    x = a.GetShares(tid).first;
    y = a.GetShares(tid).second;
    z = b.GetShares(tid).second;
  } else if (b.GetPid() - a.GetPid() == -1 || a.GetPid() - b.GetPid() == -2) {
    x = b.GetShares(tid).first;
    y = b.GetShares(tid).second;
    z = a.GetShares(tid).second;
  } else {
    printf("error\n");
    return 0.0;
  }

  return FixedToFloat(x + y + z, PRECISION);
}

void xmpc::ops::add(uint64_t id_x, uint64_t id_y, uint64_t id_result, Party &p0, Party &p1, Party &p2) {
  auto x_0 = p0.GetShares(id_x);
  auto y_0 = p0.GetShares(id_y);
  p0.InsertShares(id_result, x_0.first + y_0.first, x_0.second + y_0.second);

  auto x_1 = p1.GetShares(id_x);
  auto y_1 = p1.GetShares(id_y);
  p1.InsertShares(id_result, x_1.first + y_1.first, x_1.second + y_1.second);

  auto x_2 = p2.GetShares(id_x);
  auto y_2 = p2.GetShares(id_y);
  p2.InsertShares(id_result, x_2.first + y_2.first, x_2.second + y_2.second);
}

void xmpc::ops::mul(uint64_t id_x, uint64_t id_y, uint64_t id_result, Party &p0, Party &p1, Party &p2) {
  zero_sharing(p0, p1, p2);
  auto x_0 = p0.GetShares(id_x);
  auto y_0 = p0.GetShares(id_y);
  auto x_1 = p1.GetShares(id_x);
  auto y_1 = p1.GetShares(id_y);
  auto x_2 = p2.GetShares(id_x);
  auto y_2 = p2.GetShares(id_y);

  auto z_0 =
      (x_0.first * y_0.first + x_0.first * y_0.second + x_0.second * y_0.first + p0.GetAlpha()) / (1L << PRECISION);
  auto z_1 =
      (x_1.first * y_1.first + x_1.first * y_1.second + x_1.second * y_1.first + p1.GetAlpha()) / (1L << PRECISION);
  auto z_2 =
      (x_2.first * y_2.first + x_2.first * y_2.second + x_2.second * y_2.first + p2.GetAlpha()) / (1L << PRECISION);

  p0.InsertShares(id_result, z_0, z_1);
  p1.InsertShares(id_result, z_1, z_2);
  p2.InsertShares(id_result, z_2, z_0);
}
