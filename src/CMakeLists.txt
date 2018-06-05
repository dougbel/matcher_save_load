cmake_minimum_required(VERSION 2.6)
project( matcher_save_load )

find_package( OpenCV REQUIRED )
find_package( Boost REQUIRED COMPONENTS system thread) 


set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)

set(HEADERS
   ${HEADERS}
   ${CMAKE_CURRENT_SOURCE_DIR}/imagedescripted.h

)


add_executable( matcher_save_load mainMatcher.cpp imagedescripted.cpp )
target_link_libraries( matcher_save_load ${OpenCV_LIBS} ${Boost_LIBRARIES})
