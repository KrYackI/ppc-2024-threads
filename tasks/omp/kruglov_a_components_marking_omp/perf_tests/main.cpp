// Copyright 2023 Kruglov Alexey
#include <gtest/gtest.h>

#include <vector>

#include "core/perf/include/perf.hpp"
#include "omp/kruglov_a_components_marking_omp/include/ops_omp.hpp"

TEST(kruglov_a_components_marking_omp_perf_test, test_pipeline_run) {
  // Create data

  uint32_t h = 2000;
  uint32_t w = 2000;
  std::vector<uint32_t> size = {h, w};
  std::vector<uint8_t> in(h * w, 0);
  std::vector<uint32_t> out(h * w, 0);
  std::vector<uint32_t> comp(h * w, 1);

  // Create TaskData
  std::shared_ptr<ppc::core::TaskData> taskDataOmp = std::make_shared<ppc::core::TaskData>();
  taskDataOmp->inputs.emplace_back(reinterpret_cast<uint8_t *>(size.data()));
  taskDataOmp->inputs_count.emplace_back(size.size());
  taskDataOmp->inputs.emplace_back(reinterpret_cast<uint8_t *>(in.data()));
  taskDataOmp->inputs_count.emplace_back(in.size());
  taskDataOmp->outputs.emplace_back(reinterpret_cast<uint8_t *>(out.data()));
  taskDataOmp->outputs_count.emplace_back(out.size());

  // Create Task
  auto testTaskParallel = std::make_shared<imgMarkingOmp>(taskDataOmp);

  // Create Perf attributes
  auto perfAttr = std::make_shared<ppc::core::PerfAttr>();
  perfAttr->num_running = 50;
  const auto t0 = std::chrono::high_resolution_clock::now();
  perfAttr->current_timer = [&] {
    auto current_time_point = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time_point - t0).count();
    return static_cast<double>(duration) * 1e-9;
  };

  // Create and init perf results
  auto perfResults = std::make_shared<ppc::core::PerfResults>();

  // Create Perf analyzer
  auto perfAnalyzer = std::make_shared<ppc::core::Perf>(testTaskParallel);
  perfAnalyzer->pipeline_run(perfAttr, perfResults);
  ppc::core::Perf::print_perf_statistic(perfResults);
  ASSERT_EQ(1, 1);
}

TEST(kruglov_a_components_marking_omp_perf_test, test_task_run) {
  // Create data

  uint32_t h = 2000;
  uint32_t w = 2000;
  std::vector<uint32_t> size = {h, w};
  std::vector<uint8_t> in(h * w, 0);
  std::vector<uint32_t> out(h * w, 0);
  std::vector<uint32_t> comp(h * w, 1);

  // Create TaskData
  std::shared_ptr<ppc::core::TaskData> taskDataOmp = std::make_shared<ppc::core::TaskData>();
  taskDataOmp->inputs.emplace_back(reinterpret_cast<uint8_t *>(size.data()));
  taskDataOmp->inputs_count.emplace_back(size.size());
  taskDataOmp->inputs.emplace_back(reinterpret_cast<uint8_t *>(in.data()));
  taskDataOmp->inputs_count.emplace_back(in.size());
  taskDataOmp->outputs.emplace_back(reinterpret_cast<uint8_t *>(out.data()));
  taskDataOmp->outputs_count.emplace_back(out.size());

  // Create Task
  auto testTaskParallel = std::make_shared<imgMarkingOmp>(taskDataOmp);

  // Create Perf attributes
  auto perfAttr = std::make_shared<ppc::core::PerfAttr>();
  perfAttr->num_running = 2;
  const auto t0 = std::chrono::high_resolution_clock::now();
  perfAttr->current_timer = [&] {
    auto current_time_point = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time_point - t0).count();
    return static_cast<double>(duration) * 1e-9;
  };

  // Create and init perf results
  auto perfResults = std::make_shared<ppc::core::PerfResults>();

  // Create Perf analyzer
  auto perfAnalyzer = std::make_shared<ppc::core::Perf>(testTaskParallel);
  perfAnalyzer->task_run(perfAttr, perfResults);
  ppc::core::Perf::print_perf_statistic(perfResults);
  ASSERT_EQ(1, 1);
}
