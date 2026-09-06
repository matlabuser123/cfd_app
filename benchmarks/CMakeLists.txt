add_executable(benchmark_20x20 benchmark_20x20.cpp)
target_link_libraries(benchmark_20x20 PRIVATE cfd_core)

add_executable(benchmark_40x40 benchmark_40x40.cpp)
target_link_libraries(benchmark_40x40 PRIVATE cfd_core)

add_executable(benchmark_80x80 benchmark_80x80.cpp)
target_link_libraries(benchmark_80x80 PRIVATE cfd_core)

add_executable(benchmark_gpu_spmv benchmark_gpu_spmv.cpp)
target_link_libraries(benchmark_gpu_spmv PRIVATE cfd_core)

add_test(NAME CFDPerformance80x80 COMMAND benchmark_80x80)
set_tests_properties(CFDPerformance80x80 PROPERTIES LABELS "performance;benchmark" TIMEOUT 30)

add_test(NAME CFDThreadScaling80x80 COMMAND benchmark_80x80 --thread-scaling)
set_tests_properties(CFDThreadScaling80x80 PROPERTIES LABELS "performance;benchmark;scaling" TIMEOUT 60)

add_test(NAME CFDGPUSpMVBenchmark COMMAND benchmark_gpu_spmv)
set_tests_properties(CFDGPUSpMVBenchmark PROPERTIES LABELS "gpu;performance;benchmark" SKIP_RETURN_CODE 77 TIMEOUT 60)
