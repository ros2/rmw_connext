#include <iostream>
#include <stdexcept>

#include "ndds/ndds_cpp.h"

#include "rosidl_generator_cpp/MessageTypeSupport.h"
#include "ros_middleware_interface/handles.h"
#include "ros_middleware_connext_cpp/MessageTypeSupport.h"

namespace ros_middleware_interface
{

const char * _rti_connext_identifier = "connext_static";

ros_middleware_interface::NodeHandle create_node()
{
    std::cout << "create_node()" << std::endl;

    std::cout << "  create_node() get_instance" << std::endl;
    DDSDomainParticipantFactory* dpf_ = DDSDomainParticipantFactory::get_instance();
    if (!dpf_) {
        printf("  create_node() could not get participant factory\n");
        throw std::runtime_error("could not get participant factory");
    };

    DDS_DomainId_t domain = 0;

    std::cout << "  create_node() create_participant in domain " << domain << std::endl;
    DDSDomainParticipant* participant = dpf_->create_participant(
        domain, DDS_PARTICIPANT_QOS_DEFAULT, NULL,
        DDS_STATUS_MASK_NONE);
    if (!participant) {
        printf("  create_node() could not create participant\n");
        throw std::runtime_error("could not create participant");
    };

    std::cout << "  create_node() pass opaque node handle" << std::endl;

    ros_middleware_interface::NodeHandle node_handle = {
        _rti_connext_identifier,
        participant
    };
    return node_handle;
}

struct CustomPublisherInfo {
  DDSDataWriter * topic_writer_;
  ros_middleware_connext_cpp::MessageTypeSupportCallbacks * callbacks_;
};

ros_middleware_interface::PublisherHandle create_publisher(const ros_middleware_interface::NodeHandle& node_handle, const rosidl_generator_cpp::MessageTypeSupportHandle & type_support_handle, const char * topic_name)
{
    std::cout << "create_publisher()" << std::endl;

    if (node_handle._implementation_identifier != _rti_connext_identifier)
    {
        printf("node handle not from this implementation\n");
        printf("but from: %s\n", node_handle._implementation_identifier);
        throw std::runtime_error("node handle not from this implementation");
    }

    std::cout << "create_publisher() " << node_handle._implementation_identifier << std::endl;

    std::cout << "  create_publisher() extract participant from opaque node handle" << std::endl;
    DDSDomainParticipant* participant = (DDSDomainParticipant*)node_handle._data;

    ros_middleware_connext_cpp::MessageTypeSupportCallbacks * callbacks = (ros_middleware_connext_cpp::MessageTypeSupportCallbacks*)type_support_handle._data;
    std::string type_name = std::string(callbacks->_package_name) + "/" + callbacks->_message_name;


    std::cout << "  create_publisher() invoke register callback" << std::endl;
    callbacks->_register_type(participant, type_name.c_str());


    DDS_PublisherQos publisher_qos;
    DDS_ReturnCode_t status = participant->get_default_publisher_qos(publisher_qos);
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


    std::cout << "  create_publisher() build opaque publisher handle" << std::endl;
    CustomPublisherInfo* custom_publisher_info = new CustomPublisherInfo();
    custom_publisher_info->topic_writer_ = topic_writer;
    custom_publisher_info->callbacks_ = callbacks;

    ros_middleware_interface::PublisherHandle publisher_handle = {
        _rti_connext_identifier,
        custom_publisher_info
    };
    return publisher_handle;
}

void publish(const ros_middleware_interface::PublisherHandle& publisher_handle, const void * ros_message)
{
    //std::cout << "publish()" << std::endl;

    if (publisher_handle._implementation_identifier != _rti_connext_identifier)
    {
        printf("publisher handle not from this implementation\n");
        printf("but from: %s\n", publisher_handle._implementation_identifier);
        throw std::runtime_error("publisher handle not from this implementation");
    }

    //std::cout << "  publish() extract data writer and type code from opaque publisher handle" << std::endl;
    CustomPublisherInfo * custom_publisher_info = (CustomPublisherInfo*)publisher_handle._data;
    DDSDataWriter * topic_writer = custom_publisher_info->topic_writer_;
    const ros_middleware_connext_cpp::MessageTypeSupportCallbacks * callbacks = custom_publisher_info->callbacks_;


    //std::cout << "  publish() invoke publish callback" << std::endl;
    callbacks->_publish(topic_writer, ros_message);
}

struct CustomSubscriberInfo {
  DDSDataReader * topic_reader_;
  ros_middleware_connext_cpp::MessageTypeSupportCallbacks * callbacks_;
};

ros_middleware_interface::SubscriberHandle create_subscriber(const NodeHandle& node_handle, const rosidl_generator_cpp::MessageTypeSupportHandle & type_support_handle, const char * topic_name)
{
    std::cout << "create_subscriber()" << std::endl;

    if (node_handle._implementation_identifier != _rti_connext_identifier)
    {
        printf("node handle not from this implementation\n");
        printf("but from: %s\n", node_handle._implementation_identifier);
        throw std::runtime_error("node handle not from this implementation");
    }
    std::cout << "create_subscriber() " << node_handle._implementation_identifier << std::endl;

    std::cout << "  create_subscriber() extract participant from opaque node handle" << std::endl;
    DDSDomainParticipant* participant = (DDSDomainParticipant*)node_handle._data;

    ros_middleware_connext_cpp::MessageTypeSupportCallbacks * callbacks = (ros_middleware_connext_cpp::MessageTypeSupportCallbacks*)type_support_handle._data;
    std::string type_name = std::string(callbacks->_package_name) + "/" + callbacks->_message_name;

    std::cout << "  create_subscriber() invoke register callback" << std::endl;
    callbacks->_register_type(participant, type_name.c_str());

    DDS_SubscriberQos subscriber_qos;
    DDS_ReturnCode_t status = participant->get_default_subscriber_qos(subscriber_qos);
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

    std::cout << "  create_subscriber() create data writer" << std::endl;
    DDSDataReader* topic_reader = dds_subscriber->create_datareader(
        topic, default_datareader_qos,
        NULL, DDS_STATUS_MASK_NONE);


    std::cout << "  create_subscriber() build opaque publisher handle" << std::endl;
    CustomSubscriberInfo* custom_subscriber_info = new CustomSubscriberInfo();
    custom_subscriber_info->topic_reader_ = topic_reader;
    custom_subscriber_info->callbacks_ = callbacks;


    ros_middleware_interface::SubscriberHandle subscriber_handle = {
        _rti_connext_identifier,
        custom_subscriber_info
    };
    return subscriber_handle;
}

bool take(const ros_middleware_interface::SubscriberHandle& subscriber_handle, void * ros_message)
{
   std::cout << "take()" << std::endl;

    if (subscriber_handle.implementation_identifier_ != _rti_connext_identifier)
    {
        printf("subscriber handle not from this implementation\n");
        printf("but from: %s\n", subscriber_handle.implementation_identifier_);
        throw std::runtime_error("subscriber handle not from this implementation");
    }

    std::cout << "  take() extract data reader and type code from opaque subscriber handle" << std::endl;
    CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo*)subscriber_handle.data_;
    DDSDataReader* topic_reader = custom_subscriber_info->topic_reader_;
    const ros_middleware_connext_cpp::MessageTypeSupportCallbacks * callbacks = custom_subscriber_info->callbacks_;

    std::cout << "  take() invoke take callback" << std::endl;
    return callbacks->_take(topic_reader, ros_message);
}

ros_middleware_interface::GuardConditionHandle create_guard_condition()
{
    ros_middleware_interface::GuardConditionHandle guard_condition_handle;
    guard_condition_handle.implementation_identifier_ = _rti_connext_identifier;
    guard_condition_handle.data_ = new DDSGuardCondition();
    return guard_condition_handle;

}

void trigger_guard_condition(const GuardConditionHandle& guard_condition_handle)
{
    //std::cout << "trigger_guard_condition()" << std::endl;

    if (guard_condition_handle.implementation_identifier_ != _rti_connext_identifier)
    {
        printf("guard condition handle not from this implementation\n");
        printf("but from: %s\n", guard_condition_handle.implementation_identifier_);
        throw std::runtime_error("guard condition handle not from this implementation");
    }

    DDSGuardCondition * guard_condition = (DDSGuardCondition*)guard_condition_handle.data_;
    guard_condition->set_trigger_value(DDS_BOOLEAN_TRUE);
}

void wait(SubscriberHandles& subscriber_handles, GuardConditionHandles& guard_condition_handles)
{
    //std::cout << "wait()" << std::endl;

    DDSWaitSet waitset;

    // add a condition for each subscriber
    for (unsigned long i = 0; i < subscriber_handles.subscriber_count_; ++i)
    {
        void * data = subscriber_handles.subscribers_[i];
        CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo*)data;
        DDSDataReader* topic_reader = custom_subscriber_info->topic_reader_;
        DDSStatusCondition * condition = topic_reader->get_statuscondition();
        condition->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
        waitset.attach_condition(condition);
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
            //std::cout << "wait() no data - rinse and repeat" << std::endl;
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
        CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo*)data;
        DDSDataReader* topic_reader = custom_subscriber_info->topic_reader_;
        DDSStatusCondition * condition = topic_reader->get_statuscondition();

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
        if (!j < active_conditions.length())
        {
            subscriber_handles.subscribers_[i] = 0;
        }
    }

    // set subscriber handles to zero for all not triggered conditions
    for (unsigned long i = 0; i < guard_condition_handles.guard_condition_count_; ++i)
    {
        void * data = guard_condition_handles.guard_conditions_[i];
        DDSCondition * condition = (DDSCondition*)data;

        // search for guard condition in active set
        unsigned long j = 0;
        for (; j < active_conditions.length(); ++j)
        {
            if (active_conditions[j] == condition)
            {
                break;
            }
        }
        // if guard condition is not found in the active set
        // reset the guard handle
        if (!j < active_conditions.length())
        {
            guard_condition_handles.guard_conditions_[i] = 0;
        }
    }
}


}
