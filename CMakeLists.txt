cmake_minimum_required(VERSION 3.24)

project(LumosAI VERSION 0.1.0 LANGUAGES CXX)

option(LUMOS_BUILD_TESTS "Build Lumos tests" ON)
option(LUMOS_BUILD_UI "Build Lumos Qt UI shell when Qt is available" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/LumosWarnings.cmake)

add_library(
    lumos_core
    src/app/EnhancementController.cpp
    src/common/Telemetry.cpp
    src/engine/CpuStubPipeline.cpp
)

target_include_directories(lumos_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(lumos_core PUBLIC NOMINMAX)
lumos_set_project_warnings(lumos_core)

if(LUMOS_BUILD_UI)
    find_package(Qt6 COMPONENTS Core Gui Qml Quick QUIET)
    if(Qt6_FOUND)
        add_executable(lumos_app src/main.cpp)
        target_compile_definitions(lumos_app PRIVATE LUMOS_WITH_QT=1)
        target_link_libraries(
            lumos_app
            PRIVATE
                lumos_core
                Qt6::Core
                Qt6::Gui
                Qt6::Qml
                Qt6::Quick
        )
        lumos_set_project_warnings(lumos_app)
    else()
        message(WARNING "Qt6 not found; skipping lumos_app target.")
    endif()
endif()

if(LUMOS_BUILD_TESTS)
    enable_testing()

    add_executable(pipeline_contract_tests tests/unit/PipelineContractTests.cpp)
    target_link_libraries(pipeline_contract_tests PRIVATE lumos_core)
    target_include_directories(pipeline_contract_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(
        pipeline_contract_tests
        PRIVATE LUMOS_TEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/tests"
    )
    lumos_set_project_warnings(pipeline_contract_tests)
    add_test(NAME PipelineContractTests COMMAND pipeline_contract_tests)

    add_executable(telemetry_tests tests/unit/TelemetryTests.cpp)
    target_link_libraries(telemetry_tests PRIVATE lumos_core)
    target_include_directories(telemetry_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(
        telemetry_tests
        PRIVATE LUMOS_TEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/tests"
    )
    lumos_set_project_warnings(telemetry_tests)
    add_test(NAME TelemetryTests COMMAND telemetry_tests)

    add_executable(enhance_flow_tests tests/integration/EnhanceFlowTests.cpp)
    target_link_libraries(enhance_flow_tests PRIVATE lumos_core)
    target_include_directories(enhance_flow_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(
        enhance_flow_tests
        PRIVATE LUMOS_TEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/tests"
    )
    lumos_set_project_warnings(enhance_flow_tests)
    add_test(NAME EnhanceFlowTests COMMAND enhance_flow_tests)
endif()
