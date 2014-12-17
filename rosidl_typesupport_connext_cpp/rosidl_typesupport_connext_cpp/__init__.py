import em
import os
import re
import subprocess

from rosidl_generator_dds_idl import MSG_TYPE_TO_IDL
from rosidl_parser import parse_message_file, parse_service_file


def parse_ros_interface_files(pkg_name, ros_interface_files):
    message_specs = []
    service_specs = []
    for idl_file in ros_interface_files:
        print(pkg_name, idl_file)
        filename, extension = os.path.splitext(idl_file)
        if extension == '.msg':
            message_spec = parse_message_file(pkg_name, idl_file)
            message_specs.append(message_spec)
        elif extension == '.srv':
            service_spec = parse_service_file(pkg_name, idl_file)
            service_specs.append(service_spec)
    return (message_specs, service_specs)


def generate_dds_connext_cpp(
        pkg_name, dds_interface_files, dds_interface_base_path, deps,
        output_dir, idl_pp, message_specs, service_specs):
    try:
        os.makedirs(output_dir)
    except FileExistsError:
        pass

    include_dirs = [dds_interface_base_path]
    for dep in deps:
        dep_parts = dep.split(':')
        assert(len(dep_parts) == 2)
        idl_path = dep_parts[1]
        idl_base_path = os.path.dirname(
            os.path.dirname(os.path.dirname(os.path.normpath(idl_path))))
        if idl_base_path not in include_dirs:
            include_dirs.append(idl_base_path)

    for index, idl_file in enumerate(dds_interface_files):
        msg_name = os.path.splitext(os.path.basename(idl_file))[0]
        generated_file = os.path.join(output_dir, msg_name + '.h/cpp')
        print('Generating: %s' % generated_file)

        cmd = [idl_pp]
        for include_dir in include_dirs:
            cmd += ['-I', include_dir]
        cmd += [
            '-d', output_dir,
            '-language', 'C++',
            '-namespace',
            '-replace',
            # TODO use -unboundedSupport when it becomes available
            idl_file
        ]
        subprocess.check_call(cmd)

        # modify generated code to support unbounded sequences and strings
        # http://community.rti.com/content/forum-topic/changing-max-size-sequence
        unbounded_fields = [f for f in message_specs[index].fields
                            if f.type.is_array and
                            f.type.array_size is None and
                            not f.type.is_upper_bound]
        if unbounded_fields:
            msg_cxx_filename = os.path.join(output_dir, msg_name + '.cxx')
            _modify(msg_cxx_filename, unbounded_fields, _step_2_2)
            plugin_cxx_filename = os.path.join(output_dir, msg_name + 'Plugin.cxx')
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
                        'void* buffer = DDS_StringSeq_get_contiguous_bufferI(&sample->%s);' % field_name,
                        'if (buffer) {',
                        '    if (!RTICdrType_initStringArray(buffer, (sequence_length), (255)+1, RTI_CDR_CHAR_TYPE)) {',
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
    idl_type = MSG_TYPE_TO_IDL[field.type.type]
    return IDL_TYPE_TO_DDS[idl_type]


def generate_cpp(pkg_name, message_specs, service_specs, output_dir, template_dir):
    mapping_msgs = {
        os.path.join(template_dir, 'msg_TypeSupport.h.template'): '%s_TypeSupport.h',
        os.path.join(template_dir, 'msg_TypeSupport.cpp.template'): '%s_TypeSupport.cpp',
    }

    mapping_srvs = {
        os.path.join(template_dir, 'srv_ServiceTypeSupport.cpp.template'): '%s_ServiceTypeSupport.cpp',
    }

    for template_file in mapping_msgs.keys():
        assert(os.path.exists(template_file))

    for template_file in mapping_srvs.keys():
        assert(os.path.exists(template_file))

    try:
        os.makedirs(output_dir)
    except FileExistsError:
        pass

    for spec in message_specs:
        for template_file, generated_filename in mapping_msgs.items():
            generated_file = os.path.join(output_dir, generated_filename % spec.base_type.type)
            print('Generating MESSAGE: %s' % generated_file)

            try:
                # TODO only touch generated file if its content actually changes
                ofile = open(generated_file, 'w')
                # TODO reuse interpreter
                interpreter = em.Interpreter(
                    output=ofile,
                    options={
                        em.RAW_OPT: True,
                        em.BUFFERED_OPT: True,
                    },
                    globals={'spec': spec},
                )
                interpreter.file(open(template_file))
                interpreter.shutdown()
            except Exception:
                os.remove(generated_file)
                raise

    for spec in service_specs:
        for template_file, generated_filename in mapping_srvs.items():
            generated_file = os.path.join(output_dir, generated_filename % spec.srv_name)
            print('Generating SERVICE: %s' % generated_file)

            try:
                # TODO only touch generated file if its content actually changes
                ofile = open(generated_file, 'w')
                # TODO reuse interpreter
                interpreter = em.Interpreter(
                    output=ofile,
                    options={
                        em.RAW_OPT: True,
                        em.BUFFERED_OPT: True,
                    },
                    globals={'spec': spec},
                )
                interpreter.file(open(template_file))
                interpreter.shutdown()
            except Exception:
                os.remove(generated_file)
                raise

    return 0
