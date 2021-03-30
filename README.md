# rmw_connext
Implementation of the ROS Middleware (rmw) Interface using [RTI's Connext DDS](https://www.rti.com). 

**DEPRECATION NOTICE:** [rmw_connextdds](https://github.com/ros2/rmw_connextdds)
is a new RMW implementation for RTI Connext DDS, which supersedes the one
contained in this repository (`rmw_connext_cpp`). This new implementation was
developed by RTI in collaboration with the ROS 2 community, and it resolves
several performance issues that are present in this implementation.
`rmw_connextdds` is already available in Rolling, and it will be included in
future ROS 2 releases starting with Galactic.

## Working with rmw_connext
To use rmw_connext with ROS2 applications, set the environment variable ```RMW_IMPLEMENTATION=rmw_connext_cpp``` and run your ROS2 applications as usual:  

**Linux:**  
```export RMW_IMPLEMENTATION=rmw_connext_cpp```    
or prepend on ROS2 command line, such as:
```RMW_IMPLEMENTATION=rmw_connext_cpp ros2 run rviz2 rviz2```

**Windows**:  
```set RMW_IMPLEMENTATION=rmw_connext_cpp```  


## Binary Installation
Pre-built binaries for RTI Connext DDS are available for **x86_64 (Debian/Ubuntu) Linux** platform using the steps outlined in the [ROS2 installation wiki](https://index.ros.org/doc/ros2/Installation), available under a non-commercial license.  
Other platforms must be built from source, using a separately-installed copy of RTI Connext DDS.

## How to get RTI Connext DDS
This implementation of rmw_connext requires version 5.3.1 of RTI Connext DDS, which can be obtained through the [RTI University Program](https://index.ros.org/doc/ros2/Installation/DDS-Implementations/Install-Connext-University-Eval/#rti-university-program), purchase, or as an [evaluation](https://index.ros.org/doc/ros2/Installation/DDS-Implementations/Install-Connext-University-Eval/#rti-connext-dds-evaluation).
Note that the RTI website has [Free Trial](https://www.rti.com/free-trial) offers, but these are typically for the most-current version of RTI Connext DDS (6.0.1 as of this writing), which does not build with this implementation of rmw_connext.

## Building
Refer to the [Install DDS Implementations](https://index.ros.org/doc/ros2/Installation/DDS-Implementations) page for details on building rmw_connext for your platform.  

## Using Connext XML QoS settings

QoS profiles can be specified in XML according to the load order specified [here](https://community.rti.com/static/documentation/connext-dds/5.2.0/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_UsersManual/Content/UsersManual/How_to_Load_XML_Specified_QoS_Settings.htm). `url_profile` and `string_profile` cannot be used.

ROS will use the profile with the `is_default_qos="true"` attribute.
The policies defined in the ROS QoS profile will override those in the default profile, except when `rmw_qos_profile_system_default` is used.

For example:

```xml
<?xml version="1.0"?>
<dds xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:noNamespaceSchemaLocation="http://community.rti.com/schema/5.3.1/rti_dds_qos_profiles.xsd" version="5.3.1">
  <qos_library name="Ros2TestQosLibrary">
    <qos_profile name="Ros2TestDefaultQos" base_name="BuiltinQosLib::Baseline.5.3.0" is_default_qos="true">
      <participant_qos>
        <property>
          <value>
            <!-- 6.25 MB/sec (52 Mb/sec) flow controller -->
            <element>
              <name>dds.flow_controller.token_bucket.slow_flow.token_bucket.max_tokens</name>
              <value>8</value>
            </element>
            <element>
              <name>dds.flow_controller.token_bucket.slow_flow.token_bucket.tokens_added_per_period</name>
              <value>8</value>
            </element>
            <element>
              <name>dds.flow_controller.token_bucket.slow_flow.token_bucket.bytes_per_token</name>
              <value>8192</value>
            </element>
            <element>
              <name>dds.flow_controller.token_bucket.slow_flow.token_bucket.period.sec</name>
              <value>0</value>
            </element>
            <element>
              <name>dds.flow_controller.token_bucket.slow_flow.token_bucket.period.nanosec</name>
              <value>10000000</value>
            </element>
          </value>
        </property>
      </participant_qos>

      <datawriter_qos topic_filter="rt/my_large_data_topic">
        <reliability>
          <kind>RELIABLE_RELIABILITY_QOS</kind>
        </reliability>
        <publish_mode>
          <flow_controller_name>dds.flow_controller.token_bucket.slow_flow</flow_controller_name>
        </publish_mode>
      </datawriter_qos>
    </qos_profile>
  </qos_library>
</dds>
```

That will force all publishers in the `my_large_data_topic` to use the `slow_flow` flow controller, but the reliability specified in the ROS QoS profile will be used except if its value is `RMW_QOS_RELIABILITY_POLICY_SYSTEM_DEFAULT`. 
See `Topic Name Mangling` section to understand the `rt/` prefix.

See [RTI Connext docs](https://community.rti.com/static/documentation/connext-dds/5.2.0/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_UsersManual/Content/UsersManual/Topic_Filters.htm) to understand topic filters.

### Overriding ROS specified QoS policies for a topic

To use this feature, you must first set the following environment variable:

```bat
:: Windows
set RMW_CONNEXT_ALLOW_TOPIC_QOS_PROFILES=1
```
```bash
# Linux/MacOS
export RMW_CONNEXT_ALLOW_TOPIC_QOS_PROFILES=1
```

If the environment variable is set, when a profile name matches the dds topic name, it will be used and the ROS specified profile will be ignored.

For example:
```xml
<?xml version="1.0"?>
<dds xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:noNamespaceSchemaLocation="http://community.rti.com/schema/5.3.1/rti_dds_qos_profiles.xsd" version="5.3.1">
  <qos_library name="Ros2TestQosLibrary">
    <qos_profile name="Ros2TestDefaultQos" base_name="BuiltinQosLib::Baseline.5.3.0" is_default_qos="true">
      <participant_qos>
        <property>
          <value>
            <!-- 6.25 MB/sec (52 Mb/sec) flow controller -->
            <element>
              <name>dds.flow_controller.token_bucket.slow_flow.token_bucket.max_tokens</name>
              <value>8</value>
            </element>
            <element>
              <name>dds.flow_controller.token_bucket.slow_flow.token_bucket.tokens_added_per_period</name>
              <value>8</value>
            </element>
            <element>
              <name>dds.flow_controller.token_bucket.slow_flow.token_bucket.bytes_per_token</name>
              <value>8192</value>
            </element>
            <element>
              <name>dds.flow_controller.token_bucket.slow_flow.token_bucket.period.sec</name>
              <value>0</value>
            </element>
            <element>
              <name>dds.flow_controller.token_bucket.slow_flow.token_bucket.period.nanosec</name>
              <value>10000000</value>
            </element>
          </value>
        </property>
      </participant_qos>
    </qos_profile>
    <qos_profile name="rt/my_large_data_topic" base_name="BuiltinQosLib::Baseline.5.3.0">
      <datawriter_qos>  <!--Don't use topic filters here-->
        <publish_mode>
          <flow_controller_name>dds.flow_controller.token_bucket.slow_flow</flow_controller_name>
        </publish_mode>
        <reliability>
          <kind>RELIABLE_RELIABILITY_QOS</kind>
        </reliability>
      </datawriter_qos>
    </qos_profile>
  </qos_library>
</dds>
```

In this case, all publishers in the topic `/my_large_data_topic` will use the specified slow flow controller and have a reliable reliability (regardless of the reliability specified in ROS code).

Caveats:

- If you want to override the QoS profiles used for all publishers in a topic, the subscription profiles in the same topic will also be overriden. If you don't explicitly provide one, a default will be used.
- RTI Connext will log an error each time that it tries to find a profile that doesn't exist.
  Your will see a lot of these logs in your terminal when using `RMW_CONNEXT_ALLOW_TOPIC_QOS_PROFILES` option. 

### Specifying an specific QoS library

If you only provided one QoS library to the process, that one will be used.
If not, the `RMW_CONNEXT_QOS_PROFILE_LIBRARY` must be used:
```bat
:: Windows
set RMW_CONNEXT_QOS_PROFILE_LIBRARY=Ros2TestQosLibrary
```
```bash
# Linux/MacOS
export RMW_CONNEXT_QOS_PROFILE_LIBRARY=Ros2TestQosLibrary
```

### Specifying a different default QoS profile

You can use the `RMW_CONNEXT_DEFAULT_QOS_PROFILE` environment variable for this.
It overrides the profile marked with `is_default_qos="true"` when set.
The profile is looked up in the QoS profile library RMW connext is using.

### Using user provided publish mode

ROS is always overriding the QoS profile of datawriters to use `ASYNCHRONOUS_PUBLISH_MODE_QOS`.
To avoid that from being overriden, you can set the following environment variable:

```bat
:: Windows
set RMW_CONNEXT_DO_NOT_OVERRIDE_PUBLICATION_MODE=1
```
```bash
# Linux/MacOS
export RMW_CONNEXT_DO_NOT_OVERRIDE_PUBLICATION_MODE=1
```

## ROS topic name mangling

ROS uses the following mangled topics when the ROS QoS policy `avoid_ros_namespace_conventions` is `false`, which is the default:
- Topics are prefixed with `rt`. e.g.: `/my/fully/qualified/ros/topic` is converted to `rt/my/fully/qualified/ros/topic`.
- The service request topics are prefixed with `rq` and suffixed with `Request`. e.g.: `/my/fully/qualified/ros/service` request topic is `rq/my/fully/qualified/ros/serviceRequest`.
- The service response topics are prefixed with `rr` and suffixed with `Response`. e.g.: `/my/fully/qualified/ros/service` response topic is `rr/my/fully/qualified/ros/serviceResponse`.

## Quality Declaration (per [REP-2004](https://github.com/ros-infrastructure/rep/blob/master/rep-2004.rst))
See [RTI Quality Declaration](https://community.rti.com/static/documentation/qa/RTIConnextProQualityDeclaration(REP-2004).pdf) file, hosted on [RTI Community](https://community.rti.com) website. 
