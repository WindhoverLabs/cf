/* This file was autogenerated from template version 0.0.0.2 and autogenerator version {{cookiecutter.autogen_version}} */

#include "{{cookiecutter.app_name|lower}}_serialization.h"

#ifdef __cplusplus
extern "C" {
#endif
{# Iterate over every proto msg in json file  -#}
{% for airliner_msg, proto_data in cookiecutter.proto_msgs.iteritems() %}
uint32 {{airliner_msg}}_Enc(const {{airliner_msg}} *inObject, char *inOutBuffer, uint32 inSize)
{
	bool status = false;
	pb_ostream_t stream;

	{{ proto_data.proto_msg }} pbMsg;
{# Assign top level fields for message -#}
{%- for pb_var, var_data in proto_data.fields.iteritems() -%}
    {%- if var_data.pb_type[-3:] == "_pb" -%}
        {%- continue -%}
    {%- endif %}
    {% if var_data.pb_field_rule == "repeated" -%}
        /* Unroll indexes */
    pbMsg.{{pb_var}}_count = {{var_data.array_length}};
        {%- for i in range(0, var_data.array_length | int) -%}
            {%- if var_data.airliner_type == "char" %}
    strcpy(pbMsg.{{pb_var}}[{{loop.index0}}], inObject->{{var_data.airliner_name}}[{{loop.index0}}]);
            {%- else %}
    pbMsg.{{pb_var}}[{{loop.index0}}] = inObject->{{var_data.airliner_name}}[{{loop.index0}}];
            {%- endif -%}
        {%- endfor -%}
    {%- else -%}
        {%- if var_data.airliner_type == "char" -%}
        	strcpy(pbMsg.{{pb_var}}, inObject->{{var_data.airliner_name}});
        {%- else -%}
        	pbMsg.{{pb_var}} = inObject->{{var_data.airliner_name}};
        {%- endif -%}
    {%- endif -%}
{%- endfor -%}
{#- Iterate over required proto msgs for this proto msg #}
{%- set parent_path = ["pbMsg"] -%}
{% for req_pb_msg, req_pb_data in proto_data.required_pb_msgs.iteritems() recursive -%}
    {%- set outer_loop = loop -%}
    {%- for parent, temp in req_pb_data.parent_field.iteritems() -%}
        {%- do parent_path.append(parent) -%}
        {%- for pb_var, var_data in req_pb_data.fields.iteritems() -%}           
            {%- if var_data.pb_type[-3:] == "_pb" -%}
                {%- continue -%}
            {%- endif %}
            {%- set pb_path = [""] -%}
            {%- set air_path = [] -%}
            {%- for par in parent_path -%}
                {%- do pb_path.append(pb_path.pop() + par + ".") -%}
                {%- if loop.index0 == 0 %}
                    {%- do air_path.append("inObject->") -%}
                {%- else -%}
                    {%- do air_path.append(air_path.pop() + par + ".") -%}
                {%- endif -%}
            {%- endfor %}
    {# align #} {%- if var_data.pb_field_rule == "repeated" -%}
                /* Unroll indexes */
    {{pb_path[0]}}{{pb_var}}_count = {{var_data.array_length}};
                {%- for i in range(0, var_data.array_length | int) -%}
                    {%- if var_data.airliner_type == "char" %}
    strcpy({{pb_path[0]}}{{pb_var}}[{{loop.index0}}], {{air_path[0]}}{{var_data.airliner_name}}[{{loop.index0}}]);
                    {%- else %}
    {{pb_path[0]}}{{pb_var}}[{{loop.index0}}] = {{air_path[0]}}{{var_data.airliner_name}}[{{loop.index0}}];
                    {%- endif -%}
                {%- endfor -%}
            {%- else -%}
                {%- if var_data.airliner_type == "char" -%}
    	            strcpy({{pb_path[0]}}{{pb_var}}, {{air_path[0]}}{{var_data.airliner_name}});
                {%- else -%}
                    {{pb_path[0]}}{{pb_var}} = {{air_path[0]}}{{var_data.airliner_name}};
                {%- endif -%}
            {%- endif -%}
        {% endfor -%}
        {%- if req_pb_data.required_pb_msgs|length > 0 -%}
            {{ outer_loop(req_pb_data.required_pb_msgs.items())}}
        {%- endif -%}
        {%- do parent_path.pop() -%}
    {%- endfor -%}
{%- endfor %}

	/* Create a stream that will write to our buffer. */
	stream = pb_ostream_from_buffer((pb_byte_t *)inOutBuffer, inSize);
	
	/* Now we are ready to encode the message. */
	status = pb_encode(&stream, {{proto_data.proto_msg}}_fields, &pbMsg);

	/* Check for errors... */
	if (!status)
	{
        OS_printf("Error encoding msg: %s", PB_GET_ERROR(&stream));
		return 0;
	}

	return stream.bytes_written;
}

uint32 {{airliner_msg}}_Dec(const char *inBuffer, uint32 inSize, {{airliner_msg}} *inOutObject)
{
	bool status = false;
	pb_istream_t stream;

	{{ proto_data.proto_msg }} pbMsg;

    /* Create a stream that reads from the buffer. */
	stream = pb_istream_from_buffer((const pb_byte_t *)inBuffer, inSize);

	/* Now we are ready to decode the message. */
	status = pb_decode(&stream, {{proto_data.proto_msg}}_fields, &pbMsg); 

	/* Check for errors... */
	if (!status)
	{
        OS_printf("Error decoding msg: %s", PB_GET_ERROR(&stream));
		return 0;
	}
{# Assign top level fields for message -#}
{%- for pb_var, var_data in proto_data.fields.iteritems() -%}
    {%- if var_data.pb_type[-3:] == "_pb" -%}
        {%- continue -%}
    {%- endif %}
    {% if var_data.pb_field_rule == "repeated" -%}
        /* Unroll indexes */
        {%- for i in range(0, var_data.array_length | int) -%}
            {%- if var_data.airliner_type == "char" %}
    strcpy(inOutObject->{{pb_var}}[{{loop.index0}}], pbMsg.{{var_data.airliner_name}}[{{loop.index0}}]);
            {%- else %}
    inOutObject->{{pb_var}}[{{loop.index0}}] = pbMsg.{{var_data.airliner_name}}[{{loop.index0}}];
            {%- endif -%}
        {%- endfor -%}
    {%- else -%}
        {%- if var_data.airliner_type == "char" -%}
        	strcpy(inOutObject->{{pb_var}}, pbMsg.{{var_data.airliner_name}});
        {%- else -%}
        	inOutObject->{{pb_var}} = pbMsg.{{var_data.airliner_name}};
        {%- endif -%}
    {%- endif -%}
{%- endfor -%}
{#- Iterate over required proto msgs for this proto msg #}
{%- set parent_path = ["pbMsg"] -%}
{% for req_pb_msg, req_pb_data in proto_data.required_pb_msgs.iteritems() recursive -%}
    {%- set outer_loop = loop -%}
    {%- for parent, temp in req_pb_data.parent_field.iteritems() -%}
        {%- do parent_path.append(parent) -%}
        {%- for pb_var, var_data in req_pb_data.fields.iteritems() -%}           
            {%- if var_data.pb_type[-3:] == "_pb" -%}
                {%- continue -%}
            {%- endif %}
            {%- set pb_path = [""] -%}
            {%- set air_path = [] -%}
            {%- for par in parent_path -%}
                {%- do pb_path.append(pb_path.pop() + par + ".") -%}
                {%- if loop.index0 == 0 %}
                    {%- do air_path.append("inOutObject->") -%}
                {%- else -%}
                    {%- do air_path.append(air_path.pop() + par + ".") -%}
                {%- endif -%}
            {%- endfor %}
    {# align #} {%- if var_data.pb_field_rule == "repeated" -%}
                /* Unroll indexes */
                {%- for i in range(0, var_data.array_length | int) -%}
                    {%- if var_data.airliner_type == "char" %}
    strcpy({{air_path[0]}}{{pb_var}}[{{loop.index0}}], {{pb_path[0]}}{{var_data.airliner_name}}[{{loop.index0}}]);
                    {%- else %}
    {{air_path[0]}}{{pb_var}}[{{loop.index0}}] = {{pb_path[0]}}{{var_data.airliner_name}}[{{loop.index0}}];
                    {%- endif -%}
                {%- endfor -%}
            {%- else -%}
                {%- if var_data.airliner_type == "char" -%}
    	            strcpy({{air_path[0]}}{{pb_var}}, {{pb_path[0]}}{{var_data.airliner_name}});
                {%- else -%}
                    {{air_path[0]}}{{pb_var}} = {{pb_path[0]}}{{var_data.airliner_name}};
                {%- endif -%}
            {%- endif -%}
        {% endfor -%}
        {%- if req_pb_data.required_pb_msgs|length > 0 -%}
            {{ outer_loop(req_pb_data.required_pb_msgs.items())}}
        {%- endif -%}
        {%- do parent_path.pop() -%}
    {%- endfor -%}
{%- endfor %}

	return sizeof({{airliner_msg}});
}

{% endfor %}
#ifdef __cplusplus
} /* extern "C" */
#endif
