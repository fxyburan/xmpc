//
// Created by Xinyu Feng on 2024/1/4.
//

#include <chrono>
#include <algorithm>
#include <iostream>

#include "scheduler/scheduler_2.h"
#include "scheduler/parser.h"
#include "scheduler/graph.h"

int main() {
  auto st = std::chrono::high_resolution_clock::now();
  std::string expression = "a * log(b + d * (c - d)) + sqrt(e)";

  expression.erase(std::remove_if(expression.begin(), expression.end(), ::isspace), expression.end());
  std::cout << expression << std::endl;
  std::cout << " ----------------------------------------------- " << std::endl;

  std::vector<std::string> postfix = InfixToPostfix(expression);

  GraphNode *root = BuildGraph(postfix);

  std::vector<GraphNode *> sortedOps = TopologicalSort(root);
  std::sort(sortedOps.begin(), sortedOps.end(), xmpc::scheduler::cmp);

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
      {"d", 1.0},
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
                             return xmpc::scheduler::executeOperation(operationNode,
                                                                      variableValues);
                           },
                           opNode).get();
    results.insert({opNode->id, res});
    std::cout << "res[" << opNode->id << "] = " << res << std::endl;
  }
  double finalResult = xmpc::scheduler::executeInParallel(sortedOps, threadPool, variableValues);

  std::cout << "Result: " << finalResult << std::endl;

  return 0;
}