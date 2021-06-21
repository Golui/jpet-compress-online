############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
############################################################
open_project jpet-online-compress
set_top fpga_compress
add_files jpet-online-compress/source/ansu_fpga.hpp -cflags "-Ijpet-online-compress/lib/ansu/include -Wl,--no-undefined"
add_files jpet-online-compress/source/ansu_fpga.cpp -cflags "-Ijpet-online-compress/lib/ansu/include -Ijpet-online-compress/lib/ansu/include  -Wl,--no-undefined"
add_files -tb jpet-online-compress/lib/ansu/src/common/io/archive.cpp -cflags "-Ijpet-online-compress/lib/ansu/include -Ijpet-online-compress/source -Ijpet-online-compress/lib/ansu/cli/lib/CLI11/include -Ijpet-online-compress/lib/ansu/lib/cereal/include -Wno-unknown-pragmas"
add_files -tb jpet-online-compress/testbench/driver.hpp -cflags "-Ijpet-online-compress/lib/ansu/include -Ijpet-online-compress/source -Ijpet-online-compress/lib/ansu/cli/lib/CLI11/include -Ijpet-online-compress/lib/ansu/lib/cereal/include -Wno-unknown-pragmas"
add_files -tb jpet-online-compress/testbench/test.cpp -cflags "-Ijpet-online-compress/lib/ansu/include -Ijpet-online-compress/source -Ijpet-online-compress/lib/ansu/cli/lib/CLI11/include -Ijpet-online-compress/lib/ansu/lib/cereal/include -Wno-unknown-pragmas"
add_files -tb jpet-online-compress/lib/ansu/src/util.cpp -cflags "-Ijpet-online-compress/lib/ansu/include -Ijpet-online-compress/source -Ijpet-online-compress/lib/ansu/cli/lib/CLI11/include -Ijpet-online-compress/lib/ansu/lib/cereal/include -Wno-unknown-pragmas"
open_solution "solution1" -flow_target vivado
set_part {xczu9eg-ffvb1156-2-e}
create_clock -period 8 -name default
source "./jpet-online-compress/solution1/directives.tcl"
csim_design -argv {compress -f ../../../res/all.dij -o ../../../out/all.ansu -s human} -clean -setup
csynth_design
cosim_design
export_design -format ip_catalog
