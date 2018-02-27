

/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from ConnextStaticRawData.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

#ifndef ConnextStaticRawData_1419555957_h
#define ConnextStaticRawData_1419555957_h

#ifndef NDDS_STANDALONE_TYPE
#ifndef ndds_c_h
#include "rosidl_typesupport_connext_c/ndds_include.h"
#endif
#else
#include "ndds_standalone_type.h"
#endif

#define KEY_HASH_LENGTH_16 (16)

extern const char *ConnextStaticRawDataTYPENAME;

typedef struct ConnextStaticRawData {

    DDS_Octet   key_hash [(KEY_HASH_LENGTH_16)];
    struct    DDS_OctetSeq  serialized_key ;
    struct    DDS_OctetSeq  serialized_data ;

} ConnextStaticRawData ;
#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, start exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport __declspec(dllexport)
#endif

NDDSUSERDllExport DDS_TypeCode* ConnextStaticRawData_get_typecode(void); /* Type code */

DDS_SEQUENCE(ConnextStaticRawDataSeq, ConnextStaticRawData);

NDDSUSERDllExport
RTIBool ConnextStaticRawData_initialize(
    ConnextStaticRawData* self);

NDDSUSERDllExport
RTIBool ConnextStaticRawData_initialize_ex(
    ConnextStaticRawData* self,RTIBool allocatePointers,RTIBool allocateMemory);

NDDSUSERDllExport
RTIBool ConnextStaticRawData_initialize_w_params(
    ConnextStaticRawData* self,
    const struct DDS_TypeAllocationParams_t * allocParams);  

NDDSUSERDllExport
void ConnextStaticRawData_finalize(
    ConnextStaticRawData* self);

NDDSUSERDllExport
void ConnextStaticRawData_finalize_ex(
    ConnextStaticRawData* self,RTIBool deletePointers);

NDDSUSERDllExport
void ConnextStaticRawData_finalize_w_params(
    ConnextStaticRawData* self,
    const struct DDS_TypeDeallocationParams_t * deallocParams);

NDDSUSERDllExport
void ConnextStaticRawData_finalize_optional_members(
    ConnextStaticRawData* self, RTIBool deletePointers);  

NDDSUSERDllExport
RTIBool ConnextStaticRawData_copy(
    ConnextStaticRawData* dst,
    const ConnextStaticRawData* src);

#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, stop exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

#endif /* ConnextStaticRawData */

