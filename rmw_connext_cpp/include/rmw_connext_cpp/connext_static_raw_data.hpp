/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from ConnextStaticRawData.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

#ifndef ConnextStaticRawData_1689213465_h
#define ConnextStaticRawData_1689213465_h

#ifndef NDDS_STANDALONE_TYPE
#ifndef ndds_cpp_h
#include "rmw_connext_shared_cpp/ndds_include.hpp"
#endif
#else
#include "ndds_standalone_type.h"
#endif

static const DDS_Long KEY_HASH_LENGTH_16= 16;
extern "C" {

    extern const char *ConnextStaticRawDataTYPENAME;

}

struct ConnextStaticRawDataSeq;
#ifndef NDDS_STANDALONE_TYPE
class ConnextStaticRawDataTypeSupport;
class ConnextStaticRawDataDataWriter;
class ConnextStaticRawDataDataReader;
#endif

class ConnextStaticRawData
{
  public:
    typedef struct ConnextStaticRawDataSeq Seq;
    #ifndef NDDS_STANDALONE_TYPE
    typedef ConnextStaticRawDataTypeSupport TypeSupport;
    typedef ConnextStaticRawDataDataWriter DataWriter;
    typedef ConnextStaticRawDataDataReader DataReader;
    #endif

    DDS_Octet   key_hash [(KEY_HASH_LENGTH_16)];
    DDS_OctetSeq  serialized_key ;
    DDS_OctetSeq  serialized_data ;

};
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
