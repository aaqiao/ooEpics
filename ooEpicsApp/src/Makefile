##########################################################
# Makefile
# 
# Makefile for ooEpics module
# 
# Created by Zheqiao Geng (gengzheqiao@126.com) on 26.05.2016
##########################################################
TOP=../..

include $(TOP)/configure/CONFIG

#==================================================
# source code folders
#==================================================
OOEPICS_INTD = $(TOP)/ooEpicsApp/src/InternalData
OOEPICS_COMMON = $(TOP)/ooEpicsApp/src/Common

SRC_DIRS += $(OOEPICS_INTD)
SRC_DIRS += $(OOEPICS_COMMON)

USR_DBDFLAGS += -I $(OOEPICS_INTD)
USR_DBDFLAGS += -I $(OOEPICS_COMMON)

#==================================================
# build a support library
#==================================================
# library name
LIBRARY_IOC += ooEpics

# DBD files
DBD += ooEpics.dbd

ooEpics_DBD += base.dbd

ooEpics_DBD += InternalData_dev.dbd
ooEpics_DBD += InternalData_iocShell.dbd

ooEpics_DBD += ooEpicsRegister.dbd

# include files
INC += InternalData.h
INC += recordGenerate.h

INC += Application.h
INC += ChannelAccess.h
INC += ControlDevice.h
INC += Coordinator.h
INC += DomainDevice.h
INC += EPICSLib_wrapper.h
INC += FSM.h
INC += Job.h
INC += LocalPV.h
INC += MessageLogs.h
INC += ModuleConfig.h
INC += ModuleManager.h
INC += ooEpicsMisc.h
INC += RemotePV.h
INC += Service.h

# source files
ooEpics_SRCS += initHooks.c
ooEpics_SRCS += InternalData.c
ooEpics_SRCS += InternalData_devAi.c
ooEpics_SRCS += InternalData_devAo.c
ooEpics_SRCS += InternalData_devBi.c
ooEpics_SRCS += InternalData_devBo.c
ooEpics_SRCS += InternalData_devLi.c
ooEpics_SRCS += InternalData_devLo.c
ooEpics_SRCS += InternalData_devMbbi.c
ooEpics_SRCS += InternalData_devMbbo.c
ooEpics_SRCS += InternalData_devSi.c
ooEpics_SRCS += InternalData_devSo.c
ooEpics_SRCS += InternalData_devWf.c
ooEpics_SRCS += InternalData_iocShell.c

ooEpics_SRCS += Application.cc
ooEpics_SRCS += ChannelAccess.cc
ooEpics_SRCS += ControlDevice.cc
ooEpics_SRCS += Coordinator.cc
ooEpics_SRCS += DomainDevice.cc
ooEpics_SRCS += FSM.cc
ooEpics_SRCS += Job.cc
ooEpics_SRCS += LocalPV.cc
ooEpics_SRCS += MessageLogs.cc
ooEpics_SRCS += ModuleConfig.cc
ooEpics_SRCS += ModuleManager.cc
ooEpics_SRCS += ooEpicsMisc.cc
ooEpics_SRCS += RemotePV.cc
ooEpics_SRCS += Service.cc

# EPICS base library
ooEpics_LIBS += $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES


