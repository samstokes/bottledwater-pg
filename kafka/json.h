#ifndef JSON_H
#define JSON_H


#include "table_mapper.h"

#include <avro.h>


int json_encode_msg(
        const avro_value_t *key, char **key_out, size_t *key_len_out,
        const avro_value_t *row, char **row_out, size_t *row_len_out);


#endif /* JSON_H */
