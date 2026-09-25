vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO alandtse/CommonLibVR
    REF a898f469851c464d05137bb74b069dd234897643
    SHA512 ae454f745b31761716a40f9dfdecec806c8bd2c794067e9b05c60a7b2301c9130b1041d63d3c4ca187ec4bc6d579730f8c9bb07c79fa5e11aa7e925ecdd9dff8
    HEAD_REF NG
)


vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH2
    REPO ValveSoftware/openvr
    REF 0924064316de3effbcd1acf1e309182a2deb1c05
    SHA512 c892966d5e17b96913c6eecde86581b8e7704f6b0fdd6c629c5cae36d43d6ff313991ac529246af5862682c7e550afc949e16c4f7c62ba5bfb372910aa0c0b0a
    HEAD_REF master
)

file(GLOB OPENVR_FILES "${SOURCE_PATH2}/*")

file(COPY ${OPENVR_FILES} DESTINATION "${SOURCE_PATH}/extern/openvr")

vcpkg_configure_cmake(
    SOURCE_PATH "${SOURCE_PATH}"
    PREFER_NINJA
    OPTIONS -DBUILD_TESTS=off -DSKSE_SUPPORT_XBYAK=on
)

vcpkg_install_cmake()
vcpkg_cmake_config_fixup(PACKAGE_NAME CommonLibSSE CONFIG_PATH lib/cmake)
vcpkg_copy_pdbs()

file(GLOB CMAKE_CONFIGS "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/CommonLibSSE/*.cmake")
file(INSTALL ${CMAKE_CONFIGS} DESTINATION "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE")
file(INSTALL "${SOURCE_PATH}/cmake/CommonLibSSE.cmake" DESTINATION "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/CommonLibSSE")
