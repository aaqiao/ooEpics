# ooEpics
`ooEpics` is a C++ framework for EPICS module development. It consists of a generic EPICS device support named `InternalData` to associate EPICS records to the program's internal variables. Base classes are provided to skeleton EPICS modules, either for hardware device drivers or middle-layer application servers. `ooEpics` provides the following features:
- EPICS database can be created by creating objects of `LocalPVs` in the user code.
- Basic Channel Access functions (e.g. `caget`, `caput`, `camonitor`, w/o callbacks …) are wrapped by the `RemotePV` class. The remote channel names can be managed by a configuration file (optional).
- Provides base classes to implement finite state machines.
- Provides general IOC shell functions to create, initialize, or set up a module instance.
- Provides a template for module skeleton code (see the ooEpics_demo repository: https://github.com/aaqiao/ooEpics_demo).

## Code Compilation
Follow the steps below to compile the `ooEpics` code as a standard EPICS module.
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

