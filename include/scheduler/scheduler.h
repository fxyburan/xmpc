//
// Created by Xinyu Feng on 2024/1/4.
//

#ifndef XMPC_SCHEDULER_SCHEDULER_H_
#define XMPC_SCHEDULER_SCHEDULER_H_

#include <cmath>

#include "scheduler/parser.h"
#include "scheduler/graph.h"
#include "data.h"
#include "ops.h"

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
  static double executeOperation(const GraphNode *node, const std::map<std::string, double> &variableValues,
                                 xmpc::Party &p0, xmpc::Party &p1, xmpc::Party &p2) {
    std::string op = node->value;

    if (op == "+") {
      xmpc::ops::add(node->edges.at(0)->id, node->edges.at(1)->id, node->id, p0, p1, p2);
      //return operands[0] + operands[1];
    } else if (op == "*") {
      xmpc::ops::mul(node->edges.at(0)->id, node->edges.at(1)->id, node->id, p0, p1, p2);
    } else { // Data Sharing
      MPCData mpc_data(variableValues.at(node->value));
      xmpc::ops::share(node->id, mpc_data, p0, p1, p2);
    }

    if (node->is_root) {
      return xmpc::ops::reconstruct(node->id, p0, p1, 0);
    }
  }

  static double executeInParallel(const std::vector<GraphNode *> &sortedOps, ThreadPool &threadPool,
                                  const std::map<std::string, double> &variableValues,
                                  Party &p0, Party &p1, Party &p2) {
    std::map<GraphNode *, std::shared_future<double>> futures;

    for (GraphNode *opNode : sortedOps) {
      std::vector<std::shared_future<double>> dependencies(0);
      for (const auto &edge : opNode->edges) { dependencies.push_back(futures[edge]); }

      futures[opNode] = threadPool.enqueue([opNode, dependencies, &variableValues, &p0, &p1, &p2]() {
        for (auto &dependency : dependencies) { dependency.get(); }
        return executeOperation(opNode, variableValues, p0, p1, p2);
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
