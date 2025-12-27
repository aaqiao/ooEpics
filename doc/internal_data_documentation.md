# Source Code Documentation for `src/InternalData`

This document provides detailed documentation for the C source codes in the `src/InternalData` directory of the `ooEpics` project. This module acts as the isolation layer between EPICS records and internal code.

## 1. InternalData (`InternalData.h`)
**Module:** `InternalData`

Defines the core data structures and management routines for the Internal Data isolation layer.

### Data Structures

#### `INTD_struc_node`
The main data structure representing an internal data node (shadowing an EPICS record).

- **Members:**
  - `ELLNODE node`: Node for linked list management.
  - `char moduleName[128]`: Name of the module this data belongs to.
  - `char dataName[128]`: Internal data name.
  - `char subModuleName[128]`: Sub-module name (for DB generation).
  - `char recName[128]`: Record name (for DB generation).
  - `char supStr[256]`: Additional configuration string (for bi/bo/mbbi/mbbo).
  - `char unitStr[16]`: Unit string.
  - `char desc[128]`: Description field.
  - `char alias[128]`: Alias name.
  - `char asg[128]`: Access Security Group.
  - `void *dataPtr`: Pointer to the actual data storage.
  - `void *privateData`: Private user data passed to callbacks.
  - `unsigned int pno`: Number of points (elements).
  - `unsigned int prec`: Precision.
  - `IOSCANPVT *ioIntScan`: Pointer to I/O interrupt scan structure.
  - `INTD_enum_dataType dataType`: Data type of the variable.
  - `INTD_CALLBACK readCallback`: Function called when reading data.
  - `INTD_CALLBACK writeCallback`: Function called when writing data.
  - `dbCommon *epicsRecord`: Pointer to the associated EPICS record.
  - `epicsMutexId mutexId`: Mutex for thread safety.
  - `epicsEventId eventId`: Event for signaling access.
  - `INTD_enum_recordType recordType`: Type of the EPICS record.
  - `INTD_enum_scanType scanType`: Scan mechanism.
  - `int enableCallback`: Flag to enable/disable callbacks.
  - `epicsEnum16 nsta`: Alarm status.
  - `epicsEnum16 nsevr`: Alarm severity.

### Enums
- `INTD_enum_dataType`: `INTD_CHAR`, `INTD_SHORT`, `INTD_INT`, `INTD_LONG`, `INTD_FLOAT`, `INTD_DOUBLE`, etc.
- `INTD_enum_recordType`: `INTD_AO`, `INTD_AI`, `INTD_BO`, `INTD_BI`, `INTD_MBBO`, `INTD_WFO`, etc.
- `INTD_enum_scanType`: `INTD_PASSIVE`, `INTD_IOINT`, `INTD_10S`, `INTD_1S`, etc.

### Management Routines (API)

- `INTD_struc_node *INTD_API_createDataNode(...)`
  - **Description**: Creates and registers a new internal data node.
  - **Arguments**:
    - `moduleName`, `dataName`: Identification names.
    - `supStr`, `unitStr`: Configuration strings.
    - `dataPtr`: Pointer to the variable.
    - `privateData`: User data.
    - `pno`: Number of elements.
    - `ioIntScan`: Scan structure pointer.
    - `dataType`: Type of the data.
    - `readCallback`, `writeCallback`: Callback functions.
    - `mutexId`, `eventId`: Synchronization primitives.
    - `recordType`, `scanType`: Record configuration.

- `INTD_struc_node *INTD_API_findDataNode(const char *moduleName, const char *dataName, dbCommon *epicsRecord)`
  - **Description**: Finds an existing data node by name or record pointer.

- `int INTD_API_getData(INTD_struc_node *dataNode, unsigned int pno, void *data)`
- `int INTD_API_putData(INTD_struc_node *dataNode, unsigned int pno, void *data)`
  - **Description**: generic get/put routines to access data in the node.

- `int INTD_API_genRecord(const char *moduleName, const char *path, const char *dbFileName)`
- `int INTD_API_genSRReqt(...)`
- `int INTD_API_genArchive(...)`
- `int INTD_API_genDbList(...)`
  - **Description**: Generation routines for EPICS Database, Save/Restore, Archiver, and DB List files.

- `int INTD_API_syncWithRecords(int enaCallback)`
  - **Description**: Synchronizes internal data with EPICS records.

- `int INTD_API_forceOPVValue(INTD_struc_node *dataNode, void *dataPtr)`
  - **Description**: Forces a value update on an output PV.

- `int INTD_API_forcePVProcess(INTD_struc_node *dataNode)`
  - **Description**: Forces record processing.

- `int INTD_API_raiseAlarm(INTD_struc_node *dataNode, epicsEnum16 nsta, epicsEnum16 nsevr)`
  - **Description**: Sets the alarm status and severity.

- `int INTD_API_setDescription(...)`
- `int INTD_API_setPrecision(...)`
- `int INTD_API_setAlias(...)`
- `int INTD_API_setAsg(...)`
  - **Description**: Setters for record metadata.

- `int INTD_API_getFieldInfo(...)`
- `int INTD_API_getFieldData(...)`
- `int INTD_API_putFieldData(...)`
  - **Description**: Generic field access routines.

- `int INTD_API_getIocInitStatus()`
  - **Description**: Returns the IOC initialization status.

---

## 2. recordGenerate (`recordGenerate.h`)
**Module:** `recordGenerate`

Routines for generating EPICS record definition strings from internal data configuration.

### Functions

- `void INTD_RECORD_AO(...)`
- `void INTD_RECORD_AI(...)`
- `void INTD_RECORD_BO(...)`
- `void INTD_RECORD_BI(...)`
- `void INTD_RECORD_LO(...)`
- `void INTD_RECORD_LI(...)`
- `void INTD_RECORD_MBBO(...)`
- `void INTD_RECORD_MBBI(...)`
- `void INTD_RECORD_WFO(...)`
- `void INTD_RECORD_WFI(...)`
- `void INTD_RECORD_SO(...)`
- `void INTD_RECORD_SI(...)`
  - **Description**: Generates the record definition string (in `.db` format) for the specific record type.
  - **Common Arguments**:
    - `datName`: Data/Record name.
    - `subModName`: Sub-module name.
    - `recName`: Final record name.
    - `scanMethod`: Scan field value.
    - `unitStr`: Unit string (EGU).
    - `descStr`: Description string (DESC).
    - `aliasStr`: Alias name.
    - `asgStr`: Access security group.
    - `outStr`: Output buffer for the generated string.
  - **Specific Arguments**:
    - `supStr` (for Bi/Bo/Mbbi/Mbbo): Support string (e.g., ZNAM/ONAM or ZRVL/ZRST...).
    - `precVal` (for Ai/Ao/Calc): Precision (PREC).
    - `pno`, `dataType` (for Waveforms): Number of elements (NELM) and Field Type (FTVL).
