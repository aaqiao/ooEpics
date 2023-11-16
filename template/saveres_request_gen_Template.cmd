#######################################################################
# The script to be executed by the iocsh to generate the request files
# for an instance of module __MODULE_NAME__ 
#
# The following parameters should to be passed in by environment variables:
#    - $(MODULE_TEMPLATE)       : an template of the instance of the module needs request config files
#    - $(SR_REQ_PATH)           : path for save/restore request files
#    - $(REQ_FILE_PREFIX)       : file name prefix for the save/restore request
#    - $(AR_CFG_PATH)           : path for archiver configure files
#    - $(CFG_FILE_PREFIX)       : file name prefix for archiver configure files
#    - $(DEST_MODULE)           : appear as the module name in the request (or config) files         
#
# Auto created from the template of ooEpics framework
#######################################################################

#---------------------------------------------------
# Generate autosave request files
#---------------------------------------------------
# Input: 
#   moduleName, 
#   path, 
#   fileName, 
#   withMacro (0 - use moduleName, 1 - use macro, 2 - use destModuleName), 
#   sel (0 - scalar, 1 - waveform, 2 - all)
#   destModuleName
INTD_generateReqFile("$(MODULE_TEMPLATE)", "$(SR_REQ_PATH)", "$(DEST_MODULE)_$(REQ_FILE_PREFIX)_set1.req", 2, 0, "$(DEST_MODULE)")
INTD_generateReqFile("$(MODULE_TEMPLATE)", "$(SR_REQ_PATH)", "$(DEST_MODULE)_$(REQ_FILE_PREFIX)_set2.req", 2, 1, "$(DEST_MODULE)")

#---------------------------------------------------
# Generate archiver configuration files
#---------------------------------------------------
# Input: 
#   moduleName, 
#   path, 
#   fileName, 
#   archiveMethodStr, 
#   withMacro (0 - use moduleName, 1 - use macro, 2 - use destModuleName), 
#   sel (0 - ST, 1 - MT, 2 - LT, 3 - SwissFEL Archiver)
#   destModuleName
INTD_generateArchCfgFile("$(MODULE_TEMPLATE)", "$(AR_CFG_PATH)", "$(DEST_MODULE)_$(CFG_FILE_PREFIX).config", "Monitor 60 600",  2, 3, "$(DEST_MODULE)")

