# Class Documentation for `src/Common`

This document provides detailed documentation for the C++ classes in the `src/Common` directory of the `ooEpics` project.

## 1. Application (`Application.h`)
**Namespace:** `OOEPICS`

Basic class for the Application.

### Class: `Application`
#### Members
- `char name[OOEPICS_NAMESTR_LEN]`: Name of the application.

#### Methods
- `Application(const char *appName)`
  - **Description**: Constructor.
  - **Arguments**:
    - `appName`: Name of the application.
- `virtual ~Application()`
  - **Description**: Destructor.

---

## 2. ChannelAccess (`ChannelAccess.h`)
**Namespace:** `OOEPICS`

Object-Oriented wrapper for EPICS Channel Access routines.

### Class: `ChannelAccess`
#### Members
- `char pvName[CA_STRING_LEN]`: The remote PV name.
- `int caConnected`: Status of the channel access connection.
- `int caStatus`: The status code.

#### Methods
- `ChannelAccess(string pvNameIn, unsigned long reqElemsReadIn, CA_enum_readCtrl rdCtrlIn, CA_enum_writeCtrl wtCtrlIn, CAUSR_CALLBACK connUserCallbackIn, CAUSR_CALLBACK rdUserCallbackIn, CAUSR_CALLBACK wtUserCallbackIn, void *dataPtrIn, void *userPtrIn, EPICSLIB_type_mutexId mutexIdIn, EPICSLIB_type_eventId connEventIn, EPICSLIB_type_eventId rdEventIn, EPICSLIB_type_eventId wtEventIn, int callBackPriorityIn)`
  - **Description**: Constructor with extensive configuration for read/write control, callbacks, and events.
  - **Arguments**:
    - `pvNameIn`: Remote PV name.
    - `reqElemsReadIn`: Number of elements to request for reading.
    - `rdCtrlIn`: Reading control type (`CA_READ_DISABLED`, `CA_READ_PULL`, `CA_READ_CALLBACK`, `CA_READ_MONITOR`).
    - `wtCtrlIn`: Writing control type (`CA_WRITE_DISABLED`, `CA_WRITE_PULL`, `CA_WRITE_CALLBACK`).
    - `connUserCallbackIn`: User callback for connection events.
    - `rdUserCallbackIn`: User callback for read events.
    - `wtUserCallbackIn`: User callback for write events.
    - `dataPtrIn`: Pointer for monitored data.
    - `userPtrIn`: User pointer passed to callbacks.
    - `mutexIdIn`: Mutex for data buffer locking.
    - `connEventIn`, `rdEventIn`, `wtEventIn`: Events fired upon callbacks.
    - `callBackPriorityIn`: Priority of the callback.

- `~ChannelAccess()`
  - **Description**: Destructor.

- `void connect()`
  - **Description**: Sets up the connection which is managed by the general callback function.

- `int caReadRequest()`
  - **Description**: Requests the data from the PV. Results are saved in `dataBufRead`.

- `int caWriteRequestVal(epicsFloat64 dataIn)`
  - **Description**: Writes a scalar value (converted on server side).
  - **Arguments**:
    - `dataIn`: The value to write.

- `int caWriteRequestStr(char *strIn)`
- `int caWriteRequestStr(string strIn)`
  - **Description**: Writes a string value (for stringout records).

- `int caWriteRequestWfRaw(void *dataBufIn, unsigned long pointNum)`
  - **Description**: Writes a raw waveform without data type conversion.

- `int caWriteRequestWf(epicsInt8 *dataBufIn, unsigned long pointNum)`
- `int caWriteRequestWf(epicsUInt8 *dataBufIn, unsigned long pointNum)`
- `int caWriteRequestWf(epicsInt16 *dataBufIn, unsigned long pointNum)`
- `int caWriteRequestWf(epicsUInt16 *dataBufIn, unsigned long pointNum)`
- `int caWriteRequestWf(epicsInt32 *dataBufIn, unsigned long pointNum)`
- `int caWriteRequestWf(epicsUInt32 *dataBufIn, unsigned long pointNum)`
- `int caWriteRequestWf(epicsFloat32 *dataBufIn, unsigned long pointNum)`
- `int caWriteRequestWf(epicsFloat64 *dataBufIn, unsigned long pointNum)`
  - **Description**: Writes a waveform with data type conversion on the server side.

- `epicsInt8 getValueInt8()`
- `epicsUInt8 getValueUInt8()`
- `epicsInt16 getValueInt16()`
- `epicsUInt16 getValueUInt16()`
- `epicsInt32 getValueInt32()`
- `epicsUInt32 getValueUInt32()`
- `epicsFloat32 getValueFloat32()`
- `epicsFloat64 getValueFloat64()`
- `int getValueString(char *strOut)`
- `string getValueString()`
  - **Description**: Gets the read value, converting it to the requested type locally.

- `int getValuesRaw(void *dataBufOut, unsigned long pointNum)`
  - **Description**: Gets raw waveform data without conversion.

- `int getValues(epicsInt8 *dataBufOut, unsigned long pointNum)`
- `int getValues(epicsUInt8 *dataBufOut, unsigned long pointNum)`
- `int getValues(epicsInt16 *dataBufOut, unsigned long pointNum)`
- `int getValues(epicsUInt16 *dataBufOut, unsigned long pointNum)`
- `int getValues(epicsInt32 *dataBufOut, unsigned long pointNum)`
- `int getValues(epicsUInt32 *dataBufOut, unsigned long pointNum)`
- `int getValues(epicsFloat32 *dataBufOut, unsigned long pointNum)`
- `int getValues(epicsFloat64 *dataBufOut, unsigned long pointNum)`
  - **Description**: Gets waveform data, converting locally to the requested type one by one.

- `int getConnected()`
- `int getCAStatus()`
- `epicsTimeStamp getTimeStamp()`
- `void getTimeStampStr(char *tsStr)`
- `epicsInt16 getAlarmStatus()`
- `epicsInt16 getAlarmSeverity()`
- `unsigned long getRPVElemCount()`
  - **Description**: Accessors for status, timestamp, alarm info, and element count.

### Class: `ChannelAccessContext`
#### Methods
- `ChannelAccessContext(CA_enum_contextCtrl contextCtrl)`
  - **Description**: Constructor.
  - **Arguments**:
    - `contextCtrl`: `CA_SINGLE_THREAD` or `CA_MULTIPLE_THREAD`.
- `~ChannelAccessContext()`
- `int attachCurrentThread()`
  - **Description**: Attaches the current thread to this CA context.
- `int displayStatus()`

---

## 3. ControlDevice (`ControlDevice.h`)
**Namespace:** `OOEPICS`

Basic class for instrumentation devices inserted by the control system.

### Class: `ControlDevice`
#### Methods
- `ControlDevice(const char *deviceName)`
  - **Description**: Constructor.
- `virtual ~ControlDevice()`

---

## 4. Coordinator (`Coordinator.h`)
**Namespace:** `OOEPICS`

**Abstract** base class for a coordinator that manages job execution.

### Class: `Coordinator`
#### Methods
- `Coordinator(const char *modNameIn, const char *crdNameIn)`
  - **Description**: Constructor.
- `virtual ~Coordinator()`
- `virtual int execute() = 0`
  - **Description**: **Pure Virtual**. Main execution logic.
- `void registerJob(Job *job, int jobCode)`
  - **Description**: Registers a job with a specific code.
- `int executeJob(int jobCode, int flag)`
  - **Description**: Executes the job corresponding to the code.
- `void enableAllJobs(int enabled)`
  - **Description**: Enables (`1`) or disables (`0`) all registered jobs.
- `void enableJob(int jobCode, int enabled)`
  - **Description**: Enables or disables a specific job.
- `void sendEvent()`
- `void sendEvent(int eventCode, int cmd, int subCmd)`
  - **Description**: Sends an event to the coordinator.
- `void waitEvent()`
- `void waitEvent(int *eventCode, int *cmd, int *subCmd)`
- `void waitEventWithTimeout(double timeout)`
- `void waitEventWithTimeout(int *eventCode, int *cmd, int *subCmd, double timeout)`
  - **Description**: Waits for an event, optionally returning the code/cmd/subCmd and optionally with a timeout.
- `int withPendingEvents()`
  - **Description**: Checks if there are unsolved messages.

---

## 5. DomainDevice (`DomainDevice.h`)
**Namespace:** `OOEPICS`

Basic class for a physical device in a specific domain.

### Class: `DomainDevice`
#### Methods
- `DomainDevice(const char *deviceName)`
  - **Description**: Constructor.
- `virtual ~DomainDevice()`

---

## 6. FSM (`FSM.h`)
**Namespace:** `OOEPICS`

**Abstract** base class for a Finite State Machine.

### Class: `FSM`
#### Methods
- `FSM(const char *modNameIn, const char *fsmNameIn)`
  - **Description**: Constructor.
- `virtual ~FSM()`
- `int executeFSM()`
  - **Description**: Main FSM execution loop.
- `void printFSM()`
- `int registerState(State *state, int stateCode)`
  - **Description**: Registers a new state instance.
- `int initCurrentState(int stateCode, int sel)`
  - **Description**: Sets the initial state. `sel`: `FSM_INIT_CURR_ENTRY_EXE`, `FSM_INIT_CURR_ENTRY_NOEXE`, or `FSM_INIT_CURR_FORCE_TRANS`.
- `int setDefaultState(int stateCode)`
- `int getCurrentStateCode()`
- `void registerJob(Job *job, int jobCode)`
- `int executeJob(int jobCode, int flag)`
- `void enableAllJobs(int enabled)`
- `void enableJob(int jobCode, int enabled)`
- `void sendEvent(...)`, `waitEvent(...)`, `waitEventWithTimeout(...)`: Same as Coordinator.
- `int isMsgQFull()`
- `virtual int initFSM() = 0`
  - **Description**: **Pure Virtual**. Initialization logic, usually putting FSM into default state.
- `virtual int executeExtFunc() = 0`
  - **Description**: **Pure Virtual**. Extended execution function.

### Class: `State`
**Abstract** base class for a state in the FSM.

#### Methods
- `State(int stateCodeIn, const char *stateNameIn)`
- `virtual ~State()`
- `virtual int entry() = 0`
  - **Description**: **Pure Virtual**. Entry action.
- `virtual int do_activity() = 0`
  - **Description**: **Pure Virtual**. Main state activity. Returns transition status.
- `virtual int exit() = 0`
  - **Description**: **Pure Virtual**. Exit action.
- `void printState()`
- `void setSourceStateCode(int prevStateCode)`
- `void setDestStateCode(int nextStateCode)`
- `int getStateCode()`
- `int getSourceStateCode()`
- `int getDestStateCode()`
- `void initDelay()`
  - **Description**: Initializes the state timer.
- `int withDelay(double timeout)`
  - **Description**: Checks if `timeout` seconds have passed since `initDelay` (entry).
- `double getTimeExpectEvent()`
- `void clearTimeExpectEvent()`

### Class: `FSMEvent`
#### Methods
- `FSMEvent()`
- `~FSMEvent()`
- `int sendEvent()`
- `int sendEvent(int eventCodeIn, int cmdIn, int subCmdIn)`
- `void recvEvent()`
- `void recvEvent(int *eventCodeOut, int *cmdOut, int *subCmdOut)`
- `void recvEventWithTimeout(double timeout)`
- `void recvEventWithTimeout(int *eventCodeOut, int *cmdOut, int *subCmdOut, double timeOut)`
- `int getUnsolvedMsgNum()`

---

## 7. Job (`Job.h`)
**Namespace:** `OOEPICS`

**Abstract** base class for a job.

### Class: `Job`
#### Methods
- `Job(const char *modNameIn, const char *jobNameIn)`
- `virtual ~Job()`
- `void enableJob(short enabled)`
- `virtual void enableJobExt() = 0`
  - **Description**: **Pure Virtual**. Extended enable function by user.
- `virtual int execute(int flag) = 0`
  - **Description**: **Pure Virtual**. Execute the job.

---

## 8. LocalPV (`LocalPV.h`)
**Namespace:** `OOEPICS`

Class definition for a Local Process Variable.

### Class: `LocalPV`
#### Methods
- `LocalPV()`
- `LocalPV(...)`: Deprecated constructors for old interface (many overloads for different data types).
- `~LocalPV()`

- `void init(string moduleName, string devName, string valName, string supStr, string unitStr, unsigned int pointNum, void *privatePtr, INTD_CALLBACK callback, INTD_enum_recordType recordType, INTD_enum_scanType scanType, IOSCANPVT *ioIntScan, epicsMutexId mutexId, epicsEventId eventId)`
  - **Description**: Initializes the Local PV with full configuration.

- `epicsInt8 getValueInt8()`
- `epicsUInt8 getValueUInt8()`
- `epicsInt16 getValueInt16()`
- `epicsUInt16 getValueUInt16()`
- `epicsInt32 getValueInt32()`
- `epicsUInt32 getValueUInt32()`
- `epicsFloat32 getValueFloat32()`
- `epicsFloat64 getValueFloat64()`
- `int getValueString(char *strOut)`
- `string getValueString()`
  - **Description**: Gets the value converted to the requested type.

- `double getField(string fieldName)`
- `int putField(string fieldName, double data)`

- `int setValue(epicsFloat64 dataIn)`
- `int setValue(epicsFloat64 dataIn, LPV_enum_BIAlarm alarmSel)`
- `int setString(char *strIn)`
- `int setString(string strIn)`

- `int getValuesRaw(void *dataOut, unsigned int pointNum)`
- `int getValues(epicsInt8 *dataOut, unsigned int pointNum)`
- ... (Overloads for all basic types)
  - **Description**: Gets waveform values with optional conversion.

- `int setValuesRaw(void *dataIn, unsigned int pointNum)`
- `int setValues(epicsInt8 *dataIn, unsigned int pointNum)`
- ... (Overloads for all basic types)
  - **Description**: Sets waveform values.

- `int forceOPVValue(void *dataPtr)`
- `int forcePVProcess()`
- `int raiseAlarm(epicsEnum16 nsta, epicsEnum16 nsevr)`
- `int setTimeStamp(epicsTimeStamp tsIn)`
- `int isCreated()`
- `int isRecLinked()`
- `void setWfDataType(INTD_enum_dataType wfDataType)`
- `int setDesc(const char *descStr)`
- `int setPrec(unsigned int prec)`
- `int setAlias(const char *aliasStr)`
- `int setAsg(const char *asgStr)`

---

## 9. MessageLogs (`MessageLogs.h`)
**Namespace:** `OOEPICS`

### Class: `MessageLogs`
#### Methods
- `MessageLogs(const char *modNameIn)`
- `~MessageLogs()`
- `void postMessage(char *msg)`
  - **Description**: Posts a message to log buffer.
- `void copyMessage(char *dest, int id)`
- `void printMessage()`

---

## 10. ModuleConfig (`ModuleConfig.h`)
**Namespace:** `OOEPICS`

**Abstract** base class for device configuration.

### Class: `ModuleConfig`
#### Methods
- `ModuleConfig(const char *moduleTypeName)`
- `virtual ~ModuleConfig()`
- `virtual int moduleCreate(const char *moduleName, const char *moduleInfo, int priority) = 0`
  - **Description**: **Pure Virtual**. Creates a new module instance.
- `virtual int moduleInit(ModuleInstance *module) = 0`
  - **Description**: **Pure Virtual**. Initializes the module.
- `virtual int moduleSet(ModuleInstance *module, const char *cmd, char **data) = 0`
  - **Description**: **Pure Virtual**. Sets module parameters.

---

## 11. ModuleManager (`ModuleManager.h`)
**Namespace:** `OOEPICS`

### Class: `ModuleManager`
#### Methods
- `ModuleManager()`
- `~ModuleManager()`
- `void moduleInstanceRegister(const char *moduleTypeName, ModuleConfig *modConfig, const char *moduleName, void *module, int modCategory)`
  - **Description**: Registers a module instance.
- `void createModule(const char *moduleTypeName, const char *moduleName, const char *moduleInfo, int priority)`
- `void initModule(const char *moduleName)`
- `void setModule(const char *moduleName, const char *cmd, char **data)`
- `ModuleInstance *getModuleInstance(const char *moduleName)`
- `void printModuleType()`
- `void printModuleList()`

### Global Routine
- `void moduleTypeRegister(const char *moduleTypeName, ModuleConfig *modConfig)`

---

## 12. RemotePV (`RemotePV.h`)
**Namespace:** `OOEPICS`

### Class: `RemotePV`
#### Methods
- `RemotePV()`
- `explicit RemotePV(const std::string pvNameStrIn)`
- `RemotePV(const char *moduleName, const char *localIDStr, RemotePVList *pvList)`
- `~RemotePV()`

- `void init(string modName, string srvName, string localIDStr, RemotePVList *pvList)`
  - **Description**: Initializes the Remote PV.

- `int createCA(...)`: (See ChannelAccess constructor for similar exhaustive arguments)
- `int createCA(unsigned long reqElemsReadIn, CA_enum_readCtrl rdCtrlIn, CA_enum_writeCtrl wtCtrlIn, EPICSLIB_type_mutexId mutexIdIn, EPICSLIB_type_eventId eventIn)`
  - **Description**: Simplified version of CA creation.
- `void deleteCA()`

- `int caReadRequest()`
- `int caWriteRequestVal(epicsFloat64 dataIn)`
- `int caWriteRequestStr(char *strIn)`
- `int caWriteRequestStr(string strIn)`
- `int caWriteRequestWfRaw(void *dataBufIn, unsigned long pointNum)`
- `int caWriteRequestWf(epicsInt8 *dataBufIn, unsigned long pointNum)`
- ... (Overloads for all basic types)

- `epicsInt8 getValueInt8()`
- ... (Overloads for all getters)

- `int getValuesRaw(void *dataBufOut, unsigned long pointNum)`
- `int getValues(epicsInt8 *dataBufOut, unsigned long pointNum)`
- ... (Overloads for all waveform getters)

- `int isConnected()`
- `int isOK()`
- `epicsTimeStamp getTimeStamp()`
- `void getTimeStampStr(char *tsStr)`
- `epicsInt16 getAlarmStatus()`
- `epicsInt16 getAlarmSeverity()`

### Class: `RemotePVList`
#### Methods
- `RemotePVList()`
- `~RemotePVList()`
- `int getPVNameList(string fileName, string localMacros)`
- `int addPVNodeList(RemotePV *pv)`
- `int mapPVNodeNames(string macros)`
- `int getCntRemotePV()`
- `int getCntRemotePVMappend()`
- `int getCntRemotePVConnected()`
- `int fndPVNameList(string localIdStr, string *nameStr)`
- `int savPVNodeList(string fileName)`
- `void prtPVNameList()`
- `void prtPVNodeList()`
- `void prtPVNodeList(int sel)`

---

## 13. Service (`Service.h`)
**Namespace:** `OOEPICS`

### Class: `Service`
#### Methods
- `Service(const char *moduleName, const char *serviceName)`
- `~Service()`
