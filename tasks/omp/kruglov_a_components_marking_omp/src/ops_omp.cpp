// Copyright 2023 Kruglov Alexey

#include "omp/kruglov_a_components_marking_omp/include/ops_omp.hpp"

#include <omp.h>
#include <map>
#include <iostream>

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

// void f(uint32_t* ptrMap[i][j], uint32_t* ptrMap[i - 1][j], uint32_t* ptrMap[i + 1][j], uint32_t* ptrMap[i][j - 1]) {
//     if (ptrMap[i - 1][j] != nullptr) {
//         ptrMap[i][j] = ptrMap[i - 1][j];
//     }
//     if (ptrMap[i + 1][j] != nullptr) {
//         if (ptrMap[i][j] != nullptr) {
//             if (*ptrMap[i][j] != *ptrMap[i + 1][j])
//                 *ptrMap[i + 1][j] = *ptrMap[i][j];
//         }
//         ptrMap[i][j] = ptrMap[i + 1][j];
//     }
//     if (ptrMap[i][j - 1] != nullptr) {
//         if (ptrMap[i][j] != nullptr) {
//             if (*ptrMap[i][j] != *ptrMap[i][j - 1])
//                 *ptrMap[i][j - 1] = *ptrMap[i][j];
//         }
//         ptrMap[i][j] = ptrMap[i][j - 1];
//     }
//     if (ptrMap[i][j] == nullptr){
//             labelVec.push_back(++curLabel);
//             ptrMap[i][j] = &labelVec.back();
//           }
// }

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
//    //omp_set_num_threads(4);
//   uint32_t curLabel = 0;
//   // std::vector<imgMarkingOmp::labelStore> labels;
//   std::list<uint32_t> labelVec; // critical data
//   std::vector<std::vector<uint32_t *>> ptrMap;
//   ptrMap.resize(h);

//     omp_lock_t *lock = new omp_lock_t;
//     omp_init_lock(lock);


//   #pragma omp parallel
//   {
//   #pragma omp for
//   for (size_t i = 0; i < h; ++i) ptrMap[i].resize(w, nullptr);

//   //#pragma omp parallel num_threads(4)

//   #pragma omp for
//   for (size_t i = 0; i < h; ++i) {
//     for (size_t j = 0; j < w; ++j) {
//       if (src[i][j] == 0) {
//         if(j == 0 || src[i][j - 1] != 0){
//             omp_set_lock(lock);
//             labelVec.push_back(++curLabel);
//             ptrMap[i][j] = &labelVec.back();
//             omp_unset_lock(lock);
//             }
//         else ptrMap[i][j] = ptrMap[i][j - 1];
//         }
//       }
//     }
//   }

// #pragma omp for
//     for (size_t j = 0; j < w; ++j) {
//         for (size_t i = 1; i < h; ++i) {
    
//             if (src[i][j] != 0 || src[i - 1][j] != 0)
//                 continue;
//             {
//                 omp_set_lock(lock);
//                 *ptrMap[i-1][j] = *ptrMap[i][j];
//                 ptrMap[i-1][j] = ptrMap[i][j];
//                 omp_unset_lock(lock);
//             }
//         }
//     }

//   omp_destroy_lock(lock);

//   size_t count = 0;
//   size_t cur = 0;
//   labelVec.sort();


//   for(auto &label : labelVec) {
//     if (cur != label) {
//       cur = label;
//       count++;
//     }
//     label = count;
//   }

// #pragma omp for nowait
//   for (size_t i = 0; i < h; ++i)
//     for (size_t j = 0; j < w; ++j)
//       if (ptrMap[i][j] != nullptr) dst[i][j] = *(ptrMap[i][j]);
// }

class obj {
public:
    //vector<pair<int32_t, int32_t>> body; //pair(i * w + jf, i * w + jl)
    int32_t num;
    map<int32_t, int32_t> body;
    obj() {};
    obj(int32_t _num) : num(_num) {}
    void merge(obj* o) {
        for (auto i : o->body)
        {
            body.insert(i);
        }
        // delete o;
        o->body.clear();
    }
    bool Intersects(const obj* o, int32_t w) {
        if (body.size() == 0 || o->body.size() == 0)
            return false;
        auto end = *(o->body.rbegin()++);
        if (end.second < (*body.begin()).first - w)
            return false;
        for (auto i : body) {
            int m = min(end.second + w, i.second);
            int mx = max(end.first + w, i.first);
            if ((end.second >= i.first - w) && m >= mx)
                return true;
        }
        return false;
    }
};

void imgMarkingOmp::imgMarking() {
    uint32_t curLabel = 1;
    std::vector<std::vector<obj**>> ptrMap;
    ptrMap.resize(h);

    std::list<obj*> objVec;

    omp_lock_t* lock = new omp_lock_t;
    omp_init_lock(lock);


#pragma omp parallel num_threads(4)
    {
#pragma omp for
        for (size_t i = 0; i < h; ++i) ptrMap[i].resize(w, nullptr);

#pragma omp for //schedule(static,1)
        for (size_t i = 0; i < h; ++i) {
            for (size_t j = 0; j < w; ++j) {
                if (src[i][j] == 0) {
                    if (j == 0 || src[i][j - 1] != 0) { 
                        omp_set_lock(lock);
                        obj* o = new obj(curLabel++);
                        o->body[i * w + j] = i * w + j;
                        
                        objVec.push_back(o);
                        ptrMap[i][j] = &(objVec.back());
                        omp_unset_lock(lock);
                    }
                    else {
                        ptrMap[i][j] = ptrMap[i][j - 1];
                        if (j == w - 1 || src[i][j + 1] != 0)
                        {
                            auto iter = (*ptrMap[i][j])->body.upper_bound(i * w + j);
                                (*--iter).second = i * w + j;
                        }
                    }
                }
            }
        }

#pragma omp for
    for (size_t j = 0; j < w; ++j) {
        for (size_t i = 0; i < h - 1; ++i) {
            
                omp_set_lock(lock);
                if (src[i][j] == 0 && src[i + 1][j] == 0 && (*ptrMap[i + 1][j]) != (*ptrMap[i][j]))
                {
                    //#pragma omp critical
                    // if ((*ptrMap[i + 1][j]) != (*ptrMap[i][j])){
                    //std:: cout << (*ptrMap[i + 1][j]) << " " << (*ptrMap[i][j]) <<"\n";
                     //std::cout << "merge of " << (*ptrMap[i + 1][j])->num << " (" << (*(*ptrMap[i + 1][j])->body.begin()).first << ") with " << (*ptrMap[i][j])->num << " (" << (*(*ptrMap[i][j])->body.begin()).first << ") by thread " << omp_get_thread_num() << "\n";
                    (*ptrMap[i][j])->merge((*ptrMap[i + 1][j]));
                    *ptrMap[i + 1][j] = *ptrMap[i][j];
                    //}
                    
                }
                omp_unset_lock(lock);
            }
        }
    }
    #pragma omp barrier

    omp_destroy_lock(lock);

    // for (auto l : objVec){
    //     std::cout << l << "\n";
    // }

    objVec.sort();
    objVec.unique();

    size_t cur = 1;
    for (auto label : objVec) {
        if (label->body.size() == 0)
            continue;
        else {
            label->num = cur++;
        }
    }
//#pragma omp barrier
//#pragma omp parallel for
    for(auto label : objVec) {
        if (label->body.size() != 0)
            for (auto row : label->body) {
                //dst.insert_range(row.first, new vector<int>{ row.second - row.first + 1, label->num });
                for (int k = row.first; k <= row.second; ++k)
                    dst[k / w][k % w] = label->num;
            }
        delete label;
    }
        std::cout << "HERE\n";
}