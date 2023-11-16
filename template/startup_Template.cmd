#######################################################################
# Startup script for module __MODULE_NAME__
#
# The following parameters should to be passed in by environment variables:
#    - MODULE_INST_NAME
#    - THREAD_PRIORITY
#    - RPVMAP_PATH
#
# Auto created from the template of ooEpics framework
#######################################################################

#---------------------------------------------------
# Other settings before iocInit
#---------------------------------------------------
callbackSetQueueSize(4096)

#---------------------------------------------------
# Create the modules
#---------------------------------------------------
# Create an instance of a module
# Parameters: Module_Type, Module_Name, Module_Info, Thread_Priority
createModule("__MODULE_NAME__", "$(MODULE_INST_NAME)", "", $(THREAD_PRIORITY))

# Generate database file
# Parameters: Module_Name, File_Path, Database_File_Name
INTD_generateRecords("$(MODULE_INST_NAME)", "/tmp", "__MODULE_NAME__-$(MODULE_INST_NAME).db");

# Generate the autosave request file
# Parameters: moduleName, path, fileName, withMacro (0 - use moduleName, 1 - use macro, 2 - use destModuleName), 
#             sel (0 - scalar, 1 - waveform, 2 - all), destModuleName
INTD_generateReqFile("$(MODULE_INST_NAME)", ".", "__MODULE_NAME__-$(MODULE_INST_NAME)_set1.req", 2, 0, "$(MODULE_INST_NAME)")
INTD_generateReqFile("$(MODULE_INST_NAME)", ".", "__MODULE_NAME__-$(MODULE_INST_NAME)_set2.req", 2, 1, "$(MODULE_INST_NAME)")

# Load database (standard EPICS routine)
# Parameters: Database_File_Name, Macros
dbLoadRecords("/tmp/__MODULE_NAME__-$(MODULE_INST_NAME).db", "name_space=,module_name=$(MODULE_INST_NAME)")

#---------------------------------------------------
# Setup the modules
#---------------------------------------------------
# Set the file containing PV names to be connected with Channel Access
# Parameters: Module_Name, Command_Set_RemotePV_Map_File, Folder, RemotePV_Map_File_Name, File_Specific_Macros
setModule("$(MODULE_INST_NAME)", "SET_RPVMAP_FILE", "$(RPVMAP_PATH)", "RPVMaps___MODULE_NAME__.txt", "module_name=$(MODULE_INST_NAME)");

# Connect the remote PVs with channel access
# Parameters: Module_Name, Command_Connect_RemotePVs
setModule("$(MODULE_INST_NAME)", "CONN_RPV");

# Init the module and start the thread
# Parameters: Module_Name
initModule("$(MODULE_INST_NAME)")



