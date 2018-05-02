cmake_minimum_required(VERSION 2.8)

if(NOT APPLE)
    find_package(GLEW REQUIRED)                                                                                                                                              
    include_directories(${GLEW_INCLUDE_DIRS})
    link_libraries(${GLEW_LIBRARIES})
endif(NOT APPLE)

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIR})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})

if(NOT GLUT_FOUND)
    message(ERROR " GLUT not found!")
endif(NOT GLUT_FOUND)

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)

find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
link_directories(${LIBXML2_LIBRARIES})
add_definitions(${LIBXML2_DEFINITIONS})
if(NOT LIBXML2_FOUND)
    message(ERROR " LibXML2 not found!")
endif(NOT LIBXML2_FOUND)

find_package(DevIL REQUIRED)
link_libraries(${IL_LIBRARIES})
include_directories(${IL_INCLUDE_DIR})

if(UNIX)
    set(MATH_LIB m)
endif(UNIX)

add_executable(engine engine.c)
target_link_libraries(engine ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${LIBXML2_LIBRARIES} ${MATH_LIB})
