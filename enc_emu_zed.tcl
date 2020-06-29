## Incremental Encoder Emulator
# This project is for Zedboard
set p_device "xc7z020clg484-1"
set p_board "em.avnet.com:zed:part0:1.4"

set sys_zynq 1
set project_name enc_emu_zed

set project_system_dir "./$project_name.srcs/sources_1/bd/system"
create_project $project_name . -part $p_device -force
set_property board_part $p_board [current_project]

create_bd_design "system"

add_files -norecurse -fileset sources_1 [list \
    "enc_emu_zed.xdc" \
    "sig_gen.v"]


############## Zynq
create_bd_cell -type ip -vlnv [get_ipdefs "*processing_system7*"] sys_ps7

# Board automation
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 \
-config {make_external "FIXED_IO, DDR" apply_board_preset "1" Master "Disable" Slave "Disable" }  \
[get_bd_cells sys_ps7]


# AXI UART Lite
create_bd_cell -type ip -vlnv [get_ipdefs "*axi_uartlite*"] axi_uartlite
set_property -dict [list CONFIG.C_BAUDRATE {115200} CONFIG.PARITY {Even} CONFIG.C_USE_PARITY {1}] [get_bd_cells axi_uartlite]
apply_bd_automation \
    -rule xilinx.com:bd_rule:axi4 \
    -config { Clk_master {Auto} \
              Clk_slave {Auto} \
              Clk_xbar {Auto} \
              Master {/sys_ps7/M_AXI_GP0} \
              Slave {/axi_uartlite/S_AXI} \
              intc_ip {New AXI Interconnect} \
              master_apm {0}} \
[get_bd_intf_pins axi_uartlite/S_AXI]

make_bd_pins_external  -name uart [get_bd_pins axi_uartlite/tx]

# AXI GPIO
create_bd_cell -type ip -vlnv [get_ipdefs "*axi_gpio*"] axi_gpio
set_property -dict [list CONFIG.C_ALL_OUTPUTS {1}] [get_bd_cells axi_gpio]

# Signal generator
create_bd_cell -type module -reference sig_gen sig_gen

# automation
apply_bd_automation \
    -rule xilinx.com:bd_rule:axi4 \
    -config { \
        Clk_master {/sys_ps7/FCLK_CLK0 (100 MHz)} \
        Clk_slave {Auto} Clk_xbar {/sys_ps7/FCLK_CLK0 (100 MHz)} \
        Master {/sys_ps7/M_AXI_GP0} Slave {/axi_gpio/S_AXI} \
        intc_ip {/sys_ps7_axi_periph} \
        master_apm {0}} \
    [get_bd_intf_pins axi_gpio/S_AXI]

apply_bd_automation \
    -rule xilinx.com:bd_rule:clkrst \
    -config {Clk "/sys_ps7/FCLK_CLK0 (100 MHz)" } \
    [get_bd_pins sig_gen/clk]

# Interface pin
make_bd_pins_external  -name rot_a [get_bd_pins sig_gen/rot_a]
make_bd_pins_external  -name rot_b [get_bd_pins sig_gen/rot_b]
make_bd_pins_external  -name rot_z [get_bd_pins sig_gen/rot_z]

connect_bd_net [get_bd_pins axi_gpio/gpio_io_o] [get_bd_pins sig_gen/conf]

save_bd_design
validate_bd_design

set_property synth_checkpoint_mode None [get_files  $project_system_dir/system.bd]
generate_target {synthesis implementation} [get_files  $project_system_dir/system.bd]
make_wrapper -files [get_files $project_system_dir/system.bd] -top


import_files -force -norecurse -fileset sources_1 $project_system_dir/hdl/system_wrapper.v
set_property top system_wrapper [current_fileset]


# Synthesize
launch_runs synth_1
wait_on_run synth_1
open_run synth_1
report_timing_summary -file timing_synth.log

# Implementation
launch_runs impl_1 -to_step write_bitstream
wait_on_run impl_1
open_run impl_1
report_timing_summary -file timing_impl.log

# Make .sdk folder
file copy -force $project_name.runs/impl_1/system_top.sysdef noos/system_top.hdf
