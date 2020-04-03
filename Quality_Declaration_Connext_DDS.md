# **External Dependency Quality declaration** `rti-connext-dds-5.3.1` 

This document is a declaration of software quality for the RTI-Connect-DDS external dependency, based on the guidelines in [REP-2004](https://github.com/ros-infrastructure/rep/blob/rep-2004/rep-2004.rst).

As stated in their core documentation, *RTI® Connext® DDS solutions provide a flexible connectivity software framework for integrating data sources of all types (...) It connects data within applications as well as across devices, systems and networks. _Connext DDS_ also delivers large data sets with microsecond performance and granular quality-of-service control.* 

## Summary
The `rti-connext-dds-5.3.1` software meets most of the standards defined for Quality Level 1 defined for ROS2 packages, this includes, but not limited to, broad documentation related to the API of the software and testing of the main features of the software.

The main concerns related to this software are related to the fact that the development of it is not complete in an open-source fashion, so it is unknown the policies they have to develop their code and if they have any policies related to versioning. For the case of ROS2, as the libraries are fixed to the `5.3.1` there should not be any considering versioning and API/ABI stability. 

Without reports of coverage, it is not clear if the software is completely tested. However, according to a [blog entry](https://www.rti.com/blog/software-testing-at-rti)  from RTI, the company takes testing very seriously. 

There is a list of [known academic projects](https://community.rti.com/projects) using RTI Connext DDS and also they have published a [white paper](https://www.rti.com/whitepapers/how-to-achieve-production-grade-deployment-with-ros-2-and-rti-connext-dds) with rationale about how to implement production-grade deployments using their software in conjunction with ROS2.

TO-DO: Add stronger reasoning to consider RTI-Connext as Q1 level? Open to discuss if needed

Considering the previously mentioned reasons, we consider this library to be robust and reliable, and hence we declare it to qualify as a level 1 external dependency.

## Comparison with ROS packages quality standards

### Version policy

 1. *Must have a version policy*
There is no public information related how the RTI manages the versioning of `rti-connext-dds-5.3.1`

 2. Must be at a stable version (e.g. for semver that means version >= 1.0.0)
Current version used in ROS2 is `5.3.1`, and latest version is `6.0.1`.

3.  *Must have a strictly declared public API*
The public API is declared in their [documentation page](https://community.rti.com/documentation/rti-connext-dds-531) with sections for [C API](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/api/connext_dds/api_c/index.html), [C++ Modern API](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/api/connext_dds/api_cpp2/index.html), [C ++ Traditional API](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/api/connext_dds/api_cpp/index.html) and other programming languages.
    
4.  *Must have a policy for API stability*
There is no public policy for API stability, as the software is fixed to a particular version (5.3.1) then this won't be a problem for its use within the ROS2 environment.

5. *Must have a policy for ABI stability*
There is no public policy for ABI stability, as the software is fixed to a particular version (5.3.1) then this won't be a problem for its use within the ROS2 environment.

6.  *Must have a policy that keeps API and ABI stability within a released ROS distribution*   
There is no direct correlation between API and ABI stability of the library within ROS distributions.

### Change Control Process

7.  *Must have all code changes occur through a change request (e.g. pull request, merge request, etc.)*
It is not available to the public how the software is modified.

8.  *Must have confirmation of contributor origin (e.g. [DCO](https://developercertificate.org/), CLA, etc.)*
It is not available to the public how the software is modified.

9.  *Must have peer review policy for all change requests (e.g. require one or more reviewers)*
It is not available to the public how the software is modified.

10.  *Must have Continuous Integration (CI) policy for all change requests*
    It is not available to the public how the software is modified.
    
11.  *Must have documentation policy for all change requests*
It is not available to the public how the software is modified.    
    

### Documentation

12.  *Must have documentation for each "feature" (e.g. for rclcpp: create a node, publish a message, spin, etc.)*
The complete software is documented in its [main documentation page](https://community.rti.com/documentation/rti-connext-dds-531). The suggested reading for the features of the libraries is the user manual, which covers documentation for the basic concepts and library features implementations.

13.  *Must have documentation for each item in the public API (e.g. functions, classes, etc.)*
The [documentation page](https://community.rti.com/documentation/rti-connext-dds-531) includes several manuals for the different public APIs of the software for various programming languages.

14.  *Must have a declared license or set of licenses*
The license provided to use is the software is the *Real Time Innovations Software License Agreement*, available in their content page [here](https://community.rti.com/content/page/rti-software-license-agreement).

15.  *Must have a copyright statement in each source file*

16.  *Must have a "quality declaration" document, which declares the quality level and justifies how the package meets each of the requirements*
This document represents the quality declaration for the external dependency `rti-connext-dds-5.3.1`

### Testing

17.  *Must have system tests which cover all items in the "feature" documentation*
There is a public [blog entry](https://www.rti.com/blog/software-testing-at-rti) from the RTI company summarizing the standards they have for testing, and explaining they have unit testing, feature testing and interoperability testing. The test suite, environment nor the test code is available publicly.

18.  *Must have system, integration, and/or unit tests which cover all of the public API*
There is a public [blog entry](https://www.rti.com/blog/software-testing-at-rti) from the RTI company summarizing the standards they have for testing, and explaining they have unit testing, feature testing and interoperability testing. The test suite, environment nor the test code is available publicly.

19.  *Must have code coverage, and a policy for changes*
There is no public information regardless this software has coverage report tracking/testing.

20.  *Performance tests, and performance regression policy*
RTI provides a performance testing software [here](https://community.rti.com/downloads/rti-connext-dds-performance-test). With this tool is possible to test latency and throughput of the `rti-connext-dds-5.3.1` and according to the [blog entry](https://www.rti.com/blog/software-testing-at-rti) explaining how RTI manages testing, this tool is used so the software does not drops below a certain percentage established. 

21.  *Linters and Static Analysis*
There is no public information regardless this software has linters or static analysis.

### Dependencies

22.  Must not have direct runtime "ROS" dependencies which are not at the same level as the package in question ('Level N'), but…

`rti-connext-dds-5.3.1` depends on the external library `openssl1.1.1`. This library comes compiled and bundled with the RTI software, and as it is tied to a fixed version is expected this library is tested with the whole `rti-connext-dds-5.3.1` testing tools environment.

TODO: Confirm is this is included in the testings, ask RTI? Should it be done an extra analysis for openssl?

### Platform Support

23.  Must support all tier 1 platforms for ROS 2, as defined in [REP-2000](https://www.ros.org/reps/rep-2000.html#support-tiers)

The `rti-connext-dds-5.3.1` provides support for Linux, MacOs and Windows platforms, as it can be seen in their [supported platforms](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_ReleaseNotes/index.htm#ReleaseNotes/System_Requirements.htm#Table_SupportedPlatforms) landing page.  However, there is no support for Ubuntu Focal nor MacOs 10.14.
