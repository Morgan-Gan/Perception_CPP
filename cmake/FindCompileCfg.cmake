# 自动选择平台对应的编译器类型
SET(COMPLIE_PREFIX "" CACHE STRING INTERNAL)
IF (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    SET(COMPLIE_PREFIX "")
    SET(ARCH_DIR "x86_64")
ELSE()
    SET(ARCH_DIR "arm")
    SET(COMPLIE_PREFIX "")
ENDIF()

SET(CMAKE_C_COMPILER "${COMPLIE_PREFIX}gcc")
SET(CMAKE_CXX_COMPILER "${COMPLIE_PREFIX}g++")
SET(CMAKE_STRIP "${COMPLIE_PREFIX}strip")

#编译选项设置
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -w -Wno-deprecated -Wwrite-strings -fno-omit-frame-pointer -rdynamic -pthread")

#这个就是定义各个版本对应的后缀，例如d，debug版本后缀，当然你想定义为其他，	#自己修改这块就可以了。
SET(CMAKE_DEBUG_POSTFIX "" CACHE STRING "add a postfix, usually d on windows")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
SET(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "add a postfix, usually empty on windows")
SET(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "add a postfix, usually empty on windows")
 
# Set the build postfix extension according to what configuration is being built.
SET(CMAKE_BUILD_TYPE "Debug")   
IF (CMAKE_BUILD_TYPE MATCHES "Release")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_RELEASE_POSTFIX}")
ELSEIF(CMAKE_BUILD_TYPE MATCHES "Debug")    
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
ELSEIF(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_RELWITHDEBINFO_POSTFIX}")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_MINSIZEREL_POSTFIX}")
ELSE()
    SET(CMAKE_BUILD_POSTFIX "")
ENDIF()

# Set the library extension according to what configuration is being built.
IF(CMAKE_DEBUG_POSTFIX)
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DRW_LIBRARY_POSTFIX=${CMAKE_DEBUG_POSTFIX}")
    SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
ENDIF()

IF(CMAKE_RELEASE_POSTFIX)
    SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRW_LIBRARY_POSTFIX=${CMAKE_RELEASE_POSTFIX}")
ENDIF()

IF(CMAKE_RELWITHDEBINFO_POSTFIX)
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DRW_LIBRARY_POSTFIX=${CMAKE_RELWITHDEBINFO_POSTFIX}")
ENDIF()

IF(CMAKE_MINSIZEREL_POSTFIX)
    SET(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DRW_LIBRARY_POSTFIX=${CMAKE_MINSIZEREL_POSTFIX}")
ENDIF()

#编译器查找根目录
SET(CMAKE_FIND_ROOT_PATH /usr)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)