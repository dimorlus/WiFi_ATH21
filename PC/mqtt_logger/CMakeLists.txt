cmake_minimum_required(VERSION 3.16)
project(mqtt_logger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# prefer static linkage via vcpkg unless overridden
if(NOT DEFINED VCPKG_LIBRARY_LINKAGE)
  set(VCPKG_LIBRARY_LINKAGE "static" CACHE STRING "vcpkg library linkage")
endif()

# If VCPKG_ROOT is set, use its toolchain automatically
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Sources
set(SOURCES
    src/main.cpp
    src/config.cpp
    src/logger.cpp
    src/replace_macros.cpp   # <-- add this line
)

add_executable(mqtt_logger ${SOURCES})
# ensure replace_macros.cpp is compiled into the target
target_sources(mqtt_logger PRIVATE src/replace_macros.cpp)

# find packages provided by vcpkg
find_package(PahoMqttCpp CONFIG REQUIRED)
find_package(eclipse-paho-mqtt-c CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# Prefer static targets if available, fall back to shared targets
if(TARGET PahoMqttCpp::paho-mqttpp3-static)
  set(PAHO_CPP_TARGET PahoMqttCpp::paho-mqttpp3-static)
else()
  set(PAHO_CPP_TARGET PahoMqttCpp::paho-mqttpp3)
endif()

if(TARGET eclipse-paho-mqtt-c::paho-mqtt3as-static)
  set(PAHO_C_TARGET eclipse-paho-mqtt-c::paho-mqtt3as-static)
elseif(TARGET eclipse-paho-mqtt-c::paho-mqtt3a-static)
  set(PAHO_C_TARGET eclipse-paho-mqtt-c::paho-mqtt3a-static)
else()
  set(PAHO_C_TARGET eclipse-paho-mqtt-c::paho-mqtt3a) # fallback
endif()

target_link_libraries(mqtt_logger PRIVATE
    ${PAHO_CPP_TARGET}
    ${PAHO_C_TARGET}
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Windows-specific system libs
if(WIN32)
  target_link_libraries(mqtt_logger PRIVATE ws2_32)
endif()

# Try to reduce MinGW runtime DLLs
if(MINGW)
  # keep static-libgcc/libstdc++ as well
  target_link_options(mqtt_logger PRIVATE "-static-libgcc" "-static-libstdc++")

  # Path to real static archive (adjust if needed)
  set(WINPTHREAD_STATIC_PATH "C:/Qt/Tools/mingw1120_64/x86_64-w64-mingw32/lib/libwinpthread.a")

  if(EXISTS "${WINPTHREAD_STATIC_PATH}")
    message(STATUS "Forcing inclusion of static winpthread archive (whole-archive): ${WINPTHREAD_STATIC_PATH}")
    # Force the linker to include all objects from the static archive regardless of order
    # We add the flags as link options so the archive is embedded into the final exe.
    target_link_options(mqtt_logger PRIVATE
      "-Wl,--whole-archive"
      "-Wl,${WINPTHREAD_STATIC_PATH}"
      "-Wl,--no-whole-archive"
    )
  else()
    message(STATUS "Static libwinpthread.a not found at expected path; keeping fallback (DLL copy may be needed)")
  endif()
endif()

# Ensure libwinpthread-1.dll is copied to output so exe runs on clean machines
if(MINGW)
  set(WINPTHREAD_DLL "C:/Qt/Tools/mingw1120_64/bin/libwinpthread-1.dll")
  if(EXISTS "${WINPTHREAD_DLL}")
    add_custom_command(TARGET mqtt_logger POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${WINPTHREAD_DLL}"
        $<TARGET_FILE_DIR:mqtt_logger>
      COMMENT "Copying libwinpthread-1.dll to output directory")
  else()
    message(STATUS "libwinpthread-1.dll not found at ${WINPTHREAD_DLL}; skipping copy")
  endif()
endif()