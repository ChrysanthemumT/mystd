add_executable(vector_test vector_t)
add_executable(alloc_test allocator_t)
add_executable(spsc_test spsc_t)
add_executable(mpmc_test mpmc_t)

target_link_libraries(alloc_test PRIVATE mystd)
target_link_libraries(vector_test PRIVATE mystd)
target_link_libraries(spsc_test PRIVATE mystd)
target_link_libraries(mpmc_test PRIVATE mystd)
