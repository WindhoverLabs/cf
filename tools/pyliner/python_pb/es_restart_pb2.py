# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: _py_es_restart.proto

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
  name='_py_es_restart.proto',
  package='',
  serialized_pb=_b('\n\x14_py_es_restart.proto\",\n\x15\x65s_restart_payload_pb\x12\x13\n\x0bRestartType\x18\x01 \x01(\r\"8\n\res_restart_pb\x12\'\n\x07Payload\x18\x01 \x02(\x0b\x32\x16.es_restart_payload_pb')
)
_sym_db.RegisterFileDescriptor(DESCRIPTOR)




_ES_RESTART_PAYLOAD_PB = _descriptor.Descriptor(
  name='es_restart_payload_pb',
  full_name='es_restart_payload_pb',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='RestartType', full_name='es_restart_payload_pb.RestartType', index=0,
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
  serialized_start=24,
  serialized_end=68,
)


_ES_RESTART_PB = _descriptor.Descriptor(
  name='es_restart_pb',
  full_name='es_restart_pb',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='Payload', full_name='es_restart_pb.Payload', index=0,
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
  serialized_start=70,
  serialized_end=126,
)

_ES_RESTART_PB.fields_by_name['Payload'].message_type = _ES_RESTART_PAYLOAD_PB
DESCRIPTOR.message_types_by_name['es_restart_payload_pb'] = _ES_RESTART_PAYLOAD_PB
DESCRIPTOR.message_types_by_name['es_restart_pb'] = _ES_RESTART_PB

es_restart_payload_pb = _reflection.GeneratedProtocolMessageType('es_restart_payload_pb', (_message.Message,), dict(
  DESCRIPTOR = _ES_RESTART_PAYLOAD_PB,
  __module__ = '_py_es_restart_pb2'
  # @@protoc_insertion_point(class_scope:es_restart_payload_pb)
  ))
_sym_db.RegisterMessage(es_restart_payload_pb)

es_restart_pb = _reflection.GeneratedProtocolMessageType('es_restart_pb', (_message.Message,), dict(
  DESCRIPTOR = _ES_RESTART_PB,
  __module__ = '_py_es_restart_pb2'
  # @@protoc_insertion_point(class_scope:es_restart_pb)
  ))
_sym_db.RegisterMessage(es_restart_pb)


# @@protoc_insertion_point(module_scope)
