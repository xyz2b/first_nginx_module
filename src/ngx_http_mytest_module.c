#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#define UPSTREAM 1

#ifdef NGX
typedef struct {
    ngx_str_t   my_str;
    ngx_int_t   my_num;
    ngx_flag_t  my_flag;
    size_t      my_size;
    ngx_array_t*    my_str_array;
    ngx_array_t*    my_keyval;
    off_t           my_off;
    ngx_msec_t      my_msec;
    time_t          my_sec;
    ngx_bufs_t      my_bufs;
    ngx_uint_t      my_enum_seq;
    ngx_uint_t      my_bitmask;
    ngx_uint_t      my_access;
    ngx_path_t*     my_path;
} ngx_http_mytest_conf_t;
#endif

#ifdef USER
typedef struct {
    ngx_str_t   my_config_str;
    ngx_int_t   my_config_num;
} ngx_http_mytest_conf_t;
#endif

#ifdef UPSTREAM
// 从配置文件中解析的配置信息
typedef struct {
    ngx_http_upstream_conf_t upstream;
} ngx_htt_mytest_conf_t;

// 请求上下文，会放到请求对应的request结构体中
typedef struct {
    ngx_http_status_t   status; // process_header解析响应行时使用
} ngx_http_mytest_ctx_t;
#endif

#ifdef NGX
static ngx_conf_enum_t test_enums[] = {
        {ngx_string("apple"), 1},
        {ngx_string("banana"), 2},
        {ngx_string("orange"), 3},
        {ngx_null_string, 0}
};

static ngx_conf_bitmask_t test_bitmasks[] = {
        {ngx_string("good"), 0x0002},
        {ngx_string("better"), 0x0004},
        {ngx_string("orange"), 0x0008},
        {ngx_null_string, 0}
};
#endif

static char* ngx_http_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r);
static void* ngx_http_mytest_create_loc_conf(ngx_conf_t* cf);
#ifdef USER
static char* ngx_conf_set_myconfig(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
#endif
#ifdef NGX
static char* ngx_http_mytest_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
#endif
#ifdef UPSTREAM
static ngx_int_t mytest_upstream_create_request(ngx_http_request_t *r);
static ngx_int_t mytest_process_status_line(ngx_http_request_t *r);
#endif

static ngx_command_t ngx_http_mytest_commands[] = {
        {
            ngx_string("mytest"),
            NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LMT_CONF|NGX_CONF_NOARGS,
            ngx_http_mytest,
            NGX_HTTP_LOC_CONF_OFFSET,
            0,
            NULL
        },
#ifdef NGX
        {
            ngx_string("test_flag"),
            NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
            ngx_conf_set_flag_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_flag),
            NULL
        },
        {
            ngx_string("test_str"),
            NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,  // 配置项可以出现在http、server、location配置块中，并有且仅有1个参数
            ngx_conf_set_str_slot,
            NGX_HTTP_LOC_CONF_OFFSET,               // 将配置项存储在每个配置块(http、server、location)对应的 create_loc_conf 生成的配置结构体中
            offsetof(ngx_http_mytest_conf_t, my_str),
            NULL
        },
        {
            ngx_string("test_str_array"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
            ngx_conf_set_str_array_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_str_array),
            NULL
        },
        {
            ngx_string("test_keyval"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE2,
            ngx_conf_set_keyval_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_keyval),
            NULL
        },
        {
            ngx_string("test_num"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
            ngx_conf_set_num_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_num),
            NULL
        },
        {
            ngx_string("test_size"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
            ngx_conf_set_size_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_size),
            NULL
        },
        {
            ngx_string("test_off"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
            ngx_conf_set_off_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_off),
            NULL
        },
        {
            ngx_string("test_msec"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
            ngx_conf_set_msec_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_msec),
            NULL
        },
        {
            ngx_string("test_sec"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
            ngx_conf_set_sec_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_sec),
            NULL
        },
        {
            ngx_string("test_bufs"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE2,
            ngx_conf_set_bufs_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_bufs),
            NULL
        },
        {
            ngx_string("test_enum"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
            ngx_conf_set_enum_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_enum_seq),
            test_enums
        },
        {
            ngx_string("test_bitmask"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
            ngx_conf_set_bitmask_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_bitmask),
            test_bitmasks
        },
        {
            ngx_string("test_access"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE123,
            ngx_conf_set_access_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_access),
            NULL
        },
        {
            ngx_string("test_path"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1234,
            ngx_conf_set_path_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_path),
            NULL
        },
#endif
#ifdef USER
        {
            ngx_string("test_myconfig"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE12,
            ngx_conf_set_myconfig,
            NGX_HTTP_LOC_CONF_OFFSET,
            0,
            NULL
        },
#endif
#ifdef UPSTREAM
        {
            ngx_string("upstream_connect_timeout"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
            ngx_conf_set_msec_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, upstream.connect_timeout),
            NULL
        },
        {
            ngx_string("upstream_send_timeout"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
            ngx_conf_set_msec_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, upstream.send_timeout),
            NULL
        },
        {
            ngx_string("upstream_read_timeout"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
            ngx_conf_set_msec_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, upstream.read_timeout),
            NULL
        },
#endif
        ngx_null_command
};

/**
 * ngx_conf_set_num_slot 官方解析num类型配置项的方法
 * @param cf
 * @param cmd
 * @param conf
 * @return 成功或失败
    char *
    ngx_conf_set_num_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
    {
        // conf就是存储参数的结构体的地址，即指向ngx_http_mytest_conf_t的指针
        char  *p = conf;

        ngx_int_t        *np;
        ngx_str_t        *value;
        ngx_conf_post_t  *post;

        // 根据ngx_command_t中的offset偏移量，可以在参数结构体中找到对应的参数成员
        // 而对于ngx_conf_set_num_slot方法而言，存储数字的必须是ngx_int_t类型
        np = (ngx_int_t *) (p + cmd->offset);

        // 这里知道为什么要把使用ngx_conf_set_num_slot方法解析的成员在create_loc_conf等方法中初始化为NGX_CONF_UNSET，否则是会报错的
        if (*np != NGX_CONF_UNSET) {
            return "is duplicate";
        }

        // value指向配置文件中配置项的参数值
        value = cf->args->elts;
        // 将字符串的参数转为整型，并设置到create_loc_conf等方法生成的结构体的相关成员上
        *np = ngx_atoi(value[1].data, value[1].len);
        if (*np == NGX_ERROR) {
            return "invalid number";
        }

        // 如果ngx_command_t中的post已经实现，那么还需要调用post->post_handler方法
        if (cmd->post) {
            post = cmd->post;
            return post->post_handler(cf, post, np);
        }

        return NGX_CONF_OK;
    }
 * */

static char* ngx_http_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf;

    // 首先找到mytest配置项所属的配置块，clcf看上去像是location块内的数据结构，其实不然，它可以是main、srv或者loc级别配置项，也就是说，在每个http{}和server{}內也都有一个ngx_http_core_loc_conf_t结构体
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    // HTTP框架在处理用户请求进行到NGX_HTTP_CONTENT_PHASE阶段时，如果请求的主机域名、URI与mytest配置项所在的配置块相匹配，就将调用我们实现的ngx_http_mytest_handler方法处理这个请求
    // 这是NGX_HTTP_CONTENT_PHASE阶段排他性的实现，即这个阶段只能由这个模块处理。类似实现有反向代理模块
    /**
     * 在HTTP模块解析配置文件，检测到某个开关被打开后
     * - 进入content阶段处理请求
     * - 所有反向代理模块使用该方式，因其对其他content阶段模块具有排他性
     * */
     /**
      * 非排他性实现
      * ngx_http_module_t的postconfiguration阶段，定义回调函数
      * 即在HTTP模块解析完配置文件后
      * static ngx_int_t ngx_http_hello_init(ngx_conf_t *cf)
      * {
      *  ngx_http_handler_pt        *h;
      *  ngx_http_core_main_conf_t  *cmcf;
      *
      *  cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
      *
      *  // 将处理请求的handler加入到NGX_HTTP_CONTENT_PHASE阶段中
      *  h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
      *  if (h == NULL) {
      *    return NGX_ERROR;
      *  }
      *
      *  h = ngx_http_hello_handler;
      *
      *  return NGX_OK;
      * }
      * */
    clcf->handler = ngx_http_mytest_handler;

    return NGX_CONF_OK;
}

static ngx_http_module_t ngx_http_mytest_module_ctx = {
        NULL,                                           /* 4.preconfiguration */
        NULL,                                           /* 8.postconfiguration */

        NULL,                                           /* 1.create main configuration */
        NULL,                                           /* 5.init main configuration */

        NULL,                                           /* 2.create server configuration */
        NULL,                                           /* 6.merge server configuration */

        ngx_http_mytest_create_loc_conf,                /* 3.create location configuration */
#ifdef NGX
        ngx_http_mytest_merge_loc_conf                  /* 7.merge location configuration */
#else
        NULL
#endif
};

ngx_module_t ngx_http_mytest_module = {
        NGX_MODULE_V1,
        &ngx_http_mytest_module_ctx,                    /* module context */
        ngx_http_mytest_commands,                       /* module directives */
        NGX_HTTP_MODULE,                                /* module type */
        NULL,                                           /* init master */
        NULL,                                           /* init module */
        NULL,                                           /* init process */
        NULL,                                           /* init thread */
        NULL,                                           /* exit thread */
        NULL,                                           /* exit process */
        NULL,                                           /* exit master */
        NGX_MODULE_V1_PADDING
};

static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r)
{
#ifdef UPSTREAM
    // 首先建立HTTP上下文结构体ngx_http_mytest_ctx_t
    ngx_http_mytest_ctx_t *myctx = ngx_http_get_module_ctx(r, ngx_http_mytest_module);
    if (myctx == NULL) {
        myctx = ngx_palloc(r->pool, sizeof(ngx_http_mytest_ctx_t));
        if (myctx == NULL) {
            return NGX_ERROR;
        }
        // 将新建立的上下文与请求关联起来
        ngx_http_set_ctx(r, myctx, ngx_http_mytest_module);
    }
    // 对每个要使用upstream的请求，必须调用且只能调用1次ngx_http_upstream_create方法，它会初始化r->upstream成员
    if (ngx_http_upstream_create(r) != NGX_OK) {
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_upstream_create failed");
        return NGX_ERROR;
    }

    // 得到配置结构体ngx_http_mytest_conf_t
    ngx_http_mytest_conf_t *mycf = ngx_http_get_module_loc_conf(r, ngx_http_mytest_module);
    ngx_http_upstream_t *u = r->upstream;

    // 设置upstream参数：将解析出来的配置文件中的upstream的参数赋值给请求中的upstream参数字段
    u->conf = &mycf->upstream;

    // 决定转发包体时使用的缓冲区
    u->buffering = mycf->upstream.buffering;

    // 设置upstream上游服务地址
    // 初始化resolved结构体，用来保存上游服务地址

    // 设置回调方法
    r->upstream->create_request = mytest_upstream_create_request;
    r->upstream->process_header = mytest_process_status_line;       // 将处理上游响应header分成了两部分处理，一部分是请求行，一部分是请求头，处理完请求行之后会调用处理请求头的方法
    r->upstream->finalize_request = mytest_upstream_finalize_request;

    // 启动upstream
    r->main->count++;       // 告知HTTP框架将当前请求的引用计数加1，即告知HTTP框架暂时不要销毁请求，因为HTTP框架只有在请求引用计数为0时才能真正销毁请求
    ngx_http_upstream_init(r);
    return NGX_DONE;    // 通过返回NGX_DONE告知HTTP框架暂停执行请求的下一个阶段
#endif

#ifdef NGX
    u_char ngx_my_str[1024] = {0};

    ngx_http_mytest_conf_t *my_conf;

    my_conf = ngx_http_get_module_loc_conf(r, ngx_http_mytest_module);

    if (my_conf->my_str.len != 0) {
//        ngx_sprintf(ngx_my_str, "%s", my_conf->my_str.data);
        // %V对应的参数必须是ngx_str_t类型变量的地址，按照ngx_str_t.len长度输出ngx_str_t.data的字符串内容
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_str: %V", &my_conf->my_str);
    }

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_flag: %d", my_conf->my_flag);

    if (my_conf->my_str_array != NULL) {
        for (ngx_uint_t i = 0; i < my_conf->my_str_array->nelts; i++) {
            ngx_str_t* pstr = my_conf->my_str_array->elts;
            memset(ngx_my_str, 0, sizeof(ngx_my_str));
            ngx_sprintf(ngx_my_str, "%s", pstr[i].data);
            ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_str_array_%d: %s", i, ngx_my_str);
        }
    }

    if (my_conf->my_keyval != NULL) {
        for (ngx_uint_t i = 0; i < my_conf->my_keyval->nelts; i++) {
            ngx_keyval_t* pkv = my_conf->my_keyval->elts;
            memset(ngx_my_str, 0, sizeof(ngx_my_str));
            ngx_sprintf(ngx_my_str, "%s", pkv[i].key.data);
            ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_keyval_%d_key: %s", i, ngx_my_str);
            memset(ngx_my_str, 0, sizeof(ngx_my_str));
            ngx_sprintf(ngx_my_str, "%s", pkv[i].value.data);
            ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_keyval_%d_value: %s", i, ngx_my_str);
        }
    }

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_num: %d", my_conf->my_num);

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_size: %d", my_conf->my_size);

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_off: %d", my_conf->my_off);

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_msec: %d", my_conf->my_msec);

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_sec: %d", my_conf->my_sec);

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_bufs: %d:%d", my_conf->my_bufs.num, my_conf->my_bufs.size);

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_enum: %d", my_conf->my_enum_seq);

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_bitmask: %xd", my_conf->my_bitmask);

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_access: %d", my_conf->my_access);

    if (my_conf->my_path != NULL) {
        memset(ngx_my_str, 0, sizeof(ngx_my_str));
        ngx_sprintf(ngx_my_str, "%s", my_conf->my_path->name.data);
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_path_name: %s", ngx_my_str);
        for (int i = 0; i < 3; i++) {
            ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_path_level_%d: %d", i, my_conf->my_path->level[i]);
        }
    }
#endif

#ifdef USER
    u_char ngx_my_str[1024] = {0};

    ngx_http_mytest_conf_t *my_conf;

    my_conf = ngx_http_get_module_loc_conf(r, ngx_http_mytest_module);

    if (my_conf->my_config_str.len != 0) {
        memset(ngx_my_str, 0, sizeof(ngx_my_str));
        ngx_sprintf(ngx_my_str, "%s", my_conf->my_config_str.data);
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_config_str: %s", ngx_my_str);

        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_config_num: %d", my_conf->my_config_num);
    }

#endif

#ifndef UPSTREAM
    // 如果请求方法不为GET或PUT，就返回 405 NOT_ALLOWED
    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    // 丢弃请求body
    ngx_int_t rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return rc;
    }

    ngx_str_t type = ngx_string("text/plain");
    ngx_str_t response = ngx_string("hello world");

    // 响应的header
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = response.len;
    r->headers_out.content_type = type;

    // 发送响应header
    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    // 响应body是放在buf中的
    ngx_buf_t *b;
    b = ngx_create_temp_buf(r->pool, response.len);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_memcpy(b->pos, response.data, response.len);
    b->last = b->pos + response.len;
    // 标示是最后一块buf
    b->last_buf = 1;

    // buf使用chain包装，再传给ngx_http_output_filter方法
    ngx_chain_t out;
    out.buf = b;
    out.next = NULL;

    // 发送响应body
    return ngx_http_output_filter(r, &out);
#endif
}

static void* ngx_http_mytest_create_loc_conf(ngx_conf_t* cf)
{
    ngx_http_mytest_conf_t *mycf;

    mycf = (ngx_http_mytest_conf_t *) ngx_pcalloc(cf->pool, sizeof(ngx_http_mytest_conf_t));
    if (mycf == NULL) {
        return NULL;
    }

#ifdef NGX
    // 如果使用nginx自带的解析参数的方法，必须按照下面进行各种类型参数值的初始化
    mycf->my_flag = NGX_CONF_UNSET;
    mycf->my_num = NGX_CONF_UNSET;
    mycf->my_str_array = NGX_CONF_UNSET_PTR;
    mycf->my_keyval = NULL;
    mycf->my_off = NGX_CONF_UNSET;
    mycf->my_msec = NGX_CONF_UNSET_MSEC;
    mycf->my_sec = NGX_CONF_UNSET;
    mycf->my_size = NGX_CONF_UNSET_SIZE;
    mycf->my_enum_seq = NGX_CONF_UNSET;
    mycf->my_bitmask = 0;
    mycf->my_access = NGX_CONF_UNSET_UINT;
#endif

#ifdef UPSTREAM
    mycf->upstream.connect_timeout = NGX_CONF_UNSET_MSEC;
    mycf->upstream.send_timeout = NGX_CONF_UNSET_MSEC;
    mycf->upstream.read_timeout = NGX_CONF_UNSET_MSEC;
    mycf->upstream.store_access = 0600;

    mycf->upstream.buffering = 0;
    mycf->upstream.bufs.num = 8;
    mycf->upstream.bufs.size = ngx_pagesize;
    mycf->upstream.buffer_size = ngx_pagesize;
    mycf->upstream.busy_buffers_size = 2 * ngx_pagesize;
    mycf->upstream.temp_file_write_size = 2 * ngx_pagesize;
    mycf->upstream.max_temp_file_size = 1024 * 1024 * 1024;

    mycf->upstream.hide_headers = NGX_CONF_UNSET_PTR;
    mycf->upstream.pass_headers = NGX_CONF_UNSET_PTR;
#endif

    return mycf;
}

#ifdef USER
static char* ngx_conf_set_myconfig(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    /* 注意，参数conf就是HTTP框架传给用户的，在 ngx_http_mytest_create_loc_conf 回调方法中分配的结构体 ngx_http_mytest_conf_t */
    ngx_http_mytest_conf_t *mycf  = conf;

    /* cf->args 是1个 ngx_array_t 队列，它的成员都是 ngx_str_t 结构。我们用value指向ngx_array_t的elts内容，其中value[1]就是第一个参数，同理，value[2]就是第2个参数 */
    ngx_str_t* value = cf->args->elts;

    // ngx_array_t 的 nelts表示参数的个数
    if (cf->args->nelts > 1) {
        // 直接赋值即可，ngx_str_t结构只是指针的传递
        mycf->my_config_str = value[1];
    }

    if (cf->args->nelts > 2) {
        // 将字符串形式的第2个参数转为整形
        mycf->my_config_num = ngx_atoi(value[2].data, value[2].len);

        /* 如果字符串转化整形失败，将报“invalid number”错误，nginx启动失败 */
        if (mycf->my_config_num == NGX_ERROR) {
            return "invalid number";
        }
    }

    // 返回成功
    return NGX_CONF_OK;
}
#endif

#ifdef NGX
static char* ngx_http_mytest_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    u_char ngx_my_str[1024] = {0};

    // parent是解析父配置块生成的结构体
    ngx_http_mytest_conf_t *prev = (ngx_http_mytest_conf_t *) parent;
    // child是保存子配置块的结构体
    ngx_http_mytest_conf_t *conf = (ngx_http_mytest_conf_t *) child;

#ifdef NGX
    if (prev != NULL && prev->my_str.len > 0) {
        memset(ngx_my_str, 0, sizeof(ngx_my_str));
        ngx_sprintf(ngx_my_str, "%s", prev->my_str.data);
        ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "parent: %s", ngx_my_str);
    } else if (prev != NULL && prev->my_str.len == 0) {
        ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "parent: null");
    }

    if (conf != NULL && conf->my_str.len > 0) {
        memset(ngx_my_str, 0, sizeof(ngx_my_str));
        ngx_sprintf(ngx_my_str, "%s", conf->my_str.data);
        ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "child: %s", ngx_my_str);
    } else if (prev != NULL && conf->my_str.len == 0) {
        ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "child: null");
    }

    // 使用父配置块中的参数值替换子配置块中的参数值，如果父配置块和子配置块都没有解析到配置项，则将值设置为默认值"defaultstr"
    ngx_conf_merge_str_value(conf->my_str, prev->my_str, "defaultstr");

    if (prev != NULL && prev->my_str.len > 0) {
        memset(ngx_my_str, 0, sizeof(ngx_my_str));
        ngx_sprintf(ngx_my_str, "%s", prev->my_str.data);
        ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "after parent: %s", ngx_my_str);
    } else if (prev != NULL && prev->my_str.len == 0) {
        ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "after parent: null");
    }

    if (conf != NULL && conf->my_str.len > 0) {
        memset(ngx_my_str, 0, sizeof(ngx_my_str));
        ngx_sprintf(ngx_my_str, "%s", conf->my_str.data);
        ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "after child: %s", ngx_my_str);
    } else if (prev != NULL && conf->my_str.len == 0) {
        ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "after child: null");
    }
#endif

#ifdef UPSTREAM
    ngx_hash_init_t hash;
    hash.max_size = 100;
    hash.bucket_size = 1024;
    hash.name = "proxy_headers_hash";
    if (ngx_http_upstream_hide_headers_hash(cf, $conf->upstream, $prev->upstream, ngx_http_proxy_hide_headers, &hash) != NGX_OK) {
        return NGX_CONF_ERROR;
    }
    return NGX_CONF_OK;
#endif
    /**
     * http {
            server {
                listen       80;
                server_name  localhost;

                location / {
                    root   html;
                    index  index.html index.htm;
                }

                location /test {
                    mytest;
                    test_str apple;
                }

                test_str server80;
                location /url1 {
                    mytest;
                    test_str loc1;
                }

                location /url2 {
                    mytest;
                    test_str loc2;
                }

            }

            server {
                listen 8080;

                test_str server8080;
                location /url3 {
                    mytest;
                    test_str loc3;
                }
            }
        }


        // 输出
        下面的配置项都是存储在各个配置块(http、server、location)对应的create_loc_conf 生成的配置结构体中，因为定义command时，设置了test_str配置项存储的结构体为create_loc_conf 生成的配置结构体
        nginx: [emerg] parent: null						http配置块中没有test_str配置项，所以为空
        nginx: [emerg] child: server80 					server 80配置块中test_str配置
        nginx: [emerg] after parent: null
        nginx: [emerg] after child: server80
        nginx: [emerg] parent: server80 				server 80配置块中test_str配置
        nginx: [emerg] child: null						location /配置块中没有test_str配置项，所以为空
        nginx: [emerg] after parent: server80
        nginx: [emerg] after child: server80
        nginx: [emerg] parent: server80 				server 80配置块中test_str配置
        nginx: [emerg] child: apple						location /test配置块中test_str配置项
        nginx: [emerg] after parent: server80
        nginx: [emerg] after child: apple
        nginx: [emerg] parent: server80   				server 80配置块中test_str配置
        nginx: [emerg] child: loc1						location /url1配置块中test_str配置项
        nginx: [emerg] after parent: server80
        nginx: [emerg] after child: loc1
        nginx: [emerg] parent: server80
        nginx: [emerg] child: loc2
        nginx: [emerg] after parent: server80
        nginx: [emerg] after child: loc2
        nginx: [emerg] parent: server80
        nginx: [emerg] child: null
        nginx: [emerg] after parent: server80
        nginx: [emerg] after child: server80
        nginx: [emerg] parent: null 					http配置块中没有test_str配置项，所以为空
        nginx: [emerg] child: server8080				server 8080配置块中test_str配置
        nginx: [emerg] after parent: null
        nginx: [emerg] after child: server8080
        nginx: [emerg] parent: server8080 				server 8080配置块中test_str配置
        nginx: [emerg] child: loc3 						location /url3配置块中test_str配置项
        nginx: [emerg] after parent: server8080
        nginx: [emerg] after child: loc3
     * */

    return NGX_CONF_OK;
}
#endif

#ifdef UPSTREAM
// 创建上游请求
static ngx_int_t mytest_upstream_create_request(ngx_http_request_t *r)
{
    // 发往baidu上游服务器的请求很简单，就是模仿正常的搜索请求，以/s?wd=...的URL格式来发起搜索请求。
    static ngx_str_t backendQueryLine = ngx_string("GET /s?wd=%V HTTP/1.1\r\nHost:www.baidu.com\r\nConnection:close\r\n\r\n");
    ngx_int_t queryLineLen = backendQueryLine.len + r->args.len - 2;  // 减去%V占位符的长度
    /**
     * 必须是在内存池中申请内存，这有以下两点好处:
     * 1.在网络情况不佳的情况下，向上游服务器发送请求时，可能需要epoll多次调用send才能发送完成，这时必须保证这段内存不会被释放
     * 2.在请求结束时，这段内存会被自动释放，降低内存泄露的可能
     * */
    ngx_buf_t* b = ngx_create_temp_buf(r->pool, queryLineLen);
    if (b == NULL) {
        return NGX_ERROR;
    }
    // last需要指向buf中请求行的末尾
    b->last = b->pos + queryLineLen;
    // 作用相当于snprintf，只是它支持ngx特殊的转换格式
    ngx_snprintf(b->pos, queryLineLen, (char*)backendQueryLine.data, &r->args);
    // r->upstream-request_bufs是一个ngx_chain_t结构，它包含着发送给上游服务器的请求
    r->upstream->request_bufs = ngx_alloc_chain_link(r->pool);
    if (r->upstream->request_bufs == NULL) {
        return NGX_ERROR;
    }
    // request_bufs在这里只包含1个ngx_buf_t缓冲区
    r->upstream->request_bufs->buf = b;
    r->upstream->request_bufs->next = NULL;

    r->upstream->request_sent = 0;
    r->upstream->header_sent = 0;
    // header_hash不可以为0
    r->header_hash = 1;
    return NGX_OK;

}

// 解析上游响应行
static ngx_int_t mytest_process_status_line(ngx_http_request_t *r)
{

    return mytest_upstream_process_header(r);
}

// 解析上游响应头
static ngx_int_t mytest_upstream_process_header(ngx_http_request_t *r)
{

}

// 上游请求结束之后的收尾处理
static void mytest_upstream_finalize_request(ngx_http_request_t *r, ngx_int_t rc)
{
    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "mytest_upstream_finalize_request");
}
#endif