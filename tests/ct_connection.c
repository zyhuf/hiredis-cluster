#include "adapters/libevent.h"
#include "hircluster.h"
#include "test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLUSTER_NODE_WITH_PASSWORD "127.0.0.1:7100"
#define CLUSTER_PASSWORD "secretword"

// Connecting to a password protected cluster and
// providing a correct password.
void test_password_ok() {
    redisClusterContext *cc = redisClusterContextInit();
    assert(cc);
    redisClusterSetOptionAddNodes(cc, CLUSTER_NODE_WITH_PASSWORD);
    redisClusterSetOptionPassword(cc, CLUSTER_PASSWORD);

    int status;
    status = redisClusterConnect2(cc);
    ASSERT_MSG(status == REDIS_OK, cc->errstr);

    // Test connection
    redisReply *reply;
    reply = (redisReply *)redisClusterCommand(cc, "SET key1 Hello");
    CHECK_REPLY_OK(cc, reply);
    freeReplyObject(reply);

    redisClusterFree(cc);
}

// Connecting to a password protected cluster and
// providing wrong password.
void test_password_wrong() {
    redisClusterContext *cc = redisClusterContextInit();
    assert(cc);
    redisClusterSetOptionAddNodes(cc, CLUSTER_NODE_WITH_PASSWORD);
    redisClusterSetOptionPassword(cc, "faultypass");

    int status;
    status = redisClusterConnect2(cc);
    assert(status == REDIS_ERR);

    assert(cc->err == REDIS_ERR_OTHER);
    assert(strncmp(cc->errstr, "WRONGPASS", 9) == 0);

    redisClusterFree(cc);
}

// Connecting to a password protected cluster and
// not providing any password.
void test_password_missing() {
    redisClusterContext *cc = redisClusterContextInit();
    assert(cc);
    redisClusterSetOptionAddNodes(cc, CLUSTER_NODE_WITH_PASSWORD);

    // A password is not configured..
    int status;
    status = redisClusterConnect2(cc);
    assert(status == REDIS_ERR);

    assert(cc->err == REDIS_ERR_OTHER);
    assert(strncmp(cc->errstr, "NOAUTH", 6) == 0);

    redisClusterFree(cc);
}

//------------------------------------------------------------------------------
// Async API
//------------------------------------------------------------------------------

void callbackExpectOk(const redisAsyncContext *ac, int status) {
    UNUSED(ac);
    assert(status == REDIS_OK);
}

void commandCallback(redisClusterAsyncContext *cc, void *r, void *privdata) {
    UNUSED(r);
    UNUSED(privdata);
    redisReply *reply = (redisReply *)r;
    assert(reply != NULL);
    assert(strcmp(reply->str, "OK") == 0);
    redisClusterAsyncDisconnect(cc);
}

// Connecting to a password protected cluster using
// the async API, providing correct password.
void test_async_password_ok() {
    redisClusterAsyncContext *acc = redisClusterAsyncContextInit();
    assert(acc);
    redisClusterAsyncSetConnectCallback(acc, callbackExpectOk);
    redisClusterAsyncSetDisconnectCallback(acc, callbackExpectOk);
    redisClusterSetOptionAddNodes(acc->cc, CLUSTER_NODE_WITH_PASSWORD);
    redisClusterSetOptionPassword(acc->cc, CLUSTER_PASSWORD);

    struct event_base *base = event_base_new();
    redisClusterLibeventAttach(acc, base);

    int status;
    status = redisClusterConnect2(acc->cc);
    assert(status == REDIS_OK);
    assert(acc->err == 0);
    assert(acc->cc->err == 0);

    // Test connection
    status = redisClusterAsyncCommand(acc, commandCallback, (char *)"THE_ID",
                                      "SET key1 Hello");
    assert(status == REDIS_OK);

    event_base_dispatch(base);

    redisClusterAsyncFree(acc);
    event_base_free(base);
}

// Connecting to a password protected cluster using
// the async API, providing wrong password.
void test_async_password_wrong() {
    redisClusterAsyncContext *acc = redisClusterAsyncContextInit();
    assert(acc);
    redisClusterAsyncSetConnectCallback(acc, callbackExpectOk);
    redisClusterAsyncSetDisconnectCallback(acc, callbackExpectOk);
    redisClusterSetOptionAddNodes(acc->cc, CLUSTER_NODE_WITH_PASSWORD);
    redisClusterSetOptionPassword(acc->cc, "faultypass");

    struct event_base *base = event_base_new();
    redisClusterLibeventAttach(acc, base);

    int status;
    status = redisClusterConnect2(acc->cc);
    assert(status == REDIS_ERR);
    assert(acc->err == REDIS_OK); // TODO: This must be wrong!
    assert(acc->cc->err == REDIS_ERR_OTHER);
    assert(strncmp(acc->cc->errstr, "WRONGPASS", 6) == 0);

    // Test connection
    status = redisClusterAsyncCommand(acc, commandCallback, (char *)"THE_ID",
                                      "SET key1 Hello");
    assert(status == REDIS_ERR);
    assert(acc->err == REDIS_ERR_OTHER);
    assert(strcmp(acc->errstr, "node get by table error") == 0);

    event_base_dispatch(base);

    redisClusterAsyncFree(acc);
    event_base_free(base);
}

// Connecting to a password protected cluster using
// the async API, not providing a password.
void test_async_password_missing() {
    redisClusterAsyncContext *acc = redisClusterAsyncContextInit();
    assert(acc);
    redisClusterAsyncSetConnectCallback(acc, callbackExpectOk);
    redisClusterAsyncSetDisconnectCallback(acc, callbackExpectOk);
    redisClusterSetOptionAddNodes(acc->cc, CLUSTER_NODE_WITH_PASSWORD);
    // Password not configured

    struct event_base *base = event_base_new();
    redisClusterLibeventAttach(acc, base);

    int status;
    status = redisClusterConnect2(acc->cc);
    assert(status == REDIS_ERR);
    assert(acc->err == REDIS_OK); // TODO: This must be wrong!
    assert(acc->cc->err == REDIS_ERR_OTHER);
    assert(strncmp(acc->cc->errstr, "NOAUTH", 6) == 0);

    // No connection
    status = redisClusterAsyncCommand(acc, commandCallback, (char *)"THE_ID",
                                      "SET key1 Hello");
    assert(status == REDIS_ERR);
    assert(acc->err == REDIS_ERR_OTHER);
    assert(strcmp(acc->errstr, "node get by table error") == 0);

    event_base_dispatch(base);

    redisClusterAsyncFree(acc);
    event_base_free(base);
}

int main() {

    test_password_ok();
    test_password_wrong();
    test_password_missing();

    test_async_password_ok();
    test_async_password_wrong();
    test_async_password_missing();

    return 0;
}
