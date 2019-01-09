# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: _py_PX4_OpticalFlowMsg_t.proto

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
  name='_py_PX4_OpticalFlowMsg_t.proto',
  package='',
  serialized_pb=_b('\n\x1e_py_PX4_OpticalFlowMsg_t.proto\"\xff\x02\n\x17PX4_OpticalFlowMsg_t_pb\x12\x19\n\x11GyroXRateIntegral\x18\x01 \x02(\x02\x12\x16\n\x0eGroundDistance\x18\x02 \x02(\x02\x12\x19\n\x11GyroZRateIntegral\x18\x03 \x02(\x02\x12 \n\x18TimeSinceLastSonarUpdate\x18\x04 \x02(\r\x12\x11\n\tTimestamp\x18\x05 \x02(\x04\x12\x17\n\x0fGyroTemperature\x18\x06 \x02(\x05\x12\x1a\n\x12PixelFlowYIntegral\x18\x07 \x02(\x02\x12\"\n\x1a\x46rameCountSinceLastReadout\x18\x08 \x02(\r\x12\x11\n\tTlmHeader\x18\t \x03(\r\x12\x1b\n\x13IntegrationTimespan\x18\n \x02(\r\x12\x10\n\x08SensorID\x18\x0b \x02(\r\x12\x0f\n\x07Quality\x18\x0c \x02(\r\x12\x19\n\x11GyroYRateIntegral\x18\r \x02(\x02\x12\x1a\n\x12PixelFlowXIntegral\x18\x0e \x02(\x02')
)
_sym_db.RegisterFileDescriptor(DESCRIPTOR)




_PX4_OPTICALFLOWMSG_T_PB = _descriptor.Descriptor(
  name='PX4_OpticalFlowMsg_t_pb',
  full_name='PX4_OpticalFlowMsg_t_pb',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='GyroXRateIntegral', full_name='PX4_OpticalFlowMsg_t_pb.GyroXRateIntegral', index=0,
      number=1, type=2, cpp_type=6, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='GroundDistance', full_name='PX4_OpticalFlowMsg_t_pb.GroundDistance', index=1,
      number=2, type=2, cpp_type=6, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='GyroZRateIntegral', full_name='PX4_OpticalFlowMsg_t_pb.GyroZRateIntegral', index=2,
      number=3, type=2, cpp_type=6, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='TimeSinceLastSonarUpdate', full_name='PX4_OpticalFlowMsg_t_pb.TimeSinceLastSonarUpdate', index=3,
      number=4, type=13, cpp_type=3, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='Timestamp', full_name='PX4_OpticalFlowMsg_t_pb.Timestamp', index=4,
      number=5, type=4, cpp_type=4, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='GyroTemperature', full_name='PX4_OpticalFlowMsg_t_pb.GyroTemperature', index=5,
      number=6, type=5, cpp_type=1, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='PixelFlowYIntegral', full_name='PX4_OpticalFlowMsg_t_pb.PixelFlowYIntegral', index=6,
      number=7, type=2, cpp_type=6, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='FrameCountSinceLastReadout', full_name='PX4_OpticalFlowMsg_t_pb.FrameCountSinceLastReadout', index=7,
      number=8, type=13, cpp_type=3, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='TlmHeader', full_name='PX4_OpticalFlowMsg_t_pb.TlmHeader', index=8,
      number=9, type=13, cpp_type=3, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='IntegrationTimespan', full_name='PX4_OpticalFlowMsg_t_pb.IntegrationTimespan', index=9,
      number=10, type=13, cpp_type=3, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='SensorID', full_name='PX4_OpticalFlowMsg_t_pb.SensorID', index=10,
      number=11, type=13, cpp_type=3, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='Quality', full_name='PX4_OpticalFlowMsg_t_pb.Quality', index=11,
      number=12, type=13, cpp_type=3, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='GyroYRateIntegral', full_name='PX4_OpticalFlowMsg_t_pb.GyroYRateIntegral', index=12,
      number=13, type=2, cpp_type=6, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='PixelFlowXIntegral', full_name='PX4_OpticalFlowMsg_t_pb.PixelFlowXIntegral', index=13,
      number=14, type=2, cpp_type=6, label=2,
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
  serialized_start=35,
  serialized_end=418,
)

DESCRIPTOR.message_types_by_name['PX4_OpticalFlowMsg_t_pb'] = _PX4_OPTICALFLOWMSG_T_PB

PX4_OpticalFlowMsg_t_pb = _reflection.GeneratedProtocolMessageType('PX4_OpticalFlowMsg_t_pb', (_message.Message,), dict(
  DESCRIPTOR = _PX4_OPTICALFLOWMSG_T_PB,
  __module__ = '_py_PX4_OpticalFlowMsg_t_pb2'
  # @@protoc_insertion_point(class_scope:PX4_OpticalFlowMsg_t_pb)
  ))
_sym_db.RegisterMessage(PX4_OpticalFlowMsg_t_pb)


# @@protoc_insertion_point(module_scope)