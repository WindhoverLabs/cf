# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: _py_es_app_reload.proto

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
  name='_py_es_app_reload.proto',
  package='',
  serialized_pb=_b('\n\x17_py_es_app_reload.proto\"D\n\x18\x65s_app_reload_payload_pb\x12\x13\n\x0b\x41pplication\x18\x01 \x01(\t\x12\x13\n\x0b\x41ppFileName\x18\x02 \x01(\t\">\n\x10\x65s_app_reload_pb\x12*\n\x07Payload\x18\x01 \x02(\x0b\x32\x19.es_app_reload_payload_pb')
)
_sym_db.RegisterFileDescriptor(DESCRIPTOR)




_ES_APP_RELOAD_PAYLOAD_PB = _descriptor.Descriptor(
  name='es_app_reload_payload_pb',
  full_name='es_app_reload_payload_pb',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='Application', full_name='es_app_reload_payload_pb.Application', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='AppFileName', full_name='es_app_reload_payload_pb.AppFileName', index=1,
      number=2, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
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
  serialized_start=27,
  serialized_end=95,
)


_ES_APP_RELOAD_PB = _descriptor.Descriptor(
  name='es_app_reload_pb',
  full_name='es_app_reload_pb',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='Payload', full_name='es_app_reload_pb.Payload', index=0,
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
  serialized_start=97,
  serialized_end=159,
)

_ES_APP_RELOAD_PB.fields_by_name['Payload'].message_type = _ES_APP_RELOAD_PAYLOAD_PB
DESCRIPTOR.message_types_by_name['es_app_reload_payload_pb'] = _ES_APP_RELOAD_PAYLOAD_PB
DESCRIPTOR.message_types_by_name['es_app_reload_pb'] = _ES_APP_RELOAD_PB

es_app_reload_payload_pb = _reflection.GeneratedProtocolMessageType('es_app_reload_payload_pb', (_message.Message,), dict(
  DESCRIPTOR = _ES_APP_RELOAD_PAYLOAD_PB,
  __module__ = '_py_es_app_reload_pb2'
  # @@protoc_insertion_point(class_scope:es_app_reload_payload_pb)
  ))
_sym_db.RegisterMessage(es_app_reload_payload_pb)

es_app_reload_pb = _reflection.GeneratedProtocolMessageType('es_app_reload_pb', (_message.Message,), dict(
  DESCRIPTOR = _ES_APP_RELOAD_PB,
  __module__ = '_py_es_app_reload_pb2'
  # @@protoc_insertion_point(class_scope:es_app_reload_pb)
  ))
_sym_db.RegisterMessage(es_app_reload_pb)


# @@protoc_insertion_point(module_scope)
