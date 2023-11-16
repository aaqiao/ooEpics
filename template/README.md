# Module __MODULE_NAME__ based on ooEpics
This is an application module based on `ooEpics`. The module name is defined in `__MODULE_NAME__App/src/Makefile`. In addition, the module name should be also passed in `ModuleConfig___MODULE_NAME__` as a parameter to the parent class `ModuleConfig` defined in the `ooEpics` module. In this demo module, the following features of `ooEpics` are demonstrated:
- `Coordinator`: active class with a thread as the engine for executing the module functions (jobs).
- `FSM`: a simple state machine implementation driven by a separate thread.
- `Job`: functions triggered by events of PV writing (clicking a button on GUI).
- `Service`: wrapper of remote device access via Channel Access.

## Code Compilation
Follow the steps below to compile the `__MODULE_NAME__` code as a standard EPICS module.
1. Edit the file `configure/RELEASE` to assign the absolute paths of EPICS base and `ooEpics` (change to your installation locations):
   ```
   EPICS_BASE=/home/aqiao/epics/base-7.0.7
   OOEPICS=/home/aqiao/code_pub/ooEpics
   ```
3. Edit the file `configure/CONFIG_SITE` to define the cross-compiler targets (change to your target architectures, if left empty, will only compile for the host architecture):
   ```
   CROSS_COMPILER_TARGET_ARCHS = vxWorks-68040
   ```
4. Go to the folder of `module` and type `make` to compile it.

