#include <iostream>
#include <stdexcept>

#include "ndds/ndds_cpp.h"

#include <rmw/rmw.h>
#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include "rosidl_generator_cpp/MessageTypeSupport.h"
#include <rmw/types.h>
#include "rosidl_typesupport_introspection_cpp/FieldTypes.h"
#include "rosidl_typesupport_introspection_cpp/Identifier.h"
#include "rosidl_typesupport_introspection_cpp/MessageIntrospection.h"

#include "rosidl_generator_cpp/ServiceTypeSupport.h"

extern "C"
{

RMW_PUBLIC const char * rti_connext_dynamic_identifier = "connext_dynamic";

const char *
rmw_get_implementation_identifier()
{
    return rti_connext_dynamic_identifier;
}

rmw_ret_t
rmw_init()
{
    DDSDomainParticipantFactory* dpf_ = DDSDomainParticipantFactory::get_instance();
    if (!dpf_) {
        rmw_set_error_string("  init() could not get participant factory");
        return RMW_RET_ERROR;
    };
}

rmw_node_t *
rmw_create_node(const char * name)
{
    DDSDomainParticipantFactory* dpf_ = DDSDomainParticipantFactory::get_instance();
    if (!dpf_) {
        rmw_set_error_string("  create_node() could not get participant factory");
        return NULL;
    };

    // use loopback interface to enable cross vendor communication
    DDS_DomainParticipantQos participant_qos;
    DDS_ReturnCode_t status = dpf_->get_default_participant_qos(participant_qos);
    if (status != DDS_RETCODE_OK)
    {
        rmw_set_error_string("  create_node() could not get participant qos");
        return NULL;
    }
    status = DDSPropertyQosPolicyHelper::add_property(participant_qos.property,
        "dds.transport.UDPv4.builtin.ignore_loopback_interface",
        "0",
        DDS_BOOLEAN_FALSE);
    if (status != DDS_RETCODE_OK)
    {
        rmw_set_error_string("  create_node() could not add qos property");
        return NULL;
    }

    DDS_DomainId_t domain = 0;

    DDSDomainParticipant* participant = dpf_->create_participant(
        //domain, DDS_PARTICIPANT_QOS_DEFAULT, NULL,
        domain, participant_qos, NULL,
        DDS_STATUS_MASK_NONE);
    if (!participant) {
        rmw_set_error_string("  create_node() could not create participant");
        return NULL;
    };

    rmw_node_t * node = new rmw_node_t;
    node->implementation_identifier = rti_connext_dynamic_identifier;
    node->data = participant;
    return node;
}

DDS_TypeCode * create_type_code(std::string type_name, const rosidl_typesupport_introspection_cpp::MessageMembers * members, DDS_DomainParticipantQos& participant_qos)
{
    DDS_TypeCodeFactory * factory = NULL;
    factory = DDS_TypeCodeFactory::get_instance();
    if (!factory) {
        rmw_set_error_string("  create_type_code() could not get typecode factory");
        throw std::runtime_error("could not get typecode factory");
    };

    DDS_StructMemberSeq struct_members;
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    DDS_TypeCode * type_code = factory->create_struct_tc(type_name.c_str(), struct_members, ex);
    for(unsigned long i = 0; i < members->member_count_; ++i)
    {
        const rosidl_typesupport_introspection_cpp::MessageMember * member = members->members_ + i;
        const DDS_TypeCode * member_type_code;
        // TODO support arrays: create_array_tc / create_sequence_tc
        switch (member->type_id_)
        {
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
                member_type_code = factory->get_primitive_tc(DDS_TK_BOOLEAN);
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
                member_type_code = factory->get_primitive_tc(DDS_TK_OCTET);
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
                member_type_code = factory->get_primitive_tc(DDS_TK_CHAR);
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
                member_type_code = factory->get_primitive_tc(DDS_TK_FLOAT);
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
                member_type_code = factory->get_primitive_tc(DDS_TK_DOUBLE);
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
                member_type_code = factory->get_primitive_tc(DDS_TK_OCTET);
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
                member_type_code = factory->get_primitive_tc(DDS_TK_OCTET);
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
                member_type_code = factory->get_primitive_tc(DDS_TK_SHORT);
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
                member_type_code = factory->get_primitive_tc(DDS_TK_USHORT);
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
                member_type_code = factory->get_primitive_tc(DDS_TK_LONG);
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
                member_type_code = factory->get_primitive_tc(DDS_TK_ULONG);
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
                member_type_code = factory->get_primitive_tc(DDS_TK_LONGLONG);
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
                member_type_code = factory->get_primitive_tc(DDS_TK_ULONGLONG);
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
                {
                    DDS_UnsignedLong max_string_size;
                    if (member->string_upper_bound_)
                    {
                        max_string_size = member->string_upper_bound_;
                    }
                    else
                    {
                        max_string_size = std::string().max_size();
                    }
                    member_type_code = factory->create_string_tc(max_string_size, ex);
                }
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE:
                {
                    const ::rosidl_typesupport_introspection_cpp::MessageMembers* sub_members = (const ::rosidl_typesupport_introspection_cpp::MessageMembers*)member->members_->data;
                    std::string field_type_name = std::string(sub_members->package_name_) + "::dds_::" + sub_members->message_name_ + "_";
                    member_type_code = create_type_code(field_type_name, sub_members, participant_qos);
                }
                break;
            default:
                printf("unknown type id %u\n", member->type_id_);
                throw std::runtime_error("unknown type");
        }
        type_code->add_member((std::string(member->name_) + "_").c_str(), DDS_TYPECODE_MEMBER_ID_INVALID, member_type_code,
                    DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, ex);
    }
    type_code->print_IDL(1, ex);
    DDS_StructMemberSeq_finalize(&struct_members);
    return type_code;
}


struct CustomPublisherInfo {
  DDSDynamicDataTypeSupport * dynamic_data_type_support_;
  DDSDynamicDataWriter * dynamic_writer_;
  DDS_TypeCode * type_code_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * members_;
  DDS_DynamicData * dynamic_data;
};

rmw_publisher_t *
rmw_create_publisher(const rmw_node_t * node,
                     const rosidl_message_type_support_t * type_support,
                     const char * topic_name,
                     size_t queue_size)
{
    if (node->implementation_identifier != rti_connext_dynamic_identifier)
    {
        rmw_set_error_string("node handle not from this implementation");
        // printf("but from: %s\n", node->implementation_identifier);
        return NULL;
    }

    DDSDomainParticipant* participant = (DDSDomainParticipant*)node->data;

    if (type_support->typesupport_identifier != rosidl_typesupport_introspection_cpp::typesupport_introspection_identifier)
    {
        rmw_set_error_string("type support not from this implementation");
        // printf("but from: %s\n", type_support->typesupport_identifier);
        return NULL;
    }

    const rosidl_typesupport_introspection_cpp::MessageMembers * members = (rosidl_typesupport_introspection_cpp::MessageMembers*)type_support->data;
    std::string type_name = std::string(members->package_name_) + "::dds_::" + members->message_name_ + "_";

    DDS_DomainParticipantQos participant_qos;
    DDS_ReturnCode_t status = participant->get_qos(participant_qos);
    if (status != DDS_RETCODE_OK) {
        rmw_set_error_string("failed to get participant qos");
        // printf("get_qos() failed. Status = %d\n", status);
        return NULL;
    };

    DDS_TypeCode * type_code = create_type_code(type_name, members, participant_qos);


    DDSDynamicDataTypeSupport* ddts = new DDSDynamicDataTypeSupport(
        type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
    status = ddts->register_type(participant, type_name.c_str());
    if (status != DDS_RETCODE_OK) {
        rmw_set_error_string("failed to register type");
        // printf("register_type() failed. Status = %d\n", status);
        return NULL;
    };


    DDS_PublisherQos publisher_qos;
    status = participant->get_default_publisher_qos(publisher_qos);
    if (status != DDS_RETCODE_OK) {
        rmw_set_error_string("failed to get default publisher qos");
        // printf("get_default_publisher_qos() failed. Status = %d\n", status);
        return NULL;
    };

    DDSPublisher* dds_publisher = participant->create_publisher(
        publisher_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!dds_publisher) {
        rmw_set_error_string("  create_publisher() could not create publisher");
        return NULL;
    };


    DDS_TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS_RETCODE_OK) {
        rmw_set_error_string("failed to get default topic qos");
        // printf("get_default_topic_qos() failed. Status = %d\n", status);
        return NULL;
    };

    DDSTopic* topic = participant->create_topic(
        topic_name, type_name.c_str(), default_topic_qos, NULL,
        DDS_STATUS_MASK_NONE
    );
    if (!topic) {
        rmw_set_error_string("  create_topic() could not create topic");
        return NULL;
    };


    DDS_DataWriterQos default_datawriter_qos;
    status = participant->get_default_datawriter_qos(default_datawriter_qos);
    if (status != DDS_RETCODE_OK) {
        rmw_set_error_string("failed to get default datawriter qos");
        // printf("get_default_datawriter_qos() failed. Status = %d\n", status);
        return NULL;
    };

    DDSDataWriter* topic_writer = dds_publisher->create_datawriter(
        topic, default_datawriter_qos,
        NULL, DDS_STATUS_MASK_NONE);

    DDSDynamicDataWriter* dynamic_writer = DDSDynamicDataWriter::narrow(topic_writer);
    if (!dynamic_writer) {
        rmw_set_error_string("narrow() failed.");
        return NULL;
    };

    DDS_DynamicData * dynamic_data = ddts->create_data();

    CustomPublisherInfo* custom_publisher_info = new CustomPublisherInfo();
    custom_publisher_info->dynamic_data_type_support_ = ddts;
    custom_publisher_info->dynamic_writer_ = dynamic_writer;
    custom_publisher_info->type_code_ = type_code;
    custom_publisher_info->members_ = members;
    custom_publisher_info->dynamic_data = dynamic_data;

    rmw_publisher_t * publisher = new rmw_publisher_t;
    publisher->implementation_identifier = rti_connext_dynamic_identifier;
    publisher->data = custom_publisher_info;
    return publisher;
}

#define SET_VALUE(TYPE, METHOD_NAME) \
    { \
        TYPE value = *((TYPE*)((char*)ros_message + member->offset_)); \
        DDS_ReturnCode_t status = dynamic_data->METHOD_NAME((std::string(member->name_) + "_").c_str(), DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, value); \
        if (status != DDS_RETCODE_OK) { \
            printf(#METHOD_NAME "() failed. Status = %d\n", status); \
            throw std::runtime_error("set member failed"); \
        } \
    }

#define SET_VALUE_WITH_SUFFIX(TYPE, METHOD_NAME, SUFFIX) \
    { \
        TYPE value = *((TYPE*)((char*)ros_message + member->offset_)); \
        DDS_ReturnCode_t status = dynamic_data->METHOD_NAME((std::string(member->name_) + "_").c_str(), DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, value SUFFIX); \
        if (status != DDS_RETCODE_OK) { \
            printf(#METHOD_NAME "() failed. Status = %d\n", status); \
            throw std::runtime_error("set member failed"); \
        } \
    }

void _publish(DDS_DynamicData * dynamic_data, const void * ros_message, const rosidl_typesupport_introspection_cpp::MessageMembers * members)
{
    //DDS_DynamicData dynamic_data(type_code, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    //DDS_DynamicData * dynamic_data = ddts->create_data();
    for(unsigned long i = 0; i < members->member_count_; ++i)
    {
        const rosidl_typesupport_introspection_cpp::MessageMember * member = members->members_ + i;
        DDS_TypeCode * member_type_code;
        switch (member->type_id_)
        {
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
                SET_VALUE(bool, set_boolean)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
                SET_VALUE(uint8_t, set_octet)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
                SET_VALUE(char, set_char)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
                SET_VALUE(float, set_float)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
                SET_VALUE(double, set_double)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
                SET_VALUE(int8_t, set_octet)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
                SET_VALUE(uint8_t, set_octet)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
                SET_VALUE(int16_t, set_short)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
                SET_VALUE(uint16_t, set_ushort)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
                SET_VALUE(int32_t, set_long)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
                SET_VALUE(uint32_t, set_ulong)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
                SET_VALUE(int64_t, set_longlong)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
                SET_VALUE(uint64_t, set_ulonglong)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
                SET_VALUE_WITH_SUFFIX(std::string, set_string, .c_str())
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE:
                {
                    DDS_DynamicData sub_dynamic_data(0, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
                    dynamic_data->bind_complex_member(sub_dynamic_data, (std::string(member->name_) + "_").c_str(), DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
                    void * sub_ros_message = (void*)((char*)ros_message + member->offset_);
                    const ::rosidl_typesupport_introspection_cpp::MessageMembers* sub_members = (const ::rosidl_typesupport_introspection_cpp::MessageMembers*)member->members_->data;
                    _publish(&sub_dynamic_data, sub_ros_message, sub_members);
                    dynamic_data->unbind_complex_member(sub_dynamic_data);
                }
                break;
            default:
                printf("unknown type id %u\n", member->type_id_);
                throw std::runtime_error("unknown type");
        }
    }
}

rmw_ret_t
rmw_publish(const rmw_publisher_t * publisher, const void * ros_message)
{
    if (publisher->implementation_identifier != rti_connext_dynamic_identifier)
    {
        rmw_set_error_string("publisher handle not from this implementation");
        // printf("but from: %s\n", publisher->implementation_identifier);
        return RMW_RET_ERROR;
    }

    CustomPublisherInfo * custom_publisher_info = (CustomPublisherInfo*)publisher->data;
    DDSDynamicDataTypeSupport* ddts = custom_publisher_info->dynamic_data_type_support_;
    DDSDynamicDataWriter * dynamic_writer = custom_publisher_info->dynamic_writer_;
    DDS_TypeCode * type_code = custom_publisher_info->type_code_;
    const rosidl_typesupport_introspection_cpp::MessageMembers * members = custom_publisher_info->members_;
    DDS_DynamicData * dynamic_data = custom_publisher_info->dynamic_data;

    _publish(dynamic_data, ros_message, members);

    DDS_ReturnCode_t status = dynamic_writer->write(*dynamic_data, DDS_HANDLE_NIL);
    if (status != DDS_RETCODE_OK) {
        rmw_set_error_string("failed to write");
        // printf("write() failed. Status = %d\n", status);
        return RMW_RET_ERROR;
    };

    //ddts->delete_data(dynamic_data);
    return RMW_RET_OK;
}

struct CustomSubscriberInfo {
  DDSDynamicDataTypeSupport * dynamic_data_type_support_;
  DDSDynamicDataReader * dynamic_reader_;
  DDS_TypeCode * type_code_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * members_;
  DDS_DynamicData * dynamic_data;
};

rmw_subscription_t *
rmw_create_subscription(const rmw_node_t * node,
                        const rosidl_message_type_support_t * type_support,
                        const char * topic_name,
                        size_t queue_size)
{
    if (node->implementation_identifier != rti_connext_dynamic_identifier)
    {
        rmw_set_error_string("node handle not from this implementation");
        // printf("but from: %s\n", node->implementation_identifier);
        return NULL;
    }

    DDSDomainParticipant* participant = (DDSDomainParticipant*)node->data;

    if (type_support->typesupport_identifier != rosidl_typesupport_introspection_cpp::typesupport_introspection_identifier)
    {
        rmw_set_error_string("type support not from this implementation");
        // printf("but from: %s\n", type_support->typesupport_identifier);
        return NULL;
    }

    const rosidl_typesupport_introspection_cpp::MessageMembers * members = (rosidl_typesupport_introspection_cpp::MessageMembers*)type_support->data;
    std::string type_name = std::string(members->package_name_) + "::dds_::" + members->message_name_ + "_";

    DDS_DomainParticipantQos participant_qos;
    DDS_ReturnCode_t status = participant->get_qos(participant_qos);
    if (status != DDS_RETCODE_OK) {
        rmw_set_error_string("failed to get participant qos");
        // printf("get_qos() failed. Status = %d\n", status);
        return NULL;
    };

    DDS_TypeCode * type_code = create_type_code(type_name, members, participant_qos);


    DDSDynamicDataTypeSupport* ddts = new DDSDynamicDataTypeSupport(
        type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
    status = ddts->register_type(participant, type_name.c_str());
    if (status != DDS_RETCODE_OK) {
        rmw_set_error_string("failed to register type");
        // printf("register_type() failed. Status = %d\n", status);
        return NULL;
    };


    DDS_SubscriberQos subscriber_qos;
    status = participant->get_default_subscriber_qos(subscriber_qos);
    if (status != DDS_RETCODE_OK) {
        rmw_set_error_string("failed to get default subscriber qos");
        // printf("get_default_subscriber_qos() failed. Status = %d\n", status);
        return NULL;
    };

    DDSSubscriber* dds_subscriber = participant->create_subscriber(
        subscriber_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!dds_subscriber) {
        rmw_set_error_string("  create_subscriber() could not create subscriber");
        return NULL;
    };


    DDS_TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS_RETCODE_OK) {
        rmw_set_error_string("failed to get default topic qos");
        // printf("get_default_topic_qos() failed. Status = %d\n", status);
        return NULL;
    };

    DDSTopic* topic = participant->create_topic(
        topic_name, type_name.c_str(), default_topic_qos, NULL,
        DDS_STATUS_MASK_NONE
    );
    if (!topic) {
        rmw_set_error_string("  create_topic() could not create topic");
        return NULL;
    };


    DDS_DataReaderQos default_datareader_qos;
    status = participant->get_default_datareader_qos(default_datareader_qos);
    if (status != DDS_RETCODE_OK) {
        rmw_set_error_string("failed to get default datareader qos");
        // printf("get_default_datareader_qos() failed. Status = %d\n", status);
        return NULL;
    };

    DDSDataReader* topic_reader = dds_subscriber->create_datareader(
        topic, default_datareader_qos,
        NULL, DDS_STATUS_MASK_NONE);

    DDSDynamicDataReader* dynamic_reader = DDSDynamicDataReader::narrow(topic_reader);
    if (!dynamic_reader) {
        rmw_set_error_string("narrow() failed.\n");
        return NULL;
    };

    DDS_DynamicData * dynamic_data = ddts->create_data();

    CustomSubscriberInfo* custom_subscriber_info = new CustomSubscriberInfo();
    custom_subscriber_info->dynamic_data_type_support_ = ddts;
    custom_subscriber_info->dynamic_reader_ = dynamic_reader;
    custom_subscriber_info->type_code_ = type_code;
    custom_subscriber_info->members_ = members;
    custom_subscriber_info->dynamic_data = dynamic_data;

    rmw_subscription_t * subscription = new rmw_subscription_t;
    subscription->implementation_identifier = rti_connext_dynamic_identifier;
    subscription->data = custom_subscriber_info;
    return subscription;
}

#define GET_VALUE(TYPE, METHOD_NAME) \
    { \
        TYPE value; \
        DDS_ReturnCode_t status = dynamic_data->METHOD_NAME(value, (std::string(member->name_) + "_").c_str(), DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED); \
        if (status != DDS_RETCODE_OK) { \
            printf(#METHOD_NAME "() failed. Status = %d\n", status); \
            throw std::runtime_error("get member failed"); \
        } \
        TYPE* ros_value = (TYPE*)((char*)ros_message + member->offset_); \
        *ros_value = value; \
    }

#define GET_VALUE_WITH_DIFFERENT_TYPES(TYPE, DDS_TYPE, METHOD_NAME) \
    { \
        DDS_TYPE value; \
        DDS_ReturnCode_t status = dynamic_data->METHOD_NAME(value, (std::string(member->name_) + "_").c_str(), DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED); \
        if (status != DDS_RETCODE_OK) { \
            printf(#METHOD_NAME "() failed. Status = %d\n", status); \
            throw std::runtime_error("get member failed"); \
        } \
        TYPE* ros_value = (TYPE*)((char*)ros_message + member->offset_); \
        *ros_value = value; \
    }

void _take(DDS_DynamicData * dynamic_data, void * ros_message, const rosidl_typesupport_introspection_cpp::MessageMembers * members)
{
    //DDS_DynamicData dynamic_data(type_code, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    //DDS_DynamicData * dynamic_data = ddts->create_data();
    for(unsigned long i = 0; i < members->member_count_; ++i)
    {
        const rosidl_typesupport_introspection_cpp::MessageMember * member = members->members_ + i;
        switch (member->type_id_)
        {
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
                GET_VALUE_WITH_DIFFERENT_TYPES(bool, DDS_Boolean, get_boolean)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
                GET_VALUE(uint8_t, get_octet)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
                GET_VALUE(char, get_char)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
                GET_VALUE(float, get_float)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
                GET_VALUE(double, get_double)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
                GET_VALUE_WITH_DIFFERENT_TYPES(int8_t, DDS_Octet,get_octet)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
                GET_VALUE(uint8_t, get_octet)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
                GET_VALUE(int16_t, get_short)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
                GET_VALUE(uint16_t, get_ushort)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
                GET_VALUE(int32_t, get_long)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
                GET_VALUE(uint32_t, get_ulong)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
                GET_VALUE_WITH_DIFFERENT_TYPES(int64_t, DDS_LongLong, get_longlong)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
                GET_VALUE_WITH_DIFFERENT_TYPES(uint64_t, DDS_UnsignedLongLong, get_ulonglong)
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
                {
                    char * value = 0;
                    DDS_UnsignedLong size;
                    DDS_ReturnCode_t status = dynamic_data->get_string(value, &size, (std::string(member->name_) + "_").c_str(), DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
                    if (status != DDS_RETCODE_OK) {
                        printf("get_string() failed. Status = %d\n", status);
                        throw std::runtime_error("get member failed");
                    }
                    std::string* ros_value = (std::string*)((char*)ros_message + member->offset_);
                    *ros_value = value;
                    delete[] value;
                }
                break;
            case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE:
                {
                    DDS_DynamicData sub_dynamic_data(0, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
                    dynamic_data->bind_complex_member(sub_dynamic_data, (std::string(member->name_) + "_").c_str(), DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
                    void * sub_ros_message = (void*)((char*)ros_message + member->offset_);
                    const ::rosidl_typesupport_introspection_cpp::MessageMembers* sub_members = (const ::rosidl_typesupport_introspection_cpp::MessageMembers*)member->members_->data;
                    _take(&sub_dynamic_data, sub_ros_message, sub_members);
                    dynamic_data->unbind_complex_member(sub_dynamic_data);
                }
                break;
            default:
                printf("unknown type id %u\n", member->type_id_);
                throw std::runtime_error("unknown type");
        }
    }
}

rmw_ret_t
rmw_take(const rmw_subscription_t * subscription, void * ros_message, bool * taken)
{
    if (subscription->implementation_identifier != rti_connext_dynamic_identifier)
    {
        rmw_set_error_string("subscriber handle not from this implementation");
        // printf("but from: %s\n", subscription->implementation_identifier);
        return RMW_RET_ERROR;
    }

    CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo*)subscription->data;
    DDSDynamicDataTypeSupport* ddts = custom_subscriber_info->dynamic_data_type_support_;
    DDSDynamicDataReader * dynamic_reader = custom_subscriber_info->dynamic_reader_;
    DDS_TypeCode * type_code = custom_subscriber_info->type_code_;
    const rosidl_typesupport_introspection_cpp::MessageMembers * members = custom_subscriber_info->members_;

    DDS_DynamicDataSeq dynamic_data_sequence;
    DDS_SampleInfoSeq sample_infos;
    DDS_ReturnCode_t status = dynamic_reader->take(
        dynamic_data_sequence,
        sample_infos,
        1,
        DDS_NOT_READ_SAMPLE_STATE,
        DDS_ANY_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE);
    if (status == DDS_RETCODE_NO_DATA) {
        *taken = false;
        return RMW_RET_OK;
    }
    if (status != DDS_RETCODE_OK) {
        rmw_set_error_string("failed to take");
        // printf("take() failed. Status = %d\n", status);
        return RMW_RET_ERROR;
    };

    if (ros_message == 0) {
        rmw_set_error_string("take() invoked without a valid ROS message pointer\n");
        return RMW_RET_ERROR;
    };

    _take(&dynamic_data_sequence[0], ros_message, members);

    dynamic_reader->return_loan(dynamic_data_sequence, sample_infos);

    *taken = true;

    return RMW_RET_OK;
}

rmw_guard_condition_t *
rmw_create_guard_condition()
{
    rmw_guard_condition_t * guard_condition_handle = new rmw_guard_condition_t;
    guard_condition_handle->implementation_identifier = rti_connext_dynamic_identifier;
    guard_condition_handle->data = new DDSGuardCondition();
    return guard_condition_handle;
}

rmw_ret_t
rmw_destroy_guard_condition(rmw_guard_condition_t * guard_condition)
{
  if (guard_condition) {
    delete guard_condition->data;
    delete guard_condition;
    return RMW_RET_OK;
  }

  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_trigger_guard_condition(const rmw_guard_condition_t * guard_condition_handle)
{
    if (guard_condition_handle->implementation_identifier != rti_connext_dynamic_identifier)
    {
        rmw_set_error_string("guard condition handle not from this implementation");
        // printf("but from: %s\n", guard_condition_handle->implementation_identifier);
        return RMW_RET_ERROR;
    }

    DDSGuardCondition * guard_condition = (DDSGuardCondition*)guard_condition_handle->data;
    guard_condition->set_trigger_value(DDS_BOOLEAN_TRUE);
    return RMW_RET_OK;
}

rmw_ret_t
rmw_wait(rmw_subscriptions_t * subscriptions,
         rmw_guard_conditions_t * guard_conditions,
         rmw_services_t * services,
         rmw_clients_t * clients,
         bool non_blocking)
{
    DDSWaitSet waitset;

    // add a condition for each subscriber
    for (unsigned long i = 0; i < subscriptions->subscriber_count; ++i)
    {
        void * data = subscriptions->subscribers[i];
        CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo*)data;
        DDSDynamicDataReader * dynamic_reader = custom_subscriber_info->dynamic_reader_;
        DDSStatusCondition * condition = dynamic_reader->get_statuscondition();
        condition->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
        waitset.attach_condition(condition);
    }

    // add a condition for each guard condition
    for (unsigned long i = 0; i < guard_conditions->guard_condition_count; ++i)
    {
        void * data = guard_conditions->guard_conditions[i];
        DDSGuardCondition * guard_condition = (DDSGuardCondition*)data;
        waitset.attach_condition(guard_condition);
    }

    // invoke wait until one of the conditions triggers
    DDSConditionSeq active_conditions;
    DDS_Duration_t timeout = DDS_Duration_t::from_seconds(non_blocking ? 0 : 1);
    DDS_ReturnCode_t status = DDS_RETCODE_TIMEOUT;
    while (DDS_RETCODE_TIMEOUT == status)
    {
        status = waitset.wait(active_conditions, timeout);
        if (DDS_RETCODE_TIMEOUT == status) {
            if (non_blocking)
            {
               break;
            }
            continue;
        };
        if (status != DDS_RETCODE_OK) {
            rmw_set_error_string("fail to wait on waitset");
            // printf("wait() failed. Status = %d\n", status);
            return RMW_RET_ERROR;
        };
    }

    // set subscriber handles to zero for all not triggered conditions
    for (unsigned long i = 0; i < subscriptions->subscriber_count; ++i)
    {
        void * data = subscriptions->subscribers[i];
        CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo*)data;
        DDSDynamicDataReader * dynamic_reader = custom_subscriber_info->dynamic_reader_;
        DDSCondition * condition = dynamic_reader->get_statuscondition();

        // search for subscriber condition in active set
        unsigned long j = 0;
        for (; j < active_conditions.length(); ++j)
        {
            if (active_conditions[j] == condition)
            {
                break;
            }
        }
        // if subscriber condition is not found in the active set
        // reset the subscriber handle
        if (!(j < active_conditions.length()))
        {
            subscriptions->subscribers[i] = 0;
        }
    }

    // set subscriber handles to zero for all not triggered conditions
    for (unsigned long i = 0; i < guard_conditions->guard_condition_count; ++i)
    {
        void * data = guard_conditions->guard_conditions[i];
        DDSCondition * condition = (DDSCondition*)data;

        // search for guard condition in active set
        unsigned long j = 0;
        for (; j < active_conditions.length(); ++j)
        {
            if (active_conditions[j] == condition)
            {
                DDSGuardCondition *guard = (DDSGuardCondition*)condition;
                guard->set_trigger_value(DDS_BOOLEAN_FALSE);
                break;
            }
        }
        // if guard condition is not found in the active set
        // reset the guard handle
        if (!(j < active_conditions.length()))
        {
            guard_conditions->guard_conditions[i] = 0;
        }
    }
}

rmw_client_t *
rmw_create_client(const rmw_node_t * node,
                  const rosidl_service_type_support_t * type_support,
                  const char * service_name)
{
  return NULL;
}

rmw_ret_t
rmw_send_request(const rmw_client_t * client, const void * ros_request,
                 int64_t * sequence_id)
{
  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_take_request(const rmw_service_t * service,
                 void * ros_request_header, void * ros_request, bool * taken)
{
  *taken = false;
  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_take_response(const rmw_client_t * client,
                  void * ros_request_header, void * ros_response, bool * taken)
{
  *taken = false;
  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_send_response(const rmw_service_t * service,
                  void * ros_request, void * ros_response)
{
  return RMW_RET_ERROR;
}

rmw_service_t *
rmw_create_service(const rmw_node_t * node,
                   const rosidl_service_type_support_t * type_support,
                   const char * service_name)
{
  return NULL;
}

rmw_ret_t
rmw_destroy_service(rmw_service_t * service)
{
  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_destroy_client(rmw_client_t * client)
{
  return RMW_RET_ERROR;
}

}
