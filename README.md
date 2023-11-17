# ooEpics
`ooEpics` is a C++ framework for EPICS module development. It consists of a generic EPICS device support named `InternalData` to associate EPICS records to the program's internal variables. Base classes are provided to skeleton EPICS modules, either for hardware device drivers or middle-layer application servers. `ooEpics` provides the following features:
- EPICS database can be created by creating objects of `LocalPV`s in the user code.
- Basic Channel Access functions (e.g. `caget`, `caput`, `camonitor`, w/o callbacks …) are wrapped by the `RemotePV` class. The remote channel names can be managed by a configuration file (optional).
- Provides base classes to implement finite state machines.
- Provides general IOC shell functions to create, initialize, or set up a module instance.
- Provides a template for module skeleton code (see the ooEpics_demo repository: https://github.com/aaqiao/ooEpics_demo).

## ooEpics Compilation
Follow the steps below to compile the `ooEpics` code as a standard EPICS module:
1. Download and compile the EPICS base (tested successfully with version 3.14.12 and 7.0.7).
2. Edit the file `configure/RELEASE` to assign the absolute path of EPICS base (change to your EPICS base installation location):
   ```
   EPICS_BASE=/home/aqiao/epics/base-7.0.7
   ```
3. Edit the file `configure/CONFIG_SITE` to define the cross-compiler targets (change to your target architectures, if left empty, will only compile for the host architecture):
   ```
   CROSS_COMPILER_TARGET_ARCHS = vxWorks-68040
   ```
4. Go to the top folder of `ooEpics` and type `make` to compile it.

## ooEpics-based Module Code Generation
`ooEpics` provides templates and a script (based on `sed`) to generate codes as a starting point to implement EPICS modules. Follow the steps below:
1. Define a Linux environment variable pointing to the path of the folder `ooEpics/template` where the code templates are stored.
   ```
   export OOEPICS_TEMPLATE_PATH=[path of ooEpics/template]
   ```
2. Make a folder you want to store the generated EPICS modules and enter that folder.
   ```
   mkdir module_top
   cd module_top
   ```
3. Execute the script `ooEpicsGen.sh` (assuming its path is included in the searching paths of Linux) with a module name as the input parameter.
   ```
   ooEpicsGen.sh [module-name]
   ```
4. Now the module code has been generated, to compile it, you need to edit the `[module-name]/configure/RELEASE` to assign the correct paths of `EPICS_BASE` and `OOEPICS`.

## Disclaimer
The `ooEpics` module (including the codes and documents in this repository) comes with absolutely NO warranty. The author of the `ooEpics` module does not take any responsibility for any data loss, damage to equipments, or personnel injury that may result from the use of the `ooEpics` module.

