add_subdirectory(receive_filter)

set(FILTER demo_dataexchange_raw)

adtf_add_executable(FILTER stdafx.h stdafx.cpp dataexchange_raw.cpp)
adtf_install_program(FILTER ${CMAKE_INSTALL_PREFIX}/filter)
#adtf_set_folder(demo_dataexchange_raw examples/misc/data_exchange_raw)


#-----
#adtf_add_executable(demo_dataexchange_raw stdafx.h stdafx.cpp dataexchange_raw.cpp)
#adtf_install_program(demo_dataexchange_raw src/examples/bin)

#adtf_set_folder(demo_dataexchange_raw examples/misc/data_exchange_raw)
