#基本设置
set(libname "VisionDetectAlg")
set(version "1.0")
set(soversion "1")

#当前目录及其下子目录下所有源文件保存变量中
file(GLOB_RECURSE src_files *.h *.cpp *.c *.cc) 
create_filters(src_files)

# 追加依赖的公共对象
SET(ADD_COMM_OBJ_SRC "")
SET(AAD_OBJ_NAME_LST endecode)
add_cmm_obj(${CMMOBJ_DIR} "${AAD_OBJ_NAME_LST}")
list(APPEND CMMOBJ_SRC ${ADD_COMM_OBJ_SRC})

# 设置同时生产的静态库和动态库的存放目录
set(LIBRARY_OUTPUT_PATH ${GEN_LIB_ROOT_DIR})

# 生成静态库目标
add_library(${libname}_static STATIC ${src_files} ${CMMOBJ_SRC})

# 生成动态库目标
add_library(${libname} SHARED ${src_files} ${CMMOBJ_SRC})

# 指定静态库的输出名称
set_target_properties(${libname}_static PROPERTIES OUTPUT_NAME ${libname})

# 使动态库和静态库同时存在
set_target_properties(${libname} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(${libname}_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# 指定动态库版本
# VERSION 动态库版本
# SOVERSION API版本
set_target_properties(${libname} PROPERTIES VERSION ${version} SOVERSION ${soversion})

## 添加依赖第三方库
SET(SHARE_LINK_LIBS log4cxx opencv rknn boost)
link_static_share_lib(${libname} "${SHARE_LINK_LIBS}" ${3RDPARTY_PKG_CFG_DIR} "share")