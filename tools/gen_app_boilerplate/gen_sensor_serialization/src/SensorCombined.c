uint32 AE_SensorCombined_Enc(const PX4_SensorCombinedMsg_t *inObject, char *inOutBuffer, uint32 inSize)
{
	bool status = false;
	px4_sensor_combined_pb pbMsg;
	
	pbMsg.gyro_rad[2] = inObject->GyroRad[2];
	pbMsg.baro_alt_meter = inObject->BaroAlt;
	pbMsg.accelerometer_m_s2[2] = inObject->Acc[2];
	pbMsg.magnetometer_ga[2] = inObject->Mag[2];
	pbMsg.gyro_rad[1] = inObject->GyroRad[1];
	pbMsg.gyro_rad[0] = inObject->GyroRad[0];
	pbMsg.accelerometer_m_s2[0] = inObject->Acc[0];
	pbMsg.accelerometer_m_s2[1] = inObject->Acc[1];
	pbMsg.magnetometer_ga[1] = inObject->Mag[1];
	pbMsg.gyro_integral_dt = inObject->GyroIntegralDt;
	pbMsg.accelerometer_integral_dt = inObject->AccIntegralDt;
	pbMsg.magnetometer_ga[0] = inObject->Mag[0];
	pbMsg.baro_temp_celcius = inObject->BaroTemp;

	/* Create a stream that will write to our buffer. */
	pb_ostream_t stream = pb_ostream_from_buffer((pb_byte_t *)inOutBuffer, inSize);

	/* Now we are ready to encode the message. */
	status = pb_encode(&stream, px4_sensor_combined_pb_fields, &pbMsg); // TODO: Assumes [type]_fields is a standard
	/* Check for errors... */
	if (!status)
	{
		return 0;
	}

	return stream.bytes_written;
}

uint32 AE_SensorCombined_Dec(const char *inBuffer, uint32 inSize, PX4_SensorCombinedMsg_t *inOutObject)
{
	bool status = false;
	px4_sensor_combined_pb pbMsg;

	/* Create a stream that reads from the buffer. */
	pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)inBuffer, inSize);

	/* Now we are ready to decode the message. */
	status = pb_decode(&stream, px4_sensor_combined_pb_fields, &pbMsg); // TODO: Assumes [type]_fields is a standard

	/* Check for errors... */
	if (!status)
	{
		return 0;
	}
	
	inOutObject->GyroRad[2] = pbMsg.gyro_rad[2];
	inOutObject->BaroAlt = pbMsg.baro_alt_meter;
	inOutObject->Acc[2] = pbMsg.accelerometer_m_s2[2];
	inOutObject->Mag[2] = pbMsg.magnetometer_ga[2];
	inOutObject->GyroRad[1] = pbMsg.gyro_rad[1];
	inOutObject->GyroRad[0] = pbMsg.gyro_rad[0];
	inOutObject->Acc[0] = pbMsg.accelerometer_m_s2[0];
	inOutObject->Acc[1] = pbMsg.accelerometer_m_s2[1];
	inOutObject->Mag[1] = pbMsg.magnetometer_ga[1];
	inOutObject->GyroIntegralDt = pbMsg.gyro_integral_dt;
	inOutObject->AccIntegralDt = pbMsg.accelerometer_integral_dt;
	inOutObject->Mag[0] = pbMsg.magnetometer_ga[0];
	inOutObject->BaroTemp = pbMsg.baro_temp_celcius;

	return sizeof(PX4_SensorCombinedMsg_t);
}
