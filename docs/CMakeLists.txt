find_package(Doxygen)

if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
    set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)

    set(DOXYGEN_PROJECT_NAME "imx93-peripheral-verification-tool")
    set(DOXYGEN_PROJECT_VERSION ${PROJECT_VERSION})
    set(DOXYGEN_PROJECT_BRIEF "NXP FRDM-IMX93 Hardware Peripheral Verification Tool")
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_HTML_OUTPUT html)
    set(DOXYGEN_GENERATE_LATEX NO)

    doxygen_add_docs(doxygen
      ${CMAKE_CURRENT_SOURCE_DIR}/../include
      ${CMAKE_CURRENT_SOURCE_DIR}/../libs
      ${CMAKE_CURRENT_SOURCE_DIR}/../app
      COMMENT "Generate documentation"
    )
else()
    message(WARNING "Doxygen not found. Documentation will not be generated. Install Doxygen to enable documentation generation.")
endif()