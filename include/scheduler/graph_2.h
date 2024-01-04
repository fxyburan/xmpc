//
// Created by Xinyu Feng on 2024/1/4.
//

#ifndef XMPC_SCHEDULER_GRAPH_H
#define XMPC_SCHEDULER_GRAPH_H

#include <stack>
#include <unordered_map>
#include <map>
#include <atomic>

#include "utils.h"

struct GraphNode {
  int id{};
  int priority{};
  std::string value;
  std::vector<GraphNode *> edges;
};

enum State {
  READY, FINISH, EXECUTING
};

struct NodeState {
  State state;
};

GraphNode *BuildGraph(const std::vector<std::string> &postfix);
std::vector<GraphNode *> TopologicalSort(GraphNode *root);

#endif //XMPC_SCHEDULER_GRAPH_H
