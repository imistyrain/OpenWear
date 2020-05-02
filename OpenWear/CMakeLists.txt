include_directories(../sdm/include)
AUX_SOURCE_DIRECTORY(../sdm/include SDM_SRCS)
add_executable(OpenWear ${SDM_SRCS} Glasses.cpp 3DObject.cpp glm.cpp openwear.cpp)
target_link_libraries(OpenWear ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})