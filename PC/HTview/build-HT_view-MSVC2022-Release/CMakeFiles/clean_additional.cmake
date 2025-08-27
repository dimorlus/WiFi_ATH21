# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\HT_view_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\HT_view_autogen.dir\\ParseCache.txt"
  "HT_view_autogen"
  )
endif()
