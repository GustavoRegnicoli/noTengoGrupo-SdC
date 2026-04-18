# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gustavo/esp/esp-idf/components/bootloader/subproject"
  "/home/gustavo/noTengoGrupo-SdC/tp1_rendimiento/esp32_frecuencia/build/bootloader"
  "/home/gustavo/noTengoGrupo-SdC/tp1_rendimiento/esp32_frecuencia/build/bootloader-prefix"
  "/home/gustavo/noTengoGrupo-SdC/tp1_rendimiento/esp32_frecuencia/build/bootloader-prefix/tmp"
  "/home/gustavo/noTengoGrupo-SdC/tp1_rendimiento/esp32_frecuencia/build/bootloader-prefix/src/bootloader-stamp"
  "/home/gustavo/noTengoGrupo-SdC/tp1_rendimiento/esp32_frecuencia/build/bootloader-prefix/src"
  "/home/gustavo/noTengoGrupo-SdC/tp1_rendimiento/esp32_frecuencia/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gustavo/noTengoGrupo-SdC/tp1_rendimiento/esp32_frecuencia/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gustavo/noTengoGrupo-SdC/tp1_rendimiento/esp32_frecuencia/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
