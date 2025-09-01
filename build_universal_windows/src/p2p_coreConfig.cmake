
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was p2p_coreConfig.cmake.in                            ########

get_filename_component(PACKAGE_${CMAKE_FIND_PACKAGE_NAME}_COUNTER_1 "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

# 包含目标文件
include("${CMAKE_CURRENT_LIST_DIR}/p2p_coreTargets.cmake")

# 检查兼容性
check_required_components(p2p_core)

# 设置包含目录
set(p2p_core_INCLUDE_DIRS "")

# 设置库目录
set(p2p_core_LIBRARY_DIRS "")

# 设置库文件
set(p2p_core_LIBRARIES p2p_core::p2p_core)

# 版本信息
set(p2p_core_VERSION "")
set(p2p_core_VERSION_MAJOR "")
set(p2p_core_VERSION_MINOR "")
set(p2p_core_VERSION_PATCH "")

# 平台特定设置
if(WIN32)
    set(p2p_core_LIBRARY_NAME "p2p_core.dll")
    set(p2p_core_IMPORT_LIBRARY_NAME "p2p_core.lib")
else()
    set(p2p_core_LIBRARY_NAME "libp2p_core.so")
    set(p2p_core_IMPORT_LIBRARY_NAME "libp2p_core.a")
endif()

# 提供查找函数
function(find_p2p_core)
    if(NOT TARGET p2p_core::p2p_core)
        message(FATAL_ERROR "p2p_core target not found. Please ensure p2p_core is properly installed.")
    endif()
    
    # 设置变量
    set(p2p_core_FOUND TRUE PARENT_SCOPE)
    set(p2p_core_INCLUDE_DIRS ${p2p_core_INCLUDE_DIRS} PARENT_SCOPE)
    set(p2p_core_LIBRARIES ${p2p_core_LIBRARIES} PARENT_SCOPE)
endfunction()
