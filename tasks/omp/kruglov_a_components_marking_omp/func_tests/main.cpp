// Copyright 2023 Kruglov Alexey
#include <gtest/gtest.h>

#include <vector>
#include <iostream>

#include "omp/kruglov_a_components_marking_omp/include/ops_omp.hpp"

TEST(kruglov_a_components_marking_omp_functional, test_functional) {
  // Create data

  uint32_t h = 10;
  uint32_t w = 10;
  std::vector<uint32_t> size = {h, w};
  std::vector<uint8_t> in = {0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1,
                             1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1,
                             1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
                             1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1};
  std::vector<uint32_t> out(h * w, 0);
  std::vector<uint32_t> comp = {1, 1, 0, 0, 2, 2, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 1, 1, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 1, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 4, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4,
                                0, 0, 0, 5, 5, 0, 0, 0, 0, 4, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 0, 0};

  // Create TaskData
  std::shared_ptr<ppc::core::TaskData> taskDataOmp = std::make_shared<ppc::core::TaskData>();
  taskDataOmp->inputs.emplace_back(reinterpret_cast<uint8_t *>(size.data()));
  taskDataOmp->inputs_count.emplace_back(size.size());
  taskDataOmp->inputs.emplace_back(reinterpret_cast<uint8_t *>(in.data()));
  taskDataOmp->inputs_count.emplace_back(in.size());
  taskDataOmp->outputs.emplace_back(reinterpret_cast<uint8_t *>(out.data()));
  taskDataOmp->outputs_count.emplace_back(out.size());

  // Create Task
  imgMarkingOmp testTaskParallel(taskDataOmp);
  ASSERT_EQ(testTaskParallel.validation(), true);
  testTaskParallel.pre_processing();
  testTaskParallel.run();
  testTaskParallel.post_processing();

      for (uint32_t i = 0; i < h; ++i) {
        for (uint32_t j = 0; j < w; ++j)
            std::cout << (out[i * w + j]) << ", ";
        std::cout << '\n';
    }

  ASSERT_EQ(out, comp);
}

TEST(kruglov_a_components_marking_omp_functional, test_non_square) {
  // Create data

  uint32_t h = 5;
  uint32_t w = 10;
  std::vector<uint32_t> size = {h, w};
  std::vector<uint8_t> in = {0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1,
                             1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1};
  std::vector<uint32_t> out(h * w, 0);
  std::vector<uint32_t> comp = {1, 1, 0, 0, 2, 2, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 1, 1, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 1, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 0};

  // Create TaskData
  std::shared_ptr<ppc::core::TaskData> taskDataOmp = std::make_shared<ppc::core::TaskData>();
  taskDataOmp->inputs.emplace_back(reinterpret_cast<uint8_t *>(size.data()));
  taskDataOmp->inputs_count.emplace_back(size.size());
  taskDataOmp->inputs.emplace_back(reinterpret_cast<uint8_t *>(in.data()));
  taskDataOmp->inputs_count.emplace_back(in.size());
  taskDataOmp->outputs.emplace_back(reinterpret_cast<uint8_t *>(out.data()));
  taskDataOmp->outputs_count.emplace_back(out.size());

  // Create Task
  imgMarkingOmp testTaskParallel(taskDataOmp);
  ASSERT_EQ(testTaskParallel.validation(), true);
  testTaskParallel.pre_processing();
  testTaskParallel.run();
  testTaskParallel.post_processing();
  ASSERT_EQ(out, comp);
}

TEST(kruglov_a_components_marking_omp_functional, test_all_ones) {
  // Create data
  uint32_t h = 10;
  uint32_t w = 10;
  std::vector<uint32_t> size = {h, w};
  std::vector<uint8_t> in(h * w, 1);
  std::vector<uint32_t> out(h * w, 0);
  std::vector<uint32_t> comp(h * w, 0);

  // Create TaskData
  std::shared_ptr<ppc::core::TaskData> taskDataOmp = std::make_shared<ppc::core::TaskData>();
  taskDataOmp->inputs.emplace_back(reinterpret_cast<uint8_t *>(size.data()));
  taskDataOmp->inputs_count.emplace_back(size.size());
  taskDataOmp->inputs.emplace_back(reinterpret_cast<uint8_t *>(in.data()));
  taskDataOmp->inputs_count.emplace_back(in.size());
  taskDataOmp->outputs.emplace_back(reinterpret_cast<uint8_t *>(out.data()));
  taskDataOmp->outputs_count.emplace_back(out.size());

  // Create Task
  imgMarkingOmp testTaskParallel(taskDataOmp);
  ASSERT_EQ(testTaskParallel.validation(), true);
  testTaskParallel.pre_processing();
  testTaskParallel.run();
  testTaskParallel.post_processing();
  ASSERT_EQ(out, comp);
}

TEST(kruglov_a_components_marking_omp_functional, test_all_zeros) {
  // Create data
  uint32_t h = 10;
  uint32_t w = 10;
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
  imgMarkingOmp testTaskParallel(taskDataOmp);
  ASSERT_EQ(testTaskParallel.validation(), true);
  testTaskParallel.pre_processing();
  testTaskParallel.run();
  testTaskParallel.post_processing();

      for (uint32_t i = 0; i < h; ++i) {
        for (uint32_t j = 0; j < w; ++j)
            std::cout << (out[i * w + j]) << ", ";
        std::cout << '\n';
    }

  ASSERT_EQ(out, comp);
}

TEST(kruglov_a_components_marking_omp_functional, test_grid) {
  // Create data
  uint32_t h = 10;
  uint32_t w = 10;
  std::vector<uint32_t> size = {h, w};
  std::vector<uint8_t> in(h * w);
  std::vector<uint32_t> comp(h * w);
  for (uint32_t i = 0; i < h * w; ++i) {
    in[i] = (i / 10 + i % 10) % 2;
    comp[i] = ((i / 10 + i % 10) % 2 == 0) ? i / 2 + 1 : 0;
  }
  std::vector<uint32_t> out(h * w, 0);

  // Create TaskData
  std::shared_ptr<ppc::core::TaskData> taskDataOmp = std::make_shared<ppc::core::TaskData>();
  taskDataOmp->inputs.emplace_back(reinterpret_cast<uint8_t *>(size.data()));
  taskDataOmp->inputs_count.emplace_back(size.size());
  taskDataOmp->inputs.emplace_back(reinterpret_cast<uint8_t *>(in.data()));
  taskDataOmp->inputs_count.emplace_back(in.size());
  taskDataOmp->outputs.emplace_back(reinterpret_cast<uint8_t *>(out.data()));
  taskDataOmp->outputs_count.emplace_back(out.size());

  // Create Task
  imgMarkingOmp testTaskParallel(taskDataOmp);
  ASSERT_EQ(testTaskParallel.validation(), true);
  testTaskParallel.pre_processing();
  testTaskParallel.run();
  testTaskParallel.post_processing();

      for (uint32_t i = 0; i < h; ++i) {
        for (uint32_t j = 0; j < w; ++j)
            std::cout << (out[i * w + j]) << ", ";
        std::cout << '\n';
    }

  ASSERT_EQ(out, comp);
}