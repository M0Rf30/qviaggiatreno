# Un eseguibile per ogni sorgente di test, collegato alla libreria con i parser
foreach(test_name tst_parsertrenord tst_parserviaggiatreno)
    qt_add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE qviaggiatreno_core Qt6::Test)
    target_compile_definitions(${test_name} PRIVATE
        QVT_TEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/data")
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endforeach()
