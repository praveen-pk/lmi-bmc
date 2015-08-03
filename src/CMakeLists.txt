set(PROVIDER_NAME LMI_BMC)
set(LIBRARY_NAME cmpiLMIBmc)
set(MOF LMI_BMC.mof)
set (CIM_CLASSES LMI_BMC)

set(CMAKE_C_FLAGS " -Wformat-security -g3  -Wl,--no-undefined ${CMAKE_C_FLAGS} ")

set(LmiBmc_SRCS
    LmiBmc.c
)

konkretcmpi_generate(${MOF}
                     CIM_PROVIDERS
                     CIM_HEADERS
                    CIM_CLASSES
)

add_library(${LIBRARY_NAME} SHARED
            ${LmiBmc_SRCS}
            ${CIM_PROVIDERS}
            ${CIM_HEADERS}
)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMPI_INCLUDE_DIR} ${GLIB_INCLUDE_DIRS}
    ${KONKRETCMPI_INCLUDE_DIR}
)

target_link_libraries(${LIBRARY_NAME}  openlmicommon ${KONKRETCMPI_LIBRARIES})

# Create registration file
cim_registration(${PROVIDER_NAME} ${LIBRARY_NAME} ${MOF} "")

# A temporary fix for the registration file.
add_custom_command ( TARGET ${LIBRARY_NAME}
		     POST_BUILD
		     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/fix_reg.sh ${REG}
		     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS ${LIBRARY_NAME} DESTINATION /usr/lib${LIB_SUFFIX}/cmpi/)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/LMI_BMC.reg DESTINATION share/openlmi-providers/)
