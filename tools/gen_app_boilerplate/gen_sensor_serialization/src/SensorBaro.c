uint32 AE_SensorBaro_Enc(const PX4_SensorBaroMsg_t *inObject, char *inOutBuffer, uint32 inSize)
{
	bool status = false;
	px4_sensor_baro_pb pbMsg;
	
	pbMsg.pressure = inObject->Pressure;
	pbMsg.altitude = inObject->Altitude;
	pbMsg.error_count = inObject->ErrorCount;
	pbMsg.temperature = inObject->Temperature;

	/* Create a stream that will write to our buffer. */
	pb_ostream_t stream = pb_ostream_from_buffer((pb_byte_t *)inOutBuffer, inSize);

	/* Now we are ready to encode the message. */
	status = pb_encode(&stream, px4_sensor_baro_pb_fields, &pbMsg); // TODO: Assumes [type]_fields is a standard
	/* Check for errors... */
	if (!status)
	{
		return 0;
	}

	return stream.bytes_written;
}

uint32 AE_SensorBaro_Dec(const char *inBuffer, uint32 inSize, PX4_SensorBaroMsg_t *inOutObject)
{
	bool status = false;
	px4_sensor_baro_pb pbMsg;

	/* Create a stream that reads from the buffer. */
	pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)inBuffer, inSize);

	/* Now we are ready to decode the message. */
	status = pb_decode(&stream, px4_sensor_baro_pb_fields, &pbMsg); // TODO: Assumes [type]_fields is a standard

	/* Check for errors... */
	if (!status)
	{
		return 0;
	}
	
	inOutObject->Pressure = pbMsg.pressure;
	inOutObject->Altitude = pbMsg.altitude;
	inOutObject->ErrorCount = pbMsg.error_count;
	inOutObject->Temperature = pbMsg.temperature;

	return sizeof(PX4_SensorBaroMsg_t);
}
