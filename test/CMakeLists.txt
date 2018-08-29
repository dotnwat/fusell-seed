add_test(
  NAME postgres
  COMMAND test-runner.sh
    $<TARGET_FILE:main>
    ${CMAKE_CURRENT_SOURCE_DIR}/postgres.sh)
