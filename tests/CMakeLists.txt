if(MSVC)
	add_compile_options(/UNDEBUG)
else()
	add_compile_options(-UNDEBUG)
endif()

add_executable(cfd_mesh_tests test_mesh.cpp)
target_link_libraries(cfd_mesh_tests PRIVATE cfd_core)
add_test(NAME CFDMeshTests COMMAND cfd_mesh_tests)

add_executable(cfd_scalar_field_tests test_scalar_field.cpp)
target_link_libraries(cfd_scalar_field_tests PRIVATE cfd_core)
add_test(NAME CFDScalarFieldTests COMMAND cfd_scalar_field_tests)

add_executable(cfd_vector_field_tests test_vector_field.cpp)
target_link_libraries(cfd_vector_field_tests PRIVATE cfd_core)
add_test(NAME CFDVectorFieldTests COMMAND cfd_vector_field_tests)

add_executable(cfd_boundary_condition_tests test_boundary_conditions.cpp)
target_link_libraries(cfd_boundary_condition_tests PRIVATE cfd_core)
add_test(NAME CFDBoundaryConditionTests COMMAND cfd_boundary_condition_tests)

add_executable(cfd_case_tests test_case.cpp)
target_link_libraries(cfd_case_tests PRIVATE cfd_core)
target_compile_definitions(cfd_case_tests PRIVATE CFDAPP_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
add_test(NAME CFDCaseTests COMMAND cfd_case_tests)

add_executable(cfd_gui_controller_tests test_gui_controller.cpp ${PROJECT_SOURCE_DIR}/src/gui/CFDController.cpp)
target_link_libraries(cfd_gui_controller_tests PRIVATE cfd_core)
target_compile_definitions(cfd_gui_controller_tests PRIVATE CFDAPP_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
add_test(NAME CFDGUIControllerTests COMMAND cfd_gui_controller_tests)

add_executable(cfd_discretization_tests test_discretization.cpp)
target_link_libraries(cfd_discretization_tests PRIVATE cfd_core)
add_test(NAME CFDDiscretizationTests COMMAND cfd_discretization_tests)

add_executable(cfd_sparse_matrix_tests test_sparse_matrix.cpp)
target_link_libraries(cfd_sparse_matrix_tests PRIVATE cfd_core)
add_test(NAME CFDSparseMatrixTests COMMAND cfd_sparse_matrix_tests)

add_executable(cfd_linear_solver_tests test_linear_solver.cpp)
target_link_libraries(cfd_linear_solver_tests PRIVATE cfd_core)
add_test(NAME CFDLinearSolverTests COMMAND cfd_linear_solver_tests)

add_executable(cfd_momentum_tests test_momentum.cpp)
target_link_libraries(cfd_momentum_tests PRIVATE cfd_core)
add_test(NAME CFDMomentumTests COMMAND cfd_momentum_tests)

add_executable(cfd_pressure_tests test_pressure.cpp)
target_link_libraries(cfd_pressure_tests PRIVATE cfd_core)
add_test(NAME CFDPressureTests COMMAND cfd_pressure_tests)

add_executable(cfd_simple_tests test_simple.cpp)
target_link_libraries(cfd_simple_tests PRIVATE cfd_core)
add_test(NAME CFDSIMPLETests COMMAND cfd_simple_tests)
set_tests_properties(CFDSIMPLETests PROPERTIES LABELS "parallel;correctness")

add_executable(cfd_validation_tests test_validation.cpp)
target_link_libraries(cfd_validation_tests PRIVATE cfd_core)
add_test(NAME CFDValidationTests COMMAND cfd_validation_tests)

add_executable(cfd_profiling_tests test_profiling.cpp)
target_link_libraries(cfd_profiling_tests PRIVATE cfd_core)
add_test(NAME CFDProfilingTests COMMAND cfd_profiling_tests)

add_executable(cfd_optimization_tests test_optimization.cpp)
target_link_libraries(cfd_optimization_tests PRIVATE cfd_core)
add_test(NAME CFDOptimizationTests COMMAND cfd_optimization_tests)

add_executable(cfd_io_tests test_io.cpp)
target_link_libraries(cfd_io_tests PRIVATE cfd_core)
add_test(NAME CFDIOTests COMMAND cfd_io_tests)

add_executable(cfd_parallel_tests test_parallel.cpp)
target_link_libraries(cfd_parallel_tests PRIVATE cfd_core)
add_test(NAME CFDParallelTests COMMAND cfd_parallel_tests)
set_tests_properties(CFDParallelTests PROPERTIES LABELS "parallel;correctness")

add_executable(cfd_gpu_tests test_gpu.cpp)
target_link_libraries(cfd_gpu_tests PRIVATE cfd_core)
add_test(NAME CFDGPUTests COMMAND cfd_gpu_tests)
set_tests_properties(CFDGPUTests PROPERTIES LABELS "gpu;equivalence;correctness")

add_executable(cfd_solver_backend_tests test_solver_backend.cpp)
target_link_libraries(cfd_solver_backend_tests PRIVATE cfd_core)
add_test(NAME CFDSolverBackendTests COMMAND cfd_solver_backend_tests)
set_tests_properties(CFDSolverBackendTests PROPERTIES LABELS "parallel;gpu;correctness")

add_executable(cfd_cli_case_tests test_cli_case.cpp)
target_link_libraries(cfd_cli_case_tests PRIVATE cfd_core)
target_compile_definitions(cfd_cli_case_tests PRIVATE
    CFDAPP_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
    CFDAPP_EXECUTABLE_PATH="$<TARGET_FILE:cfdapp>"
)
add_test(NAME CFDCLICaseTests COMMAND cfd_cli_case_tests)
