function(add_fs_test name script)
  add_test(
    NAME ${name}
    COMMAND test-runner.sh
      $<TARGET_FILE:heap_fs>
      ${CMAKE_CURRENT_SOURCE_DIR}/${script})
endfunction()

add_fs_test(postgres postgres.sh)
add_fs_test(kernel kernel.sh)
add_fs_test(bamsort bamsort.sh)
