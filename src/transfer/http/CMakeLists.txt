# 添加头文件包含目录
INCLUDE_DIRECTORIES(".")

#当前目录及其下子目录下所有源文件保存变量中
file(GLOB_RECURSE src_files *.h *.cpp *.c *.cc) 
create_filters(src_files)

# 生成静态库目标
add_library(httpservice_static STATIC ${src_files} ${CMMOBJ_SRC})

# 指定静态库的输出名称
set_target_properties(httpservice_static PROPERTIES OUTPUT_NAME "httpservice")

# 链接第三方库
##生成的静态库不需要显式链接第三方的静态或动态，在最后可执行文件生成时按动态库和静态库的方法进行链接
