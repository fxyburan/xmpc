//
// Created by Xinyu Feng on 2024/1/4.
//

#ifndef XMPC_SCHEDULER_UTILS_H
#define XMPC_SCHEDULER_UTILS_H

#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <condition_variable>
#include <future>
#include <functional>


inline bool IsOperator(const std::string &token) {
  return (token == "+" || token == "-" || token == "*" || token == "/"
      || token == "log" || token == "exp" || token == "sqrt");
}

inline bool IsUnaryOperator(const std::string &token) {
  return (token == "log" || token == "exp" || token == "sqrt");
}


inline int precedence(const std::string &op) {
  if (op == "+" || op == "-") { return 2; }
  else if (op == "log" || op == "exp" || op == "sqrt") { return 1; }
  else if (op == "*" || op == "/") { return 3; }
  else if (op == "(" || op == ")") { return -1; }
  return 0;
}

#endif //XMPC_SCHEDULER_UTILS_H

