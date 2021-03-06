cmake_minimum_required(VERSION 2.8.3)
project(ros_dumper)
 
#We are using a stripped acquisition
add_definitions(-DALL_HEADERS_IN_SAME_DIR)
 
if(NOT OpenCV_DIR)
 find_path(OpenCV_DIR "OpenCVConfig.cmake" DOC "Root directory of OpenCV" HINTS    "/opt/ros/kinetic/share/OpenCV/")
endif(NOT OpenCV_DIR)
FIND_PACKAGE(OpenCV REQUIRED)
 

find_package(catkin REQUIRED COMPONENTS 
  sensor_msgs 
  OpenCV
  cv_bridge 
  roscpp 
  std_msgs 
  genmsg 
  image_transport 
  geometry_msgs 
  roslib 
  tf)
  
#Messages -----------------------------------
#add_message_files(DIRECTORY msg FILES a.msg )

#Services -----------------------------------
#add_service_files(DIRECTORY srv FILES SetScale.srv )

## Generate added messages and services
#generate_messages(DEPENDENCIES std_msgs ros_dumper)

#This needs to be after generate messages
catkin_package()

include_directories( ${catkin_INCLUDEDIR} ${catkin_INCLUDE_DIRS} ${OpenCV_INCLUDEDIR} )
add_executable(ros_dumper  src/main.cpp   )
target_link_libraries(ros_dumper ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} )

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_gencpp)
 
#Put output at bin folder
set_target_properties(${PROJECT_NAME} PROPERTIES 
                          ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                          LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                          RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                         )

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
install(FILES ${PROJECT_NAME}.xml DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

