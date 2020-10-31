#静态库或动态库链接(通过pkg-config方式)
function(link_static_share_lib proj_name pkg_lst pkgcfg_path pkg_type)
    #重新设置pkgcfg查找路径
    SET(ENV{PKG_CONFIG_PATH} ${pkgcfg_path})
    find_package(PkgConfig)

    foreach(pkg ${pkg_lst})
        #搜索包
        string(TOUPPER ${pkg} uppername)
        pkg_search_module(${uppername} REQUIRED ${pkg})

        #根据包类型进行关联
        IF(pkg_type MATCHES "static")
            foreach(lib ${${uppername}_LINK_LIBRARIES})
                #根据配置信息，进行静态库关联
                add_library(${lib} STATIC IMPORTED)
                set_property(TARGET ${lib} PROPERTY  IMPORTED_LOCATION ${${uppername}_LIBRARY_DIRS}/lib${lib}.a)
                target_link_libraries(${proj_name} ${lib})
            endforeach(lib)
        ELSEIF(pkg_type MATCHES "share")
            #根据配置信息，进行动态库关联
            target_link_libraries(${proj_name} ${${uppername}_LDFLAGS})
        ENDIF()
    endforeach(pkg)
endfunction(link_static_share_lib)

# 追加依赖的公共对象
function(add_cmm_obj obj_dir obj_name)
    foreach(sub_name ${obj_name})
        string(TOUPPER ${sub_name} uppername)
        SET(CMMOBJ_${uppername}_DIR ${obj_dir}/${sub_name})
        file(GLOB_RECURSE ${uppername}_SRC "${CMMOBJ_${uppername}_DIR}/*.h" "${CMMOBJ_${uppername}_DIR}/*.c" "${CMMOBJ_${uppername}_DIR}/*.cpp" "${CMMOBJ_${uppername}_DIR}/*.hpp")
        create_filters(${uppername}_SRC)
        list(APPEND CMMOBJ_SRC ${${uppername}_SRC})
    endforeach(sub_name)
    SET(ADD_COMM_OBJ_SRC ${CMMOBJ_SRC} PARENT_SCOPE)
endfunction(add_cmm_obj)