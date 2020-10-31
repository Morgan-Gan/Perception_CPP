# 添加源码需包含的公共头文件目录
SET(CMMOBJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common/cmmobj)
SET(TEMPLATE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common/template)
INCLUDE_DIRECTORIES(${CMMOBJ_DIR})
INCLUDE_DIRECTORIES(${TEMPLATE_DIR})

# 添加公共对象源码路径(用于需要用到公共对象的模块编译链接)
SET(CMMOBJ_SRC "")
add_subdirectory(common)

# 添加编译目录
add_subdirectory(alg)
add_subdirectory(app)
add_subdirectory(db)
add_subdirectory(transfer)
add_subdirectory(main)