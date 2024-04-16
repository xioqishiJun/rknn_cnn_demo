cmake_minimum_required(VERSION 3.4.1)

project(rknn_cnn_demo)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# 获取HOME环境变量
set(HOME_DIR $ENV{HOME})


# rknn api
set(RKNN_API_PATH ${HOME_DIR}/librknn_api)
set(RKNN_RT_LIB ${HOME_DIR}/librknn_api/aarch64/librknnrt.so)
set(RGA_LIB ${HOME_DIR}/librknn_api/3rdparty/rga/libs/Linux/gcc-aarch64/librga.so)

# opencv
set(OpenCV_DIR ${HOME_DIR}/librknn_api/3rdparty/opencv/opencv-linux-aarch64/share/OpenCV)
find_package(OpenCV REQUIRED)

include_directories(${HOME_DIR}/librknn_api/3rdparty/rk_mpi_mmz/include)
include_directories(${RKNN_API_PATH}/include)
include_directories(${HOME_DIR}/librknn_api/3rdparty)
include_directories(${HOME_DIR}/librknn_api/3rdparty/rga/include)
set(CMAKE_INSTALL_RPATH "lib")
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(rknn_cnn_demo
    src/main.cc
)

target_link_libraries(rknn_cnn_demo
        ${RKNN_RT_LIB} ${RGA_LIB} ${MPI_MMZ_LIB} ${OpenCV_LIBS}
)

