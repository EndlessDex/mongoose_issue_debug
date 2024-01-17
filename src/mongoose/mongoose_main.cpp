/*
 * Filename:        mongoose_main.cpp
 * Classification:  UNCLASSIFIED
 *
 * Copyright (c) 2021 Viasat, Inc.
 *
 * All rights reserved.
 * The information in this software is subject to change without notice and
 * should not be construed as a commitment by Viasat, Inc.
 *
 * Viasat Proprietary
 * The Proprietary Information provided herein is proprietary to Viasat and
 * must be protected from further distribution and use. Disclosure to others,
 * use or copying without express written authorization of Viasat, is strictly
 * prohibited.
 */

#include "mongoose/mongoose.h"
#include "nonsense.h"
#include <map>
#include <string>

#define BUFSIZE                 65536
#define HTTP_POLL_INTERVAL      500
#define WEBSOCKET_READ_INTERVAL 500

static const char *rootDir = "/tmp/httpd"; // default root directory

static void EventHandler(struct mg_connection *c, int ev, void *ev_data, void *fn_data);
static void WebSocketConnect(struct mg_connection *c);
static void WebSocketReceive(struct mg_connection *c, struct mg_ws_message *wm);
static void WebSocketClose(struct mg_connection *c);
static void WebSocketSend(void *arg);

static std::map<struct mg_connection *, std::string> websocket_map; // map of websocket connections to index

static void EventHandler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        if (mg_http_match_uri(hm, "/websocket")) {
            mg_ws_upgrade(c, hm, NULL);
        } else {
            struct mg_http_serve_opts serve_opts;
            serve_opts.root_dir = rootDir;
            serve_opts.ssi_pattern = "";
            serve_opts.extra_headers = "X-Content-Type-Options: nosniff\r\nCache-Control: no-cache\r\n";
            serve_opts.mime_types = "woff2=font/woff2,tar=application/tar";
            serve_opts.fs = NULL;
            mg_http_serve_dir(c, hm, &serve_opts);
        }
    } else if (ev == MG_EV_WS_OPEN) {
        WebSocketConnect(c);
    } else if (ev == MG_EV_WS_MSG) {
        WebSocketReceive(c, (struct mg_ws_message *)ev_data);
    } else if (ev == MG_EV_CLOSE) {
        WebSocketClose(c);
    }
}

static void usage(const char *prog) {
    fprintf(stderr,
            "Mongoose v.%s, built " __DATE__ " " __TIME__
            "\nUsage: %s OPTIONS\n"
            "  -d DIR    - directory to serve, default: '%s'\n" MG_VERSION,
            prog, rootDir);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    struct mg_mgr mgr;

    // Parse command-line flags
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            rootDir = argv[++i];
        } else {
            usage(argv[0]);
        }
    }

    // Initialization
    mg_log_set(MG_LL_DEBUG);
    mg_mgr_init(&mgr);

    // Create listening socket connection for http
    if (mg_http_listen(&mgr, "http://0.0.0.0:80", EventHandler, &mgr) == NULL) {
        MG_ERROR(("Cannot listen on http://0.0.0.0:80."));
        exit(EXIT_FAILURE);
    }

    MG_INFO(("Starting Mongoose v%s, serving [%s]", MG_VERSION, rootDir));

    mg_timer_add(&mgr, WEBSOCKET_READ_INTERVAL, MG_TIMER_REPEAT, WebSocketSend, &mgr);

    while (true) {
        mg_mgr_poll(&mgr, HTTP_POLL_INTERVAL);
    }

    mg_mgr_free(&mgr);

    return 0;
}

static void WebSocketConnect(struct mg_connection *c) {
    char clientIpAddress[64] = {0};
    int ipLen = mg_snprintf(clientIpAddress, sizeof(clientIpAddress), "%M", mg_print_ip_port, &c->rem);
    MG_INFO(("WebSocketConnect: %s", clientIpAddress));

    websocket_map[c] = std::string(clientIpAddress);
}

static void WebSocketReceive(struct mg_connection *c, struct mg_ws_message *wm) {
    static char rxBuf[BUFSIZE] = {0};
    if (auto it = websocket_map.find(c); it != websocket_map.end()) {
        int len = mg_base64_decode(wm->data.ptr, wm->data.len, rxBuf, BUFSIZE);
        MG_DEBUG(("WebSocketReceive: %s: %s", it->second.c_str(), rxBuf));

    } else {
        MG_ERROR(("WebSocketReceive: WebSocket not found: %p", c));
    }
}

static void WebSocketClose(struct mg_connection *c) {
    MG_INFO(("WebSocketClose: %s", websocket_map[c].c_str()));
    websocket_map.erase(c);
}

static unsigned packet_counter = 0;

static void WebSocketSend(void *arg) {
    static unsigned char message[BUFSIZE] = {0};
    static char message_enc[BUFSIZE] = {0};

    int len = snprintf(reinterpret_cast<char *>(message), BUFSIZE, "%d: %s", packet_counter++, nonsense);

    // Encode the data
    len = mg_base64_encode(message, len, message_enc, BUFSIZE);

    for (auto it = websocket_map.begin(); it != websocket_map.end(); ++it) {
        mg_ws_send(it->first, message_enc, len, WEBSOCKET_OP_TEXT);
        MG_DEBUG(("WebSocketSend: %s", it->second.c_str()));
    }

    struct mg_mgr *mgr = (struct mg_mgr *)arg;
    struct mg_connection *c;
    for (c = mgr->conns; c != NULL; c = c->next) {
        if (c->is_websocket) {
            MG_DEBUG(("%p %lu : txBuf %p/%u/%u/%u    rxBuf %p/%u/%u/%u", c, c->id,
                      c->send.buf, c->send.len, c->send.size, c->send.align,
                      c->recv.buf, c->recv.len, c->recv.size, c->recv.align));
        }
    }
}
