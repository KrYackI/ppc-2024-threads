// Copyright 2023 Kruglov Alexey

#include "omp/kruglov_a_components_marking_omp/include/ops_omp.hpp"

#include <omp.h>

#include <iostream>
#include <map>

using namespace std;
using namespace std::chrono_literals;

bool imgMarkingOmp::pre_processing() {
  internal_order_test();
  // Init value for input and output
  h = reinterpret_cast<uint32_t *>(taskData->inputs[0])[0];
  w = reinterpret_cast<uint32_t *>(taskData->inputs[0])[1];
  src.resize(h);
  dst.resize(h);
  for (size_t i = 0; i < h; ++i) {
    for (size_t j = 0; j < w; ++j) src[i].push_back(reinterpret_cast<uint8_t *>(taskData->inputs[1])[i * w + j]);
    dst[i].resize(w, 0);
  }
  return true;
}

bool imgMarkingOmp::validation() {
  internal_order_test();
  // Check count elements of output
  h = reinterpret_cast<uint32_t *>(taskData->inputs[0])[0];
  w = reinterpret_cast<uint32_t *>(taskData->inputs[0])[1];
  return (h * w == taskData->inputs_count[1] && taskData->inputs_count[1] == taskData->outputs_count[0]);
}

bool imgMarkingOmp::run() {
  internal_order_test();
  imgMarking();
  return true;
}

bool imgMarkingOmp::post_processing() {
  internal_order_test();
  for (size_t i = 0; i < h; ++i)
    for (size_t j = 0; j < w; ++j) reinterpret_cast<uint32_t *>(taskData->outputs[0])[i * w + j] = dst[i][j];
  return true;
}

// void imgMarkingOmp::imgMarking() {
//    //omp_set_num_threads(4);
//   uint32_t curLabel = 0;
//   // std::vector<imgMarkingOmp::labelStore> labels;
//   std::list<uint32_t> labelVec; // critical data
//   std::vector<std::vector<uint32_t *>> ptrMap;
//   ptrMap.resize(h);

//   omp_lock_t *lock = new omp_lock_t[w];
//   for (size_t i = 0; i < w; ++i){
//     omp_init_lock(lock + i);
//   }

//   #pragma omp parallel
//   {
//   #pragma omp for
//   for (size_t i = 0; i < h; ++i) ptrMap[i].resize(w, nullptr);

//   #pragma omp single
//   for (size_t i = 0; i < w; ++i) {

//     if (src[0][i] == 0) {
//       if (i == 0 || ptrMap[0][i - 1] == nullptr) {

//         labelVec.push_back(++curLabel);
//         ptrMap[0][i] = &labelVec.back();
//       } else
//         ptrMap[0][i] = ptrMap[0][i - 1];
//     }
//     if (src[h - 1][i] == 0) {
//       if (i == 0 || ptrMap[h - 1][i - 1] == nullptr) {

//         labelVec.push_back(++curLabel);
//         ptrMap[h - 1][i] = &labelVec.back();
//       } else
//         ptrMap[h - 1][i] = ptrMap[h - 1][i - 1];
//     }

//   }

//   //#pragma omp parallel num_threads(4)

//   #pragma omp for  nowait
//   for (size_t i = 1; i < h - 1; ++i) {
//       //#pragma omp critical
//     if (src[i][0] == 0) {
//       if (ptrMap[i - 1][0] == nullptr) {
//   #pragma omp critical
//         labelVec.push_back(++curLabel);
//         ptrMap[i][0] = &labelVec.back();
//       } else
//         ptrMap[i][0] = ptrMap[i - 1][0];
//     }
//     {
//     //    #pragma omp critical
//     // std::cout << i << " -> " << omp_get_thread_num() << '\n';
//     }
//     for (size_t j = 1; j < w; ++j) {
//        //#pragma omp critical
//       if (src[i][j] == 0) {

//       //   if (ptrMap[i - 1][j] == ptrMap[i][j - 1]) {
//       //     if (ptrMap[i - 1][j] == nullptr) {
//       //       labelVec.push_back(++curLabel);
//       //       ptrMap[i][j] = &labelVec.back();
//       //     } else
//       //       ptrMap[i][j] = ptrMap[i][j - 1];
//       //   } else {
//       //     if (ptrMap[i - 1][j] == nullptr)
//       //       ptrMap[i][j] = ptrMap[i][j - 1];
//       //     else if (ptrMap[i][j - 1] == nullptr)
//       //       ptrMap[i][j] = ptrMap[i - 1][j];
//       //     else { // critical
//       //       *(ptrMap[i][j - 1]) = *(ptrMap[i - 1][j]);
//       //       ptrMap[i][j] = ptrMap[i - 1][j];
//       //     }
//       //   }
//       // }
//        //#pragma omp critical
//        omp_set_lock(lock + j);
//           if (ptrMap[i - 1][j] != nullptr) {
//               ptrMap[i][j] = ptrMap[i - 1][j];
//           }
//           if (ptrMap[i + 1][j] != nullptr) {
//               if (ptrMap[i][j] != nullptr && *ptrMap[i][j] != *ptrMap[i + 1][j]) {
//     //#pragma omp critical
//                       *ptrMap[i + 1][j] = *ptrMap[i][j];
//               }
//               ptrMap[i][j] = ptrMap[i + 1][j];
//           }
//           if (ptrMap[i][j - 1] != nullptr) {
//               if (ptrMap[i][j] != nullptr && *ptrMap[i][j] != *ptrMap[i][j - 1]) {
//     //#pragma omp critical
//                       *ptrMap[i][j - 1] = *ptrMap[i][j];
//               }
//               ptrMap[i][j] = ptrMap[i][j - 1];
//           }
//           if (ptrMap[i][j] == nullptr){
//     #pragma omp critical
//                   labelVec.push_back(++curLabel);
//                   ptrMap[i][j] = &labelVec.back();
//           }
//           omp_unset_lock(lock + j);
//       }
//     }
//   }

//   // #pragma omp master
//   // for (size_t i = 0; i < w; ++i) {
//   //   if (src[h - 1][i] == 0) {
//   //     omp_set_lock(lock + i);
//   //         if (ptrMap[h - 2][i] != nullptr) {
//   //             ptrMap[h - 1][i] = ptrMap[h - 2][i];
//   //         }
//   //         if (i != 0 && ptrMap[h - 1][i - 1] != nullptr) {
//   //             if (ptrMap[h - 1][i] != nullptr && *ptrMap[h - 1][i] != *ptrMap[h - 1][i - 1]) {
//   //   //#pragma omp critical
//   //                     *ptrMap[h - 1][i - 1] = *ptrMap[h - 1][i];
//   //             }
//   //             ptrMap[h-1][i] = ptrMap[h - 1][i - 1];
//   //         }
//   //         if (ptrMap[h - 1][i] == nullptr){
//   //   #pragma omp critical
//   //                 labelVec.push_back(++curLabel);
//   //                 ptrMap[h - 1][i] = &labelVec.back();
//   //         }
//   //         omp_unset_lock(lock + i);
//   // }
//   // }
//   }
//   for (size_t i = 0; i < w; ++i)
//     omp_destroy_lock(lock + i);

//   size_t count = 0;
//   size_t cur = 0;
//   labelVec.sort();
//   for (auto &label : labelVec) {
//     if (cur != label) {
//       cur = label;
//       count++;
//     }
//     label = count;
//   }

//   for (size_t i = 0; i < h; ++i)
//     for (size_t j = 0; j < w; ++j)
//       if (ptrMap[i][j] != nullptr) dst[i][j] = *(ptrMap[i][j]);
// }

// void imgMarkingOmp::imgMarking() {
//   // omp_set_num_threads(4);
//   uint32_t curLabel = 0;
//   // std::vector<imgMarkingOmp::labelStore> labels;
//   std::list<uint32_t> labelVec;  // critical data
//   std::list<uint32_t *> groupVec;
//   std::vector<std::vector<uint32_t **>> ptrMap;
//   ptrMap.resize(h);

//   //   omp_lock_t *lock = new omp_lock_t[w];
//   //   for (size_t i = 0; i < w; ++i){
//   //     omp_init_lock(lock + i);
//   //   }

//   omp_lock_t *lock = new omp_lock_t;
//   omp_init_lock(lock);

//   uint32_t num_threads = omp_get_thread_limit();
//   if (num_threads > h) num_threads = 1;

// #pragma omp parallel num_threads(num_threads)
//   {
// #pragma omp for
//     for (size_t i = 0; i < h; ++i) ptrMap[i].resize(w, nullptr);

//     int32_t d = h / omp_get_num_threads();
//     // if (h % omp_get_num_threads()) d++;
//     size_t h0;
//     size_t h1;
//     if (omp_get_thread_num()) {
//       h0 = d * (omp_get_thread_num() - 1);
//       h1 = d * (omp_get_thread_num());
//     } else {
//       h0 = d * (omp_get_num_threads() - 1);
//       h1 = h;
//     }

//     for (size_t i = 0; i < w; ++i) {
//       if (src[h0][i] == 0) {
//         if (i == 0 || ptrMap[h0][i - 1] == nullptr) {
//           omp_set_lock(lock);
//           labelVec.push_back(++curLabel);
//           groupVec.push_back(&labelVec.back());
//           ptrMap[h0][i] = &groupVec.back();
//           omp_unset_lock(lock);
//         } else
//           ptrMap[h0][i] = ptrMap[h0][i - 1];
//       }
//       // if (src[h1 - 1][i] == 0) {
//       //   if (i == 0 || ptrMap[h1 - 1][i - 1] == nullptr) {
//       //     omp_set_lock(lock);
//       //     labelVec.push_back(++curLabel);
//       //     ptrMap[h1 - 1][i] = &labelVec.back();
//       //     omp_unset_lock(lock);
//       //   } else
//       //     ptrMap[h1 - 1][i] = ptrMap[h1 - 1][i - 1];
//       // }
//     }
//     for (size_t i = 1 + h0; i < h1; ++i) {
//       if (src[i][0] == 0) {
//         if (ptrMap[i - 1][0] == nullptr) {
//           omp_set_lock(lock);
//           labelVec.push_back(++curLabel);
//           groupVec.push_back(&labelVec.back());
//           ptrMap[i][0] = &groupVec.back();
//           omp_unset_lock(lock);
//         } else
//           ptrMap[i][0] = ptrMap[i - 1][0];
//       }

//       //#pragma omp barrier
//       //     {
//       //         #pragma omp critical
//       // std::cout << "HERE - " << omp_get_thread_num() << "\n";
//       //     }
//       for (size_t j = 1; j < w; ++j) {
//         if (src[i][j] == 0) {
//           // if (ptrMap[i - 1][j] == ptrMap[i][j - 1]) {
//           //   if (ptrMap[i - 1][j] == nullptr) {
//           //     labelVec.push_back(++curLabel);
//           //     ptrMap[i][j] = &labelVec.back();
//           //   } else
//           //     ptrMap[i][j] = ptrMap[i][j - 1];
//           // } else {
//           //   if (ptrMap[i - 1][j] == nullptr)
//           //     ptrMap[i][j] = ptrMap[i][j - 1];
//           //   else if (ptrMap[i][j - 1] == nullptr)
//           //     ptrMap[i][j] = ptrMap[i - 1][j];
//           //   else {
//           //     *(ptrMap[i - 1][j]) = *(ptrMap[i][j - 1]);
//           //     ptrMap[i][j] = ptrMap[i][j - 1];
//           //   }
//           // }

//           uint32_t **ptr = nullptr;
//           if (ptrMap[i - 1][j] != nullptr) ptr = ptrMap[i - 1][j];
//           if (ptrMap[i][j - 1] != nullptr) {
//             if (ptr != nullptr && *ptrMap[i][j - 1] != *ptr) *ptr = *ptrMap[i][j - 1];
//             ptr = ptrMap[i][j - 1];
//           }

//           if (ptr == nullptr) {
//             omp_set_lock(lock);
//             labelVec.push_back(++curLabel);
//             groupVec.push_back(&labelVec.back());
//             ptrMap[i][j] = &groupVec.back();
//             omp_unset_lock(lock);
//           } else
//             ptrMap[i][j] = ptr;
//         }
//       }
//     }
//     cout << ptrMap[2][0];
// #pragma omp barrier
//     if (omp_get_thread_num()) {
//       for (size_t j = 0; j < w; ++j) {
//         if (src[h1][j] == 0) {
//           // if (ptrMap[i - 1][j] == ptrMap[i][j - 1]) {
//           //   if (ptrMap[i - 1][j] == nullptr) {
//           //     labelVec.push_back(++curLabel);
//           //     ptrMap[i][j] = &labelVec.back();
//           //   } else
//           //     ptrMap[i][j] = ptrMap[i][j - 1];
//           // } else {
//           //   if (ptrMap[i - 1][j] == nullptr)
//           //     ptrMap[i][j] = ptrMap[i][j - 1];
//           //   else if (ptrMap[i][j - 1] == nullptr)
//           //     ptrMap[i][j] = ptrMap[i - 1][j];
//           //   else {
//           //     *(ptrMap[i - 1][j]) = *(ptrMap[i][j - 1]);
//           //     ptrMap[i][j] = ptrMap[i][j - 1];
//           //   }
//           // }

//           if (ptrMap[h1 - 1][j] != nullptr && ptrMap[h1][j] != nullptr && *ptrMap[h1 - 1][j] != *ptrMap[h1][j]) {
//             omp_set_lock(lock);
//             *ptrMap[h1 - 1][j] = *ptrMap[h1][j];
//             ptrMap[h1][j] = ptrMap[h1 - 1][j];
//             omp_unset_lock(lock);
//           }
//         }
//       }
//     }

//   //   for (size_t i = 0; i < w; ++i)
//   // omp_destroy_lock(lock + i);
//   omp_destroy_lock(lock);
// #pragma omp single
// {
//   size_t cur = 1;
//   groupVec.sort();
//   groupVec.unique();
//   for (auto &label : groupVec) {
//     *label = cur++;
//   }
// }

// #pragma omp for
//   for (size_t i = 0; i < h; ++i)
//     for (size_t j = 0; j < w; ++j)
//       if (ptrMap[i][j] != nullptr) dst[i][j] = **(ptrMap[i][j]);
//   }
// }

void imgMarkingOmp::imgMarking() {
  uint32_t curLabel = 1;
  std::list<uint32_t> labelVec;  // critical data
  std::list<uint32_t *> groupVec;
  std::vector<std::vector<uint32_t **>> ptrMap;
  ptrMap.resize(h);

  omp_lock_t *lock = new omp_lock_t;
  omp_init_lock(lock);

  uint32_t num_threads = omp_get_thread_limit();
  if (num_threads > h) num_threads = 1;

#pragma omp parallel num_threads(num_threads)
  {
#pragma omp for
    for (size_t i = 0; i < h; ++i) ptrMap[i].resize(w, nullptr);

    int32_t d = h / omp_get_num_threads();
    size_t h0;
    size_t h1;
    if (omp_get_thread_num()) {
      h0 = d * (omp_get_thread_num() - 1);
      h1 = d * (omp_get_thread_num());
    } else {
      h0 = d * (omp_get_num_threads() - 1);
      h1 = h;
    }

    for (size_t i = 0; i < w; ++i) {
      if (src[h0][i] == 0) {
        if (i == 0 || ptrMap[h0][i - 1] == nullptr) {
          omp_set_lock(lock);
          labelVec.push_back(curLabel++);
          groupVec.push_back(&labelVec.back());
          ptrMap[h0][i] = &groupVec.back();
          omp_unset_lock(lock);
        } else
          ptrMap[h0][i] = ptrMap[h0][i - 1];
      }
    }
    for (size_t i = 1 + h0; i < h1; ++i) {
      if (src[i][0] == 0) {
        if (ptrMap[i - 1][0] == nullptr) {
          omp_set_lock(lock);
          labelVec.push_back(curLabel++);
          groupVec.push_back(&labelVec.back());
          ptrMap[i][0] = &groupVec.back();
          omp_unset_lock(lock);
        } else
          ptrMap[i][0] = ptrMap[i - 1][0];
      }
      for (size_t j = 1; j < w; ++j) {
        if (src[i][j] == 0) {
          uint32_t **ptr = nullptr;
          if (ptrMap[i - 1][j] != nullptr) ptr = ptrMap[i - 1][j];
          if (ptrMap[i][j - 1] != nullptr) {
            if (ptr != nullptr && *ptrMap[i][j - 1] != *ptr) *ptrMap[i - 1][j] = *ptrMap[i][j - 1];
            ptr = ptrMap[i][j - 1];
          }

          if (ptr == nullptr) {
            omp_set_lock(lock);
            labelVec.push_back(curLabel++);
            groupVec.push_back(&labelVec.back());
            ptrMap[i][j] = &groupVec.back();
            omp_unset_lock(lock);
          } else
            ptrMap[i][j] = ptr;
        }
      }
    }
#pragma omp barrier
    if (omp_get_thread_num()) {
      for (size_t j = 0; j < w; ++j) {
        if (src[h1][j] == 0) {
          if (ptrMap[h1 - 1][j] != nullptr && ptrMap[h1][j] != nullptr && *ptrMap[h1 - 1][j] != *ptrMap[h1][j]) {
            omp_set_lock(lock);
            *ptrMap[h1 - 1][j] = *ptrMap[h1][j];
            ptrMap[h1][j] = ptrMap[h1 - 1][j];
            omp_unset_lock(lock);
          }
        }
      }
    }

    //   for (size_t i = 0; i < w; ++i)
    // omp_destroy_lock(lock + i);
    omp_destroy_lock(lock);
#pragma omp single
    {
      size_t cur = 1;
      groupVec.sort();
      groupVec.unique();
      for (auto &label : groupVec) {
        *label = cur++;
      }
    }

#pragma omp for
    for (size_t i = 0; i < h; ++i)
      for (size_t j = 0; j < w; ++j)
        if (ptrMap[i][j] != nullptr) dst[i][j] = **(ptrMap[i][j]);
  }
}