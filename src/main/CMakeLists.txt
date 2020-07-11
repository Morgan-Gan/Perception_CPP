# 包含头文件目录
INCLUDE_DIRECTORIES(".")

#当前目录及其下子目录下所有源文件保存变量中
file(GLOB_RECURSE src_files *.h *.cpp *.c *.cc) 
create_filters(src_files)

# 追加依赖的公共对象
SET(ADD_COMM_OBJ_SRC "")
SET(AAD_OBJ_NAME_LST thread)
add_cmm_obj("${AAD_OBJ_NAME_LST}")
list(APPEND CMMOBJ_SRC ${ADD_COMM_OBJ_SRC})

#编译可执行文件
add_executable(${PROJECT_NAME} ${src_files} ${CMMOBJ_SRC})

# 依赖生成和第三方的静态库和动态库的文件及其目录
##依赖静态库(同一目录的放一起)
SET(GEN_STATIC_LINK_LIBS gen_static_pkg)
link_static_share_lib("${GEN_STATIC_LINK_LIBS}" ${GEN_PKG_CFG_DIR} "static")

SET(3RD_STATIC_LINK_LIBS muduo)
link_static_share_lib("${3RD_STATIC_LINK_LIBS}" ${3RDPARTY_PKG_CFG_DIR} "static")

## 依赖动态库
SET(SHARE_LINK_LIBS hiredis thrift log4cxx boost live555 ffmpeg opencv)
link_static_share_lib("${SHARE_LINK_LIBS}" ${3RDPARTY_PKG_CFG_DIR} "share")

#程序安装设置
##目标文件的安装
INSTALL(TARGETS UbiquitousPerception
RUNTIME DESTINATION . 
LIBRARY DESTINATION lib
ARCHIVE DESTINATION libstatic)

##普通文件的安装
INSTALL(FILES ../../readme.md DESTINATION doc)

##非目标文件的可执行程序安装(比如脚本之类)
INSTALL(PROGRAMS ../../script/run.sh DESTINATION . PERMISSIONS OWNER_EXECUTE)
INSTALL(PROGRAMS ../../bin/log.properties DESTINATION . PERMISSIONS OWNER_EXECUTE)

##目录的安装
INSTALL(DIRECTORY ../../doc DESTINATION .)
INSTALL(DIRECTORY ../../bin/model DESTINATION .)
INSTALL(DIRECTORY ../../bin/config DESTINATION .)
#INSTALL(DIRECTORY ../../include/interface_inc/ DESTINATION include)
INSTALL(DIRECTORY ../../bin/lib/ DESTINATION lib PATTERN "*.a" EXCLUDE)
INSTALL(DIRECTORY ../../3rdparty/lib_${ARCH_DIR}/ DESTINATION 3rdparty PATTERN "*.a" EXCLUDE)