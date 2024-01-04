//
// Created by Xinyu Feng on 2024/1/4.
//

#include "scheduler/graph.h"

std::atomic<int> nodeIdCounter(1);

GraphNode *BuildGraph(const std::vector<std::string> &postfix) {
  std::stack<GraphNode *> stack;
  for (const auto &i : postfix) {
    auto node = new GraphNode();
    node->value = i;
    node->id = nodeIdCounter.fetch_add(1);

    if (IsUnaryOperator(i)) {
      GraphNode *right_operand = nullptr;
      if (!stack.empty()) {
        right_operand = stack.top();
        stack.pop();
      } else { throw std::runtime_error("Invalid expression"); }
      node->edges.push_back(right_operand);
    } else if (IsOperator(i)) {
      GraphNode *right_operand = nullptr;
      GraphNode *left_operand = nullptr;

      if (!stack.empty()) {
        right_operand = stack.top();
        stack.pop();
      } else { throw std::runtime_error("Invalid expression"); }
      if (!stack.empty()) {
        left_operand = stack.top();
        stack.pop();
      } else { throw std::runtime_error("Invalid expression"); }

      node->edges.push_back(left_operand);
      node->edges.push_back(right_operand);
    } else if (isalnum(i[0]) || i[0] == '.') {
    } else { throw std::runtime_error("Invalid expression"); }

    stack.push(node);
  }

  if (stack.size() != 1) { throw std::runtime_error("Invalid expression"); }

  return stack.top();
}

int GetNodePriority(const GraphNode *node, std::unordered_map<int, int> &priority_cache) {
  if (node->edges.empty()) { return 0; }

  if (priority_cache.find(node->id) != priority_cache.end()) { return priority_cache[node->id]; }

  int max_priority = 0;
  for (const auto &edge : node->edges) {
    int edge_priority = GetNodePriority(edge, priority_cache);
    max_priority = std::max(max_priority, edge_priority);
  }

  int priority = max_priority + 1;
  priority_cache[node->id] = priority;
  return priority;
}

void Visit(GraphNode *node, std::vector<GraphNode *> &sorted, std::map<GraphNode *, bool> &visited) {
  if (visited[node]) { return; }
  visited[node] = true;
  for (auto neighbor : node->edges) { Visit(neighbor, sorted, visited); }
  sorted.push_back(node);
}

std::vector<GraphNode *> TopologicalSort(GraphNode *root) {
  std::vector<GraphNode *> sorted_nodes;
  std::unordered_map<GraphNode *, int> in_degree;
  std::queue<GraphNode *> zero_in_degree_nodes;

  std::function<void(GraphNode *)> visit = [&](GraphNode *node) {
    if (in_degree.find(node) != in_degree.end()) { return; }

    in_degree[node] = 0;
    for (auto &edge : node->edges) {
      visit(edge);
      in_degree[edge]++;
    }
  };

  visit(root);
  for (const auto &entry : in_degree) {
    if (entry.second == 0) { zero_in_degree_nodes.push(entry.first); }
  }

  while (!zero_in_degree_nodes.empty()) {
    GraphNode *current = zero_in_degree_nodes.front();
    zero_in_degree_nodes.pop();
    sorted_nodes.push_back(current);
    for (auto &edge : current->edges) {
      in_degree[edge]--;
      if (in_degree[edge] == 0) { zero_in_degree_nodes.push(edge); }
    }
  }

  std::unordered_map<int, int> priority_cache;
  for (auto node : sorted_nodes) { node->priority = GetNodePriority(node, priority_cache); }

  return sorted_nodes;
}
