// Copyright 2023 Kruglov Alexey
#pragma once

#include <list>
#include <random>
#include <thread>
#include <utility>
#include <vector>

#include "core/task/include/task.hpp"
namespace KruglovOmpTask {

class imgMarkingOmp : public ppc::core::Task {
 public:
  explicit imgMarkingOmp(std::shared_ptr<ppc::core::TaskData> taskData_) : Task(std::move(taskData_)) {}
  bool pre_processing() override;
  bool validation() override;
  bool run() override;
  bool post_processing() override;

 private:
  std::vector<std::vector<uint8_t>> src = {};
  std::vector<std::vector<uint32_t>> dst = {};
  uint32_t h{}, w{};
  void imgMarking();
};
}  // namespace KruglovOmpTask
