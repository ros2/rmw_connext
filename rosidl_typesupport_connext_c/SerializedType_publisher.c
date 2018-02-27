
/**/

#include <stdio.h>
#include <stdlib.h>
#include "ndds/ndds_c.h"
#include "SerializedType.h"
#include "SerializedTypeSupport.h"

//#include "ShapeType.h"
//#include "ShapeTypePlugin.h"
#include "DynamicArrayPrimitives_.h"
#include "DynamicArrayPrimitives_Plugin.h"
#include "DynamicArrayPrimitives_Support.h"

/* Delete all entities */
static int publisher_shutdown(
    DDS_DomainParticipant *participant)
{
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (participant != NULL) {
        retcode = DDS_DomainParticipant_delete_contained_entities(participant);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDS_DomainParticipantFactory_delete_participant(
            DDS_TheParticipantFactory, participant);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_participant error %d\n", retcode);
            status = -1;
        }
    }

    /* RTI Data Distribution Service provides finalize_instance() method on
    domain participant factory for people who want to release memory used
    by the participant factory. Uncomment the following block of code for
    clean destruction of the singleton. */
    /*
    retcode = DDS_DomainParticipantFactory_finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "finalize_instance error %d\n", retcode);
        status = -1;
    }
    */

    return status;
}

void strcpy_s(char * str, int lenMax, const char * strToCopy) {
    if (!str || !strToCopy) {
        return;
    }
    int strToCopyLen = strlen(strToCopy) + 1;
    if(strToCopyLen < lenMax) {
        lenMax = strToCopyLen;
    }
    strncpy(str, strToCopy, lenMax);
    str[lenMax - 1] = '\0';
}

static int publisher_main(int domainId, int sample_count)
{
    DDS_DomainParticipant *participant = NULL;
    DDS_Publisher *publisher = NULL;
    DDS_Topic *topic = NULL;
    DDS_DataWriter *writer = NULL;
    SerializedTypeDataWriter *SerializedType_writer = NULL;
    SerializedType *instance = NULL;
    DDS_ReturnCode_t retcode;
    DDS_InstanceHandle_t instance_handle = DDS_HANDLE_NIL;
    int count = 0, i = 0;  
    struct DDS_Duration_t send_period = {1,0};

    /* To customize participant QoS, use 
    the configuration file USER_QOS_PROFILES.xml */
    participant = DDS_DomainParticipantFactory_create_participant(
        DDS_TheParticipantFactory, domainId, &DDS_PARTICIPANT_QOS_DEFAULT,
        NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        fprintf(stderr, "create_participant error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* To customize publisher QoS, use 
    the configuration file USER_QOS_PROFILES.xml */
    publisher = DDS_DomainParticipant_create_publisher(
        participant, &DDS_PUBLISHER_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (publisher == NULL) {
        fprintf(stderr, "create_publisher error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* When regiatering the Data-Type with DDS it is important to use
	   the correct type code which corresponds to the serilized data-format
	   otherwise the type would be seen as incompatible by others and tools
	   like visualization will not work.

	   Here since the serialized data corresponds to a ShapeType we
	   use the ShapeType_get_typecode()
	*/
  const char * type_name = test_msgs_msg_dds__DynamicArrayPrimitives_TypeSupport_get_type_name();
  struct DDS_TypeCode * type_code = test_msgs_msg_dds__DynamicArrayPrimitives__get_typecode();
	retcode = SerializedTypeTypeSupport_register_external_type(
		participant, type_name, type_code);

	if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "register_type error %d\n", retcode);
        publisher_shutdown(participant);
        return -1;
    }

    /* To customize topic QoS, use 
    the configuration file USER_QOS_PROFILES.xml */
    topic = DDS_DomainParticipant_create_topic(
        participant, "my_generic_type",
        type_name, &DDS_TOPIC_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        fprintf(stderr, "create_topic error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* To customize data writer QoS, use 
    the configuration file USER_QOS_PROFILES.xml */
    writer = DDS_Publisher_create_datawriter(
        publisher, topic,
        &DDS_DATAWRITER_QOS_DEFAULT, NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (writer == NULL) {
        fprintf(stderr, "create_datawriter error\n");
        publisher_shutdown(participant);
        return -1;
    }
    SerializedType_writer = SerializedTypeDataWriter_narrow(writer);
    if (SerializedType_writer == NULL) {
        fprintf(stderr, "DataWriter narrow error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* Create data sample for writing */
    instance = SerializedTypeTypeSupport_create_data_ex(DDS_BOOLEAN_TRUE);
    if (instance == NULL) {
        fprintf(stderr, "SerializedTypeTypeSupport_create_data error\n");
        publisher_shutdown(participant);
        return -1;
    }
//	/* We take advantage of the ShapeTypePlugin to serialize the data
//	 * At the application layer
//	 */
//	ShapeType shapeType;
//	ShapeType_initialize(&shapeType);
//
//#define NUMBER_OF_COLORS (4)
//	char *colors[NUMBER_OF_COLORS] = { "GREEN", "RED", "BLUE", "YELLOW" };
//	int  xbase[NUMBER_OF_COLORS] = { 10, 50, 100, 150 };
//	int  ybase[NUMBER_OF_COLORS] = {  0,  0,  0,   0 };
//
//	/* Memory area where to put the serialized (ShapeType) data */
//	unsigned int serializationLength;
//	unsigned int serializationBufferSize = ShapeTypePlugin_get_serialized_sample_max_size(NULL, RTI_TRUE, 0, 0);
//
//	/* RTI_CDR_MAX_SERIALIZED_SIZE indites the type is unbounded normally the application
//	   would have some knwledge of the size. Here we print an error in this situation */
//	if (serializationBufferSize == RTI_CDR_MAX_SERIALIZED_SIZE) {
//		fprintf(stderr, "Type is unbounded. Please enter buffer size manually here\n");
//		return publisher_shutdown(participant);
//	}

  test_msgs_msg_dds__DynamicArrayPrimitives_ dynamic_array_instance;
  test_msgs_msg_dds__DynamicArrayPrimitives__initialize(&dynamic_array_instance);
  DDS_LongSeq_initialize(&dynamic_array_instance.int32_values_);
  size_t length = 100;
  DDS_LongSeq_set_length(&dynamic_array_instance.int32_values_, length);
  for (size_t i = 0; i < length; ++i) {
    DDS_Long * element = DDS_LongSeq_get_reference(&dynamic_array_instance.int32_values_, i);
    *element = i;
  }

  unsigned int serializationLength;
  unsigned int serializationBufferSize = test_msgs_msg_dds__DynamicArrayPrimitives_Plugin_get_serialized_sample_max_size(NULL, RTI_TRUE, 0, 0);

	DDS_Octet *serializationBuffer = (DDS_Octet *)malloc(serializationBufferSize);

	for (count = 0; (sample_count == 0) || (count < sample_count); ++count) {

		printf("Writing DynamicAraryPrimitives, count %d\n", count);

		/* Modify the data to be written here */

		/* Use ShapeTypePlugin to serialize into an application buffer.
		   Serialization includes the 4-byte SerializationHeader.

		   Note: serializationLength on input it is the maximum size.
		         On successful output it is the number of bytes used for 
				 the serialization 
	    */
		serializationLength = serializationBufferSize;
		if (!test_msgs_msg_dds__DynamicArrayPrimitives_Plugin_serialize_to_cdr_buffer((char *)serializationBuffer, &serializationLength, &dynamic_array_instance)) {
			fprintf(stderr, "Serialization of ShapeType failed\n");
		}
		else {
			/* At this point:
			      serializationBuffer  - contains the serialized shapeType
				  serializationLength  - contains the number of bytes in serializationBuffer used by the serialization 
		     */
			/* Use DDS_OctetSeq_loan_contiguous() instead of DDS_OctetSeq_copy() to save one copy */
			DDS_OctetSeq_loan_contiguous(&(instance->serialized_data), serializationBuffer,
				serializationLength, serializationBufferSize);

			/* TODO: Use ShapeType_serialize_key */ 
//			for (i = 0; i < 16; ++i) {
//				instance->key_hash[i] = (char)(count % NUMBER_OF_COLORS);
//			}

			retcode = SerializedTypeDataWriter_write(
				SerializedType_writer, instance, &instance_handle);
			if (retcode != DDS_RETCODE_OK) {
				fprintf(stderr, "write error %d\n", retcode);
			}
			DDS_OctetSeq_unloan(&(instance->serialized_data));
		}

        NDDS_Utility_sleep(&send_period);
    }

    /*
    retcode = SerializedTypeDataWriter_unregister_instance(
        SerializedType_writer, instance, &instance_handle);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "unregister instance error %d\n", retcode);
    }
    */

    /* Delete data sample */
    retcode = SerializedTypeTypeSupport_delete_data_ex(instance, DDS_BOOLEAN_TRUE);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "SerializedTypeTypeSupport_delete_data error %d\n", retcode);
    }

    /* Cleanup and delete delete all entities */ 
	free(serializationBuffer);
    return publisher_shutdown(participant);
}

int main(int argc, char *argv[])
{
    int domainId = 0;
    int sample_count = 0; /* infinite loop */

    if (argc >= 2) {
        domainId = atoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }

    /* Uncomment this to turn on additional logging
    NDDS_Config_Logger_set_verbosity_by_category(
        NDDS_Config_Logger_get_instance(),
        NDDS_CONFIG_LOG_CATEGORY_API, 
        NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
    */

    return publisher_main(domainId, sample_count);
}

