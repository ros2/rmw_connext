find_package(ament_cmake_gtest REQUIRED)

ament_add_gtest(test_topic_cache test_topic_cache.cpp)
if(TARGET test_topic_cache)
    ament_target_dependencies(test_topic_cache)
    target_link_libraries(test_topic_cache ${PROJECT_NAME})
endif()
