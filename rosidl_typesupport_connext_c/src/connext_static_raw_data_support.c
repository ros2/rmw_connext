
/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from ConnextStaticRawData.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

#include "rosidl_typesupport_connext_c/connext_static_raw_data_support.h"
#include "rosidl_typesupport_connext_c/connext_static_raw_data_plugin.h"

/* ========================================================================= */
/**
<<IMPLEMENTATION>>

Defines:   TData,
TDataWriter,
TDataReader,
TTypeSupport

Configure and implement 'ConnextStaticRawData' support classes.

Note: Only the #defined classes get defined
*/

/* ----------------------------------------------------------------- */
/* DDSDataWriter
*/

/**
<<IMPLEMENTATION >>

Defines:   TDataWriter, TData
*/

/* Requires */
#define TTYPENAME   ConnextStaticRawDataTYPENAME

/* Defines */
#define TDataWriter ConnextStaticRawDataDataWriter
#define TData       ConnextStaticRawData

#include "dds_c/generic/dds_c_data_TDataWriter.gen"

#undef TDataWriter
#undef TData

#undef TTYPENAME

/* ----------------------------------------------------------------- */
/* DDSDataReader
*/

/**
<<IMPLEMENTATION >>

Defines:   TDataReader, TDataSeq, TData
*/

/* Requires */
#define TTYPENAME   ConnextStaticRawDataTYPENAME

/* Defines */
#define TDataReader ConnextStaticRawDataDataReader
#define TDataSeq    ConnextStaticRawDataSeq
#define TData       ConnextStaticRawData

#include "dds_c/generic/dds_c_data_TDataReader.gen"

#undef TDataReader
#undef TDataSeq
#undef TData

#undef TTYPENAME

/* ----------------------------------------------------------------- */
/* TypeSupport

<<IMPLEMENTATION >>

Requires:  TTYPENAME,
TPlugin_new
TPlugin_delete
Defines:   TTypeSupport, TData, TDataReader, TDataWriter
*/

/* Requires */
#define TTYPENAME    ConnextStaticRawDataTYPENAME
#define TPlugin_new  ConnextStaticRawDataPlugin_new
#define TPlugin_delete  ConnextStaticRawDataPlugin_delete

/* Defines */
#define TTypeSupport ConnextStaticRawDataTypeSupport
#define TData        ConnextStaticRawData
#define TDataReader  ConnextStaticRawDataDataReader
#define TDataWriter  ConnextStaticRawDataDataWriter
#define TGENERATE_SER_CODE
#define TGENERATE_TYPECODE

#include "dds_c/generic/dds_c_data_TTypeSupport.gen"

#undef TTypeSupport
#undef TData
#undef TDataReader
#undef TDataWriter
#undef TGENERATE_TYPECODE
#undef TGENERATE_SER_CODE
#undef TTYPENAME
#undef TPlugin_new
#undef TPlugin_delete

/*
   The ConnextStaticRawData needs to be registered passing the
   TypeCode of the underlying type. That will ensure the right
   type information is propagated on the wire.

   Note that ConnextStaticRawDataTypeSupport_register_type() is defined by
   the macros above. We can't avoid it the way the code is generated.
   That function should not be called. Instead the application should
   call TTypeSupport_register_external_type()

   serialized_key_max_size should be set to 0 if the type is Unkeyed. Otherwise
   it shoud be set to the maximum size of the serialized key of the
   underlying type defined by the type_code paramameter.

   TODO: Would be nice to redefine ConnextStaticRawDataTypeSupport_register_type()
   to have the implementation below instead of having to add a new register_external_type()
   function. At least we could find a way to prevent register_type() from being
   called...
*/
DDS_ReturnCode_t ConnextStaticRawDataTypeSupport_register_external_type(
	DDS_DomainParticipant* participant,
	const char* type_name,
	struct DDS_TypeCode *type_code)
{
	struct PRESTypePlugin *presTypePlugin = NULL;
	DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

	if (participant == NULL) {
		goto finError;
	}

	/* TODO pass the type_code */
	presTypePlugin = ConnextStaticRawDataPlugin_new_external(type_code);
	if (presTypePlugin == NULL) {
		goto finError;
	}

	retcode = DDS_DomainParticipant_register_type(
		participant,
		type_name,
		presTypePlugin,
		NULL /* registration_data */);
	if (retcode != DDS_RETCODE_OK) {
		goto finError;
	}
	return DDS_RETCODE_OK;

finError:
	if (presTypePlugin != NULL) {
		ConnextStaticRawDataPlugin_delete(presTypePlugin);
	}

	return retcode;
}
