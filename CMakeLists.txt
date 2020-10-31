CMAKE_MINIMUM_REQUIRED(VERSION 3.16)   
PROJECT(UbiquitousPerception)   
SET(PROJECT_NAME "UbiquitousPerception") 

# 判断操作系统类型
IF (WIN32)
	MESSAGE(STATUS "Now is windows")
ELSEIF (APPLE)
	MESSAGE(STATUS "Now is Apple systens.")
ELSEIF (UNIX)
	MESSAGE(STATUS "Now is UNIX-like OS's. Including aPPLE os x  and CygWin")
ENDIF ()

# 加载cmake文件
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake) 
include(FindCompileCfg)
include(FindCreateFilter)
include(FindCommonFunc)

# 编译宏定义
ADD_DEFINITIONS(-D__linux)

# 设置编译时公共包含文件的查找目录
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/3rdparty/include")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include/common_inc")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include/interface_inc")

# 设置编译时链接库(第三方或自研的)的相关目录
##设置pkg-config配置文件的路径变量
SET(GEN_PKG_CFG_DIR ${PROJECT_SOURCE_DIR}/bin/pkgconfig)
SET(3RDPARTY_PKG_CFG_DIR ${PROJECT_SOURCE_DIR}/3rdparty/pkgconfig)

##修改pkg-config配置文件关于依赖库的相关依赖和路径信息
execute_process(COMMAND sh ../script/3rdpkgcfg.sh "${PROJECT_SOURCE_DIR}/3rdparty" ${ARCH_DIR})
execute_process(COMMAND sh ../script/genpkgcfg.sh "${PROJECT_SOURCE_DIR}" ${ARCH_DIR})

##设置编译时依赖的第三方库的存放根目录
SET(3RDPARTY_LIB_ROOT_DIR ${PROJECT_SOURCE_DIR}/3rdparty/lib_${ARCH_DIR})
LINK_DIRECTORIES("${3RDPARTY_LIB_ROOT_DIR}")

##设置编译时依赖的生成库的存放根目录
SET(GEN_LIB_ROOT_DIR ${PROJECT_SOURCE_DIR}/bin/lib)
LINK_DIRECTORIES(${GEN_LIB_ROOT_DIR})

# 设置生成库Debug或Release版本存放目录
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${GEN_LIB_ROOT_DIR})   
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${GEN_LIB_ROOT_DIR})

# 设置生成可执行文件Debug或Release版本存放目录
SET(BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_DIR})   
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_DIR}) 

# 设置默认安装目录
SET(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/build/bin")

# 设置运行时链接库的查找目录
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_INSTALL_RPATH "./3rdparty:./lib")

#显示执行构建过程中详细的信息
SET(CMAKE_VERBOSE_MAKEFILE ON)

# 添加编译源码目录
add_subdirectory(src)