####################################################################################################
#
####################################################################################################


####################################################################################################
#Directories for library files 
####################################################################################################
vob_lib_dirs :=

####################################################################################################
#library files
####################################################################################################
vob_lib_files :=

####################################################################################################
#Directories for include files
####################################################################################################
include $(CODEC_MK_FILE_PATH)/dsp_code_gu_modem_core_med.inc

####################################################################################################
#Directories for source files
####################################################################################################
vob_src_dirs := $(CODEC_SRC_FILE_PATH)/drv/src

####################################################################################################
#Source files
####################################################################################################
vob_src_files :=$(vob_src_dirs)/drv_dma.c        \
	$(vob_src_dirs)/drv_ipc.c        \
	$(vob_src_dirs)/drv_mailbox.c    \
	$(vob_src_dirs)/drv_sio.c        \
	$(vob_src_dirs)/drv_timer.c      \
	$(vob_src_dirs)/drv_watchdog.c   \
	$(vob_src_dirs)/drv_uart.c       \
	

####################################################################################################
#general make rules
####################################################################################################


####################################################################################################
#
####################################################################################################


####################################################################################################
#
####################################################################################################