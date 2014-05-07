set(PROVIDER_NAME LMI_BMC)
set(LIBRARY_NAME cmpiLMIBmc)
set(MOF LMI_BMC.mof)
set (CIM_CLASSES LMI_BMC)

#set(LmiBmc_SRCS
#    LmiBmc.c
#)

konkretcmpi_generate(${MOF}
                     CIM_PROVIDERS
                     CIM_HEADERS
                    CIM_CLASSES
)

add_library(${LIBRARY_NAME} SHARED
            ${tutorial_SRCS}
            ${CIM_PROVIDERS}
            ${CIM_HEADERS}
)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMPI_INCLUDE_DIR}
    ${KONKRETCMPI_INCLUDE_DIR}
)

target_link_libraries(${LIBRARY_NAME} ${KONKRETCMPI_LIBRARIES})

# Create registration file
cim_registration(${PROVIDER_NAME} ${LIBRARY_NAME} ${MOF} "")

install(TARGETS ${LIBRARY_NAME} DESTINATION /usr/lib${LIB_SUFFIX}/cmpi/)

