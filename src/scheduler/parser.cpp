//
// Created by Xinyu Feng on 2024/1/4.
//

#include "scheduler/parser.h"

std::vector<std::string> InfixToPostfix(const std::string &expression) {
  std::vector<std::string> postfix;
  std::stack<std::string> op_stack;
  std::string current_token;

  for (const char &c : expression) {
    if (isalnum(c) || c == '.') { current_token += c; }
    else {
      if (!current_token.empty() && !IsOperator(current_token)) {
        postfix.push_back(current_token);
        current_token.clear();
      }

      if (c == '(') {
        op_stack.emplace(1, c);
        if (!current_token.empty() && IsUnaryOperator(current_token)) {
          op_stack.push(current_token);
          current_token.clear();
        }

      } else if (c == ')') {
        while (!op_stack.empty() && op_stack.top() != "(") {
          postfix.push_back(op_stack.top());
          op_stack.pop();
        }
        if (!op_stack.empty() && op_stack.top() == "(") { op_stack.pop(); }
      } else {
        std::string op(1, c);
        if (IsOperator(op)) {
          while (!op_stack.empty() && precedence(op) <= precedence(op_stack.top())) {
            postfix.push_back(op_stack.top());
            op_stack.pop();
          }
          op_stack.push(op);
        }
      }
    }
  }

  if (!current_token.empty()) { postfix.push_back(current_token); }
  while (!op_stack.empty()) {
    postfix.push_back(op_stack.top());
    op_stack.pop();
  }

  return postfix;
}
