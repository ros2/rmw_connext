find_package(ament_cmake_gtest REQUIRED)

ament_add_gtest(test_topic_cache test_topic_cache.cpp)
if(TARGET test_topic_cache)
    ament_target_dependencies(test_topic_cache)
    target_link_libraries(test_topic_cache ${PROJECT_NAME})
endif()

ament_add_gtest(test_qos_no_profile_file test_qos_profiles/test_qos_no_profile_file.cpp)
if(TARGET test_qos_no_profile_file)
    # required for qos_impl.hpp
    target_include_directories(test_qos_no_profile_file PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src)
    target_link_libraries(test_qos_no_profile_file ${PROJECT_NAME})
endif()

ament_add_gtest(test_profile_no_env_variables
    test_qos_profiles/test_profile_no_env_variables.cpp
    ENV "NDDS_QOS_PROFILES=${CMAKE_CURRENT_SOURCE_DIR}/test_qos_profiles/CONNEXT_QOS_PROFILES.xml")
if(TARGET test_profile_no_env_variables)
    target_link_libraries(test_profile_no_env_variables ${PROJECT_NAME})
endif()

ament_add_gtest(test_profile_topic_override_set
    test_qos_profiles/test_profile_topic_override_set.cpp
    ENV "NDDS_QOS_PROFILES=${CMAKE_CURRENT_SOURCE_DIR}/test_qos_profiles/CONNEXT_QOS_PROFILES.xml")
if(TARGET test_profile_topic_override_set)
    target_link_libraries(test_profile_topic_override_set ${PROJECT_NAME})
endif()

ament_add_gmock(test_security_logging test_security_logging.cpp)
if(TARGET test_security_logging)
    ament_target_dependencies(test_security_logging)
    target_link_libraries(test_security_logging ${PROJECT_NAME})
endif()
