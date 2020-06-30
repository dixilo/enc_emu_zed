set cpu_name ps7_cortexa9_0
set hdf_file system_wrapper.hdf


# Initialization
setws .
createhw -name hw -hwspec $hdf_file

# Creation of BSP (board support package)
createbsp -name bsp -proc $cpu_name -hwproject hw -os standalone

## LWIP
#setlib -hw hw -bsp bsp -lib lwip202
setlib -hw hw -bsp bsp -lib xilffs
setlib -hw hw -bsp bsp -lib xilrsa
updatemss -hw hw -mss bsp/system.mss
regenbsp -hw hw -bsp bsp

# Creation of app
createapp -name sw -hwproject hw -proc $cpu_name -bsp bsp -app {Empty Application}
file copy -force main.c sw/src/
file copy -force platform_config.h sw/src/
file copy -force platform.h sw/src/
file copy -force platform.c sw/src/
#file copy -force platform_zynq.c sw/src/

createapp -name fsbl -hwproject hw -proc $cpu_name -bsp bsp -app {Zynq FSBL}

configapp -app sw build-config release
configapp -app fsbl build-config release

# compile
projects -build
exec bootgen -arch zynq -image sw.bif -w -o BOOT.bin
