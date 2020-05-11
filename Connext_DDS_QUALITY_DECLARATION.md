# **External Dependency Quality declaration** `RTI-Connext-DDS`

This document is a declaration of software quality for the `RTI-Connext-DDS` external dependency, based on the guidelines in [REP-2004](https://github.com/ros-infrastructure/rep/blob/master/rep-2004.rst).

As stated in their core documentation, *RTI® Connext® DDS solutions provide a flexible connectivity software framework for integrating data sources of all types (...) It connects data within applications as well as across devices, systems and networks. _Connext DDS_ also delivers large data sets with microsecond performance and granular quality-of-service control.* 

This Quality Declaration claims that the external dependency `RTI-Connext-DDS` is in the Quality Level 4 category.

Below are the rationales, notes, and caveats for this claim, organized by each requirement listed in the [Package Requirements for Quality Level 4 in REP-2004](https://www.ros.org/reps/rep-2004.html).

## Summary
`RTI-Connext-DDS` meets most of the standards defined for Quality Level 1, including broad documentation and testing of its features and API.

As this package is not open-source, many policies related to quality control are not openly documented. For example, it is difficult to find information relating to its versioning policy. For ROS 2 distributions, packages may need to rely on strict versions, like `5.3.1`, to ensure API/ABI stability.

There are no clear public reports of code coverage, so it is not clear how much of their code is tested. However, they do describe their approach to testing in a [blog entry](https://www.rti.com/blog/software-testing-at-rti).

This software is used in several markets as the automotive, energy, aerospace and oil markets. [Here](https://www.mocana.com/rti) we can see a statement of a partnership with Mocana to provide Critical Industrial IoT systems. We can also see [here](https://www.ddci.com/pr2001/) an annoucement of its integration within the DCC-I's Deos avionics real-time operating system.

There is a [white paper](https://www.rti.com/whitepapers/how-to-achieve-production-grade-deployment-with-ros-2-and-rti-connext-dds) with rationale about how to implement production-grade deployments using their software in conjunction with ROS 2.

In terms of ROS 2 package metrics this library is considered to be Quality Level 4. There should more information about how `RTI-Connext-DDS` deals with the issues listed in the [REP-2004](https://github.com/ros-infrastructure/rep/blob/master/rep-2004.rst) so consumers of the package have a better understanding about how to depend on it. This could achieved with some transparency on their internal policies or with a public statement similar to this [blog entry](https://www.rti.com/blog/software-testing-at-rti).

## Version Policy [1]

### Version Scheme [1.i]

`RTI-Connext-DDS` does not have a declared versioning scheme. However, each new release of the software comes with detailed [What's new](https://community.rti.com/static/documentation/connext-dds/5.3.0/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_WhatsNew/index.htm#WhatsNew/WhatsNew530/WhatsNew530.htm) and [Release notes](https://community.rti.com/static/documentation/connext-dds/5.3.0/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_ReleaseNotes/index.htm) documentation that provides users with information that includes new features, supported platforms and known issues.

### Version Stability [1.ii]

The library has a version >= 1.0.0.

### Public API Declaration [1.iii]

The public API is declared in their [documentation page](https://community.rti.com/documentation/rti-connext-dds-531) with sections for [C API](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/api/connext_dds/api_c/index.html), [C++ Modern API](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/api/connext_dds/api_cpp2/index.html), [C ++ Traditional API](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/api/connext_dds/api_cpp/index.html) and other programming languages.

### API Stability Policy [1.iv]/[1.vi]

There is no declared policy for API stability and it's unclear if breaking changes may be introduced during minor or patch versions. Packages that depend on `RTI-Connext-DDS` may need to limit the versions they rely on to ensure API stability.

### ABI Stability Policy [1.v]/[1.vi]

There is no declared policy for ABI stability and it's unclear if breaking changes may be introduced during minor or patch versions. Packages that depend on `RTI-Connext-DDS` may need to limit the versions they rely on to ensure API stability.

## Change Control Process [2]

The change control process of `RTI-Connext-DDS` is not publicly documented (sections [2.i - 2.v]).

## Documentation [3]

### Feature Documentation [3.i]

`RTI-Connext-DDS` provides extensive and high quality documentation hosted on their [main documentation page](https://community.rti.com/documentation/rti-connext-dds-531).

The documentation includes a [Getting Started Guide](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_GettingStarted/index.htm), [Core Libraries User Manual](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_UsersManual/index.htm), [Core Libraries What's New](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_WhatsNew/index.htm), [Core Library Release Notes](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_ReleaseNotes/index.htm), several references for their different APIs and different topics related to the software usage.

### Public API Documentation [3.ii]

The [documentation page](https://community.rti.com/documentation/rti-connext-dds-531) includes several manuals for the different public APIs of the software for various programming languages.

### License [3.iii]

`RTI-Connext-DDS` is governed by the [*Real Time Innovations Software License Agreement*](https://community.rti.com/content/page/rti-software-license-agreement).

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

RTI provides performance testing software [here](https://community.rti.com/downloads/rti-connext-dds-performance-test). With this tool, it is possible to test latency and throughput of `RTI-Connext-DDS`. According to this [blog entry](https://www.rti.com/blog/software-testing-at-rti) explaining how RTI manages testing, this tool is used to ensure the software does not regress below a certain percentage when new features are added.

### Linters and Static Analysis [4.v]

There is no public information available about this software's linting or static analysis.

## Dependencies [5]

### Direct/Optional Runtime ROS Dependencies [5.i]/[5.ii]

`RTI-Connext-DDS` has no direct/optional runtime ROS dependencies.

### Direct Runtime non-ROS Dependency [5.iii]

`RTI-Connext-DDS` depends on the external library `openssl1.1.1`. This library comes compiled and bundled with the RTI software. It is expected that this library is tested as a part of the `RTI-Connext-DDS` testing tools environment.

## Platform Support [6]

The `RTI-Connext-DDS` provides support for Linux, MacOs and Windows platforms, as it can be seen in their [supported platforms](https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_ReleaseNotes/index.htm#ReleaseNotes/System_Requirements.htm#Table_SupportedPlatforms) landing page.  However, there is no declared support for Ubuntu Focal nor MacOs 10.14.

## Security [7]

###  Vulnerability Disclosure Policy [7.i]

`RTI-Connext-DDS` does not have a Vulnerability Disclosure Policy.
