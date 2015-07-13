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
import re
import subprocess

from rosidl_cmake import convert_camel_case_to_lower_case_underscore
from rosidl_cmake import expand_template
from rosidl_cmake import get_newest_modification_time
from rosidl_generator_dds_idl import MSG_TYPE_TO_IDL
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
            '-replace',
            # TODO use -unboundedSupport when it becomes available
            idl_file
        ]
        subprocess.check_call(cmd)

        # modify generated code to support unbounded sequences and strings
        # http://community.rti.com/content/forum-topic/changing-max-size-sequence
        unbounded_fields = [f for f in message_specs[index][1].fields
                            if f.type.is_array and
                            f.type.array_size is None and
                            not f.type.is_upper_bound]
        if unbounded_fields:
            msg_name = os.path.splitext(os.path.basename(idl_file))[0]
            msg_cxx_filename = os.path.join(output_path, msg_name + '.cxx')
            _modify(msg_cxx_filename, unbounded_fields, _step_2_2)
            plugin_cxx_filename = os.path.join(output_path, msg_name + 'Plugin.cxx')
            _modify(plugin_cxx_filename, unbounded_fields, _step_2_1_and_2_3_and_2_4)

    return 0


def _modify(filename, unbounded_fields, callback):
    with open(filename, 'r') as h:
        lines = h.read().split('\n')
    modified = callback(unbounded_fields, lines)
    if modified:
        with open(filename, 'w') as h:
            h.write('\n'.join(lines))


def _step_2_2(unbounded_fields, lines):
    modified = _step_2_2_set_maximum(unbounded_fields, lines)
    modified |= _step_2_2_init_string(unbounded_fields, lines)
    return modified


# disable default initialization of sequences with fixed upper bound
def _step_2_2_set_maximum(unbounded_fields, lines):
    pattern = re.compile(
        '.*' +
        re.escape('Seq_set_maximum(&sample->') +
        '([A-Za-z0-9_]+)' +
        re.escape(' , (100))') +
        '.*')
    modified = False
    field_names = ['%s_' % f.name for f in unbounded_fields]
    for index, line in enumerate(lines):
        match = re.match(pattern, line)
        if match and match.group(1) in field_names:
            if 'DDS_StringSeq' in line:
                lines[index] = line.replace('(100)', '(1)') + '  /* ROSIDL */'
            else:
                lines[index] = line.replace('(100)', '(0)') + '  /* ROSIDL */'
            modified = True
    return modified


def _step_2_2_init_string(unbounded_fields, lines):
    modified = False
    index = 1
    while index < len(lines):
        previous_line = lines[index - 1]
        if previous_line.strip() == 'if (!RTICdrType_initStringArray(buffer,':
            next_line = lines[index]
            if next_line.strip() == '(100),':
                lines[index] = next_line.replace('(100)', '(1)') + \
                    '  /* ROSIDL */'
                modified = True
        index += 1
    return modified


def _step_2_1_and_2_3_and_2_4(unbounded_fields, lines):
    modified = _step_2_1(unbounded_fields, lines)
    modified |= _step_2_3(unbounded_fields, lines)
    modified |= _step_2_4(unbounded_fields, lines)
    return modified


# change upper bound in serialization to max int
def _step_2_1(unbounded_fields, lines):
    field_names = ['%s_' % f.name for f in unbounded_fields]
    field_pattern = re.compile(
        '.*' +
        re.escape('Seq_get_length(&sample->') +
        '(%s)' % '|'.join(field_names) +
        re.escape('),'))
    modified = False
    index = 1
    while index < len(lines):
        previous_line = lines[index - 1]
        if field_pattern.match(previous_line):
            next_line = lines[index]
            if next_line.strip() == '(100),':
                lines[index] = '%s(%u),  /* ROSIDL */' % \
                    (' ' * 33, 2 ** 32 - 1)
                modified = True
        index += 1
    return modified


# change maximum based on size of incoming sequence
def _step_2_3(unbounded_fields, lines):
    field_names = ['%s_' % f.name for f in unbounded_fields]
    field_pattern = re.compile(
        '.*' +
        re.escape('Seq_get_contiguous_bufferI(&sample->') +
        '(%s)' % '|'.join(field_names) +
        re.escape(')'))
    modified = False
    index = 1
    while index < len(lines):
        previous_line = lines[index - 1]
        next_line = lines[index]
        if 'RTICdrUnsignedLong sequence_length;' in previous_line:
            match = field_pattern.match(next_line)
            if match:
                field_name = match.group(1)
                dds_type = _get_dds_type(unbounded_fields, field_name)
                lines_inserted = [
                    'RTICdrStream_deserializeLong(stream, &sequence_length);',
                    'RTICdrStream_incrementCurrentPosition(stream, -4);',
                    '%sSeq_set_maximum(&sample->%s, sequence_length);' %
                    (dds_type, field_name)
                ]
                if dds_type == 'DDS_String':
                    lines_inserted += [
                        'void* buffer = DDS_StringSeq_get_contiguous_bufferI(&sample->%s);' %
                        field_name,
                        'if (buffer) {',
                        '    if (!RTICdrType_initStringArray(buffer, (sequence_length), ' +
                        '(255)+1, RTI_CDR_CHAR_TYPE)) {',
                        '        goto fin;',
                        '    }',
                        '}',
                    ]
                lines[index:index] = ['%s%s  /* ROSIDL */' % (' ' * 20, l)
                                      for l in lines_inserted]
                modified = True
        index += 1
    return modified


# change maximum back to zero when samples are returned
def _step_2_4(unbounded_fields, lines):
    field_names = ['%s_' % f.name for f in unbounded_fields]
    modified = False
    for index, line in enumerate(lines):
        if '__finalize_optional_members(sample, RTI_TRUE);' in line:
            for field_name in reversed(field_names):
                lines[index:index] = [
                    ' ' * 12 +
                    '%sSeq_set_maximum(&sample->%s, 0);' %
                    (_get_dds_type(unbounded_fields, field_name), field_name) +
                    '  /* ROSIDL */']
                if _get_dds_type(unbounded_fields, field_name) == 'DDS_String':
                    lines[index] = lines[index].replace(', 0)', ', 1)')
                modified = True
            break
    return modified


IDL_TYPE_TO_DDS = {
    'boolean': 'DDS_Boolean',
    'char': 'DDS_Char',
    'octet': 'DDS_Octet',
    'short': 'DDS_Short',
    'unsigned short': 'DDS_UnsignedShort',
    'long': 'DDS_Long',
    'unsigned long': 'DDS_UnsignedLong',
    'long long': 'DDS_LongLong',
    'unsigned long long': 'DDS_UnsignedLongLong',
    'float': 'DDS_Float',
    'double': 'DDS_Double',
    'string': 'DDS_String',
}


def _get_dds_type(fields, field_name):
    field = [f for f in fields if '%s_' % f.name == field_name][0]
    if field.type.is_primitive_type():
        idl_type = MSG_TYPE_TO_IDL[field.type.type]
        return IDL_TYPE_TO_DDS[idl_type]
    else:
        return '%s::msg::dds_::%s_' % (field.type.pkg_name, field.type.type)


def generate_cpp(args, message_specs, service_specs, known_msg_types):
    template_dir = args['template_dir']
    mapping_msgs = {
        os.path.join(template_dir, 'msg__type_support.hpp.template'): '%s__type_support.hpp',
        os.path.join(template_dir, 'msg__type_support.cpp.template'): '%s__type_support.cpp',
    }
    mapping_srvs = {
        os.path.join(template_dir, 'srv__type_support.cpp.template'):
        '%s__type_support.cpp',
    }

    for template_file in mapping_msgs.keys():
        assert os.path.exists(template_file), 'Could not find template: ' + template_file
    for template_file in mapping_srvs.keys():
        assert os.path.exists(template_file), 'Could not find template: ' + template_file

    functions = {
        'get_header_filename_from_msg_name': convert_camel_case_to_lower_case_underscore,
    }
    latest_target_timestamp = get_newest_modification_time(args['target_dependencies'])

    for idl_file, spec in message_specs:
        validate_field_types(spec, known_msg_types)
        subfolder = os.path.basename(os.path.dirname(idl_file))
        for template_file, generated_filename in mapping_msgs.items():
            generated_file = os.path.join(
                args['output_dir'], subfolder, 'dds_connext', generated_filename %
                convert_camel_case_to_lower_case_underscore(spec.base_type.type))

            data = {'spec': spec, 'subfolder': subfolder}
            data.update(functions)
            expand_template(
                template_file, data, generated_file,
                minimum_timestamp=latest_target_timestamp)

    for spec in service_specs:
        validate_field_types(spec, known_msg_types)
        for template_file, generated_filename in mapping_srvs.items():
            generated_file = os.path.join(
                args['output_dir'], 'srv', 'dds_connext', generated_filename %
                convert_camel_case_to_lower_case_underscore(spec.srv_name))

            data = {'spec': spec}
            data.update(functions)
            expand_template(
                template_file, data, generated_file,
                minimum_timestamp=latest_target_timestamp)

    return 0
