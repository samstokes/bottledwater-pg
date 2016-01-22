/* JSON encoding for messages written to Kafka.
 *
 * The JSON format is defined by libavro's avro_value_to_json function, which
 * produces JSON as defined in the Avro spec:
 * https://avro.apache.org/docs/1.7.7/spec.html#json_encoding
 *
 * Examples:
 *
 *  * {"id": {"int": 1}} // an integer key
 *  * {"id": {"int": 3}, "title": {"string": "Man Bites Dog"}} // a row with two fields
 *
 * N.B. the output JSON does contain whitespace (as in the above examples), and
 * so may be rejected by strict JSON parsers.
 */

#include "json.h"

#include <stdio.h>

int avro_to_json(const avro_value_t *val,
        char **val_out, size_t *val_len_out);


int json_encode_msg(
        const avro_value_t *key, char **key_out, size_t *key_len_out,
        const avro_value_t *row, char **row_out, size_t *row_len_out) {
    int err;
    err = avro_to_json(key, key_out, key_len_out);
    if (err) {
      fprintf(stderr, "json: error encoding key: %s\n", avro_strerror());
      return err;
    }
    err = avro_to_json(row, row_out, row_len_out);
    if (err) {
      fprintf(stderr, "json: error encoding row: %s\n", avro_strerror());
      return err;
    }

    return 0;
}


int avro_to_json(const avro_value_t *val,
        char **val_out, size_t *val_len_out) {
    if (!val) {
        *val_out = NULL;
        return 0;
    }

    int err = avro_value_to_json(val, 1, val_out);
    if (err) {
        return err;
    }

    *val_len_out = strlen(*val_out); // not including null terminator - to librdkafka it's just bytes

    return 0;
}
