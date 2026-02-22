include(FetchContent)

message(STATUS "Fetching nlohmann_json")
FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP NEW
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(nlohmann_json)

message(STATUS "Fetching GTEST")
FetchContent_Declare(
  GTest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP NEW
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(GTest)
