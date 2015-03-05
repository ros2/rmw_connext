#include <rmw/types.h>

#include <iostream>
#include <stdexcept>

#include "ndds/ndds_cpp.h"
#include "ndds/ndds_requestreply_cpp.h"

#include <rmw/rmw.h>
#include "rosidl_generator_cpp/MessageTypeSupport.h"
#include "rosidl_typesupport_connext_cpp/MessageTypeSupport.h"

#include "rosidl_generator_cpp/ServiceTypeSupport.h"
#include "rosidl_typesupport_connext_cpp/ServiceTypeSupport.h"

namespace ros_middleware_interface
{

const char * _rti_connext_identifier = "connext_static";


struct CustomServiceInfo {
  void * replier_;
  DDSDataReader * request_datareader_;
  rmw_connext_cpp::ServiceTypeSupportCallbacks * callbacks_;
};

struct CustomClientInfo {
  void * requester_;
  DDSDataReader * response_datareader_;
  rmw_connext_cpp::ServiceTypeSupportCallbacks * callbacks_;
};

rmw_ret_t
rmw_init()
{
    std::cout << "init()" << std::endl;
    std::cout << "  init() get_instance" << std::endl;
    DDSDomainParticipantFactory* dpf_ = DDSDomainParticipantFactory::get_instance();
    if (!dpf_) {
        printf("  init() could not get participant factory\n");
        return RMW_RET_ERROR;
    };
    return RMW_RET_OK;
}

rmw_node_t *
rmw_create_node(const char * name)
{
    std::cout << "create_node()" << std::endl;

    std::cout << "  create_node() " << _rti_connext_identifier << std::endl;

    std::cout << "  create_node() get_instance" << std::endl;
    DDSDomainParticipantFactory* dpf_ = DDSDomainParticipantFactory::get_instance();
    if (!dpf_) {
        printf("  create_node() could not get participant factory\n");
        throw std::runtime_error("could not get participant factory");
    };

    // use loopback interface to enable cross vendor communication
    DDS_DomainParticipantQos participant_qos;
    DDS_ReturnCode_t status = dpf_->get_default_participant_qos(participant_qos);
    if (status != DDS_RETCODE_OK)
    {
        printf("  create_node() could not get participant qos\n");
        throw std::runtime_error("could not get participant qos");
    }
    status = DDSPropertyQosPolicyHelper::add_property(participant_qos.property,
        "dds.transport.UDPv4.builtin.ignore_loopback_interface",
        "0",
        DDS_BOOLEAN_FALSE);
    if (status != DDS_RETCODE_OK)
    {
        printf("  create_node() could not add qos property\n");
        throw std::runtime_error("could not add qos property");
    }
    std::cout << "  create_node() disable shared memory, enable loopback interface" << std::endl;

    DDS_DomainId_t domain = 0;

    std::cout << "  create_node() create_participant in domain " << domain << std::endl;
    DDSDomainParticipant* participant = dpf_->create_participant(
        //domain, DDS_PARTICIPANT_QOS_DEFAULT, NULL,
        domain, participant_qos, NULL,
        DDS_STATUS_MASK_NONE);
    if (!participant) {
        printf("  create_node() could not create participant\n");
        throw std::runtime_error("could not create participant");
    };

    std::cout << "  create_node() pass opaque node handle" << std::endl;

    rmw_node_t * node_handle = new rmw_node_t;
    node_handle->implementation_identifier = _rti_connext_identifier;
    node_handle->data = participant;

    return node_handle;
}

struct CustomPublisherInfo {
  DDSDataWriter * topic_writer_;
  rmw_connext_cpp::MessageTypeSupportCallbacks * callbacks_;
};

rmw_publisher_t * create_publisher(
  const rmw_node_t * node, const rosidl_message_type_support_t * type_support,
  const char * topic_name, size_t queue_size)
{
    std::cout << "create_publisher()" << std::endl;

    if (node->implementation_identifier != _rti_connext_identifier)
    {
        printf("node handle not from this implementation\n");
        printf("but from: %s\n", node->implementation_identifier);
        throw std::runtime_error("node handle not from this implementation");
    }

    std::cout << "create_publisher() " << node->implementation_identifier << std::endl;

    std::cout << "  create_publisher() extract participant from opaque node handle" << std::endl;
    DDSDomainParticipant* participant = (DDSDomainParticipant*)node->data;

    rmw_connext_cpp::MessageTypeSupportCallbacks * callbacks = (rmw_connext_cpp::MessageTypeSupportCallbacks*)type_support->data;
    std::string type_name = std::string(callbacks->_package_name) + "::dds_::" + callbacks->_message_name + "_";


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


    DDS_DataWriterQos datawriter_qos;
    status = participant->get_default_datawriter_qos(datawriter_qos);
    if (status != DDS_RETCODE_OK) {
        printf("get_default_datawriter_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default datawriter qos failed");
    };

    status = DDSPropertyQosPolicyHelper::add_property(datawriter_qos.property,
        "dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size",
        "4096",
        DDS_BOOLEAN_FALSE);
    if (status != DDS_RETCODE_OK)
    {
        printf("  add_property() could not add qos property\n");
        throw std::runtime_error("could not add qos property");
    }
    std::cout << "  create_publisher() limit preallocated sample size" << std::endl;

    std::cout << "  create_publisher() create data writer" << std::endl;
    DDSDataWriter* topic_writer = dds_publisher->create_datawriter(
        topic, datawriter_qos,
        NULL, DDS_STATUS_MASK_NONE);


    std::cout << "  create_publisher() build opaque publisher handle" << std::endl;
    CustomPublisherInfo* custom_publisher_info = new CustomPublisherInfo();
    custom_publisher_info->topic_writer_ = topic_writer;
    custom_publisher_info->callbacks_ = callbacks;

    rmw_publisher_t * publisher = new rmw_publisher_t;
    publisher->implementation_identifier = _rti_connext_identifier;
    publisher->data = custom_publisher_info;
    return publisher;
}

rmw_ret_t
rmw_publish(const rmw_publisher_t * publisher, const void * ros_message)
{
    //std::cout << "publish()" << std::endl;

    if (publisher->implementation_identifier != _rti_connext_identifier)
    {
        printf("publisher handle not from this implementation\n");
        printf("but from: %s\n", publisher->implementation_identifier);
        return RMW_RET_ERROR;
    }

    //std::cout << "  publish() extract data writer and type code from opaque publisher handle" << std::endl;
    CustomPublisherInfo * custom_publisher_info = (CustomPublisherInfo*)publisher->data;
    DDSDataWriter * topic_writer = custom_publisher_info->topic_writer_;
    const rmw_connext_cpp::MessageTypeSupportCallbacks * callbacks = custom_publisher_info->callbacks_;


    //std::cout << "  publish() invoke publish callback" << std::endl;
    callbacks->_publish(topic_writer, ros_message);
    return RMW_RET_OK;
}

struct CustomSubscriberInfo {
  DDSDataReader * topic_reader_;
  rmw_connext_cpp::MessageTypeSupportCallbacks * callbacks_;
};

rmw_subscription_t * create_subscription(
  const rmw_node_t * node, const rosidl_message_type_support_t * type_support,
  const char * topic_name)
{
    std::cout << "create_subscriber()" << std::endl;

    if (node->implementation_identifier != _rti_connext_identifier)
    {
        printf("node handle not from this implementation\n");
        printf("but from: %s\n", node->implementation_identifier);
        throw std::runtime_error("node handle not from this implementation");
    }
    std::cout << "create_subscriber() " << node->implementation_identifier << std::endl;

    std::cout << "  create_subscriber() extract participant from opaque node handle" << std::endl;
    DDSDomainParticipant* participant = (DDSDomainParticipant*)node->data;

    rmw_connext_cpp::MessageTypeSupportCallbacks * callbacks = (rmw_connext_cpp::MessageTypeSupportCallbacks*)type_support->data;
    std::string type_name = std::string(callbacks->_package_name) + "::dds_::" + callbacks->_message_name + "_";

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

    rmw_subscription_t * subscription = new rmw_subscription_t;
    subscription->implementation_identifier = _rti_connext_identifier;
    subscription->data = custom_subscriber_info;
    return subscription;
}

rmw_ret_t
take(const rmw_subscription_t * subscription, void * ros_message)
{
    if (subscription->implementation_identifier != _rti_connext_identifier)
    {
        printf("subscriber handle not from this implementation\n");
        printf("but from: %s\n", subscription->implementation_identifier);
        throw std::runtime_error("subscriber handle not from this implementation");
    }

    CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo*)subscription->data;
    DDSDataReader* topic_reader = custom_subscriber_info->topic_reader_;
    const rmw_connext_cpp::MessageTypeSupportCallbacks * callbacks = custom_subscriber_info->callbacks_;

    return callbacks->_take(topic_reader, ros_message);
}

rmw_guard_condition_t * create_guard_condition()
{
    rmw_guard_condition_t * guard_condition = new rmw_guard_condition_t;
    guard_condition->implementation_identifier = _rti_connext_identifier;
    guard_condition->data = new DDSGuardCondition();
    return guard_condition;

}

rmw_ret_t
rmw_trigger_guard_condition(const rmw_guard_condition_t * guard_condition_handle)
{
    //std::cout << "trigger_guard_condition()" << std::endl;

    if (guard_condition_handle->implementation_identifier != _rti_connext_identifier)
    {
        printf("guard condition handle not from this implementation\n");
        printf("but from: %s\n", guard_condition_handle->implementation_identifier);
        throw std::runtime_error("guard condition handle not from this implementation");
    }

    DDSGuardCondition * guard_condition = (DDSGuardCondition*)guard_condition_handle->data;
    guard_condition->set_trigger_value(DDS_BOOLEAN_TRUE);
}

rmw_ret_t
rmw_wait(rmw_subscriptions_t * subscriptions,
         rmw_guard_conditions_t * guard_conditions,
         rmw_services_t * services,
         rmw_clients_t * clients,
         bool non_blocking)
{
    //std::cout << "wait()" << std::endl;

    DDSWaitSet waitset;

    // add a condition for each subscriber
    for (unsigned long i = 0; i < subscriptions->subscriber_count; ++i)
    {
        void * data = subscriptions->subscribers[i];
        CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo*)data;
        DDSDataReader* topic_reader = custom_subscriber_info->topic_reader_;
        DDSStatusCondition * condition = topic_reader->get_statuscondition();
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

    // add a condition for each service
    for (unsigned long i = 0; i < services->service_count; ++i)
    {
        void * data = services->services[i];
        CustomServiceInfo * custom_service_info = (CustomServiceInfo*)data;
        DDSDataReader* request_datareader = custom_service_info->request_datareader_;
        DDSStatusCondition * condition = request_datareader->get_statuscondition();
        condition->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);

        waitset.attach_condition(condition);
    }

    // add a condition for each client
    for (unsigned long i = 0; i < clients->client_count; ++i)
    {
        void * data = clients->clients[i];
        CustomClientInfo * custom_client_info = (CustomClientInfo*)data;
        DDSDataReader* response_datareader = custom_client_info->response_datareader_;
        DDSStatusCondition * condition = response_datareader->get_statuscondition();
        condition->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);

        waitset.attach_condition(condition);
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
            //std::cout << "wait() no data - rinse and repeat" << std::endl;
            continue;
        };
        if (status != DDS_RETCODE_OK) {
            printf("wait() failed. Status = %d\n", status);
            throw std::runtime_error("wait failed");
        };
    }

    // set subscriber handles to zero for all not triggered conditions
    for (unsigned long i = 0; i < subscriptions->subscriber_count; ++i)
    {
        void * data = subscriptions->subscribers[i];
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

    // set service handles to zero for all not triggered conditions
    for (unsigned long i = 0; i < services->service_count; ++i)
    {
        void * data = services->services[i];
        CustomServiceInfo * custom_service_info = (CustomServiceInfo*)data;
        DDSDataReader* request_datareader = custom_service_info->request_datareader_;
        DDSStatusCondition * condition = request_datareader->get_statuscondition();

        // search for service condition in active set
        unsigned long j = 0;
        for (; j < active_conditions.length(); ++j)
        {
            if (active_conditions[j] == condition)
            {
                break;
            }
        }
        // if service condition is not found in the active set
        // reset the subscriber handle
        if (!(j < active_conditions.length()))
        {
            services->services[i] = 0;
        }
    }

    // set client handles to zero for all not triggered conditions
    for (unsigned long i = 0; i < clients->client_count; ++i)
    {
        void * data = clients->clients[i];
        CustomClientInfo * custom_client_info = (CustomClientInfo*)data;
        DDSDataReader* response_datareader = custom_client_info->response_datareader_;
        DDSStatusCondition * condition = response_datareader->get_statuscondition();

        // search for service condition in active set
        unsigned long j = 0;
        for (; j < active_conditions.length(); ++j)
        {
            if (active_conditions[j] == condition)
            {
                break;
            }
        }
        // if client condition is not found in the active set
        // reset the subscriber handle
        if (!(j < active_conditions.length()))
        {
            clients->clients[i] = 0;
        }
    }
    return RMW_RET_OK;
}

rmw_client_t * create_client(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name)
{
    std::cout << "create_client()" << std::endl;

    if (node->implementation_identifier != _rti_connext_identifier)
    {
        printf("node handle not from this implementation\n");
        printf("but from: %s\n", node->implementation_identifier);
        throw std::runtime_error("node handle not from this implementation");
    }

    std::cout << "create_client() " << node->implementation_identifier << std::endl;

    std::cout << "  create_client() extract participant from opaque node handle" << std::endl;

    DDSDomainParticipant* participant = (DDSDomainParticipant*)node->data;

    rmw_connext_cpp::ServiceTypeSupportCallbacks * callbacks = (rmw_connext_cpp::ServiceTypeSupportCallbacks*)type_support->data;

    DDSDataReader * response_datareader;

    void * requester = callbacks->_create_requester(participant, service_name, &response_datareader);

    std::cout << "  create_client() build opaque publisher handle" << std::endl;
    CustomClientInfo* custom_client_info = new CustomClientInfo();
    custom_client_info->requester_ = requester;
    custom_client_info->callbacks_ = callbacks;
    custom_client_info->response_datareader_ = response_datareader;

    rmw_client_t * client = new rmw_client_t;
    client->implementation_identifier =_rti_connext_identifier;
    client->data = custom_client_info;
    return client;
}

rmw_ret_t
rmw_send_request(const rmw_client_t * client, const void * ros_request,
                 int64_t * sequence_id)
{
    if (client->implementation_identifier != _rti_connext_identifier)
    {
        printf("client handle not from this implementation\n");
        printf("but from: %s\n", client->implementation_identifier);
        throw std::runtime_error("client handle not from this implementation");
    }

    CustomClientInfo * custom_client_info = (CustomClientInfo*)client->data;
    void * requester = custom_client_info->requester_;
    const rmw_connext_cpp::ServiceTypeSupportCallbacks * callbacks = custom_client_info->callbacks_;

    *sequence_id = callbacks->_send_request(requester, ros_request);
    return RMW_RET_OK;
}

rmw_service_t *
rmw_create_service(const rmw_node_t * node,
                   const rosidl_service_type_support_t * type_support,
                   const char * service_name)
{
    std::cout << "create_service()" << std::endl;

    if (node->implementation_identifier != _rti_connext_identifier)
    {
        printf("node handle not from this implementation\n");
        printf("but from: %s\n", node->implementation_identifier);
        throw std::runtime_error("node handle not from this implementation");
    }

    std::cout << "create_service() " << node->implementation_identifier << std::endl;

    std::cout << "  create_service() extract participant from opaque node handle" << std::endl;

    DDSDomainParticipant* participant = (DDSDomainParticipant*)node->data;

    rmw_connext_cpp::ServiceTypeSupportCallbacks * callbacks = (rmw_connext_cpp::ServiceTypeSupportCallbacks*)type_support->data;

    DDSDataReader * request_datareader;

    void * replier = callbacks->_create_replier(participant, service_name, &request_datareader);

    std::cout << "  create_service() build opaque publisher handle" << std::endl;
    CustomServiceInfo* custom_service_info = new CustomServiceInfo();
    custom_service_info->replier_ = replier;
    custom_service_info->callbacks_ = callbacks;
    custom_service_info->request_datareader_ = request_datareader;

    rmw_service_t * service = new rmw_service_t;
    service->implementation_identifier =_rti_connext_identifier;
    service->data = custom_service_info;
    return service;
}

rmw_ret_t
rmw_take_request(const rmw_service_t * service,
                 void * ros_request_header, void * ros_request)
{
    if (service->implementation_identifier != _rti_connext_identifier)
    {
        printf("service handle not from this implementation\n");
        printf("but from: %s\n", service->implementation_identifier);
        throw std::runtime_error("service handle not from this implementation");
    }

    CustomServiceInfo * custom_service_info = (CustomServiceInfo*)service->data;

    void * replier = custom_service_info->replier_;

    const rmw_connext_cpp::ServiceTypeSupportCallbacks * callbacks = custom_service_info->callbacks_;

    if(!(callbacks->_take_request(replier, ros_request, ros_request_header))) {
      ros_request_header = NULL;
      ros_request = NULL;
    }

    return RMW_RET_OK;
}

rmw_ret_t
rmw_take_response(const rmw_client_t * client,
                  void * ros_request_header, void * ros_response)
{
    if (client->implementation_identifier != _rti_connext_identifier)
    {
        printf("client handle not from this implementation\n");
        printf("but from: %s\n", client->implementation_identifier);
        throw std::runtime_error("client handle not from this implementation");
    }

    CustomClientInfo * custom_client_info = (CustomClientInfo*)client->data;

    void * requester = custom_client_info->requester_;

    const rmw_connext_cpp::ServiceTypeSupportCallbacks * callbacks = custom_client_info->callbacks_;

    if(!(callbacks->_take_response(requester, ros_response, ros_request_header))) {
      ros_request_header = NULL;
      ros_response = NULL;
    }

    return RMW_RET_OK;
}

rmw_ret_t
rmw_send_response(const rmw_service_t * service,
                  void * ros_request, void * ros_response)
{
    if (service->implementation_identifier != _rti_connext_identifier)
    {
        printf("service handle not from this implementation\n");
        printf("but from: %s\n", service->implementation_identifier);
        throw std::runtime_error("service handle not from this implementation");
    }

    CustomServiceInfo * custom_service_info = (CustomServiceInfo*)service->data;

    void * replier = custom_service_info->replier_;

    const rmw_connext_cpp::ServiceTypeSupportCallbacks * callbacks = custom_service_info->callbacks_;

    callbacks->_send_response(replier, ros_request, ros_response);
    return RMW_RET_OK;
}

}
