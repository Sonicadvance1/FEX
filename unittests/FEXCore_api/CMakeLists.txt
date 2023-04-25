list(APPEND LIBS dl)

add_executable(APITest
  Test.cpp
  APITest.cpp)

# Enable FEX APIs to be used by targets that use target_link_libraries on FEXLoader
set_target_properties(APITest PROPERTIES ENABLE_EXPORTS 1)

target_link_libraries(APITest
  PRIVATE
    ${LIBS}
    fmt
)


set_target_properties(APITest PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/FEXCore_Test/")

# This test needs to include the FEXCore headers without linking to the library directly.
# This way it can behave like a library where headers are visible but we are going to dlopen it instead of direct link.
target_include_directories(APITest PRIVATE "$<TARGET_PROPERTY:FEXCore,INCLUDE_DIRECTORIES>")
