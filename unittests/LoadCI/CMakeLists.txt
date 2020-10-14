
set(OUTPUT_RESULTS_FOLDER "${CMAKE_BINARY_DIR}/LoadCI")
add_custom_command(OUTPUT ${OUTPUT_RESULTS_FOLDER}
  COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_RESULTS_FOLDER}")

set(TEST_ARGS
  "None" "native"
  "-c irjit -n 500 -- "    "jit_500"
  "-c irjit -n 500 -m -- " "jit_500_m"
  )

set(JSON_RESULTS "")

list(LENGTH TEST_ARGS ARG_COUNT)
math(EXPR ARG_COUNT "${ARG_COUNT}-1")

add_custom_target(upload_results)

foreach(Index RANGE 0 ${ARG_COUNT} 2)
  math(EXPR TEST_NAME_INDEX "${Index}+1")

  list(GET TEST_ARGS ${Index} ARGS)
  list(GET TEST_ARGS ${TEST_NAME_INDEX} TEST_DESC)

  set(TEST_NAME "loadci_${TEST_DESC}")

  set(JSON_NAME "${OUTPUT_RESULTS_FOLDER}/loadci_${TEST_DESC}.json")

  list(APPEND JSON_RESULTS "${JSON_NAME}")

  if (TEST_DESC MATCHES "native")
    add_custom_command(OUTPUT ${JSON_NAME}
      COMMAND
        ${CMAKE_COMMAND}
        "-E" "env"
        "FEXLoader=''"
        "FEXArgs=''"
        "pytest-3"
        "--benchmark-min-rounds=2"
        "--benchmark-json"
        "${JSON_NAME}"
        "${CMAKE_SOURCE_DIR}/Scripts/Execute_FEX.py"
      DEPENDS "${CMAKE_BINARY_DIR}/Bin/FEXLoader"
      DEPENDS "${OUTPUT_RESULTS_FOLDER}"
    )
  else()
    add_custom_command(OUTPUT ${JSON_NAME}
      COMMAND
        ${CMAKE_COMMAND}
        "-E" "env"
        "FEXLoader=${CMAKE_BINARY_DIR}/Bin/FEXLoader "
        "FEXArgs=${ARGS}"
        "pytest-3"
        "--benchmark-min-rounds=2"
        "--benchmark-json"
        "${JSON_NAME}"
        "${CMAKE_SOURCE_DIR}/Scripts/Execute_FEX.py"
      DEPENDS "${CMAKE_BINARY_DIR}/Bin/FEXLoader"
      DEPENDS "${OUTPUT_RESULTS_FOLDER}"
    )
  endif()

  # Upload targets
  set(UPLOAD_TARGET "Upload_${TEST_DESC}")
  add_custom_command(TARGET upload_results
    COMMAND
    "python" "${CMAKE_SOURCE_DIR}/Scripts/Upload_LoadCI.py"
    "${JSON_NAME}"
    "${TEST_DESC}"
    DEPENDS "${JSON_NAME}")
endforeach()

add_custom_target(loadci_results
  DEPENDS "${JSON_RESULTS}")

