# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: _py_es_overwrite_sys_log.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='_py_es_overwrite_sys_log.proto',
  package='',
  serialized_pb=_b('\n\x1e_py_es_overwrite_sys_log.proto\"/\n\x1f\x65s_overwrite_sys_log_payload_pb\x12\x0c\n\x04Mode\x18\x01 \x01(\r\"L\n\x17\x65s_overwrite_sys_log_pb\x12\x31\n\x07Payload\x18\x01 \x02(\x0b\x32 .es_overwrite_sys_log_payload_pb')
)
_sym_db.RegisterFileDescriptor(DESCRIPTOR)




_ES_OVERWRITE_SYS_LOG_PAYLOAD_PB = _descriptor.Descriptor(
  name='es_overwrite_sys_log_payload_pb',
  full_name='es_overwrite_sys_log_payload_pb',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='Mode', full_name='es_overwrite_sys_log_payload_pb.Mode', index=0,
      number=1, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=34,
  serialized_end=81,
)


_ES_OVERWRITE_SYS_LOG_PB = _descriptor.Descriptor(
  name='es_overwrite_sys_log_pb',
  full_name='es_overwrite_sys_log_pb',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='Payload', full_name='es_overwrite_sys_log_pb.Payload', index=0,
      number=1, type=11, cpp_type=10, label=2,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=83,
  serialized_end=159,
)

_ES_OVERWRITE_SYS_LOG_PB.fields_by_name['Payload'].message_type = _ES_OVERWRITE_SYS_LOG_PAYLOAD_PB
DESCRIPTOR.message_types_by_name['es_overwrite_sys_log_payload_pb'] = _ES_OVERWRITE_SYS_LOG_PAYLOAD_PB
DESCRIPTOR.message_types_by_name['es_overwrite_sys_log_pb'] = _ES_OVERWRITE_SYS_LOG_PB

es_overwrite_sys_log_payload_pb = _reflection.GeneratedProtocolMessageType('es_overwrite_sys_log_payload_pb', (_message.Message,), dict(
  DESCRIPTOR = _ES_OVERWRITE_SYS_LOG_PAYLOAD_PB,
  __module__ = '_py_es_overwrite_sys_log_pb2'
  # @@protoc_insertion_point(class_scope:es_overwrite_sys_log_payload_pb)
  ))
_sym_db.RegisterMessage(es_overwrite_sys_log_payload_pb)

es_overwrite_sys_log_pb = _reflection.GeneratedProtocolMessageType('es_overwrite_sys_log_pb', (_message.Message,), dict(
  DESCRIPTOR = _ES_OVERWRITE_SYS_LOG_PB,
  __module__ = '_py_es_overwrite_sys_log_pb2'
  # @@protoc_insertion_point(class_scope:es_overwrite_sys_log_pb)
  ))
_sym_db.RegisterMessage(es_overwrite_sys_log_pb)


# @@protoc_insertion_point(module_scope)
