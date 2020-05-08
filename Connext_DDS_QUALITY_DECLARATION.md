# **External Dependency Quality declaration** `rti-connext-dds-5.3.1` 

This document is a declaration of software quality for the `RTI-Connect-DDS` external dependency, based on the guidelines in [REP-2004](https://github.com/ros-infrastructure/rep/blob/master/rep-2004.rst).

As stated in their core documentation, *RTI® Connext® DDS solutions provide a flexible connectivity software framework for integrating data sources of all types (...) It connects data within applications as well as across devices, systems and networks. _Connext DDS_ also delivers large data sets with microsecond performance and granular quality-of-service control.* 

This Quality Declaration claims that the external dependency `RTI-Connect-DDS` is in the Quality Level 4 category.

Below are the rationales, notes, and caveats for this claim, organized by each requirement listed in the [Package Requirements for Quality Level 4 in REP-2004](https://www.ros.org/reps/rep-2004.html).

## Summary
The `rti-connext-dds-5.3.1` software meets most of the standards defined for Quality Level 1 defined for ROS2 packages, this includes, but not limited to, broad documentation related to the API of the software and testing of the main features of the software.

As this package is not open-source, many policies related to quality control are not openly documented. For example, it is difficult to find information relating to its versioning policy. For ROS 2 distributions, packages may need to rely on strict versions, like `5.3.1`, to ensure API/ABI stability. 

There are no clear public reports of code coverage, so it is not clear how much of their code is tested. However, they do describe their approach to testing in a [blog entry](https://www.rti.com/blog/software-testing-at-rti). 

There is a list of [known academic projects](https://community.rti.com/projects) using RTI Connext DDS and also they have published a [white paper](https://www.rti.com/whitepapers/how-to-achieve-production-grade-deployment-with-ros-2-and-rti-connext-dds) with rationale about how to implement production-grade deployments using their software in conjunction with ROS2.

In terms of ROS2 package metrics this library is considered to be Quality Level 4. Adding unit testing for the functions used in ROS2 packages, coverage statistics and version pinning will be needed to achieve Quality Level 1.

# Comparison with ROS packages quality standards

## Version Policy [1]

### Version Scheme [1.i]

There is no public information related how the RTI manages the versioning of `rti-connext-dds-5.3.1`

### Version Stability [1.ii]

The library has a version >= 1.0.0.

### Public API Declaration [1.iii]

The public API is declared in their [documentation page](https://community.rti.com/documentation/rti-connext-dds-531) with sections for [C API](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/api/connext_dds/api_c/index.html), [C++ Modern API](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/api/connext_dds/api_cpp2/index.html), [C ++ Traditional API](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/api/connext_dds/api_cpp/index.html) and other programming languages.

### API Stability Policy [1.iv]

There is no explicit policy related to API stability. This package should be pinned to a particular RTI-Connect-DDS version to be considered high quality.

### ABI Stability Policy [1.v]

There is no explicit policy related to ABI stability. This package should be pinned to a particular RTI-Connect-DDS version to be considered high quality.

### ABI and ABI Stability Within a Released ROS Distribution [1.vi]

Without a clear versioning policy for `rti-connext-dds`, ROS distributions may need declare specific versions of `rti-connext-dds` that they support to ensure API and ABI stability within a ROS distribution.

## Change Control Process [2]

### Change Requests [2.i]

It is not available to the public how the software is modified.

### Contributor Origin [2.ii]

It is not available to the public how the software is modified.

### Peer Review Policy [2.iii]

It is not available to the public how the software is modified.

### Continuous Integration [2.iv]

It is not available to the public how the software is modified.

###  Documentation Policy [2.v]

It is not available to the public how the software is modified.

## Documentation [3]

### Feature Documentation [3.i]

The complete software is documented in its [main documentation page](https://community.rti.com/documentation/rti-connext-dds-531). The suggested reading for the features of the libraries is the user manual, which covers documentation for the basic concepts and library features implementations.

### Public API Documentation [3.ii]

The [documentation page](https://community.rti.com/documentation/rti-connext-dds-531) includes several manuals for the different public APIs of the software for various programming languages.

### License [3.iii]

`rti-connext-dds` is governed by the [*Real Time Innovations Software License Agreement*](https://community.rti.com/content/page/rti-software-license-agreement).

### Copyright Statements [3.iv]

The header files provided in the library indicate they are copyrighted by *“Real-Time Innovations, Inc.”*. It is not stated if this is enforced with any kind of linter analysis.

## Testing [4]

### Feature Testing [4.i]

There is a public [blog entry](https://www.rti.com/blog/software-testing-at-rti) from the RTI company summarizing the standards they have for testing, and explaining they have unit testing, feature testing and interoperability testing. The test suite, environment nor the test code is available publicly.

### Public API Testing [4.ii]

There is a public [blog entry](https://www.rti.com/blog/software-testing-at-rti) from the RTI company summarizing the standards they have for testing, and explaining they have unit testing, feature testing and interoperability testing. The test suite, environment nor the test code is available publicly.

### Coverage [4.iii]

There is no public information available regarding the software's code coverage analysis or results.

### Performance [4.iv]

RTI provides performance testing software [here](https://community.rti.com/downloads/rti-connext-dds-performance-test). With this tool, it is possible to test latency and throughput of `rti-connext-dds`. According to this [blog entry](https://www.rti.com/blog/software-testing-at-rti) explaining how RTI manages testing, this tool is used to ensure the software does not regress below a certain percentage when new features are added. 

### Linters and Static Analysis [4.v]

There is no public information available about this software's linting or static analysis.

## Dependencies [5]

### Direct/Optional Runtime ROS Dependencies [5.i]/[5.ii]

`RTI-Connect-DDS` has not direct/optional runtime ROS dependencies.

### Direct Runtime non-ROS Dependency [5.iii]

`rti-connext-dds-5.3.1` depends on the external library `openssl1.1.1`. This library comes compiled and bundled with the RTI software, and as it is tied to a fixed version is expected this library is tested with the whole `rti-connext-dds-5.3.1` testing tools environment.

## Platform Support [6]

The `rti-connext-dds-5.3.1` provides support for Linux, MacOs and Windows platforms, as it can be seen in their [supported platforms](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_ReleaseNotes/index.htm#ReleaseNotes/System_Requirements.htm#Table_SupportedPlatforms) landing page.  However, there is no declared support for Ubuntu Focal nor MacOs 10.14.

## Security [7]

###  Vulnerability Disclosure Policy [7.i]

`RTI-Connect-DDS` does not have a Vulnerability Disclosure Policy.
