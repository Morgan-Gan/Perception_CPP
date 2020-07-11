#定义宏，按目录结构加入文件
macro(create_filters source_files)
    # 获取当前目录
    set(current_dir ${CMAKE_CURRENT_SOURCE_DIR})
    foreach(src_file ${${source_files}})
        # 求出相对路径
        string(REPLACE ${current_dir}/ "" rel_path_name ${src_file})

        # 删除相对路径中的文件名部分
        string(REGEX REPLACE "(.*)/.*" \\1 rel_path ${rel_path_name})

        # 比较是否是当前路径下的文件
        string(COMPARE EQUAL ${rel_path_name} ${rel_path} is_same_path)

        # 替换成平台的路径分隔符
        string(REPLACE "/" "\\" rel_path ${rel_path})
        if(is_same_path)
            set(rel_path "\\")
        endif(is_same_path)

        # CMake 命令
        source_group(${rel_path} FILES ${src_file})
    endforeach(src_file)
endmacro(create_filters)	