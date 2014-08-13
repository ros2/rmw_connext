#include <iostream>
#include <stdexcept>

#include "ndds/ndds_cpp.h"

#include "rosidl_generator_cpp/MessageTypeSupport.h"
#include "ros_middleware_interface/handles.h"
#include "rosidl_typesupport_introspection_cpp/MessageIntrospection.h"

namespace ros_middleware_interface
{

const char * _rti_connext_dynamic_identifier = "connext_dynamic";

ros_middleware_interface::NodeHandle create_node()
{
    std::cout << "create_node()" << std::endl;

    std::cout << "  create_node() get_instance" << std::endl;
    DDSDomainParticipantFactory* dpf_ = DDSDomainParticipantFactory::get_instance();
    if (!dpf_) {
        printf("  create_node() could not get participant factory\n");
        throw std::runtime_error("could not get participant factory");
    };

    DDS_DomainId_t domain = 23;

    std::cout << "  create_node() create_participant" << std::endl;
    DDSDomainParticipant* participant = dpf_->create_participant(
        domain, DDS_PARTICIPANT_QOS_DEFAULT, NULL,
        DDS_STATUS_MASK_NONE);
    if (!participant) {
        printf("  create_node() could not create participant\n");
        throw std::runtime_error("could not create participant");
    };

    std::cout << "  create_node() pass opaque node handle" << std::endl;

    ros_middleware_interface::NodeHandle node_handle = {
        _rti_connext_dynamic_identifier,
        participant
    };
    return node_handle;
}

struct CustomPublisherInfo {
  DDSDynamicDataTypeSupport * dynamic_data_type_support_;
  DDSDynamicDataWriter * dynamic_writer_;
  DDS_TypeCode * type_code_;
  rosidl_typesupport_introspection_cpp::MessageMembers * members_;
  DDS_DynamicData * dynamic_data;
};

ros_middleware_interface::PublisherHandle create_publisher(const ros_middleware_interface::NodeHandle& node_handle, const rosidl_generator_cpp::MessageTypeSupportHandle & type_support_handle, const char * topic_name)
{
    std::cout << "create_publisher()" << std::endl;

    if (node_handle._implementation_identifier != _rti_connext_dynamic_identifier)
    {
        printf("node handle not from this implementation\n");
        printf("but from: %s\n", node_handle._implementation_identifier);
        throw std::runtime_error("node handle not from this implementation");
    }

    std::cout << "create_publisher() " << node_handle._implementation_identifier << std::endl;

    std::cout << "  create_publisher() extract participant from opaque node handle" << std::endl;
    DDSDomainParticipant* participant = (DDSDomainParticipant*)node_handle._data;

    if (type_support_handle._typesupport_identifier != rosidl_typesupport_introspection_cpp::typesupport_introspection_identifier)
    {
        printf("type support not from this implementation\n");
        printf("but from: %s\n", type_support_handle._typesupport_identifier);
        throw std::runtime_error("type support not from this implementation");
    }

    rosidl_typesupport_introspection_cpp::MessageMembers * members = (rosidl_typesupport_introspection_cpp::MessageMembers*)type_support_handle._data;
    std::string type_name = std::string(members->package_name_) + "/" + members->message_name_;

    std::cout << "  create_publisher() create type code for " << type_name.c_str() << std::endl;
    DDS_TypeCode * type_code;
    {
        DDS_TypeCodeFactory * factory = NULL;
        DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
        DDS_StructMemberSeq struct_members;
        factory = DDS_TypeCodeFactory::get_instance();
        if (!factory) {
            printf("  create_publisher() could not get typecode factory\n");
            throw std::runtime_error("could not get typecode factory");
        };

        type_code = factory->create_struct_tc(type_name.c_str(), struct_members, ex);
        for(unsigned long i = 0; i < members->member_count_; ++i)
        {
            const rosidl_typesupport_introspection_cpp::MessageMember * member = members->members_ + i * sizeof(rosidl_typesupport_introspection_cpp::MessageMember);
            std::cout << "  create_publisher() create type code - add member " << i << ": " << member->name_ << "" << std::endl;
            const DDS_TypeCode * member_type_code;
            if (strcmp(member->type_, "int32") == 0)
            {
                member_type_code = factory->get_primitive_tc(DDS_TK_LONG);
            }
            else
            {
                printf("unknown type %s\n", member->type_);
                throw std::runtime_error("unknown type");
            }
            type_code->add_member(member->name_, DDS_TYPECODE_MEMBER_ID_INVALID, member_type_code,
                        DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, ex);
        }
        DDS_StructMemberSeq_finalize(&struct_members);
    }


    std::cout << "  create_publisher() register type code" << std::endl;
    DDSDynamicDataTypeSupport* ddts = new DDSDynamicDataTypeSupport(
        type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
    DDS_ReturnCode_t status = ddts->register_type(participant, type_name.c_str());
    if (status != DDS_RETCODE_OK) {
        printf("register_type() failed. Status = %d\n", status);
        throw std::runtime_error("register type failed");
    };


    DDS_PublisherQos publisher_qos;
    status = participant->get_default_publisher_qos(publisher_qos);
    if (status != DDS_RETCODE_OK) {
        printf("get_default_publisher_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default publisher qos failed");
    };

    std::cout << "  create_publisher() create dds publisher" << std::endl;
    DDSPublisher* dds_publisher = participant->create_publisher(
        publisher_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!dds_publisher) {
        printf("  create_publisher() could not create publisher\n");
        throw std::runtime_error("could not create publisher");
    };


    DDS_TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS_RETCODE_OK) {
        printf("get_default_topic_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default topic qos failed");
    };

    std::cout << "  create_publisher() create topic" << std::endl;
    DDSTopic* topic = participant->create_topic(
        topic_name, type_name.c_str(), default_topic_qos, NULL,
        DDS_STATUS_MASK_NONE
    );
    if (!topic) {
        printf("  create_topic() could not create topic\n");
        throw std::runtime_error("could not create topic");
    };


    DDS_DataWriterQos default_datawriter_qos;
    status = participant->get_default_datawriter_qos(default_datawriter_qos);
    if (status != DDS_RETCODE_OK) {
        printf("get_default_datawriter_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default datawriter qos failed");
    };

    std::cout << "  create_publisher() create data writer" << std::endl;
    DDSDataWriter* topic_writer = dds_publisher->create_datawriter(
        topic, default_datawriter_qos,
        NULL, DDS_STATUS_MASK_NONE);

    DDSDynamicDataWriter* dynamic_writer = DDSDynamicDataWriter::narrow(topic_writer);
    if (!dynamic_writer) {
        printf("narrow() failed.\n");
        throw std::runtime_error("narrow datawriter failed");
    };

    DDS_DynamicData * dynamic_data = ddts->create_data();

    std::cout << "  create_publisher() build opaque publisher handle" << std::endl;
    CustomPublisherInfo* custom_publisher_info = new CustomPublisherInfo();
    custom_publisher_info->dynamic_data_type_support_ = ddts;
    custom_publisher_info->dynamic_writer_ = dynamic_writer;
    custom_publisher_info->type_code_ = type_code;
    custom_publisher_info->members_ = members;
    custom_publisher_info->dynamic_data = dynamic_data;

    ros_middleware_interface::PublisherHandle publisher_handle = {
        _rti_connext_dynamic_identifier,
        custom_publisher_info
    };
    return publisher_handle;
}

void publish(const ros_middleware_interface::PublisherHandle& publisher_handle, const void * ros_message)
{
    //std::cout << "publish()" << std::endl;


    if (publisher_handle._implementation_identifier != _rti_connext_dynamic_identifier)
    {
        printf("publisher handle not from this implementation\n");
        printf("but from: %s\n", publisher_handle._implementation_identifier);
        throw std::runtime_error("publisher handle not from this implementation");
    }

    //std::cout << "  publish() extract data writer and type code from opaque publisher handle" << std::endl;
    CustomPublisherInfo * custom_publisher_info = (CustomPublisherInfo*)publisher_handle._data;
    DDSDynamicDataTypeSupport* ddts = custom_publisher_info->dynamic_data_type_support_;
    DDSDynamicDataWriter * dynamic_writer = custom_publisher_info->dynamic_writer_;
    DDS_TypeCode * type_code = custom_publisher_info->type_code_;
    const rosidl_typesupport_introspection_cpp::MessageMembers * members = custom_publisher_info->members_;
    DDS_DynamicData * dynamic_data = custom_publisher_info->dynamic_data;

    //std::cout << "  publish() create " << members->package_name_ << "/" << members->message_name_ << " and populate dynamic data" << std::endl;
    //DDS_DynamicData dynamic_data(type_code, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    //DDS_DynamicData * dynamic_data = ddts->create_data();
    for(unsigned long i = 0; i < members->member_count_; ++i)
    {
        const rosidl_typesupport_introspection_cpp::MessageMember * member = members->members_ + i * sizeof(rosidl_typesupport_introspection_cpp::MessageMember);
        //std::cout << "  publish() set member " << i << ": " << member->_name << "" << std::endl;
        DDS_TypeCode * member_type_code;
        if (strcmp(member->type_, "int32") == 0)
        {
            int value = *((int*)((char*)ros_message + member->offset_));
            //std::cout << "  publish() set member " << i << ": " << member->name_ << " = " << value << std::endl;
            DDS_ReturnCode_t status = dynamic_data->set_long(member->name_, DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, value);
            if (status != DDS_RETCODE_OK) {
                printf("set_long() failed. Status = %d\n", status);
                throw std::runtime_error("set member failed");
            };
        }
        else
        {
            printf("unknown type %s\n", member->type_);
            throw std::runtime_error("unknown type");
        }
    }

    //std::cout << "  publish() write dynamic data" << std::endl;
    DDS_ReturnCode_t status = dynamic_writer->write(*dynamic_data, DDS_HANDLE_NIL);
    if (status != DDS_RETCODE_OK) {
        printf("write() failed. Status = %d\n", status);
        throw std::runtime_error("write failed");
    };

    //ddts->delete_data(dynamic_data);
}

struct CustomSubscriberInfo {
  DDSDynamicDataTypeSupport * dynamic_data_type_support_;
  DDSDynamicDataReader * dynamic_reader_;
  DDS_TypeCode * type_code_;
  rosidl_typesupport_introspection_cpp::MessageMembers * members_;
  DDS_DynamicData * dynamic_data;
};

ros_middleware_interface::SubscriberHandle create_subscriber(const ros_middleware_interface::NodeHandle& node_handle, const rosidl_generator_cpp::MessageTypeSupportHandle & type_support_handle, const char * topic_name)
{
    std::cout << "create_subscriber()" << std::endl;

    if (node_handle._implementation_identifier != _rti_connext_dynamic_identifier)
    {
        printf("node handle not from this implementation\n");
        printf("but from: %s\n", node_handle._implementation_identifier);
        throw std::runtime_error("node handle not from this implementation");
    }

    std::cout << "create_subscriber() " << node_handle._implementation_identifier << std::endl;

    std::cout << "  create_subscriber() extract participant from opaque node handle" << std::endl;
    DDSDomainParticipant* participant = (DDSDomainParticipant*)node_handle._data;

    if (type_support_handle._typesupport_identifier != rosidl_typesupport_introspection_cpp::typesupport_introspection_identifier)
    {
        printf("type support not from this implementation\n");
        printf("but from: %s\n", type_support_handle._typesupport_identifier);
        throw std::runtime_error("type support not from this implementation");
    }

    rosidl_typesupport_introspection_cpp::MessageMembers * members = (rosidl_typesupport_introspection_cpp::MessageMembers*)type_support_handle._data;
    std::string type_name = std::string(members->package_name_) + "/" + members->message_name_;

    std::cout << "  create_subscriber() create type code for " << type_name.c_str() << std::endl;
    DDS_TypeCode * type_code;
    {
        DDS_TypeCodeFactory * factory = NULL;
        DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
        DDS_StructMemberSeq struct_members;
        factory = DDS_TypeCodeFactory::get_instance();
        if (!factory) {
            printf("  create_subscriber() could not get typecode factory\n");
            throw std::runtime_error("could not get typecode factory");
        };

        type_code = factory->create_struct_tc(type_name.c_str(), struct_members, ex);
        for(unsigned long i = 0; i < members->member_count_; ++i)
        {
            const rosidl_typesupport_introspection_cpp::MessageMember * member = members->members_ + i * sizeof(rosidl_typesupport_introspection_cpp::MessageMember);
            std::cout << "  create_subscriber() create type code - add member " << i << ": " << member->name_ << "" << std::endl;
            const DDS_TypeCode * member_type_code;
            if (strcmp(member->type_, "int32") == 0)
            {
                member_type_code = factory->get_primitive_tc(DDS_TK_LONG);
            }
            else
            {
                printf("unknown type %s\n", member->type_);
                throw std::runtime_error("unknown type");
            }
            type_code->add_member(member->name_, DDS_TYPECODE_MEMBER_ID_INVALID, member_type_code,
                        DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, ex);
        }
        DDS_StructMemberSeq_finalize(&struct_members);
    }


    std::cout << "  create_subscriber() register type code" << std::endl;
    DDSDynamicDataTypeSupport* ddts = new DDSDynamicDataTypeSupport(
        type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
    DDS_ReturnCode_t status = ddts->register_type(participant, type_name.c_str());
    if (status != DDS_RETCODE_OK) {
        printf("register_type() failed. Status = %d\n", status);
        throw std::runtime_error("register type failed");
    };


    DDS_SubscriberQos subscriber_qos;
    status = participant->get_default_subscriber_qos(subscriber_qos);
    if (status != DDS_RETCODE_OK) {
        printf("get_default_subscriber_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default subscriber qos failed");
    };

    std::cout << "  create_subscriber() create dds subscriber" << std::endl;
    DDSSubscriber* dds_subscriber = participant->create_subscriber(
        subscriber_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!dds_subscriber) {
        printf("  create_subscriber() could not create subscriber\n");
        throw std::runtime_error("could not create subscriber");
    };


    DDS_TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS_RETCODE_OK) {
        printf("get_default_topic_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default topic qos failed");
    };

    std::cout << "  create_subscriber() create topic" << std::endl;
    DDSTopic* topic = participant->create_topic(
        topic_name, type_name.c_str(), default_topic_qos, NULL,
        DDS_STATUS_MASK_NONE
    );
    if (!topic) {
        printf("  create_topic() could not create topic\n");
        throw std::runtime_error("could not create topic");
    };


    DDS_DataReaderQos default_datareader_qos;
    status = participant->get_default_datareader_qos(default_datareader_qos);
    if (status != DDS_RETCODE_OK) {
        printf("get_default_datareader_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default datareader qos failed");
    };

    std::cout << "  create_subscriber() create data reader" << std::endl;
    DDSDataReader* topic_reader = dds_subscriber->create_datareader(
        topic, default_datareader_qos,
        NULL, DDS_STATUS_MASK_NONE);

    DDSDynamicDataReader* dynamic_reader = DDSDynamicDataReader::narrow(topic_reader);
    if (!dynamic_reader) {
        printf("narrow() failed.\n");
        throw std::runtime_error("narrow datareader failed");
    };

    DDS_DynamicData * dynamic_data = ddts->create_data();

    std::cout << "  create_subscriber() build opaque subscriber handle" << std::endl;
    CustomSubscriberInfo* custom_subscriber_info = new CustomSubscriberInfo();
    custom_subscriber_info->dynamic_data_type_support_ = ddts;
    custom_subscriber_info->dynamic_reader_ = dynamic_reader;
    custom_subscriber_info->type_code_ = type_code;
    custom_subscriber_info->members_ = members;
    custom_subscriber_info->dynamic_data = dynamic_data;

    ros_middleware_interface::SubscriberHandle subscriber_handle = {
        _rti_connext_dynamic_identifier,
        custom_subscriber_info
    };
    return subscriber_handle;
}

bool take(const ros_middleware_interface::SubscriberHandle& subscriber_handle, void * ros_message)
{
    //std::cout << "take()" << std::endl;


    if (subscriber_handle.implementation_identifier_ != _rti_connext_dynamic_identifier)
    {
        printf("subscriber handle not from this implementation\n");
        printf("but from: %s\n", subscriber_handle.implementation_identifier_);
        throw std::runtime_error("subscriber handle not from this implementation");
    }

    //std::cout << "  take() extract data writer and type code from opaque subscriber handle" << std::endl;
    CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo*)subscriber_handle.data_;
    DDSDynamicDataTypeSupport* ddts = custom_subscriber_info->dynamic_data_type_support_;
    DDSDynamicDataReader * dynamic_reader = custom_subscriber_info->dynamic_reader_;
    DDS_TypeCode * type_code = custom_subscriber_info->type_code_;
    const rosidl_typesupport_introspection_cpp::MessageMembers * members = custom_subscriber_info->members_;
    DDS_DynamicData * dynamic_data = custom_subscriber_info->dynamic_data;

    DDS_SampleInfo sample_info;
    DDS_ReturnCode_t status = dynamic_reader->take_next_sample(*dynamic_data, sample_info);
    if (status == DDS_RETCODE_NO_DATA) {
        return false;
    }
    if (status != DDS_RETCODE_OK) {
        printf("take_next_sample() failed. Status = %d\n", status);
        throw std::runtime_error("take next sample failed");
    };

    if (ros_message == 0) {
        printf("take() invoked without a valid ROS message pointer\n");
        throw std::runtime_error("invalid ROS message pointer");
    };

    //std::cout << "  take() create " << members->package_name_ << "/" << members->message_name_ << " and populate dynamic data" << std::endl;
    //DDS_DynamicData dynamic_data(type_code, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    //DDS_DynamicData * dynamic_data = ddts->create_data();
    for(unsigned long i = 0; i < members->member_count_; ++i)
    {
        const rosidl_typesupport_introspection_cpp::MessageMember * member = members->members_ + i * sizeof(rosidl_typesupport_introspection_cpp::MessageMember);
        //std::cout << "  take() set member " << i << ": " << member->_name << "" << std::endl;
        DDS_TypeCode * member_type_code;
        if (strcmp(member->type_, "int32") == 0)
        {
            int value = 0;
            status = dynamic_data->get_long(value, member->name_, DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
            if (status != DDS_RETCODE_OK) {
                printf("get_long() failed. Status = %d\n", status);
                throw std::runtime_error("get member failed");
            };
            //std::cout << "  take() get member " << i << ": " << member->name_ << " = " << value << std::endl;
            int* ros_value = (int*)((char*)ros_message + member->offset_);
            *ros_value = value;
        }
        else
        {
            printf("unknown type %s\n", member->type_);
            throw std::runtime_error("unknown type");
        }
    }

    //ddts->delete_data(dynamic_data);

    return true;
}

ros_middleware_interface::GuardConditionHandle create_guard_condition()
{
    ros_middleware_interface::GuardConditionHandle guard_condition_handle;
    guard_condition_handle.implementation_identifier_ = _rti_connext_dynamic_identifier;
    guard_condition_handle.data_ = new DDSGuardCondition();
    return guard_condition_handle;
}

void trigger_guard_condition(const ros_middleware_interface::GuardConditionHandle& guard_condition_handle)
{
    //std::cout << "trigger_guard_condition()" << std::endl;

    if (guard_condition_handle.implementation_identifier_ != _rti_connext_dynamic_identifier)
    {
        printf("guard condition handle not from this implementation\n");
        printf("but from: %s\n", guard_condition_handle.implementation_identifier_);
        throw std::runtime_error("guard condition handle not from this implementation");
    }

    DDSGuardCondition * guard_condition = (DDSGuardCondition*)guard_condition_handle.data_;
    guard_condition->set_trigger_value(DDS_BOOLEAN_TRUE);
}

void wait(ros_middleware_interface::SubscriberHandles& subscriber_handles, ros_middleware_interface::GuardConditionHandles& guard_condition_handles)
{
    DDSWaitSet waitset;

    // add a condition for each subscriber
    for (unsigned long i = 0; i < subscriber_handles.subscriber_count_; ++i)
    {
        void * data = subscriber_handles.subscribers_[i];
        CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo*)data;
        DDSDynamicDataReader * dynamic_reader = custom_subscriber_info->dynamic_reader_;
        waitset.attach_condition(dynamic_reader->get_statuscondition());
    }

    // add a condition for each guard condition
    for (unsigned long i = 0; i < guard_condition_handles.guard_condition_count_; ++i)
    {
        void * data = guard_condition_handles.guard_conditions_[i];
        DDSGuardCondition * guard_condition = (DDSGuardCondition*)data;
        waitset.attach_condition(guard_condition);
    }

    // invoke wait until one of the conditions triggers
    DDSConditionSeq active_conditions;
    DDS_Duration_t timeout = DDS_Duration_t::from_seconds(1);
    DDS_ReturnCode_t status = DDS_RETCODE_TIMEOUT;
    while (DDS_RETCODE_TIMEOUT == status)
    {
        status = waitset.wait(active_conditions, timeout);
        if (DDS_RETCODE_TIMEOUT == status) {
            continue;
        };
        if (status != DDS_RETCODE_OK) {
            printf("wait() failed. Status = %d\n", status);
            throw std::runtime_error("wait failed");
        };
    }

    // set subscriber handles to zero for all not triggered conditions
    for (unsigned long i = 0; i < subscriber_handles.subscriber_count_; ++i)
    {
        void * data = subscriber_handles.subscribers_[i];
        if (!active_conditions[i]->get_trigger_value())
        {
            data = 0;
        }
    }

    // set subscriber handles to zero for all not triggered conditions
    for (unsigned long i = 0; i < guard_condition_handles.guard_condition_count_; ++i)
    {
        void * data = guard_condition_handles.guard_conditions_[subscriber_handles.subscriber_count_ + i];
        if (!active_conditions[i]->get_trigger_value())
        {
            data = 0;
        }
    }
}

}
