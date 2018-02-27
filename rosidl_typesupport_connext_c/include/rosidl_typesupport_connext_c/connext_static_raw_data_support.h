
/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from ConnextStaticRawData.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

#ifndef ConnextStaticRawDataSupport_1419555957_h
#define ConnextStaticRawDataSupport_1419555957_h

/* Uses */
#include "rosidl_typesupport_connext_c/connext_static_raw_data.h"

#ifndef ndds_c_h
#include "rosidl_typesupport_connext_c/ndds_include.h"
#endif

#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)

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

DDS_TYPESUPPORT_C(ConnextStaticRawDataTypeSupport, ConnextStaticRawData);
DDS_DATAWRITER_C(ConnextStaticRawDataDataWriter, ConnextStaticRawData);
DDS_DATAREADER_C(ConnextStaticRawDataDataReader, ConnextStaticRawDataSeq, ConnextStaticRawData);

NDDSUSERDllExport DDSCDllExport
DDS_ReturnCode_t ConnextStaticRawDataTypeSupport_register_external_type(
	DDS_DomainParticipant* participant,
	const char* type_name,
	struct DDS_TypeCode *type_code);

#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, stop exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

#endif  /* ConnextStaticRawDataSupport_1419555957_h */

