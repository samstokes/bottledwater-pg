#include <stdio.h>
#include <string.h>

#include "connect.h"


#define APP_NAME "bottledwater"
#define DEFAULT_REPLICATION_SLOT "bottledwater"
#define OUTPUT_PLUGIN "bottledwater"


client_context_t new_client(const char *conninfo);
int start(client_context_t client);
void destroy(client_context_t client);

char* get_client_error(client_context_t client);
int client_poll(client_context_t client);
int client_wait(client_context_t client);
void client_fsync(client_context_t client, uint64_t fsync_lsn);

void client_on_insert_row(client_context_t client, insert_row_cb callback);
void client_on_table_schema(client_context_t client, table_schema_cb callback);

static client_context_t init_client(const char *conninfo);


client_context_t new_client(const char *conninfo) {
    client_context_t client = init_client(conninfo);
    if (client == NULL) return NULL;
    return client;
}

void client_on_insert_row(client_context_t client, insert_row_cb callback) {
    client->repl.frame_reader->on_insert_row = callback;
}

void client_on_table_schema(client_context_t client, table_schema_cb callback) {
    client->repl.frame_reader->on_table_schema = callback;
}

int start(client_context_t client) {
    int err = db_client_start(client);
    if (err) return err;

    replication_stream_t stream = &client->repl;

    if (!client->taking_snapshot) {
        fprintf(stderr, "Replication slot \"%s\" exists, streaming changes from %X/%X.\n",
                stream->slot_name,
                (uint32) (stream->start_lsn >> 32), (uint32) stream->start_lsn);
    }

    return 0;
}

char* get_client_error(client_context_t client) {
    return strdup(client->error);
}

int client_poll(client_context_t client) {
    return db_client_poll(client);
}

int client_wait(client_context_t client) {
    return db_client_wait(client);
}

void client_fsync(client_context_t client, uint64_t fsync_lsn) {
    client->repl.fsync_lsn = fsync_lsn;
}

static client_context_t init_client(const char *conninfo) {
    frame_reader_t frame_reader = frame_reader_new();

    client_context_t client = db_client_new();
    client->conninfo = strdup(conninfo);
    client->app_name = APP_NAME;
    client->allow_unkeyed = true;
    client->repl.slot_name = DEFAULT_REPLICATION_SLOT;
    client->repl.output_plugin = OUTPUT_PLUGIN;
    client->repl.frame_reader = frame_reader;
    return client;
}


void destroy(client_context_t client) {
    fprintf(stderr, "leaving\n");

    // If a snapshot was in progress and not yet complete, and an error occurred, try to
    // drop the replication slot, so that the snapshot is retried when the user tries again.
    if (client->taking_snapshot) {
        fprintf(stderr, "Dropping replication slot since the snapshot did not complete successfully.\n");
        if (replication_slot_drop(&client->repl) != 0) {
            fprintf(stderr, "%s\n", client->repl.error);
        }
    }

    frame_reader_free(client->repl.frame_reader);
    db_client_free(client);
}
