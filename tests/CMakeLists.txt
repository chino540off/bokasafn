add_executable(bokasafn-tests
  epoll.cc
  timer.cc

  net/socket.cc

  size/literals.cc

  cache/common.cc
  cache/lru.cc
)

add_dependencies(bokasafn-tests googletest)

target_include_directories(bokasafn-tests
  PUBLIC
    ${GTEST_INCLUDE_DIRS}
    ${BOKASAFN_INCLUDE_DIRS}
)

target_link_libraries(bokasafn-tests
  ${GTEST_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
)

add_test(NAME bokasafn-unit-tests COMMAND ./bokasafn-tests)

if(BOKASAFN_BUILD_COVERAGE)
set(COVERAGE_LCOV_EXTRACT
  '*/bokasafn/include/*'
)

setup_TARGET_FOR_COVERAGE_LCOV(
    NAME coverage
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/bokasafn-tests
    DEPENDENCIES bokasafn-tests
)
endif()
