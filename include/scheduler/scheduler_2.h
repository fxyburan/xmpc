//
// Created by Xinyu Feng on 2024/1/4.
//

#ifndef XMPC_SCHEDULER_SCHEDULER_H_
#define XMPC_SCHEDULER_SCHEDULER_H_

#include <cmath>

#include "scheduler/parser.h"
#include "scheduler/graph.h"

class ThreadPool {
 public:
  explicit ThreadPool(size_t numThreads);

  ~ThreadPool();

  template<class F, class... Args>
  auto enqueue(F &&f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

 private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;

  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop;
};

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
  for (size_t i = 0; i < numThreads; ++i) {
    workers.emplace_back([this] {
      while (true) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(queue_mutex);
          condition.wait(lock, [this] { return stop || !tasks.empty(); });
          if (stop && tasks.empty()) {
            return;
          }
          task = std::move(tasks.front());
          tasks.pop();
        }
        task();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop = true;
  }
  condition.notify_all();
  for (std::thread &worker : workers) { worker.join(); }
}

template<class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
  using return_type = typename std::result_of<F(Args...)>::type;

  auto task =
      std::make_shared<std::packaged_task<return_type()>>(
          std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    if (stop) { throw std::runtime_error("enqueue on stopped ThreadPool"); }
    tasks.emplace([task] { (*task)(); });
  }
  condition.notify_one();
  return res;
}

std::map<int, NodeState> state_map;
std::map<int, double> results;

namespace xmpc {
class scheduler {
 public:
  static double executeOperation(const GraphNode *node, const std::map<std::string, double> &variableValues) {
    std::string op = node->value;
    std::vector<double> operands;

    for (const auto &edge : node->edges) {
      if ((edge->value[0] >= '0' && edge->value[0] <= '9') || edge->value[0] == '.') {
        operands.push_back(std::stod(edge->value));
      } else if (variableValues.count(edge->value) > 0) {
        operands.push_back(variableValues.at(edge->value));
      } else {
        while (true) {
          if (results.find(edge->id) == results.end()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
          } else {
            operands.push_back(results[edge->id]);
            break;
          }
        }
      }
    }

    if (op == "+") {
      return operands[0] + operands[1];
    } else if (op == "-") {
      return operands[0] - operands[1];
    } else if (op == "*") { return operands[0] * operands[1]; }
    else if (op == "/") { return operands[0] / operands[1]; }
    else if (op == "log") { return std::log(operands[0]); }
    else if (op == "exp") { return std::exp(operands[0]); }
    else if (op == "sqrt") { return std::sqrt(operands[0]); }
    else {
      if (variableValues.find(op) != variableValues.end()) {
        return variableValues.at(op);
      } else {
        return std::stod(op);
      }
    }
  }

  static double executeInParallel(const std::vector<GraphNode *> &sortedOps, ThreadPool &threadPool,
                                  const std::map<std::string, double> &variableValues) {
    std::map<GraphNode *, std::shared_future<double>> futures;

    for (GraphNode *opNode : sortedOps) {
      std::vector<std::shared_future<double>> dependencies(0);
      for (const auto &edge : opNode->edges) { dependencies.push_back(futures[edge]); }

      futures[opNode] = threadPool.enqueue([opNode, dependencies, &variableValues]() {
        for (auto &dependency : dependencies) { dependency.get(); }
        return executeOperation(opNode, variableValues);
      }).share();
    }

    return futures[sortedOps.back()].get();
  }

  static bool cmp(GraphNode *a, GraphNode *b) {
    if (a->priority == b->priority) { return a->id < b->id; }
    return a->priority <= b->priority;
  }
};
}

#endif //XMPC_SCHEDULER_SCHEDULER_H_
