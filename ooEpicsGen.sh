#!/bin/bash
#####################################################################
#  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
#  All rights reserved.
#  Authors: Zheqiao Geng
#####################################################################
# -------------------------------------------------------------------
# Generate the template of ooEpics based modules
# 
# Input arguments:
#     - Module name
# Pre-definition:
#     $OOEPICS_TEMPLATE_PATH    : path to contain the ooEpics template
# -------------------------------------------------------------------

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Initialization
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Get the input
MODNAME=$1

# Print the message
echo "---------------------------------------------------------------"
echo "Create framework of ooEpics based module $MODNAME"
echo "---------------------------------------------------------------"

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Generate the architecture of folders
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
WORK_DIR=$(pwd)
echo "Current folder is $WORK_DIR"

mkdir ./$MODNAME
echo "Generate folder $MODNAME"

cd $MODNAME
mkdir ./configure
mkdir ./gui
mkdir ./"$MODNAME"App
mkdir ./test_bench

PATH_TOP=$WORK_DIR/$MODNAME
PATH_CFG=$PATH_TOP/configure
PATH_GUI=$PATH_TOP/gui
PATH_APP=$PATH_TOP/"$MODNAME"App
PATH_TST=$PATH_TOP/test_bench

echo "Generate folder $PATH_CFG"
echo "Generate folder $PATH_GUI"
echo "Generate folder $PATH_APP"
echo "Generate folder $PATH_TST"
echo ""

cd $PATH_APP
mkdir ./Db
mkdir ./src
PATH_DB=$PATH_APP/Db
PATH_SRC=$PATH_APP/src
echo "Generate folder $PATH_APP/Db"
echo "Generate folder $PATH_APP/src"
echo ""

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copy or generate the template files
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
cd $PATH_APP
echo "Current path $(pwd)"
echo ""

echo "Generate in ./src module top class ..."
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/Module_Template.h  > $PATH_SRC/Module_$MODNAME.h
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/Module_Template.cc > $PATH_SRC/Module_$MODNAME.cc
echo "Module_$MODNAME.h and Module_$MODNAME.cc generated"
echo ""

echo "Generate in ./src module configure class ..."
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/ModuleConfig_Template.h  > $PATH_SRC/ModuleConfig_$MODNAME.h
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/ModuleConfig_Template.cc > $PATH_SRC/ModuleConfig_$MODNAME.cc
echo "ModuleConfig_$MODNAME.h and ModuleConfig_$MODNAME.cc generated"
echo ""

echo "Generate in ./src job coordinator class ..."
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/Coordinator_Jobs.h  > $PATH_SRC/Coordinator_Jobs.h
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/Coordinator_Jobs.cc > $PATH_SRC/Coordinator_Jobs.cc
echo "Coordinator_Jobs.h and Coordinator_Jobs.cc generated"
echo ""

echo "Generate in ./src example job class ..."
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/Job_Example.h  > $PATH_SRC/Job_Example.h
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/Job_Example.cc > $PATH_SRC/Job_Example.cc
echo "Job_Example.h and Job_Example.cc generated"
echo ""

echo "Generate in ./src example service class ..."
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/Service_Example.h  > $PATH_SRC/Service_Example.h
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/Service_Example.cc > $PATH_SRC/Service_Example.cc
echo "Service_Example.h and Service_Example.cc generated"
echo ""

echo "Generate in ./src example FSM class ..."
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/FSM_Example.h  > $PATH_SRC/FSM_Example.h
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/FSM_Example.cc > $PATH_SRC/FSM_Example.cc
echo "FSM_Example.h and FSM_Example.cc generated"
echo ""

echo "Generate in ./src module definitions ..."
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/ModuleDefs_Template.h  > $PATH_SRC/ModuleDefs_$MODNAME.h
echo "ModuleDefs_$MODNAME.h generated"
echo ""

cd $PATH_TOP
echo "Current path $(pwd)"
echo ""

echo "Generate in . remote PV name mapping file ..."
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/RPVMaps_Template.txt  > $PATH_TOP/RPVMaps_$MODNAME.txt
echo "RPVMaps_$MODNAME.txt generated"
echo ""

echo "Generate in ./gui example GUIs ..."
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/GUI_Example_Top.ui             > $PATH_GUI/GUI_Example_Top.ui
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/GUI_Example_RunTimeMessages.ui > $PATH_GUI/GUI_Example_RunTimeMessages.ui
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/start_gui                      > $PATH_GUI/start_gui
echo "GUI_Example_Top.ui generated"
echo "GUI_Example_RunTimeMessages.ui generated"
echo "start_gui generated"
echo ""

chmod +x $PATH_GUI/start_gui

echo "Generate Makefile ..."
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/Makefile     > $PATH_TOP/Makefile
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/Makefile_app > $PATH_APP/Makefile
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/Makefile_db  > $PATH_DB/Makefile
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/Makefile_src > $PATH_SRC/Makefile
echo "Makefile generated"
echo ""

echo "Generate Release_Note ..."
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/Release_Note  > $PATH_TOP/Release_Note
echo "Release_Note generated"
echo ""

echo "Generate README.md ..."
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/README.md  > $PATH_TOP/README.md
echo "README.md generated"
echo ""

echo "Generate startup script for the module ..."
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/startup_Template.cmd  > $PATH_TOP/startup_$MODNAME.cmd
echo "startup_$MODNAME.cmd generated"
echo ""

echo "Generate test IOC startup script ..."
sed "s/__MODULE_NAME__/$MODNAME/g" $OOEPICS_TEMPLATE_PATH/TestSoftIOC_startup.script  > $PATH_TST/TestSoftIOC_startup.script
echo "TestSoftIOC_startup.script generated"
echo ""

echo "Generate configuration folder for the module ..."
cp -r $OOEPICS_TEMPLATE_PATH/configure $PATH_TOP
echo "configuration folder generated"
echo ""

echo "Code generation finished for $MODNAME"
echo ""





