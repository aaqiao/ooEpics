//=========================================================
// RemotePV.cc
// 
// Class definition for the remote PV
// 
// Created by: Zheqiao Geng
// Created on: 12/11/2012
// Description: Initial creation 
//
// Modified by Zheqiao Geng on 2014.04.08
// Description: updated with the new Channel Access implementation
// 
// Modified by Zheqiao Geng on 2014.04.16
// Description: make the files for PV name mapping more formal
//
// Modified by Zheqiao Geng on 2014.05.19
// Applied macro to the remote PV mapping files
//
// Modified by Zheqiao Geng on 02.04.2015
// Disable the message printing when the action is successful
//
// Modified by Andreas Hauff on 16+.11.2015
// Additional constructor to use it without PV mapping
//
// Modified by Zheqiao Geng on 26.11.2015
// 1. Make the macro settings to PV names in two stages, the mapping
// file specific macros will be substituted when loading the file, 
// and the general macros tends to be used for dynamic remote PV 
// connection will be substituted in the function dedicated for macro
// mapping.
// 2. Extend the function to print the remote PV mapping details
//
// Modified by Zheqiao Geng on 14.12.2015
// Fixed the problem for empty global macros for remote PV mapping;
// Added more choices for remote PV list printing
//=========================================================
#include "RemotePV.h"

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CLASS FOR REMOTEPV LIST
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//-----------------------------------------------
// construction of the remote PVName list
//-----------------------------------------------
RemotePVList::RemotePVList()
{
    // initialize the list
    EPICSLIB_func_LinkedListInit(pvNameList);
    EPICSLIB_func_LinkedListInit(pvNodeList);

    cntRemotePV          = 0;
    cntRemotePVMapped    = 0;
}

//-----------------------------------------------
// destruction of the remote PVName list
//-----------------------------------------------
RemotePVList::~RemotePVList()
{
    RemotePVName *pvName = NULL;
    RemotePVNode *pvNode = NULL;  
    
    // delete the PV name data node
    while((pvName = (RemotePVName *)ellLast(&pvNameList))) {
        ellDelete(&pvNameList, &pvName -> node);
        delete pvName;   
    }    

    cout << "INFO:RemotePVList: remote PV name list deleted!" << endl;

    // delete the PV data node
    while((pvNode = (RemotePVNode *)ellLast(&pvNodeList))) {
        ellDelete(&pvNodeList, &pvNode -> node);
        delete pvNode;   
    }    

    cout << "INFO:RemotePVList: remote PV list deleted!" << endl;
}

//-----------------------------------------------
// get a PV name vs local ID map table from file
// Input:
//     fileName     : a string for file name
//     localMacros  : a string for the macros specific for this file
// Output:
//     0 - sucess
//     1 - failed
//-----------------------------------------------
int RemotePVList::getPVNameList(string fileName, string localMacros)
{
    RemotePVName *pvName;

    string strLine;
    string strItem;
    string strItemValue;

    int posComment;
    int posEqual;
    
    MAC_HANDLE *macHandle = NULL;
    char       **macPairs = NULL;

    char strMacroed1[256];
    char strMacroed2[256];

    // test if the filename is Ok or not
    if(fileName.empty()) {
        cout << "ERROR:RemotePVList::getPVNameList: Wrong file name!" << endl;
        return 1;
    }

    // open the file
    ifstream sourceFile(fileName.c_str(), ios::in);

    if(!sourceFile.good()) {
        cout << "ERROR:RemotePVList::getPVNameList: Bad file of " << fileName << endl;
        return 1;
    }

    // make up the macro for this mapping file (study if there is memory leakge or not! 20150914)
    if(!localMacros.empty()) {
        if(macCreateHandle(&macHandle, NULL) == 0) {
            macParseDefns(macHandle, localMacros.c_str(), &macPairs);
            if(macPairs == NULL) {
                macDeleteHandle(macHandle);
                macHandle = NULL;
            } else {
                macInstallMacros(macHandle, macPairs);
                free((void *)macPairs);
            }
        }        
    }

    cout << "INFO:RemotePVList::getPVNameList: load from file " << fileName << ", with specific macros of " << localMacros << endl;
    //cout << "-----------------------------------------------------------" << endl;

    // read the file
    while(!sourceFile.eof() && sourceFile.good()) {
        // read a line
        getline(sourceFile, strLine);

        // remove the comments
        posComment = strLine.find_first_of("#");  

        if(posComment == 0)                             // this line is a comment
            continue;                     
        else if(posComment > 0)                         // comment is in later of the line
            strLine = strLine.substr(0, posComment);

        // get the data needed
        posEqual = strLine.find("=");    

        if(posEqual < 0)
            continue;
        else {
            strItem         = strLine.substr(0, posEqual);
            strItemValue    = strLine.substr(posEqual + 1);

            // erase the space and the tab
            eraseChar(&strItem,         " ");
            eraseChar(&strItem,         "\t");
            eraseChar(&strItemValue,    " ");
            eraseChar(&strItemValue,    "\t");           
        }

        // apply the file specific macro substitutions on the strings
        if(macHandle) {
            macExpandString(macHandle, strItem.c_str(),      strMacroed1, 256);
            macExpandString(macHandle, strItemValue.c_str(), strMacroed2, 256);
        }

        // create a new node and assign the data
        pvName = new RemotePVName;
        if(pvName) {

            if(macHandle) {
                pvName -> pvLocalIdStrWithMacro.assign(strMacroed1);        // still with general macro
                pvName -> pvNameStrWithMacro.assign(strMacroed2);           
            } else {
                pvName -> pvLocalIdStrWithMacro.assign(strItem); 
                pvName -> pvNameStrWithMacro.assign(strItemValue); 
            }

            pvName -> pvLocalIdStr.assign("");
            pvName -> pvNameStr.assign("");
            EPICSLIB_func_LinkedListInsert(pvNameList, pvName -> node);

            //cout << "INFO:RemotePVList::getPVNameList: with local subs " << pvName -> pvLocalIdStrWithMacro << " <> " << pvName -> pvNameStrWithMacro << endl;
        }
    }

    // close the file
    sourceFile.close();  

    // delete the macro staff
    if(macHandle) macDeleteHandle(macHandle);

    return 0;
}

//-----------------------------------------------
// Add a remote PV node into the pvNodeList
// Input:
//     pv - pointer to the RemotePV object
// Output:
//     0 - sucess
//     1 - failed
//-----------------------------------------------
int RemotePVList::addPVNodeList(RemotePV *pv)
{
    RemotePVNode *pvNode;    

    // check the input
    if(!pv) {
        cout << "ERROR:RemotePVList::addPVNodeList: Illegal RemotePV object!" << endl;
        return 1;
    }

    // create a node and add to the list
    pvNode = new RemotePVNode;
    if(pvNode) {
        pvNode -> pv = pv;
        EPICSLIB_func_LinkedListInsert(pvNodeList, pvNode -> node);
        cntRemotePV ++;
    }

    return 0;
}

//-----------------------------------------------
// Map each remote PV to a PV name with the substitution from a general macro valid for all remote PVs
// Input:
//     macros   - the general macros for the PV name mappings
// Output:
//     0 - sucess
//     1 - failed
//-----------------------------------------------
int RemotePVList::mapPVNodeNames(string macros)
{
    RemotePVNode *pvNode;
    RemotePVName *pvName;
    string pvNameStr;

    MAC_HANDLE *macHandle = NULL;
    char       **macPairs = NULL;

    char strMacroed1[256];
    char strMacroed2[256];

    // make up the macro (study if there is memory leakge or not! 20150914)
    if(!macros.empty()) {
        if(macCreateHandle(&macHandle, NULL) == 0) {
            macParseDefns(macHandle, macros.c_str(), &macPairs);
            if(macPairs == NULL) {
                macDeleteHandle(macHandle);
                macHandle = NULL;
            } else {
                macInstallMacros(macHandle, macPairs);
                free((void *)macPairs);
            }
        }        
    }

    // apply the macro to the pvNameList
    if(macHandle) {
        for(pvName = (RemotePVName *)EPICSLIB_func_LinkedListFindFirst(pvNameList);
            pvName;
            pvName = (RemotePVName *)EPICSLIB_func_LinkedListFindNext(pvName -> node)) {

            macExpandString(macHandle, pvName -> pvLocalIdStrWithMacro.c_str(), strMacroed1, 256);
            macExpandString(macHandle, pvName -> pvNameStrWithMacro.c_str(),    strMacroed2, 256);

            pvName -> pvLocalIdStr.assign(strMacroed1);
            pvName -> pvNameStr.assign(strMacroed2);

            //cout << "INFO:RemotePVList::mapPVNodeNames: map " << strMacroed1 << " to " << strMacroed2 << endl;
        }
    } else {
        for(pvName = (RemotePVName *)EPICSLIB_func_LinkedListFindFirst(pvNameList);
            pvName;
            pvName = (RemotePVName *)EPICSLIB_func_LinkedListFindNext(pvName -> node)) {

            pvName -> pvLocalIdStr  = pvName -> pvLocalIdStrWithMacro;
            pvName -> pvNameStr     = pvName -> pvNameStrWithMacro;
        }
    }

    // delete the macro staff
    if(macHandle) macDeleteHandle(macHandle);

    // clean the map counter
    cntRemotePVMapped = 0;

    // go through the PV node list and then lookup the PV name list for the PV names
    for(pvNode = (RemotePVNode *)EPICSLIB_func_LinkedListFindFirst(pvNodeList);
        pvNode;
        pvNode = (RemotePVNode *)EPICSLIB_func_LinkedListFindNext(pvNode -> node)) {
        if(pvNode -> pv) {
            if(fndPVNameList(pvNode -> pv -> pvLocalIdStr, &pvNameStr) == 0) {

                pvNode -> pv -> pvNameStr = pvNameStr;
                cntRemotePVMapped ++;

                //cout << "INFO:RemotePVList::mapPVNodeNames: Find remote PV " << pvNode -> pv -> pvNameStr << " for " << pvNode -> pv -> pvLocalIdStr << endl; 
            }
        }
    }    

    return 0;
}

//-----------------------------------------------
// Get counters
//-----------------------------------------------
int RemotePVList::getCntRemotePV         () {return cntRemotePV;}
int RemotePVList::getCntRemotePVMappend  () {return cntRemotePVMapped;}
int RemotePVList::getCntRemotePVConnected()
{
    int cntRemotePVConnected = 0;
    RemotePVNode *pvNode;
    
    // go through the PV node list and then count the connections
    for(pvNode = (RemotePVNode *)EPICSLIB_func_LinkedListFindFirst(pvNodeList);
        pvNode;
        pvNode = (RemotePVNode *)EPICSLIB_func_LinkedListFindNext(pvNode -> node)) {

        if(pvNode -> pv && pvNode -> pv -> isConnected()) 
            cntRemotePVConnected ++;
    }    

    return cntRemotePVConnected;
}

//-----------------------------------------------
// lookup the PVName list
// Input:
//     localIdStr - the local ID string
//     nameStr    - the returned EPICS PV name
// Output:
//     0 - sucess
//     1 - failed
//-----------------------------------------------
int RemotePVList::fndPVNameList(string localIdStr, string *nameStr)
{
    int found = 0;
    RemotePVName *pvName;

    // check the input
    if(localIdStr.empty() || !nameStr) return 1;

    // lookup the matched PV name (find the first matched one)
    for(pvName = (RemotePVName *)EPICSLIB_func_LinkedListFindFirst(pvNameList);
        pvName;
        pvName = (RemotePVName *)EPICSLIB_func_LinkedListFindNext(pvName -> node)) {

        if(localIdStr.compare(pvName -> pvLocalIdStr) == 0) {
            *(nameStr) = pvName -> pvNameStr;
            found = 1;
            break;
        }   
    }

    if(found && !nameStr -> empty())
        return 0;
    else
        return 1;
}

//-----------------------------------------------
// save the PV list to files, this can help to automate the mapping file generation
//-----------------------------------------------
int RemotePVList::savPVNodeList(string fileName)
{
    int i;
    RemotePVNode *pvNode;

    char *strTime = getSystemTime();

    // test if the filename is Ok or not
    if(fileName.empty()) {
        cout << "ERROR:RemotePVList::savPVNodeList: Wrong file name!" << endl;
        return 1;
    }

    // open the file
    ofstream outFile(fileName.c_str(), ios::out);

    if(!outFile.good()) {
        cout << "ERROR:RemotePVList::savPVNodeList: Bad file!" << endl;
        return 1;
    }

    // write the header of the file
    outFile << "#------------------------------------------------------------" << endl;
    outFile << "# " << fileName << endl;
    outFile << "# " << endl;
    outFile << "# File to map internal remote PV ID and the real remote PV name" << endl;
    outFile << "# " << endl;
    outFile << "# Auto created on " << strTime;
    outFile << "# Format: [Internal Remote PV ID] = [Remote PV Name]" << endl;
    outFile << "#------------------------------------------------------------" << endl;

    // write to the file
    for(pvNode = (RemotePVNode *)EPICSLIB_func_LinkedListFindFirst(pvNodeList);
        pvNode;
        pvNode = (RemotePVNode *)EPICSLIB_func_LinkedListFindNext(pvNode -> node)) {
        if(pvNode -> pv) {
            outFile << pvNode -> pv -> pvLocalIdStr;
        
            for (i = 0; i < 80 - (int)pvNode -> pv -> pvLocalIdStr.length(); i ++)
                outFile << " ";
            
            outFile << "= " << pvNode -> pv -> pvNameStr << endl;
        }
    }

    outFile.close();

    return 0;
}

//-----------------------------------------------
// print the PV name list
//-----------------------------------------------
void RemotePVList::prtPVNameList()
{
    RemotePVName *pvName;

    cout << "PV Name List" << endl;
    cout << "--------------------------------------------------------------" << endl;

    for(pvName = (RemotePVName *)EPICSLIB_func_LinkedListFindFirst(pvNameList);
        pvName;
        pvName = (RemotePVName *)EPICSLIB_func_LinkedListFindNext(pvName -> node)) {
        cout << pvName -> pvLocalIdStr << "\t\t" << pvName -> pvNameStr << endl;
    }
    cout << "--------------------------------------------------------------" << endl;
}

//-----------------------------------------------
// print the PV node list. Show the detailed information of all the remote PVs
// items to be displayed:
//  a. general information of the counters: remotePV count, remotePV with good map count, remote PV with connections OK count
//  b. all remote PVs: remote PV ID string, remote PV name, connection status
//-----------------------------------------------
void RemotePVList::prtPVNodeList()
{
    RemotePVNode *pvNode;

    // print the general information
    cout << "======================================================================" << endl;
    cout << "REMOTE PV SUMMARY" << endl;
    cout << "======================================================================" << endl;
    cout << "General Information" << endl;
    cout << "--------------------------------------------------------------------"  << endl;
    cout << "Number of remote PV objects:           " << getCntRemotePV()           << endl;
    cout << "Number of remote PVs with name mapped: " << getCntRemotePVMappend()    << endl;
    cout << "Number of remote PVs with connection:  " << getCntRemotePVConnected()  << endl;
    cout << "--------------------------------------------------------------------"  << endl;
    cout << "Remote PV ID String        Remote PV Name         Connection Status"   << endl;
    cout << "--------------------------------------------------------------------"  << endl;
    for(pvNode = (RemotePVNode *)EPICSLIB_func_LinkedListFindFirst(pvNodeList);
        pvNode;
        pvNode = (RemotePVNode *)EPICSLIB_func_LinkedListFindNext(pvNode -> node)) {

        prtRPVInfo(pvNode -> pv);
    }
}

// another implementation (keep the above one for interface consistency)
void RemotePVList::prtPVNodeList(int sel)
{
    RemotePVNode *pvNode;

    // print the general information
    cout << "======================================================================" << endl;
    cout << "REMOTE PV SUMMARY" << endl;
    cout << "======================================================================" << endl;
    cout << "General Information" << endl;
    cout << "--------------------------------------------------------------------"  << endl;
    cout << "Number of remote PV objects:           " << getCntRemotePV()           << endl;
    cout << "Number of remote PVs with name mapped: " << getCntRemotePVMappend()    << endl;
    cout << "Number of remote PVs with connection:  " << getCntRemotePVConnected()  << endl;

    // print the title
    if( sel == RPVLIST_PNT_CONN     ||
        sel == RPVLIST_PNT_NCONN    ||
        sel == RPVLIST_PNT_ALL) {
        cout << "--------------------------------------------------------------------"  << endl;
        cout << "Remote PV ID String        Remote PV Name         Connection Status"   << endl;
        cout << "--------------------------------------------------------------------"  << endl;
    }

    // print based on selection
    switch(sel) {
        case RPVLIST_PNT_CONN:
            for(pvNode = (RemotePVNode *)EPICSLIB_func_LinkedListFindFirst(pvNodeList);
                pvNode;
                pvNode = (RemotePVNode *)EPICSLIB_func_LinkedListFindNext(pvNode -> node)) {
                if(pvNode -> pv && pvNode -> pv -> isConnected()) 
                    prtRPVInfo(pvNode -> pv);
            }
            break;

        case RPVLIST_PNT_NCONN:
            for(pvNode = (RemotePVNode *)EPICSLIB_func_LinkedListFindFirst(pvNodeList);
                pvNode;
                pvNode = (RemotePVNode *)EPICSLIB_func_LinkedListFindNext(pvNode -> node)) {
                if(pvNode -> pv && !pvNode -> pv -> isConnected()) 
                    prtRPVInfo(pvNode -> pv);
            }
            break;

        case RPVLIST_PNT_ALL:
            for(pvNode = (RemotePVNode *)EPICSLIB_func_LinkedListFindFirst(pvNodeList);
                pvNode;
                pvNode = (RemotePVNode *)EPICSLIB_func_LinkedListFindNext(pvNode -> node)) {
                if(pvNode -> pv) 
                    prtRPVInfo(pvNode -> pv);
            }
            break;

        default:
            break;
    }
}

// common private function
void RemotePVList::prtRPVInfo(RemotePV *pv)
{
    int i;

    if(pv) {
        cout << pv -> pvLocalIdStr;
        for (i = 0; i < 60 - (int)pv -> pvLocalIdStr.length(); i ++) cout << " ";

        cout << pv -> pvNameStr;           
        for (i = 0; i < 60 - (int)pv -> pvNameStr.length(); i ++) cout << " ";

        cout << pv -> isConnected() << endl;
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CLASS FOR REMOTEPV
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//-----------------------------------------------
// construction for RemotePV
//-----------------------------------------------
RemotePV::RemotePV()
{
    pvLocalIdStr.clear();
    pvNameStr.clear();
    pvCAChannel = NULL;
}

RemotePV::RemotePV(const std::string pvNameStrIn)
{
    pvLocalIdStr.clear();
    pvNameStr.assign(pvNameStrIn);
    pvCAChannel = NULL;
}

RemotePV::RemotePV(const char *moduleName, const char *localIDStr, RemotePVList *pvList)    
{
    // init the variables
    pvLocalIdStr.assign(moduleName);
    pvLocalIdStr.append(".");
    pvLocalIdStr.append(localIDStr);
    pvNameStr.clear();
    pvCAChannel = NULL;

    // add ths remote PV to the indicated pv list
    if(pvList)
        pvList -> addPVNodeList(this);
}

//-----------------------------------------------
// destruction for RemotePV
//-----------------------------------------------
RemotePV::~RemotePV()
{
    deleteCA();
}

//-----------------------------------------------
// init the RemotePV for the default construction
//-----------------------------------------------
void RemotePV::init(string modName, string srvName, string localIDStr, RemotePVList *pvList)
{
    // init the variables
    pvLocalIdStr.assign(modName);
    pvLocalIdStr.append("-");
    pvLocalIdStr.append(srvName);
    pvLocalIdStr.append(".");
    pvLocalIdStr.append(localIDStr);

    // add ths remote PV to the indicated pv list
    if(pvList)
        pvList -> addPVNodeList(this);
}

//-----------------------------------------------
// Create the channel access for this object
// Output:
//     0 - sucess
//     1 - failed
//-----------------------------------------------
int RemotePV::createCA(unsigned long             reqElemsReadIn,
                       CA_enum_readCtrl          rdCtrlIn,        
                       CA_enum_writeCtrl         wtCtrlIn,     
                       CAUSR_CALLBACK            connUserCallbackIn,   
                       CAUSR_CALLBACK            rdUserCallbackIn,
                       CAUSR_CALLBACK            wtUserCallbackIn,
                       void                     *dataPtrIn,       
                       void                     *userPtrIn, 
                       EPICSLIB_type_mutexId     mutexIdIn,
                       EPICSLIB_type_eventId     connEventIn,      
                       EPICSLIB_type_eventId     rdEventIn,       
                       EPICSLIB_type_eventId     wtEventIn)
{
    // only do it when the PV name is not empty
    if(!pvNameStr.empty() && pvCAChannel == NULL) {
        pvCAChannel = new ChannelAccess(pvNameStr, 
                                        reqElemsReadIn, 
                                        rdCtrlIn, 
                                        wtCtrlIn, 
                                        connUserCallbackIn,
                                        rdUserCallbackIn, 
                                        wtUserCallbackIn, 
                                        dataPtrIn, 
                                        userPtrIn, 
                                        mutexIdIn,
                                        connEventIn,
                                        rdEventIn, 
                                        wtEventIn, 
                                        CA_DEFAULT_PRIORITY);

        if(pvCAChannel) {
            pvCAChannel -> connect();       
            return 0;
        } else {
            cout << "ERROR:RemotePV::createCA: Failed to create " << pvLocalIdStr << " / " << pvNameStr << "!" << endl;
            return 1;
        }
    }
    else
        cout<< "ERROR:RemotePV::createCA: Empty PV name for " << pvLocalIdStr << endl;

    return 1;
}

//-----------------------------------------------
// Create the channel access for this object without callback or events
// Output:
//     0 - sucess
//     1 - failed
//-----------------------------------------------
int RemotePV::createCA  (unsigned long             reqElemsReadIn,
                         CA_enum_readCtrl          rdCtrlIn,
                         CA_enum_writeCtrl         wtCtrlIn,
                         EPICSLIB_type_mutexId     mutexIdIn,
                         EPICSLIB_type_eventId     eventIn)
{
    // only do it when the PV name is not empty
    if(!pvNameStr.empty() && pvCAChannel == NULL) {
        pvCAChannel = new ChannelAccess(pvNameStr, 
                                        reqElemsReadIn, 
                                        rdCtrlIn, 
                                        wtCtrlIn, 
                                        NULL,
                                        NULL, 
                                        NULL, 
                                        NULL, 
                                        NULL, 
                                        mutexIdIn,
                                        eventIn,                // use the same event for simplification, should be enough for many cases
                                        eventIn, 
                                        eventIn, 
                                        CA_DEFAULT_PRIORITY);

        if(pvCAChannel) {
            pvCAChannel -> connect();       
            return 0;
        } else {
            cout << "ERROR:RemotePV::createCA: Failed to create " << pvLocalIdStr << " / " << pvNameStr << "!" << endl;
            return 1;
        }
    }
    else
        cout<< "ERROR:RemotePV::createCA: Empty PV name for " << pvLocalIdStr << endl;

    return 1;
}

//-----------------------------------------------
// Delete the channel access for this object
//-----------------------------------------------
void RemotePV::deleteCA()
{
    if(pvCAChannel) {
        delete pvCAChannel;
        pvCAChannel = NULL;
    }
}

//-----------------------------------------------
// Interface for CA access (direct wrapper of Channel Access class)
//-----------------------------------------------
int RemotePV::caReadRequest       ()                                                    {if(!pvCAChannel) return 1; return pvCAChannel -> caReadRequest();}

int RemotePV::caWriteRequestVal   (epicsFloat64   dataIn)                               {if(!pvCAChannel) return 1; return pvCAChannel -> caWriteRequestVal(dataIn);}
int RemotePV::caWriteRequestStr   (char          *strIn)                                {if(!pvCAChannel) return 1; return pvCAChannel -> caWriteRequestStr(strIn);}
int RemotePV::caWriteRequestStr   (string         strIn)                                {if(!pvCAChannel) return 1; return pvCAChannel -> caWriteRequestStr(strIn);}

int RemotePV::caWriteRequestWfRaw (void           *dataBufIn, unsigned long pointNum)   {if(!pvCAChannel) return 1; return pvCAChannel -> caWriteRequestWfRaw(dataBufIn, pointNum);}
int RemotePV::caWriteRequestWf    (epicsInt8      *dataBufIn, unsigned long pointNum)   {if(!pvCAChannel) return 1; return pvCAChannel -> caWriteRequestWf   (dataBufIn, pointNum);}
int RemotePV::caWriteRequestWf    (epicsUInt8     *dataBufIn, unsigned long pointNum)   {if(!pvCAChannel) return 1; return pvCAChannel -> caWriteRequestWf   (dataBufIn, pointNum);}
int RemotePV::caWriteRequestWf    (epicsInt16     *dataBufIn, unsigned long pointNum)   {if(!pvCAChannel) return 1; return pvCAChannel -> caWriteRequestWf   (dataBufIn, pointNum);}
int RemotePV::caWriteRequestWf    (epicsUInt16    *dataBufIn, unsigned long pointNum)   {if(!pvCAChannel) return 1; return pvCAChannel -> caWriteRequestWf   (dataBufIn, pointNum);}
int RemotePV::caWriteRequestWf    (epicsInt32     *dataBufIn, unsigned long pointNum)   {if(!pvCAChannel) return 1; return pvCAChannel -> caWriteRequestWf   (dataBufIn, pointNum);}
int RemotePV::caWriteRequestWf    (epicsUInt32    *dataBufIn, unsigned long pointNum)   {if(!pvCAChannel) return 1; return pvCAChannel -> caWriteRequestWf   (dataBufIn, pointNum);}
int RemotePV::caWriteRequestWf    (epicsFloat32   *dataBufIn, unsigned long pointNum)   {if(!pvCAChannel) return 1; return pvCAChannel -> caWriteRequestWf   (dataBufIn, pointNum);}
int RemotePV::caWriteRequestWf    (epicsFloat64   *dataBufIn, unsigned long pointNum)   {if(!pvCAChannel) return 1; return pvCAChannel -> caWriteRequestWf   (dataBufIn, pointNum);}

epicsInt8       RemotePV::getValueInt8    ()                                            {if(!pvCAChannel) return 0; return pvCAChannel -> getValueInt8();}
epicsUInt8      RemotePV::getValueUInt8   ()                                            {if(!pvCAChannel) return 0; return pvCAChannel -> getValueUInt8();}
epicsInt16      RemotePV::getValueInt16   ()                                            {if(!pvCAChannel) return 0; return pvCAChannel -> getValueInt16();}
epicsUInt16     RemotePV::getValueUInt16  ()                                            {if(!pvCAChannel) return 0; return pvCAChannel -> getValueUInt16();}
epicsInt32      RemotePV::getValueInt32   ()                                            {if(!pvCAChannel) return 0; return pvCAChannel -> getValueInt32();}
epicsUInt32     RemotePV::getValueUInt32  ()                                            {if(!pvCAChannel) return 0; return pvCAChannel -> getValueUInt32();}
epicsFloat32    RemotePV::getValueFloat32 ()                                            {if(!pvCAChannel) return 0; return pvCAChannel -> getValueFloat32();}
epicsFloat64    RemotePV::getValueFloat64 ()                                            {if(!pvCAChannel) return 0; return pvCAChannel -> getValueFloat64();}
int             RemotePV::getValueString  (char *strOut)                                {if(!pvCAChannel) return 0; return pvCAChannel -> getValueString(strOut);}
string          RemotePV::getValueString  ()                                            {if(!pvCAChannel) return 0; return pvCAChannel -> getValueString();}

int RemotePV::getValuesRaw(void          *dataBufOut, unsigned long pointNum)           {if(!pvCAChannel) return 0; return pvCAChannel -> getValuesRaw(dataBufOut, pointNum);}
int RemotePV::getValues   (epicsInt8     *dataBufOut, unsigned long pointNum)           {if(!pvCAChannel) return 0; return pvCAChannel -> getValues   (dataBufOut, pointNum);}
int RemotePV::getValues   (epicsUInt8    *dataBufOut, unsigned long pointNum)           {if(!pvCAChannel) return 0; return pvCAChannel -> getValues   (dataBufOut, pointNum);}
int RemotePV::getValues   (epicsInt16    *dataBufOut, unsigned long pointNum)           {if(!pvCAChannel) return 0; return pvCAChannel -> getValues   (dataBufOut, pointNum);}
int RemotePV::getValues   (epicsUInt16   *dataBufOut, unsigned long pointNum)           {if(!pvCAChannel) return 0; return pvCAChannel -> getValues   (dataBufOut, pointNum);}
int RemotePV::getValues   (epicsInt32    *dataBufOut, unsigned long pointNum)           {if(!pvCAChannel) return 0; return pvCAChannel -> getValues   (dataBufOut, pointNum);}
int RemotePV::getValues   (epicsUInt32   *dataBufOut, unsigned long pointNum)           {if(!pvCAChannel) return 0; return pvCAChannel -> getValues   (dataBufOut, pointNum);}
int RemotePV::getValues   (epicsFloat32  *dataBufOut, unsigned long pointNum)           {if(!pvCAChannel) return 0; return pvCAChannel -> getValues   (dataBufOut, pointNum);}
int RemotePV::getValues   (epicsFloat64  *dataBufOut, unsigned long pointNum)           {if(!pvCAChannel) return 0; return pvCAChannel -> getValues   (dataBufOut, pointNum);}

//-----------------------------------------------
// Get status
//-----------------------------------------------
int RemotePV::isConnected() 
{
    if(pvCAChannel) return pvCAChannel -> getConnected();
    else            return 0;        
}

int RemotePV::isOK() 
{
    if(pvCAChannel && pvCAChannel -> getCAStatus() == ECA_NORMAL) return 1;
    else return 0;        
}

epicsTimeStamp RemotePV::getTimeStamp()
{
    epicsTimeStamp tsNull;
    memset(&tsNull, 0, sizeof(tsNull));

    if(!pvCAChannel) 
        return tsNull; 

    return pvCAChannel -> getTimeStamp();
}

void        RemotePV::getTimeStampStr (char *tsStr) {if(!pvCAChannel) return;             pvCAChannel -> getTimeStampStr    (tsStr);}
epicsInt16  RemotePV::getAlarmStatus  ()            {if(!pvCAChannel) return -1;   return pvCAChannel -> getAlarmStatus     ();}
epicsInt16  RemotePV::getAlarmSeverity()            {if(!pvCAChannel) return -1;   return pvCAChannel -> getAlarmSeverity   ();}

} 
//******************************************************
// NAME SPACE OOEPICS
//******************************************************


