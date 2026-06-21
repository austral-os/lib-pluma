include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(pluma_tests 
    test_core_types.cpp
    test_piece_table.cpp
    test_transactions.cpp
    test_styles.cpp
    test_typography.cpp
    test_layout.cpp
    test_incremental_layout.cpp
    test_display_list.cpp
    test_renderer.cpp
    test_dom.cpp
    test_selection.cpp
    test_input.cpp
    test_editor.cpp
    test_advanced_pagination.cpp
    test_virtualization.cpp
    test_services.cpp
    test_spell_checker.cpp
    test_a11y.cpp
    test_plugins.cpp
    test_diagnostics.cpp
    test_optimization.cpp
)

target_link_libraries(pluma_tests PRIVATE pluma::pluma Catch2::Catch2WithMain)
target_include_directories(pluma_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(Catch)
catch_discover_tests(pluma_tests)
