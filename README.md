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

### Quality Declaration (per [REP-2004](https://github.com/ros-infrastructure/rep/blob/master/rep-2004.rst))
See [RTI Quality Declaration](https://community.rti.com/static/documentation/qa/RTIConnextProQualityDeclaration(REP-2004).pdf) file, hosted on [RTI Community](https://community.rti.com) website. 
