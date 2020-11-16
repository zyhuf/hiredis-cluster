#ifndef __TEST_UTILS_H__
#define __TEST_UTILS_H__

#define ASSERT_MSG(_x, _msg)                                                   \
    if (!(_x)) {                                                               \
        fprintf(stderr, "ERROR: %s (%s)\n", _msg, #_x);                        \
        assert(_x);                                                            \
    }

#define CHECK_REPLY(_ctx, _reply)                                              \
    if ((_reply) == NULL) {                                                    \
        fprintf(stderr, "ERROR: reply=NULL  =>  ");                            \
        if ((_ctx) != NULL) {                                                  \
            ASSERT_MSG(_reply, _ctx->errstr);                                  \
        } else {                                                               \
            ASSERT_MSG(_reply, "context is NULL");                             \
        }                                                                      \
    }

#define CHECK_REPLY_TYPE(_reply, _type)                                        \
    { ASSERT_MSG((_reply->type == _type), "Reply type incorrect"); }

#define CHECK_REPLY_OK(_ctx, _reply)                                           \
    {                                                                          \
        CHECK_REPLY(_ctx, _reply);                                             \
        CHECK_REPLY_TYPE(_reply, REDIS_REPLY_STATUS);                          \
        ASSERT_MSG((strcmp(_reply->str, "OK") == 0), _ctx->errstr);            \
    }

#define CHECK_REPLY_QUEUED(_ctx, _reply)                                       \
    {                                                                          \
        CHECK_REPLY(_ctx, _reply);                                             \
        CHECK_REPLY_TYPE(_reply, REDIS_REPLY_STATUS);                          \
        ASSERT_MSG((strcmp(_reply->str, "QUEUED") == 0), _ctx->errstr);        \
    }

#define CHECK_REPLY_INT(_ctx, _reply, _value)                                  \
    {                                                                          \
        CHECK_REPLY(_ctx, _reply);                                             \
        CHECK_REPLY_TYPE(_reply, REDIS_REPLY_INTEGER);                         \
        ASSERT_MSG((_reply->integer == _value), _ctx->errstr);                 \
    }

#define CHECK_REPLY_STR(_ctx, _reply, _str)                                    \
    {                                                                          \
        CHECK_REPLY(_ctx, _reply);                                             \
        CHECK_REPLY_TYPE(_reply, REDIS_REPLY_STRING);                          \
        ASSERT_MSG((strcmp(_reply->str, _str) == 0), _ctx->errstr);            \
    }

#define CHECK_REPLY_ARRAY(_ctx, _reply, _num_of_elements)                      \
    {                                                                          \
        CHECK_REPLY(_ctx, _reply);                                             \
        CHECK_REPLY_TYPE(_reply, REDIS_REPLY_ARRAY);                           \
        ASSERT_MSG(_reply->elements == _num_of_elements, _ctx->errstr);        \
    }

#endif
