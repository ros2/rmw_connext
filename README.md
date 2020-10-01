# rmw_connext
Implementation of the ROS Middleware (rmw) Interface using [RTI's Connext DDS](https://www.rti.com). 

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
That profile will be used as a base and it will be overridden with the ROS specific policies except when `rmw_qos_profile_system_default` is used.
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
        <publish_mode>
          <flow_controller_name>dds.flow_controller.token_bucket.slow_flow</flow_controller_name>
        </publish_mode>
      </datawriter_qos>
    </qos_profile>
  </qos_library>
</dds>
```

That will force all publishers in the `my_large_data_topic` to use the `slow_flow` flow controller.
See `Topic Name Mangling` section to understand the `rt/` prefix.

### Using different profiles for different nodes

To use this feature, you must first set the following environment variable:

```bat
:: Windows
set RMW_CONNEXT_ALLOW_NODE_QOS_PROFILES=1
```
```bash
# Linux/MacOS
export RMW_CONNEXT_ALLOW_NODE_QOS_PROFILES=1
```

If the environment variable is set, if a profile name matches the fully qualified node name (e.g.: `/my/full/namespace/my_node_name`), it will be used instead of the default profile.
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
    <qos_profile name="/my_node" base_name="BuiltinQosLib::Baseline.5.3.0">
      <datawriter_qos topic_filter="rt/my_large_data_topic">
        <publish_mode>
          <flow_controller_name>dds.flow_controller.token_bucket.slow_flow</flow_controller_name>
        </publish_mode>
      </datawriter_qos>
    </qos_profile>
  </qos_library>
</dds>
```

Will use the `slow_flow` flow controller for publishers in the `my_large_data_topic` of the `/my_node` node.
All other nodes will use the default profile.

The profiles matching the node name will only be used for `datawriter` and `datareader` qos policies.
Participant/Publisher/Subscription policies will always use the default profile.

RTI Connext will log an error each time that it tries to find a profile that doesn't exist, if you didn't add a profile for each node those logs will appear.

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

### Ignoring the ROS specified QoS

If you want to override the QoS profile specified in ROS code, you can do that by setting:

```bat
:: Windows
set RMW_CONNEXT_IGNORE_ROS_QOS=1
```
```bash
# Linux/MacOS
export RMW_CONNEXT_IGNORE_ROS_QOS=1
```

Modifying the QoS profile a Node specified may break a contract the node's author stipulated, use under your own risk.
If you use this option, the ROS provided QoS will be completely ignored, either if a profile matching the node name is found or not.

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

## Quality Declaration (per [REP-2004](https://github.com/ros-infrastructure/rep/blob/master/rep-2004.rst))
See [RTI Quality Declaration](https://community.rti.com/static/documentation/qa/RTIConnextProQualityDeclaration(REP-2004).pdf) file, hosted on [RTI Community](https://community.rti.com) website. 
