//
// Created by Xinyu Feng on 2024/1/4.
//

#include <thread>
#include <iostream>

#include "data.h"
#include "config.h"
#include "scheduler/scheduler.h"

void A2B(xmpc::Party &a, xmpc::Party &b) {

}

int main() {
  xmpc::PRECISION = 20;

  xmpc::Party p0(0);
  xmpc::Party p1(1);
  xmpc::Party p2(2);

  auto st = std::chrono::high_resolution_clock::now();
  std::string expression = "(a+b)*(c+d)";

  expression.erase(std::remove_if(expression.begin(), expression.end(), ::isspace), expression.end());
  std::cout << expression << std::endl;
  std::cout << " ----------------------------------------------- " << std::endl;

  std::vector<std::string> postfix = InfixToPostfix(expression);

  GraphNode *root = BuildGraph(postfix);
  root->is_root = true;

  std::vector<GraphNode *> sortedOps = TopologicalSort(root);
  std::sort(sortedOps.begin(), sortedOps.end(), xmpc::scheduler::cmp);
  sortedOps.back()->is_root = true;

  for (auto &node : sortedOps) {
    std::cout << "<" << node->id << ", " << node->value;
    for (auto &edge_node : node->edges) { std::cout << ", " << edge_node->id; }
    std::cout << ", Priority: " << node->priority << ">" << std::endl;
  }

  auto ed = std::chrono::high_resolution_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::microseconds>(ed - st).count() << " (us)\n";

  std::map<std::string, double> variableValues = {
      {"a", 1.0},
      {"b", 3.0},
      {"c", 2.0},
      {"d", 1.375},
      {"e", 2.0},
      {"f", 3.0},
  };

  std::queue<GraphNode *> node_queue;
  for (auto i : sortedOps) {
    if (i->priority) {
      NodeState ns = {FINISH};
      state_map.insert({i->id, ns});
    }
    node_queue.push(i);
  }

  ThreadPool threadPool(4);

  for (GraphNode *opNode : sortedOps) {
    auto res =
        threadPool.enqueue([&](GraphNode *operationNode) {
                             return xmpc::scheduler::executeOperation(operationNode, variableValues,
                                                                      p0, p1, p2);
                           },
                           opNode).get();
  }

  double finalResult = xmpc::scheduler::executeInParallel(sortedOps, threadPool, variableValues, p0, p1, p2);

  std::cout << "Result: " << finalResult << std::endl;

  return 0;
}

