# 追加公共对象源码路径
SET(ADD_COMM_OBJ_SRC "")
SET(AAD_OBJ_NAME_LST log4cxx mutex)
add_cmm_obj(${CMMOBJ_DIR} "${AAD_OBJ_NAME_LST}")

# 修改父目录普通变量(通过在最后添加PARENT_SCOPE,否则无法修改)
SET(CMMOBJ_SRC ${ADD_COMM_OBJ_SRC} PARENT_SCOPE)