
/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from ConnextStaticRawData.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

#include <string.h>

#ifndef ndds_cpp_h
#include "rosidl_typesupport_connext_cpp/ndds_include.hpp"
#endif

#ifndef osapi_type_h
#include "osapi/osapi_type.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif

#ifndef osapi_utility_h
#include "osapi/osapi_utility.h"
#endif

#ifndef cdr_type_h
#include "cdr/cdr_type.h"
#endif

#ifndef cdr_type_object_h
#include "cdr/cdr_typeObject.h"
#endif

#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
#endif

#ifndef cdr_stream_h
#include "cdr/cdr_stream.h"
#endif

#ifndef cdr_log_h
#include "cdr/cdr_log.h"
#endif

#ifndef pres_typePlugin_h
#include "pres/pres_typePlugin.h"
#endif

#define RTI_CDR_CURRENT_SUBMODULE RTI_CDR_SUBMODULE_MASK_STREAM

#include <new>

#include "rosidl_typesupport_connext_cpp/connext_static_raw_data_plugin.h"

/* ----------------------------------------------------------------------------
*  Type ConnextStaticRawData
* -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
Support functions:
* -------------------------------------------------------------------------- */

ConnextStaticRawData*
ConnextStaticRawDataPluginSupport_create_data_w_params(
    const struct DDS_TypeAllocationParams_t * alloc_params) 
{
    ConnextStaticRawData *sample = NULL;

    sample = new (std::nothrow) ConnextStaticRawData ;
    if (sample == NULL) {
        return NULL;
    }

    if (!ConnextStaticRawData_initialize_w_params(sample,alloc_params)) {
        delete  sample;
        sample=NULL;
    }
    return sample; 
} 

ConnextStaticRawData *
ConnextStaticRawDataPluginSupport_create_data_ex(RTIBool allocate_pointers) 
{
    ConnextStaticRawData *sample = NULL;

    sample = new (std::nothrow) ConnextStaticRawData ;

    if(sample == NULL) {
        return NULL;
    }

    if (!ConnextStaticRawData_initialize_ex(sample,allocate_pointers, RTI_TRUE)) {
        delete  sample;
        sample=NULL;
    }

    return sample; 
}

ConnextStaticRawData *
ConnextStaticRawDataPluginSupport_create_data(void)
{
    return ConnextStaticRawDataPluginSupport_create_data_ex(RTI_TRUE);
}

void 
ConnextStaticRawDataPluginSupport_destroy_data_w_params(
    ConnextStaticRawData *sample,
    const struct DDS_TypeDeallocationParams_t * dealloc_params) {

    ConnextStaticRawData_finalize_w_params(sample,dealloc_params);

    delete  sample;
    sample=NULL;
}

void 
ConnextStaticRawDataPluginSupport_destroy_data_ex(
    ConnextStaticRawData *sample,RTIBool deallocate_pointers) {

    ConnextStaticRawData_finalize_ex(sample,deallocate_pointers);

    delete  sample;
    sample=NULL;
}

void 
ConnextStaticRawDataPluginSupport_destroy_data(
    ConnextStaticRawData *sample) {

    ConnextStaticRawDataPluginSupport_destroy_data_ex(sample,RTI_TRUE);

}

RTIBool 
ConnextStaticRawDataPluginSupport_copy_data(
    ConnextStaticRawData *dst,
    const ConnextStaticRawData *src)
{
    return ConnextStaticRawData_copy(dst,(const ConnextStaticRawData*) src);
}

void 
ConnextStaticRawDataPluginSupport_print_data(
    const ConnextStaticRawData *sample,
    const char *desc,
    unsigned int indent_level)
{

    RTICdrType_printIndent(indent_level);

    if (desc != NULL) {
        RTILog_debug("%s:\n", desc);
    } else {
        RTILog_debug("\n");
    }

    if (sample == NULL) {
        RTILog_debug("NULL\n");
        return;
    }

    RTICdrType_printArray(
        sample->key_hash, ((KEY_HASH_LENGTH_16)), RTI_CDR_OCTET_SIZE,
        (RTICdrTypePrintFunction)RTICdrType_printOctet, 
        "key_hash", indent_level + 1);        

    if (DDS_OctetSeq_get_contiguous_bufferI(&sample->serialized_key) != NULL) {
        RTICdrType_printArray(
            DDS_OctetSeq_get_contiguous_bufferI(&sample->serialized_key),
            DDS_OctetSeq_get_length(&sample->serialized_key),
            RTI_CDR_OCTET_SIZE,
            (RTICdrTypePrintFunction)RTICdrType_printOctet,
            "serialized_key", indent_level + 1);
    } else {
        RTICdrType_printPointerArray(
            DDS_OctetSeq_get_discontiguous_bufferI(&sample->serialized_key),
            DDS_OctetSeq_get_length(&sample->serialized_key ),
            (RTICdrTypePrintFunction)RTICdrType_printOctet,
            "serialized_key", indent_level + 1);
    }

    if (DDS_OctetSeq_get_contiguous_bufferI(&sample->serialized_data) != NULL) {
        RTICdrType_printArray(
            DDS_OctetSeq_get_contiguous_bufferI(&sample->serialized_data),
            DDS_OctetSeq_get_length(&sample->serialized_data),
            RTI_CDR_OCTET_SIZE,
            (RTICdrTypePrintFunction)RTICdrType_printOctet,
            "serialized_data", indent_level + 1);
    } else {
        RTICdrType_printPointerArray(
            DDS_OctetSeq_get_discontiguous_bufferI(&sample->serialized_data),
            DDS_OctetSeq_get_length(&sample->serialized_data ),
            (RTICdrTypePrintFunction)RTICdrType_printOctet,
            "serialized_data", indent_level + 1);
    }

}
ConnextStaticRawData *
ConnextStaticRawDataPluginSupport_create_key_ex(RTIBool allocate_pointers){
    ConnextStaticRawData *key = NULL;

    key = new (std::nothrow) ConnextStaticRawDataKeyHolder ;

    ConnextStaticRawData_initialize_ex(key,allocate_pointers, RTI_TRUE);

    return key;
}

ConnextStaticRawData *
ConnextStaticRawDataPluginSupport_create_key(void)
{
    return  ConnextStaticRawDataPluginSupport_create_key_ex(RTI_TRUE);
}

void 
ConnextStaticRawDataPluginSupport_destroy_key_ex(
    ConnextStaticRawDataKeyHolder *key,RTIBool deallocate_pointers)
{
    ConnextStaticRawData_finalize_ex(key,deallocate_pointers);

    delete  key;
    key=NULL;

}

void 
ConnextStaticRawDataPluginSupport_destroy_key(
    ConnextStaticRawDataKeyHolder *key) {

    ConnextStaticRawDataPluginSupport_destroy_key_ex(key,RTI_TRUE);

}

/* ----------------------------------------------------------------------------
Callback functions:
* ---------------------------------------------------------------------------- */

PRESTypePluginParticipantData 
ConnextStaticRawDataPlugin_on_participant_attached(
    void *registration_data,
    const struct PRESTypePluginParticipantInfo *participant_info,
    RTIBool top_level_registration,
    void *container_plugin_context,
    RTICdrTypeCode *type_code)
{
    if (registration_data) {} /* To avoid warnings */
    if (participant_info) {} /* To avoid warnings */
    if (top_level_registration) {} /* To avoid warnings */
    if (container_plugin_context) {} /* To avoid warnings */
    if (type_code) {} /* To avoid warnings */

    return PRESTypePluginDefaultParticipantData_new(participant_info);

}

void 
ConnextStaticRawDataPlugin_on_participant_detached(
    PRESTypePluginParticipantData participant_data)
{

    PRESTypePluginDefaultParticipantData_delete(participant_data);
}

PRESTypePluginEndpointData
ConnextStaticRawDataPlugin_on_endpoint_attached(
    PRESTypePluginParticipantData participant_data,
    const struct PRESTypePluginEndpointInfo *endpoint_info,
    RTIBool top_level_registration, 
    void *containerPluginContext)
{
    PRESTypePluginEndpointData epd = NULL;

    unsigned int serializedSampleMaxSize;

    unsigned int serializedKeyMaxSize;

    if (top_level_registration) {} /* To avoid warnings */
    if (containerPluginContext) {} /* To avoid warnings */

    epd = PRESTypePluginDefaultEndpointData_new(
        participant_data,
        endpoint_info,
        (PRESTypePluginDefaultEndpointDataCreateSampleFunction)
        ConnextStaticRawDataPluginSupport_create_data,
        (PRESTypePluginDefaultEndpointDataDestroySampleFunction)
        ConnextStaticRawDataPluginSupport_destroy_data,
        (PRESTypePluginDefaultEndpointDataCreateKeyFunction)
        ConnextStaticRawDataPluginSupport_create_key ,            
        (PRESTypePluginDefaultEndpointDataDestroyKeyFunction)
        ConnextStaticRawDataPluginSupport_destroy_key);

    if (epd == NULL) {
        return NULL;
    } 
    serializedKeyMaxSize =  ConnextStaticRawDataPlugin_get_serialized_key_max_size(
        epd,RTI_FALSE,RTI_CDR_ENCAPSULATION_ID_CDR_BE,0);

    if(!PRESTypePluginDefaultEndpointData_createMD5StreamWithInfo(
        epd,endpoint_info,serializedKeyMaxSize))  
    {
        PRESTypePluginDefaultEndpointData_delete(epd);
        return NULL;
    }

    if (endpoint_info->endpointKind == PRES_TYPEPLUGIN_ENDPOINT_WRITER) {
        serializedSampleMaxSize = ConnextStaticRawDataPlugin_get_serialized_sample_max_size(
            epd,RTI_FALSE,RTI_CDR_ENCAPSULATION_ID_CDR_BE,0);

        PRESTypePluginDefaultEndpointData_setMaxSizeSerializedSample(epd, serializedSampleMaxSize);

        if (PRESTypePluginDefaultEndpointData_createWriterPool(
            epd,
            endpoint_info,
            (PRESTypePluginGetSerializedSampleMaxSizeFunction)
            ConnextStaticRawDataPlugin_get_serialized_sample_max_size, epd,
            (PRESTypePluginGetSerializedSampleSizeFunction)
            ConnextStaticRawDataPlugin_get_serialized_sample_size,
            epd) == RTI_FALSE) {
            PRESTypePluginDefaultEndpointData_delete(epd);
            return NULL;
        }
    }

    return epd;    
}

void 
ConnextStaticRawDataPlugin_on_endpoint_detached(
    PRESTypePluginEndpointData endpoint_data)
{  

    PRESTypePluginDefaultEndpointData_delete(endpoint_data);
}

void    
ConnextStaticRawDataPlugin_return_sample(
    PRESTypePluginEndpointData endpoint_data,
    ConnextStaticRawData *sample,
    void *handle)
{

    ConnextStaticRawData_finalize_optional_members(sample, RTI_TRUE);

    PRESTypePluginDefaultEndpointData_returnSample(
        endpoint_data, sample, handle);
}

RTIBool 
ConnextStaticRawDataPlugin_copy_sample(
    PRESTypePluginEndpointData endpoint_data,
    ConnextStaticRawData *dst,
    const ConnextStaticRawData *src)
{
    if (endpoint_data) {} /* To avoid warnings */
    return ConnextStaticRawDataPluginSupport_copy_data(dst,src);
}

/* ----------------------------------------------------------------------------
(De)Serialize functions:
* ------------------------------------------------------------------------- */
unsigned int 
ConnextStaticRawDataPlugin_get_serialized_sample_max_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment);

RTIBool 
ConnextStaticRawDataPlugin_serialize(
    PRESTypePluginEndpointData endpoint_data,
    const ConnextStaticRawData *sample, 
    struct RTICdrStream *stream,    
    RTIBool serialize_encapsulation,
    RTIEncapsulationId encapsulation_id,
    RTIBool serialize_sample, 
    void *endpoint_plugin_qos)
{
    char * position = NULL;
    RTIBool retval = RTI_TRUE;

    if (endpoint_data) {} /* To avoid warnings */
    if (endpoint_plugin_qos) {} /* To avoid warnings */

    /* This plugin can only be used to publish the top-level DDS Topic-Type
     * in which case serialize_encapsulation==TRUE. If that is not
     * the case then it is an error.
     */
    if (!serialize_encapsulation) {
        return RTI_FALSE;
    }

    position = RTICdrStream_resetAlignment(stream);

    if(serialize_sample) {
        /* The sample->serialized_data contains the serialized encapsulation followed by the serialized
         * data, so we only need to copy that into
         * the CDR stream. Not the key_hash, not the length of the data itself
         * The SerializedType sample->serialized_data is always a contiguous buffer
         */
        DDS_Octet *buffer = DDS_OctetSeq_get_contiguous_bufferI(&sample->serialized_data);
        if ( buffer == NULL ) {
            return RTI_FALSE;
        }

        /* The encapsulation_id appears in the sample->serialized_data as octet[2] using big-endian
         * byte order
         */
        if ( encapsulation_id != (buffer[0] * 256 + buffer[1]) ) {
            return RTI_FALSE;
        }

        /* Use RTICdrStream_serializePrimitiveArray so that there is no additional length prepended */
        if (!RTICdrStream_serializePrimitiveArray(
                stream, (void*)buffer,
            DDS_OctetSeq_get_length(&sample->serialized_data),
            RTI_CDR_OCTET_TYPE)) {
            return RTI_FALSE;
        }
    }

    RTICdrStream_restoreAlignment(stream ,position);

    return retval;
}

/**
    TODO. The code-block below does not belong here.
    It should be pushed to the CDR module, perhaps inside
    RTICdrStream_deserializeAndSetCdrEncapsulation so that the
    stream size is alredy correct when SerializedTypePlugin_deserialize_sample
    is called.

    Adjust the size of the CDR stream to not include the alignment
    padding. See http://issues.omg.org/browse/DDSXTY12-10

    @precondition The RTICdrStream *stream has alreadt processed
                  the encapsulation header and therefore has set the
                  encapsulation options returned by
                  RTICdrStream_getEncapsulationOptions()
*/
void
ConnextStaticRawDataPlugin_remove_padding_from_stream(struct RTICdrStream *stream)
{
    /* See http://issues.omg.org/browse/DDSXTY12-10 */
    DDS_UnsignedShort padding_size_mask = 0x0003;
    DDS_UnsignedShort padding_size;
    int adjustedBufferLength;

    padding_size = RTICdrStream_getEncapsulationOptions(stream) & padding_size_mask;
    adjustedBufferLength = RTICdrStream_getBufferLength(stream) - padding_size;
    RTICdrStream_setBufferLength(stream, adjustedBufferLength);
}

RTIBool 
ConnextStaticRawDataPlugin_deserialize_sample(
    PRESTypePluginEndpointData endpoint_data,
    ConnextStaticRawData *sample,
    struct RTICdrStream *stream,   
    RTIBool deserialize_encapsulation,
    RTIBool deserialize_sample, 
    void *endpoint_plugin_qos)
{
    char * position = NULL;
    RTIBool done = RTI_FALSE;

    if (endpoint_data) {} /* To avoid warnings */
    if (endpoint_plugin_qos) {} /* To avoid warnings */

    /* This plugin can only be used to publish the top-level DDS Topic-Type
     * in which case deserialize_encapsulation==TRUE. If that is not
     * the case then it is an error.
     */
    if (!deserialize_encapsulation) {
        return RTI_FALSE;
    }

    position = RTICdrStream_resetAlignment(stream);

    /* TODO. The call does not belong here. It should be pushed
     * inside RTICdrStream_deserializeAndSetCdrEncapsulation
     */
    ConnextStaticRawDataPlugin_remove_padding_from_stream(stream);

    if (deserialize_sample) {
        /* Note that sample->key_hash was already set by SerializedTypePlugin_deserialize()
           it is done there because SerializedTypePlugin_deserialize_sample does not
           have access to the SampleInfo where that information is
        */

        /* We do not set the serialized_key on deserialization */
        DDS_OctetSeq_set_length(&sample->serialized_key, 0);

        /* We copy everything that remains in the CDR stream */
        int bytesLeftInStream = RTICdrStream_getRemainder(stream);
        DDS_Octet *cdrBufferPtr  = (DDS_Octet *) RTICdrStream_getCurrentPosition(stream);
        if (cdrBufferPtr == NULL) {
            goto fin;
        }

        /* Do not call SerializedType_initialize_ex initialize here
           because it would override the key_hash field
           SerializedType_initialize_ex(sample, RTI_FALSE, RTI_FALSE);
         */
        if (!DDS_OctetSeq_from_array(&sample->serialized_data, cdrBufferPtr, bytesLeftInStream) ) {
            goto fin;
        }
        RTICdrStream_incrementCurrentPosition(stream, bytesLeftInStream);
    }

    done = RTI_TRUE;

  fin:
    if ( (done != RTI_TRUE) &&
         (RTICdrStream_getRemainder(stream) >= RTI_CDR_PARAMETER_HEADER_ALIGNMENT) ) {
        return RTI_FALSE;
    }

    RTICdrStream_restoreAlignment(stream,position);

    return RTI_TRUE;
}

RTIBool
ConnextStaticRawDataPlugin_serialize_to_cdr_buffer(
    char * buffer,
    unsigned int * length,
    const ConnextStaticRawData *sample)
{
    struct RTICdrStream stream;
    struct PRESTypePluginDefaultEndpointData epd;
    RTIBool result;

    if (length == NULL) {
        return RTI_FALSE;
    }

    epd._maxSizeSerializedSample =
    ConnextStaticRawDataPlugin_get_serialized_sample_max_size(
        NULL, RTI_TRUE, RTICdrEncapsulation_getNativeCdrEncapsulationId(), 0);

    if (buffer == NULL) {
        *length = 
        ConnextStaticRawDataPlugin_get_serialized_sample_size(
            (PRESTypePluginEndpointData)&epd,
            RTI_TRUE,
            RTICdrEncapsulation_getNativeCdrEncapsulationId(),
            0,
            sample);

        if (*length == 0) {
            return RTI_FALSE;
        }

        return RTI_TRUE;
    }    

    RTICdrStream_init(&stream);
    RTICdrStream_set(&stream, (char *)buffer, *length);

    result = ConnextStaticRawDataPlugin_serialize(
        (PRESTypePluginEndpointData)&epd, sample, &stream, 
        RTI_TRUE, RTICdrEncapsulation_getNativeCdrEncapsulationId(), 
        RTI_TRUE, NULL);  

    *length = RTICdrStream_getCurrentPositionOffset(&stream);
    return result;     
}

RTIBool
ConnextStaticRawDataPlugin_deserialize_from_cdr_buffer(
    ConnextStaticRawData *sample,
    const char * buffer,
    unsigned int length)
{
    struct RTICdrStream stream;

    RTICdrStream_init(&stream);
    RTICdrStream_set(&stream, (char *)buffer, length);

    ConnextStaticRawData_finalize_optional_members(sample, RTI_TRUE);
    return ConnextStaticRawDataPlugin_deserialize_sample( 
        NULL, sample,
        &stream, RTI_TRUE, RTI_TRUE, 
        NULL);
}

DDS_ReturnCode_t
ConnextStaticRawDataPlugin_data_to_string(
    const ConnextStaticRawData *sample,
    char *str,
    DDS_UnsignedLong *str_size, 
    const struct DDS_PrintFormatProperty *property)
{
    DDS_DynamicData *data = NULL;
    char *buffer = NULL;
    unsigned int length = 0;
    struct DDS_PrintFormat printFormat;
    DDS_ReturnCode_t retCode = DDS_RETCODE_ERROR;

    if (sample == NULL) {
        return DDS_RETCODE_BAD_PARAMETER;
    }

    if (str_size == NULL) {
        return DDS_RETCODE_BAD_PARAMETER;
    }

    if (property == NULL) {
        return DDS_RETCODE_BAD_PARAMETER;
    }

    if (!ConnextStaticRawDataPlugin_serialize_to_cdr_buffer(
        NULL, 
        &length, 
        sample)) {
        return DDS_RETCODE_ERROR;
    }

    RTIOsapiHeap_allocateBuffer(&buffer, length, RTI_OSAPI_ALIGNMENT_DEFAULT);
    if (buffer == NULL) {
        return DDS_RETCODE_ERROR;
    }

    if (!ConnextStaticRawDataPlugin_serialize_to_cdr_buffer(
        buffer, 
        &length, 
        sample)) {
        RTIOsapiHeap_freeBuffer(buffer);
        return DDS_RETCODE_ERROR;
    }

    data = DDS_DynamicData_new(
        ConnextStaticRawData_get_typecode(), 
        &DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    if (data == NULL) {
        RTIOsapiHeap_freeBuffer(buffer);
        return DDS_RETCODE_ERROR;
    }

    retCode = DDS_DynamicData_from_cdr_buffer(data, buffer, length);
    if (retCode != DDS_RETCODE_OK) {
        RTIOsapiHeap_freeBuffer(buffer);
        DDS_DynamicData_delete(data);
        return retCode;
    }

    retCode = DDS_PrintFormatProperty_to_print_format(
        property, 
        &printFormat);
    if (retCode != DDS_RETCODE_OK) {
        RTIOsapiHeap_freeBuffer(buffer);
        DDS_DynamicData_delete(data);
        return retCode;
    }

    retCode = DDS_DynamicDataFormatter_to_string_w_format(
        data, 
        str,
        str_size, 
        &printFormat);
    if (retCode != DDS_RETCODE_OK) {
        RTIOsapiHeap_freeBuffer(buffer);
        DDS_DynamicData_delete(data);
        return retCode;
    }

    RTIOsapiHeap_freeBuffer(buffer);
    DDS_DynamicData_delete(data);
    return DDS_RETCODE_OK;
}

RTIBool 
ConnextStaticRawDataPlugin_deserialize(
    PRESTypePluginEndpointData endpoint_data,
    ConnextStaticRawData **sample,
    RTIBool * drop_sample,
    struct RTICdrStream *stream,   
    RTIBool deserialize_encapsulation,
    RTIBool deserialize_sample, 
    void *endpoint_plugin_qos)
{

    RTIBool result;
    const char *METHOD_NAME = "ConnextStaticRawDataPlugin_deserialize";
    if (drop_sample) {} /* To avoid warnings */

    stream->_xTypesState.unassignable = RTI_FALSE;
    result= ConnextStaticRawDataPlugin_deserialize_sample( 
        endpoint_data, (sample != NULL)?*sample:NULL,
        stream, deserialize_encapsulation, deserialize_sample, 
        endpoint_plugin_qos);
    if (result) {
        if (stream->_xTypesState.unassignable) {
            result = RTI_FALSE;
        }
    }
    if (!result && stream->_xTypesState.unassignable ) {

        RTICdrLog_exception(
            METHOD_NAME, 
            &RTI_CDR_LOG_UNASSIGNABLE_SAMPLE_OF_TYPE_s, 
            "ConnextStaticRawData");

    }

    return result;
}

RTIBool ConnextStaticRawDataPlugin_skip(
    PRESTypePluginEndpointData endpoint_data,
    struct RTICdrStream *stream,   
    RTIBool skip_encapsulation,
    RTIBool skip_sample, 
    void *endpoint_plugin_qos)
{
    char * position = NULL;

    RTIBool done = RTI_FALSE;

    if (endpoint_data) {} /* To avoid warnings */
    if (endpoint_plugin_qos) {} /* To avoid warnings */

    if(skip_encapsulation) {
        if (!RTICdrStream_skipEncapsulation(stream)) {
            return RTI_FALSE;
        }

        position = RTICdrStream_resetAlignment(stream);
    }

    if (skip_sample) {

        if (!RTICdrStream_skipPrimitiveArray(
            stream, ((KEY_HASH_LENGTH_16)), RTI_CDR_OCTET_TYPE)) {
            goto fin; 
        }      
        {
            RTICdrUnsignedLong sequence_length;
            if (!RTICdrStream_skipPrimitiveSequence(
                stream,
                &sequence_length,
                RTI_CDR_OCTET_TYPE)){
                goto fin; 
            }
        }
        {
            RTICdrUnsignedLong sequence_length;
            if (!RTICdrStream_skipPrimitiveSequence(
                stream,
                &sequence_length,
                RTI_CDR_OCTET_TYPE)){
                goto fin; 
            }
        }
    }

    done = RTI_TRUE;
  fin:
    if (done != RTI_TRUE && 
    RTICdrStream_getRemainder(stream) >=
    RTI_CDR_PARAMETER_HEADER_ALIGNMENT) {
        return RTI_FALSE;   
    }
    if(skip_encapsulation) {
        RTICdrStream_restoreAlignment(stream,position);
    }

    return RTI_TRUE;
}

unsigned int 
ConnextStaticRawDataPlugin_get_serialized_sample_max_size_ex(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool * overflow,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment)
{

    if (endpoint_data) {} /* To avoid warnings */
    if (include_encapsulation) {}
    if (encapsulation_id) {}
    if (current_alignment) {}

    if (overflow != NULL) {
        *overflow = RTI_TRUE;
    }

    return RTI_CDR_MAX_SERIALIZED_SIZE;

}

unsigned int 
ConnextStaticRawDataPlugin_get_serialized_sample_max_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment)
{
    unsigned int size;
    RTIBool overflow = RTI_FALSE;

    size = ConnextStaticRawDataPlugin_get_serialized_sample_max_size_ex(
        endpoint_data,&overflow,include_encapsulation,encapsulation_id,current_alignment);

    if (overflow) {
        size = RTI_CDR_MAX_SERIALIZED_SIZE;
    }

    return size;
}

unsigned int 
ConnextStaticRawDataPlugin_get_serialized_sample_min_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment)
{

    unsigned int initial_alignment = current_alignment;

    unsigned int encapsulation_size = current_alignment;

    if (endpoint_data) {} /* To avoid warnings */ 

    if (include_encapsulation) {

        if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
            return 1;
        }
        RTICdrStream_getEncapsulationSize(encapsulation_size);
        encapsulation_size -= current_alignment;
        current_alignment = 0;
        initial_alignment = 0;
    }

    current_alignment +=RTICdrType_getPrimitiveArrayMaxSizeSerialized(
        current_alignment, ((KEY_HASH_LENGTH_16)), RTI_CDR_OCTET_TYPE);
    current_alignment +=    RTICdrType_getPrimitiveSequenceMaxSizeSerialized(
        current_alignment,0, RTI_CDR_OCTET_TYPE);
    current_alignment +=    RTICdrType_getPrimitiveSequenceMaxSizeSerialized(
        current_alignment,0, RTI_CDR_OCTET_TYPE);

    if (include_encapsulation) {
        current_alignment += encapsulation_size;
    }
    return  current_alignment - initial_alignment;
}

/* Returns the size of the sample in its serialized form (in bytes).
* It can also be an estimation in excess of the real buffer needed 
* during a call to the serialize() function.
* The value reported does not have to include the space for the
* encapsulation flags.
*/
unsigned int
ConnextStaticRawDataPlugin_get_serialized_sample_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment,
    const ConnextStaticRawData * sample) 
{

    unsigned int initial_alignment = current_alignment;

    unsigned int encapsulation_size = current_alignment;
    struct PRESTypePluginDefaultEndpointData epd;   

    if (sample==NULL) {
        return 0;
    }
    if (endpoint_data == NULL) {
        endpoint_data = (PRESTypePluginEndpointData) &epd;
        PRESTypePluginDefaultEndpointData_setBaseAlignment(
            endpoint_data,
            current_alignment);        
    }

    if (include_encapsulation) {

        if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
            return 1;
        }
        RTICdrStream_getEncapsulationSize(encapsulation_size);
        encapsulation_size -= current_alignment;
        current_alignment = 0;
        initial_alignment = 0;
        PRESTypePluginDefaultEndpointData_setBaseAlignment(
            endpoint_data,
            current_alignment);
    }

    current_alignment += RTICdrType_getPrimitiveArrayMaxSizeSerialized(
        PRESTypePluginDefaultEndpointData_getAlignment(
            endpoint_data, current_alignment), 
            ((KEY_HASH_LENGTH_16)), RTI_CDR_OCTET_TYPE);  

    current_alignment += RTICdrType_getPrimitiveSequenceSerializedSize(
        PRESTypePluginDefaultEndpointData_getAlignment(
            endpoint_data, current_alignment), 
            DDS_OctetSeq_get_length(&sample->serialized_key),
            RTI_CDR_OCTET_TYPE);

    current_alignment += RTICdrType_getPrimitiveSequenceSerializedSize(
        PRESTypePluginDefaultEndpointData_getAlignment(
            endpoint_data, current_alignment), 
            DDS_OctetSeq_get_length(&sample->serialized_data),
            RTI_CDR_OCTET_TYPE);

    if (include_encapsulation) {
        current_alignment += encapsulation_size;
    }
    return current_alignment - initial_alignment;
}

/* --------------------------------------------------------------------------------------
Key Management functions:
* -------------------------------------------------------------------------------------- */

PRESTypePluginKeyKind 
ConnextStaticRawDataPlugin_get_key_kind(void)
{
    return PRES_TYPEPLUGIN_NO_KEY;
}

RTIBool 
ConnextStaticRawDataPlugin_serialize_key(
    PRESTypePluginEndpointData endpoint_data,
    const ConnextStaticRawData *sample, 
    struct RTICdrStream *stream,    
    RTIBool serialize_encapsulation,
    RTIEncapsulationId encapsulation_id,
    RTIBool serialize_key,
    void *endpoint_plugin_qos)
{
    char * position = NULL;

    if (endpoint_data) {} /* To avoid warnings */
    if (endpoint_plugin_qos) {} /* To avoid warnings */

    if(serialize_encapsulation) {
        if (!RTICdrStream_serializeAndSetCdrEncapsulation(stream , encapsulation_id)) {
            return RTI_FALSE;
        }

        position = RTICdrStream_resetAlignment(stream);
    }

    if(serialize_key) {

        if (!RTICdrStream_serializePrimitiveArray(
            stream, (void*) sample->key_hash, ((KEY_HASH_LENGTH_16)), RTI_CDR_OCTET_TYPE)) {
            return RTI_FALSE;
        }

    }

    if(serialize_encapsulation) {
        RTICdrStream_restoreAlignment(stream,position);
    }

    return RTI_TRUE;
}

RTIBool ConnextStaticRawDataPlugin_deserialize_key_sample(
    PRESTypePluginEndpointData endpoint_data,
    ConnextStaticRawData *sample, 
    struct RTICdrStream *stream,
    RTIBool deserialize_encapsulation,
    RTIBool deserialize_key,
    void *endpoint_plugin_qos)
{
    try {

        char * position = NULL;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */

        if(deserialize_encapsulation) {

            if (!RTICdrStream_deserializeAndSetCdrEncapsulation(stream)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }
        if (deserialize_key) {

            if (!RTICdrStream_deserializePrimitiveArray(
                stream, (void*) sample->key_hash, ((KEY_HASH_LENGTH_16)), RTI_CDR_OCTET_TYPE)) {
                return RTI_FALSE;
            }

        }

        if(deserialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;

    } catch (std::bad_alloc&) {
        return RTI_FALSE;
    }
}

RTIBool ConnextStaticRawDataPlugin_deserialize_key(
    PRESTypePluginEndpointData endpoint_data,
    ConnextStaticRawData **sample, 
    RTIBool * drop_sample,
    struct RTICdrStream *stream,
    RTIBool deserialize_encapsulation,
    RTIBool deserialize_key,
    void *endpoint_plugin_qos)
{
    RTIBool result;
    if (drop_sample) {} /* To avoid warnings */
    stream->_xTypesState.unassignable = RTI_FALSE;
    result= ConnextStaticRawDataPlugin_deserialize_key_sample(
        endpoint_data, (sample != NULL)?*sample:NULL, stream,
        deserialize_encapsulation, deserialize_key, endpoint_plugin_qos);
    if (result) {
        if (stream->_xTypesState.unassignable) {
            result = RTI_FALSE;
        }
    }

    return result;    

}

unsigned int
ConnextStaticRawDataPlugin_get_serialized_key_max_size_ex(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool * overflow,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment)
{

    unsigned int initial_alignment = current_alignment;

    unsigned int encapsulation_size = current_alignment;

    if (endpoint_data) {} /* To avoid warnings */
    if (overflow) {} /* To avoid warnings */

    if (include_encapsulation) {

        if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
            return 1;
        }
        RTICdrStream_getEncapsulationSize(encapsulation_size);
        encapsulation_size -= current_alignment;
        current_alignment = 0;
        initial_alignment = 0;
    }

    current_alignment +=RTICdrType_getPrimitiveArrayMaxSizeSerialized(
        current_alignment, ((KEY_HASH_LENGTH_16)),  RTI_CDR_OCTET_TYPE);

    if (include_encapsulation) {
        current_alignment += encapsulation_size;
    }
    return current_alignment - initial_alignment;
}

unsigned int
ConnextStaticRawDataPlugin_get_serialized_key_max_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment)
{
    unsigned int size;
    RTIBool overflow = RTI_FALSE;

    size = ConnextStaticRawDataPlugin_get_serialized_key_max_size_ex(
        endpoint_data,&overflow,include_encapsulation,encapsulation_id,current_alignment);

    if (overflow) {
        size = RTI_CDR_MAX_SERIALIZED_SIZE;
    }

    return size;
}

RTIBool 
ConnextStaticRawDataPlugin_serialized_sample_to_key(
    PRESTypePluginEndpointData endpoint_data,
    ConnextStaticRawData *sample,
    struct RTICdrStream *stream, 
    RTIBool deserialize_encapsulation,  
    RTIBool deserialize_key, 
    void *endpoint_plugin_qos)
{
    char * position = NULL;

    RTIBool done = RTI_FALSE;
    RTIBool error = RTI_FALSE;

    if (endpoint_data) {} /* To avoid warnings */
    if (endpoint_plugin_qos) {} /* To avoid warnings */

    if (stream == NULL) {
        error = RTI_TRUE;
        goto fin;
    }
    if(deserialize_encapsulation) {
        if (!RTICdrStream_deserializeAndSetCdrEncapsulation(stream)) {
            return RTI_FALSE;
        }
        position = RTICdrStream_resetAlignment(stream);
    }

    if (deserialize_key) {

        if (!RTICdrStream_deserializePrimitiveArray(
            stream, (void*) sample->key_hash, ((KEY_HASH_LENGTH_16)), RTI_CDR_OCTET_TYPE)) {
            return RTI_FALSE;
        }

        {
            RTICdrUnsignedLong sequence_length;
            if (!RTICdrStream_skipPrimitiveSequence(
                stream,
                &sequence_length,
                RTI_CDR_OCTET_TYPE)){
                goto fin; 
            }
        }

        {
            RTICdrUnsignedLong sequence_length;
            if (!RTICdrStream_skipPrimitiveSequence(
                stream,
                &sequence_length,
                RTI_CDR_OCTET_TYPE)){
                goto fin; 
            }
        }

    }

    done = RTI_TRUE;
  fin:
    if(!error) {
        if (done != RTI_TRUE && 
        RTICdrStream_getRemainder(stream) >=
        RTI_CDR_PARAMETER_HEADER_ALIGNMENT) {
            return RTI_FALSE;   
        }
    } else {
        return RTI_FALSE;
    }       

    if(deserialize_encapsulation) {
        RTICdrStream_restoreAlignment(stream,position);
    }

    return RTI_TRUE;
}

RTIBool 
ConnextStaticRawDataPlugin_instance_to_key(
    PRESTypePluginEndpointData endpoint_data,
    ConnextStaticRawDataKeyHolder *dst, 
    const ConnextStaticRawData *src)
{

    if (endpoint_data) {} /* To avoid warnings */   

    if (!RTICdrType_copyArray(
        dst->key_hash ,src->key_hash,((KEY_HASH_LENGTH_16)), RTI_CDR_OCTET_SIZE)) {
        return RTI_FALSE;
    }
    return RTI_TRUE;
}

RTIBool 
ConnextStaticRawDataPlugin_key_to_instance(
    PRESTypePluginEndpointData endpoint_data,
    ConnextStaticRawData *dst, const
    ConnextStaticRawDataKeyHolder *src)
{

    if (endpoint_data) {} /* To avoid warnings */   
    if (!RTICdrType_copyArray(
        dst->key_hash ,src->key_hash,((KEY_HASH_LENGTH_16)), RTI_CDR_OCTET_SIZE)) {
        return RTI_FALSE;
    }
    return RTI_TRUE;
}

RTIBool 
ConnextStaticRawDataPlugin_instance_to_keyhash(
    PRESTypePluginEndpointData endpoint_data,
    DDS_KeyHash_t *keyhash,
    const ConnextStaticRawData *instance)
{
    struct RTICdrStream * md5Stream = NULL;
    struct RTICdrStreamState cdrState;
    char * buffer = NULL;

    RTICdrStreamState_init(&cdrState);
    md5Stream = PRESTypePluginDefaultEndpointData_getMD5Stream(endpoint_data);

    if (md5Stream == NULL) {
        return RTI_FALSE;
    }

    RTICdrStream_resetPosition(md5Stream);
    RTICdrStream_setDirtyBit(md5Stream, RTI_TRUE);

    if (!ConnextStaticRawDataPlugin_serialize_key(
        endpoint_data,
        instance,
        md5Stream, 
        RTI_FALSE, 
        RTI_CDR_ENCAPSULATION_ID_CDR_BE, 
        RTI_TRUE,
        NULL)) 
    {
        int size;

        RTICdrStream_pushState(md5Stream, &cdrState, -1);

        size = (int)ConnextStaticRawDataPlugin_get_serialized_sample_size(
            endpoint_data,
            RTI_FALSE,
            RTI_CDR_ENCAPSULATION_ID_CDR_BE,
            0,
            instance);

        if (size <= RTICdrStream_getBufferLength(md5Stream)) {
            RTICdrStream_popState(md5Stream, &cdrState);        
            return RTI_FALSE;
        }   

        RTIOsapiHeap_allocateBuffer(&buffer,size,0);

        if (buffer == NULL) {
            RTICdrStream_popState(md5Stream, &cdrState);
            return RTI_FALSE;
        }

        RTICdrStream_set(md5Stream, buffer, size);
        RTIOsapiMemory_zero(
            RTICdrStream_getBuffer(md5Stream),
            RTICdrStream_getBufferLength(md5Stream));
        RTICdrStream_resetPosition(md5Stream);
        RTICdrStream_setDirtyBit(md5Stream, RTI_TRUE);
        if (!ConnextStaticRawDataPlugin_serialize_key(
            endpoint_data,
            instance,
            md5Stream, 
            RTI_FALSE, 
            RTI_CDR_ENCAPSULATION_ID_CDR_BE, 
            RTI_TRUE,
            NULL)) 
        {
            RTICdrStream_popState(md5Stream, &cdrState);
            RTIOsapiHeap_freeBuffer(buffer);
            return RTI_FALSE;
        }        
    }   

    if (PRESTypePluginDefaultEndpointData_getMaxSizeSerializedKey(endpoint_data) > 
    (unsigned int)(MIG_RTPS_KEY_HASH_MAX_LENGTH) ||
    PRESTypePluginDefaultEndpointData_forceMD5KeyHash(endpoint_data)) {
        RTICdrStream_computeMD5(md5Stream, keyhash->value);
    } else {
        RTIOsapiMemory_zero(keyhash->value,MIG_RTPS_KEY_HASH_MAX_LENGTH);
        RTIOsapiMemory_copy(
            keyhash->value, 
            RTICdrStream_getBuffer(md5Stream), 
            RTICdrStream_getCurrentPositionOffset(md5Stream));
    }

    keyhash->length = MIG_RTPS_KEY_HASH_MAX_LENGTH;

    if (buffer != NULL) {
        RTICdrStream_popState(md5Stream, &cdrState);
        RTIOsapiHeap_freeBuffer(buffer);
    }

    return RTI_TRUE;
}

RTIBool 
ConnextStaticRawDataPlugin_serialized_sample_to_keyhash(
    PRESTypePluginEndpointData endpoint_data,
    struct RTICdrStream *stream, 
    DDS_KeyHash_t *keyhash,
    RTIBool deserialize_encapsulation,
    void *endpoint_plugin_qos) 
{   
    char * position = NULL;

    RTIBool done = RTI_FALSE;
    RTIBool error = RTI_FALSE;
    ConnextStaticRawData * sample=NULL;

    if (endpoint_plugin_qos) {} /* To avoid warnings */
    if (stream == NULL) {
        error = RTI_TRUE;
        goto fin;
    }

    if(deserialize_encapsulation) {
        if (!RTICdrStream_deserializeAndSetCdrEncapsulation(stream)) {
            return RTI_FALSE;
        }

        position = RTICdrStream_resetAlignment(stream);
    }

    sample = (ConnextStaticRawData *)
    PRESTypePluginDefaultEndpointData_getTempSample(endpoint_data);

    if (sample == NULL) {
        return RTI_FALSE;
    }

    if (!RTICdrStream_deserializePrimitiveArray(
        stream, (void*) sample->key_hash, ((KEY_HASH_LENGTH_16)), RTI_CDR_OCTET_TYPE)) {
        return RTI_FALSE;
    }

    done = RTI_TRUE;
  fin:
    if(!error) {
        if (done != RTI_TRUE && 
        RTICdrStream_getRemainder(stream) >=
        RTI_CDR_PARAMETER_HEADER_ALIGNMENT) {
            return RTI_FALSE;   
        }
    } else {
        return RTI_FALSE;
    } 

    if(deserialize_encapsulation) {
        RTICdrStream_restoreAlignment(stream,position);
    }

    if (!ConnextStaticRawDataPlugin_instance_to_keyhash(
        endpoint_data, keyhash, sample)) {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/* ------------------------------------------------------------------------
* Plug-in Installation Methods
* ------------------------------------------------------------------------ */
struct PRESTypePlugin *ConnextStaticRawDataPlugin_new(void) 
{ 
  return NULL;
}

struct PRESTypePlugin *
ConnextStaticRawDataPlugin_new_external(struct DDS_TypeCode * external_type_code)
{
    if (external_type_code == NULL) {
      return NULL;
    }

    struct PRESTypePlugin *plugin = NULL;
    const struct PRESTypePluginVersion PLUGIN_VERSION =
    PRES_TYPE_PLUGIN_VERSION_2_0;

    RTIOsapiHeap_allocateStructure(
        &plugin, struct PRESTypePlugin);

    if (plugin == NULL) {
        return NULL;
    }

    plugin->version = PLUGIN_VERSION;

    /* set up parent's function pointers */
    plugin->onParticipantAttached =
    (PRESTypePluginOnParticipantAttachedCallback)
    ConnextStaticRawDataPlugin_on_participant_attached;
    plugin->onParticipantDetached =
    (PRESTypePluginOnParticipantDetachedCallback)
    ConnextStaticRawDataPlugin_on_participant_detached;
    plugin->onEndpointAttached =
    (PRESTypePluginOnEndpointAttachedCallback)
    ConnextStaticRawDataPlugin_on_endpoint_attached;
    plugin->onEndpointDetached =
    (PRESTypePluginOnEndpointDetachedCallback)
    ConnextStaticRawDataPlugin_on_endpoint_detached;

    plugin->copySampleFnc =
    (PRESTypePluginCopySampleFunction)
    ConnextStaticRawDataPlugin_copy_sample;
    plugin->createSampleFnc =
    (PRESTypePluginCreateSampleFunction)
    ConnextStaticRawDataPlugin_create_sample;
    plugin->destroySampleFnc =
    (PRESTypePluginDestroySampleFunction)
    ConnextStaticRawDataPlugin_destroy_sample;

    plugin->serializeFnc =
    (PRESTypePluginSerializeFunction)
    ConnextStaticRawDataPlugin_serialize;
    plugin->deserializeFnc =
    (PRESTypePluginDeserializeFunction)
    ConnextStaticRawDataPlugin_deserialize;
    plugin->getSerializedSampleMaxSizeFnc =
    (PRESTypePluginGetSerializedSampleMaxSizeFunction)
    ConnextStaticRawDataPlugin_get_serialized_sample_max_size;
    plugin->getSerializedSampleMinSizeFnc =
    (PRESTypePluginGetSerializedSampleMinSizeFunction)
    ConnextStaticRawDataPlugin_get_serialized_sample_min_size;

    plugin->getSampleFnc =
    (PRESTypePluginGetSampleFunction)
    ConnextStaticRawDataPlugin_get_sample;
    plugin->returnSampleFnc =
    (PRESTypePluginReturnSampleFunction)
    ConnextStaticRawDataPlugin_return_sample;

    //plugin->getKeyKindFnc = ConnextStaticRawDataPlugin_get_external_key_kind(external_type_code);
    plugin->getKeyKindFnc = (PRESTypePluginGetKeyKindFunction)
      ConnextStaticRawDataPlugin_get_key_kind;

    plugin->getSerializedKeyMaxSizeFnc =
    (PRESTypePluginGetSerializedKeyMaxSizeFunction)
    ConnextStaticRawDataPlugin_get_serialized_key_max_size;
    plugin->serializeKeyFnc =
    (PRESTypePluginSerializeKeyFunction)
    ConnextStaticRawDataPlugin_serialize_key;
    plugin->deserializeKeyFnc =
    (PRESTypePluginDeserializeKeyFunction)
    ConnextStaticRawDataPlugin_deserialize_key;
    plugin->deserializeKeySampleFnc =
    (PRESTypePluginDeserializeKeySampleFunction)
    ConnextStaticRawDataPlugin_deserialize_key_sample;

    plugin-> instanceToKeyHashFnc =
    (PRESTypePluginInstanceToKeyHashFunction)
    ConnextStaticRawDataPlugin_instance_to_keyhash;
    plugin->serializedSampleToKeyHashFnc =
    (PRESTypePluginSerializedSampleToKeyHashFunction)
    ConnextStaticRawDataPlugin_serialized_sample_to_keyhash;

    plugin->getKeyFnc =
    (PRESTypePluginGetKeyFunction)
    ConnextStaticRawDataPlugin_get_key;
    plugin->returnKeyFnc =
    (PRESTypePluginReturnKeyFunction)
    ConnextStaticRawDataPlugin_return_key;

    plugin->instanceToKeyFnc =
    (PRESTypePluginInstanceToKeyFunction)
    ConnextStaticRawDataPlugin_instance_to_key;
    plugin->keyToInstanceFnc =
    (PRESTypePluginKeyToInstanceFunction)
    ConnextStaticRawDataPlugin_key_to_instance;
    plugin->serializedKeyToKeyHashFnc = NULL; /* Not supported yet */
    //plugin->typeCode =  (struct RTICdrTypeCode *)ConnextStaticRawData_get_typecode();
    plugin->typeCode =  (struct RTICdrTypeCode *)external_type_code;

    plugin->languageKind = PRES_TYPEPLUGIN_CPP_LANG;

    /* Serialized buffer */
    plugin->getBuffer =
    (PRESTypePluginGetBufferFunction)
    ConnextStaticRawDataPlugin_get_buffer;
    plugin->returnBuffer =
    (PRESTypePluginReturnBufferFunction)
    ConnextStaticRawDataPlugin_return_buffer;
    plugin->getSerializedSampleSizeFnc =
    (PRESTypePluginGetSerializedSampleSizeFunction)
    ConnextStaticRawDataPlugin_get_serialized_sample_size;

    plugin->endpointTypeName = ConnextStaticRawDataTYPENAME;

    return plugin;
}

void
ConnextStaticRawDataPlugin_delete(struct PRESTypePlugin *plugin)
{
    RTIOsapiHeap_freeStructure(plugin);
} 
#undef RTI_CDR_CURRENT_SUBMODULE 
