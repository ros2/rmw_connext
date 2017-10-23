# Copyright 2014 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import subprocess
import sys

from rosidl_cmake import convert_camel_case_to_lower_case_underscore
from rosidl_cmake import expand_template
from rosidl_cmake import get_newest_modification_time
from rosidl_parser import parse_message_file
from rosidl_parser import parse_service_file
from rosidl_parser import validate_field_types


def parse_ros_interface_files(pkg_name, ros_interface_files):
    message_specs = []
    service_specs = []
    for idl_file in ros_interface_files:
        extension = os.path.splitext(idl_file)[1]
        if extension == '.msg':
            message_spec = parse_message_file(pkg_name, idl_file)
            message_specs.append((idl_file, message_spec))
        elif extension == '.srv':
            service_spec = parse_service_file(pkg_name, idl_file)
            service_specs.append(service_spec)
    return (message_specs, service_specs)


def generate_dds_connext_cpp(
        pkg_name, dds_interface_files, dds_interface_base_path, deps,
        output_basepath, idl_pp, message_specs, service_specs):

    include_dirs = [dds_interface_base_path]
    for dep in deps:
        # Only take the first : for separation, as Windows follows with a C:\
        dep_parts = dep.split(':', 1)
        assert len(dep_parts) == 2, "The dependency '%s' must contain a double colon" % dep
        idl_path = dep_parts[1]
        idl_base_path = os.path.dirname(
            os.path.dirname(os.path.dirname(os.path.normpath(idl_path))))
        if idl_base_path not in include_dirs:
            include_dirs.append(idl_base_path)

    for index, idl_file in enumerate(dds_interface_files):
        assert os.path.exists(idl_file), 'Could not find IDL file: ' + idl_file

        # get two level of parent folders for idl file
        folder = os.path.dirname(idl_file)
        parent_folder = os.path.dirname(folder)
        output_path = os.path.join(
            output_basepath,
            os.path.basename(parent_folder),
            os.path.basename(folder))
        try:
            os.makedirs(output_path)
        except FileExistsError:
            pass

        cmd = [idl_pp]
        for include_dir in include_dirs:
            cmd += ['-I', include_dir]
        cmd += [
            '-d', output_path,
            '-language', 'C++',
            '-namespace',
            '-update', 'typefiles',
            '-unboundedSupport',
            idl_file
        ]
        if os.name == 'nt':
            cmd[-5:-5] = ['-dllExportMacroSuffix', pkg_name]

        msg_name = os.path.splitext(os.path.basename(idl_file))[0]
        count = 1
        max_count = 5
        while True:
            subprocess.check_call(cmd)

            # fail safe if the generator does not work as expected
            any_missing = False
            for suffix in ['.h', '.cxx', 'Plugin.h', 'Plugin.cxx', 'Support.h', 'Support.cxx']:
                filename = os.path.join(output_path, msg_name + suffix)
                if not os.path.exists(filename):
                    any_missing = True
                    break
            if not any_missing:
                break
            print("'%s' failed to generate the expected files for '%s/%s'" %
                  (idl_pp, pkg_name, msg_name), file=sys.stderr)
            if count < max_count:
                count += 1
                print('Running code generator again (retry %d of %d)...' %
                      (count, max_count), file=sys.stderr)
                continue
            raise RuntimeError('failed to generate the expected files')

        if os.name != 'nt':
            # modify generated code to avoid unsed global variable warning
            # which can't be suppressed non-globally with gcc
            msg_filename = os.path.join(output_path, msg_name + '.h')
            # TODO(karsten1987): Modify should take array of callbacks
            # to avoid multiple file readings
            _modify(msg_filename, pkg_name, msg_name, _inject_unused_attribute)

            plugin_filename = os.path.join(output_path, msg_name + 'Plugin.cxx')
            _modify(plugin_filename, pkg_name, msg_name, _modify_plugin_serialize_function)

    return 0


def _get_serialization_code(msg_name, indentation):
    val = ("{{\n"
           "{indentation}// MODIFIED FOR ROS2 PURPOSES\n"
           "{indentation}const ConnextStaticMessageHandle * message_handle =\n"
           "{indentation}  reinterpret_cast<const ConnextStaticMessageHandle *>(fake_sample);\n"
           "{indentation}if (message_handle->raw_message) {{\n"
           "{indentation}  memcpy(stream->_buffer, message_handle->raw_message, "
           "*(message_handle->raw_message_length));\n"
           "{indentation}  stream->_relativeBuffer = stream->_buffer;\n"
           "{indentation}  stream->_tmpRelativeBuffer = stream->_buffer;\n"
           "{indentation}  stream->_buffer = stream->_buffer;\n"
           "{indentation}  //stream->_endian = \'\\x01\';\n"
           "{indentation}  //stream->_nativeEndian = \'\\x01\';\n"
           "{indentation}  //stream->_encapsulationKind = 1;\n"
           "{indentation}  //stream->_zeroOnAlign = 0;\n"
           "{indentation}  stream->_currentPosition = "
           "stream->_buffer + *(message_handle->raw_message_length);\n"
           "{indentation}  return RTI_TRUE;\n"
           "{indentation}}}\n"
           "{indentation}const {msg_name} * sample = reinterpret_cast<const {msg_name} *> "
           "(message_handle->untyped_dds_message);\n"
           .format(indentation=indentation, msg_name=msg_name))
    return val


def _modify_plugin_serialize_function(pkg_name, msg_name, lines):
    # set include correctly - line 49 is the last generated include
    if lines[49] == '':
        lines[49] = ('\n// MODIFIED FOR ROS2 PURPOSES\n#include \"'
                     'rosidl_typesupport_connext_cpp/connext_static_message_handle.hpp\"\n')

    serialize_fcn_signature = msg_name + 'Plugin_serialize('
    print("looking for '%s' serialize function" % serialize_fcn_signature)
    signature_found = False
    injection_start = None
    for index, line in enumerate(lines):
        if not signature_found:
            if line.lstrip().startswith(serialize_fcn_signature):
                signature_found = True
        else:
            if '{' in line.lstrip():
                print("found %s serialize function in line: %d" % (msg_name, index))
                injection_start = index
                break
    if not signature_found:
        raise RuntimeError('failed to locate %sPlugin_serialize function' % msg_name)

    # rename message argument from sample to fake_sample
    # this eases the modification within the serialize function
    print(lines[injection_start - 6])
    lines[injection_start - 6] = lines[injection_start - 6].replace('sample', 'fake_sample')
    indentation = ' ' * 16
    lines[injection_start] = line.replace('{', _get_serialization_code(msg_name, indentation))
    return True


def _inject_unused_attribute(pkg_name, msg_name, lines):
    # prepend attribute before constants of string type
    prefix = 'static const DDS_Char * Constants__'
    inject_prefix = '__attribute__((unused)) '
    for index, line in enumerate(lines):
        if not line.lstrip().startswith(prefix):
            continue
        lines[index] = line.replace(prefix, inject_prefix + prefix)
    return True


def _modify(filename, pkg_name, msg_name, callback):
    with open(filename, 'r') as h:
        lines = h.read().split('\n')
    modified = callback(pkg_name, msg_name, lines)
    if modified:
        with open(filename, 'w') as h:
            h.write('\n'.join(lines))


def generate_cpp(args, message_specs, service_specs, known_msg_types):
    template_dir = args['template_dir']
    mapping_msgs = {
        os.path.join(template_dir, 'msg__rosidl_typesupport_connext_cpp.hpp.em'):
        '%s__rosidl_typesupport_connext_cpp.hpp',
        os.path.join(template_dir, 'msg__type_support.cpp.em'):
        '%s__type_support.cpp',
    }
    mapping_srvs = {
        os.path.join(template_dir, 'srv__rosidl_typesupport_connext_cpp.hpp.em'):
        '%s__rosidl_typesupport_connext_cpp.hpp',
        os.path.join(template_dir, 'srv__type_support.cpp.em'):
        '%s__type_support.cpp',
    }

    for template_file in mapping_msgs.keys():
        assert os.path.exists(template_file), 'Could not find template: ' + template_file
    for template_file in mapping_srvs.keys():
        assert os.path.exists(template_file), 'Could not find template: ' + template_file

    functions = {
        'get_header_filename_from_msg_name': convert_camel_case_to_lower_case_underscore,
    }
    # generate_dds_connext_cpp() and therefore the make target depend on the additional files
    # therefore they must be listed here even if the generated type support files are independent
    latest_target_timestamp = get_newest_modification_time(
        args['target_dependencies'] + args.get('additional_files', []))

    for idl_file, spec in message_specs:
        validate_field_types(spec, known_msg_types)
        subfolder = os.path.basename(os.path.dirname(idl_file))
        for template_file, generated_filename in mapping_msgs.items():
            generated_file = os.path.join(args['output_dir'], subfolder)
            if generated_filename.endswith('.cpp'):
                generated_file = os.path.join(generated_file, 'dds_connext')
            generated_file = os.path.join(
                generated_file, generated_filename %
                convert_camel_case_to_lower_case_underscore(spec.base_type.type))

            data = {'spec': spec, 'subfolder': subfolder}
            data.update(functions)
            expand_template(
                template_file, data, generated_file,
                minimum_timestamp=latest_target_timestamp)

    for spec in service_specs:
        validate_field_types(spec, known_msg_types)
        for template_file, generated_filename in mapping_srvs.items():
            generated_file = os.path.join(args['output_dir'], 'srv')
            if generated_filename.endswith('.cpp'):
                generated_file = os.path.join(generated_file, 'dds_connext')
            generated_file = os.path.join(
                generated_file, generated_filename %
                convert_camel_case_to_lower_case_underscore(spec.srv_name))

            data = {'spec': spec}
            data.update(functions)
            expand_template(
                template_file, data, generated_file,
                minimum_timestamp=latest_target_timestamp)

    return 0
