/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from ConnextStaticRawData.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

#ifndef ConnextStaticRawDataSupport_1689213465_h
#define ConnextStaticRawDataSupport_1689213465_h

/* Uses */
#include "rmw_connext_cpp/connext_static_raw_data.hpp"

#ifndef ndds_cpp_h
#include "rmw_connext_shared_cpp/ndds_include.hpp"
#endif

#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)

class __declspec(dllimport) DDSTypeSupport;
class __declspec(dllimport) DDSDataWriter;
class __declspec(dllimport) DDSDataReader;

#endif

/* ========================================================================= */
/**
Uses:     T

Defines:  TTypeSupport, TDataWriter, TDataReader

Organized using the well-documented "Generics Pattern" for
implementing generics in C and C++.
*/

#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, start exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport __declspec(dllexport)

#endif

class NDDSUSERDllExport DDSCPPDllExport ConnextStaticRawDataTypeSupport : public ::DDSTypeSupport
{
public:
  ConnextStaticRawDataTypeSupport(bool osrf)
  {
    (void) osrf;
  };

  ~ConnextStaticRawDataTypeSupport();

  static DDS_ReturnCode_t register_type(
      DDSDomainParticipant* participant,
      const char* type_name = "ConnextStaticRawData");

  static DDS_ReturnCode_t unregister_type(
      DDSDomainParticipant* participant,
      const char* type_name = "ConnextStaticRawData");

  static const char* get_type_name();

  static ConnextStaticRawData* create_data_ex(DDS_Boolean allocatePointers);

  static ConnextStaticRawData* create_data(
      const DDS_TypeAllocationParams_t & alloc_params =
      DDS_TYPE_ALLOCATION_PARAMS_DEFAULT);

  static DDS_ReturnCode_t delete_data_ex(ConnextStaticRawData* a_data,
      DDS_Boolean deletePointers);

  static DDS_ReturnCode_t delete_data(
      ConnextStaticRawData* a_data,
      const DDS_TypeDeallocationParams_t & dealloc_params =
      DDS_TYPE_DEALLOCATION_PARAMS_DEFAULT);

  static void print_data(const ConnextStaticRawData* a_data);

  static DDS_ReturnCode_t copy_data(ConnextStaticRawData* dst_data, const ConnextStaticRawData* src_data);

  static DDS_ReturnCode_t initialize_data_ex(ConnextStaticRawData* a_data,
      DDS_Boolean allocatePointers);

  static DDS_ReturnCode_t initialize_data(
      ConnextStaticRawData* a_data,
      const DDS_TypeAllocationParams_t & alloc_params =
      DDS_TYPE_ALLOCATION_PARAMS_DEFAULT);

  static DDS_ReturnCode_t finalize_data_ex(ConnextStaticRawData* a_data,
      DDS_Boolean deletePointers);
  static DDS_ReturnCode_t finalize_data(
      ConnextStaticRawData* a_data,
      const DDS_TypeDeallocationParams_t & dealloc_params =
      DDS_TYPE_DEALLOCATION_PARAMS_DEFAULT);

  DDSDataReader* create_datareaderI(DDSDataReader* dataReader);
  DDS_ReturnCode_t destroy_datareaderI(DDSDataReader* dataReader);
  DDSDataWriter* create_datawriterI(DDSDataWriter* dataWriter);
  DDS_ReturnCode_t destroy_datawriterI(DDSDataWriter* dataWriter);

  static DDS_TypeCode* get_typecode();

  static DDS_ReturnCode_t serialize_data_to_cdr_buffer(
      char * buffer,
      unsigned int & length,
      const ConnextStaticRawData *a_data);

  static DDS_ReturnCode_t deserialize_data_from_cdr_buffer(
      ConnextStaticRawData *a_data,
      const char * buffer,
      unsigned int length);

  static DDS_ReturnCode_t data_to_string(
      ConnextStaticRawData *sample,
      char *str,
      DDS_UnsignedLong& str_size,
      const DDS_PrintFormatProperty& property);

  static void finalize();

private:
  ConnextStaticRawDataTypeSupport();
};

DDS_DATAWRITER_CPP(ConnextStaticRawDataDataWriter, ConnextStaticRawData);
DDS_DATAREADER_CPP(ConnextStaticRawDataDataReader, ConnextStaticRawDataSeq, ConnextStaticRawData);

NDDSUSERDllExport
DDS_ReturnCode_t
ConnextStaticRawDataSupport_register_external_type(
  DDSDomainParticipant * participant,
  const char * type_name,
  struct DDS_TypeCode * type_code);

#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, stop exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

#endif  /* ConnextStaticRawDataSupport_1689213465_h */
