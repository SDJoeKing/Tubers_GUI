# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/Tubers_GUI_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/Tubers_GUI_autogen.dir/ParseCache.txt"
  "Tubers_GUI_autogen"
  )
endif()
