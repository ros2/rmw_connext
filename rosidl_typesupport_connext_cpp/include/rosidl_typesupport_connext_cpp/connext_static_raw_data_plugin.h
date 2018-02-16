

/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from ConnextStaticRawData.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

#ifndef ConnextStaticRawDataPlugin_1689213465_h
#define ConnextStaticRawDataPlugin_1689213465_h

#include "connext_static_raw_data.h"

struct RTICdrStream;

#ifndef pres_typePlugin_h
#include "pres/pres_typePlugin.h"
#endif

#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, start exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport __declspec(dllexport)
#endif

extern "C" {

    /* The type used to store keys for instances of type struct
    * AnotherSimple.
    *
    * By default, this type is struct ConnextStaticRawData
    * itself. However, if for some reason this choice is not practical for your
    * system (e.g. if sizeof(struct ConnextStaticRawData)
    * is very large), you may redefine this typedef in terms of another type of
    * your choosing. HOWEVER, if you define the KeyHolder type to be something
    * other than struct AnotherSimple, the
    * following restriction applies: the key of struct
    * ConnextStaticRawData must consist of a
    * single field of your redefined KeyHolder type and that field must be the
    * first field in struct ConnextStaticRawData.
    */
    typedef  class ConnextStaticRawData ConnextStaticRawDataKeyHolder;

    #define ConnextStaticRawDataPlugin_get_sample PRESTypePluginDefaultEndpointData_getSample 
    #define ConnextStaticRawDataPlugin_get_buffer PRESTypePluginDefaultEndpointData_getBuffer 
    #define ConnextStaticRawDataPlugin_return_buffer PRESTypePluginDefaultEndpointData_returnBuffer 

    #define ConnextStaticRawDataPlugin_get_key PRESTypePluginDefaultEndpointData_getKey 
    #define ConnextStaticRawDataPlugin_return_key PRESTypePluginDefaultEndpointData_returnKey

    #define ConnextStaticRawDataPlugin_create_sample PRESTypePluginDefaultEndpointData_createSample 
    #define ConnextStaticRawDataPlugin_destroy_sample PRESTypePluginDefaultEndpointData_deleteSample 

    /* --------------------------------------------------------------------------------------
    Support functions:
    * -------------------------------------------------------------------------------------- */

    NDDSUSERDllExport extern ConnextStaticRawData*
    ConnextStaticRawDataPluginSupport_create_data_w_params(
        const struct DDS_TypeAllocationParams_t * alloc_params);

    NDDSUSERDllExport extern ConnextStaticRawData*
    ConnextStaticRawDataPluginSupport_create_data_ex(RTIBool allocate_pointers);

    NDDSUSERDllExport extern ConnextStaticRawData*
    ConnextStaticRawDataPluginSupport_create_data(void);

    NDDSUSERDllExport extern RTIBool 
    ConnextStaticRawDataPluginSupport_copy_data(
        ConnextStaticRawData *out,
        const ConnextStaticRawData *in);

    NDDSUSERDllExport extern void 
    ConnextStaticRawDataPluginSupport_destroy_data_w_params(
        ConnextStaticRawData *sample,
        const struct DDS_TypeDeallocationParams_t * dealloc_params);

    NDDSUSERDllExport extern void 
    ConnextStaticRawDataPluginSupport_destroy_data_ex(
        ConnextStaticRawData *sample,RTIBool deallocate_pointers);

    NDDSUSERDllExport extern void 
    ConnextStaticRawDataPluginSupport_destroy_data(
        ConnextStaticRawData *sample);

    NDDSUSERDllExport extern void 
    ConnextStaticRawDataPluginSupport_print_data(
        const ConnextStaticRawData *sample,
        const char *desc,
        unsigned int indent);

    NDDSUSERDllExport extern ConnextStaticRawData*
    ConnextStaticRawDataPluginSupport_create_key_ex(RTIBool allocate_pointers);

    NDDSUSERDllExport extern ConnextStaticRawData*
    ConnextStaticRawDataPluginSupport_create_key(void);

    NDDSUSERDllExport extern void 
    ConnextStaticRawDataPluginSupport_destroy_key_ex(
        ConnextStaticRawDataKeyHolder *key,RTIBool deallocate_pointers);

    NDDSUSERDllExport extern void 
    ConnextStaticRawDataPluginSupport_destroy_key(
        ConnextStaticRawDataKeyHolder *key);

    /* ----------------------------------------------------------------------------
    Callback functions:
    * ---------------------------------------------------------------------------- */

    NDDSUSERDllExport extern PRESTypePluginParticipantData 
    ConnextStaticRawDataPlugin_on_participant_attached(
        void *registration_data, 
        const struct PRESTypePluginParticipantInfo *participant_info,
        RTIBool top_level_registration, 
        void *container_plugin_context,
        RTICdrTypeCode *typeCode);

    NDDSUSERDllExport extern void 
    ConnextStaticRawDataPlugin_on_participant_detached(
        PRESTypePluginParticipantData participant_data);

    NDDSUSERDllExport extern PRESTypePluginEndpointData 
    ConnextStaticRawDataPlugin_on_endpoint_attached(
        PRESTypePluginParticipantData participant_data,
        const struct PRESTypePluginEndpointInfo *endpoint_info,
        RTIBool top_level_registration, 
        void *container_plugin_context);

    NDDSUSERDllExport extern void 
    ConnextStaticRawDataPlugin_on_endpoint_detached(
        PRESTypePluginEndpointData endpoint_data);

    NDDSUSERDllExport extern void    
    ConnextStaticRawDataPlugin_return_sample(
        PRESTypePluginEndpointData endpoint_data,
        ConnextStaticRawData *sample,
        void *handle);    

    NDDSUSERDllExport extern RTIBool 
    ConnextStaticRawDataPlugin_copy_sample(
        PRESTypePluginEndpointData endpoint_data,
        ConnextStaticRawData *out,
        const ConnextStaticRawData *in);

    /* ----------------------------------------------------------------------------
    (De)Serialize functions:
    * ------------------------------------------------------------------------- */

    NDDSUSERDllExport extern RTIBool 
    ConnextStaticRawDataPlugin_serialize(
        PRESTypePluginEndpointData endpoint_data,
        const ConnextStaticRawData *sample,
        struct RTICdrStream *stream, 
        RTIBool serialize_encapsulation,
        RTIEncapsulationId encapsulation_id,
        RTIBool serialize_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool 
    ConnextStaticRawDataPlugin_deserialize_sample(
        PRESTypePluginEndpointData endpoint_data,
        ConnextStaticRawData *sample, 
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool
    ConnextStaticRawDataPlugin_serialize_to_cdr_buffer(
        char * buffer,
        unsigned int * length,
        const ConnextStaticRawData *sample); 

    NDDSUSERDllExport extern RTIBool 
    ConnextStaticRawDataPlugin_deserialize(
        PRESTypePluginEndpointData endpoint_data,
        ConnextStaticRawData **sample, 
        RTIBool * drop_sample,
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool
    ConnextStaticRawDataPlugin_deserialize_from_cdr_buffer(
        ConnextStaticRawData *sample,
        const char * buffer,
        unsigned int length);    
    NDDSUSERDllExport extern DDS_ReturnCode_t
    ConnextStaticRawDataPlugin_data_to_string(
        const ConnextStaticRawData *sample,
        char *str,
        DDS_UnsignedLong *str_size, 
        const struct DDS_PrintFormatProperty *property);    

    NDDSUSERDllExport extern RTIBool
    ConnextStaticRawDataPlugin_skip(
        PRESTypePluginEndpointData endpoint_data,
        struct RTICdrStream *stream, 
        RTIBool skip_encapsulation,  
        RTIBool skip_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern unsigned int 
    ConnextStaticRawDataPlugin_get_serialized_sample_max_size_ex(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool * overflow,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);    

    NDDSUSERDllExport extern unsigned int 
    ConnextStaticRawDataPlugin_get_serialized_sample_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern unsigned int 
    ConnextStaticRawDataPlugin_get_serialized_sample_min_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern unsigned int
    ConnextStaticRawDataPlugin_get_serialized_sample_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment,
        const ConnextStaticRawData * sample);

    /* --------------------------------------------------------------------------------------
    Key Management functions:
    * -------------------------------------------------------------------------------------- */
    NDDSUSERDllExport extern PRESTypePluginKeyKind 
    ConnextStaticRawDataPlugin_get_key_kind(void);

    NDDSUSERDllExport extern unsigned int 
    ConnextStaticRawDataPlugin_get_serialized_key_max_size_ex(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool * overflow,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern unsigned int 
    ConnextStaticRawDataPlugin_get_serialized_key_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern RTIBool 
    ConnextStaticRawDataPlugin_serialize_key(
        PRESTypePluginEndpointData endpoint_data,
        const ConnextStaticRawData *sample,
        struct RTICdrStream *stream,
        RTIBool serialize_encapsulation,
        RTIEncapsulationId encapsulation_id,
        RTIBool serialize_key,
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool 
    ConnextStaticRawDataPlugin_deserialize_key_sample(
        PRESTypePluginEndpointData endpoint_data,
        ConnextStaticRawData * sample,
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_key,
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool 
    ConnextStaticRawDataPlugin_deserialize_key(
        PRESTypePluginEndpointData endpoint_data,
        ConnextStaticRawData ** sample,
        RTIBool * drop_sample,
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_key,
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool
    ConnextStaticRawDataPlugin_serialized_sample_to_key(
        PRESTypePluginEndpointData endpoint_data,
        ConnextStaticRawData *sample,
        struct RTICdrStream *stream, 
        RTIBool deserialize_encapsulation,  
        RTIBool deserialize_key, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool 
    ConnextStaticRawDataPlugin_instance_to_key(
        PRESTypePluginEndpointData endpoint_data,
        ConnextStaticRawDataKeyHolder *key, 
        const ConnextStaticRawData *instance);

    NDDSUSERDllExport extern RTIBool 
    ConnextStaticRawDataPlugin_key_to_instance(
        PRESTypePluginEndpointData endpoint_data,
        ConnextStaticRawData *instance, 
        const ConnextStaticRawDataKeyHolder *key);

    NDDSUSERDllExport extern RTIBool 
    ConnextStaticRawDataPlugin_instance_to_keyhash(
        PRESTypePluginEndpointData endpoint_data,
        DDS_KeyHash_t *keyhash,
        const ConnextStaticRawData *instance);

    NDDSUSERDllExport extern RTIBool 
    ConnextStaticRawDataPlugin_serialized_sample_to_keyhash(
        PRESTypePluginEndpointData endpoint_data,
        struct RTICdrStream *stream, 
        DDS_KeyHash_t *keyhash,
        RTIBool deserialize_encapsulation,
        void *endpoint_plugin_qos); 

    /* Plugin Functions */
    NDDSUSERDllExport extern struct PRESTypePlugin*
    ConnextStaticRawDataPlugin_new(void);

    NDDSUSERDllExport
    extern struct PRESTypePlugin *
    ConnextStaticRawDataPlugin_new_external(struct DDS_TypeCode * external_type_code);

    NDDSUSERDllExport extern void
    ConnextStaticRawDataPlugin_delete(struct PRESTypePlugin *);

}

#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, stop exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

#endif /* ConnextStaticRawDataPlugin_1689213465_h */

