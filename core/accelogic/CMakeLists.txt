# Copyright (C) 1995-2019, Rene Brun and Fons Rademakers.
# All rights reserved.
#
# For the licensing terms see $ROOTSYS/LICENSE.
# For the list of contributors see $ROOTSYS/README/CREDITS.

############################################################################
# CMakeLists.txt file for building ROOT core/accelogic package
############################################################################

target_sources(Core PRIVATE
    src/ZipAccelogic.cxx)

target_link_libraries(Core PRIVATE ${LIBACCELOGICBLAST_LIBRARIES})

target_include_directories(Core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
  $<BUILD_INTERFACE:${LIBACCELOGICBLAST_INCLUDE_DIR}>
)

ROOT_INSTALL_HEADERS()
