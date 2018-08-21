/*
 * A simple HTTP/SERVER library
 * Copyright (C) 2018 A. C. Open Hardware Ideas Lab
 *
 * Authors:
 *  Marco Giammarini <m.giammarini@warcomeb.it>
 *  Gianluca Calignano <g.calignano97@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @mainpage HTTP/SERVER library with @a libohiboard
 *
 * This project...
 *
 * @section changelog ChangeLog
 *
 * @li v1.0 of 2018/08/21 - First release
 *
 * @section library External Library
 *
 * The library use the following external library
 * @li libohiboard https://github.com/ohilab/libohiboard a C framework for
 * NXP Kinetis microcontroller
 * @li ethern-socket https://github.com/ohilab/ethernet-socket a C
 * library to manage client/server socket
 *
 * @section thanksto Thanks to...
 * @li Marco Giammarini
 * @li Gianluca Calignano
 */

#ifndef __OHILAB_HTTPSERVER_H
#define __OHILAB_HTTPSERVER_H

#define OHILAB_HTTPSERVER_LIBRARY_VERSION     "1.0.0"
#define OHILAB_HTTPSERVER_LIBRARY_VERSION_M   1
#define OHILAB_HTTPSERVER_LIBRARY_VERSION_m   0
#define OHILAB_HTTPSERVER_LIBRARY_VERSION_bug 0
#define OHILAB_HTTPSERVER_LIBRARY_TIME        0

#include "libohiboard.h"
//Ethernet server socket
#include "ethernet-socket/ethernet-serversocket.h"

/*
 * The user must define these label... TODO
 */
#ifndef __NO_BOARD_H
#include "board.h"
#endif

/**
 * @defgroup httpServer_functions HTTP server functions
 * The HTTP server function group
 *
 * @defgroup httpServer_macros HTTP server macros
 * @ingroup httpServer_functions
 * Macros MUST be defined to enable the library to work properly.
 * They could be defined in board.h but there is a check in http-server.h which
 * avoid silly problems if macros are not defined.
 */

/**
 * @ingroup httpServer_macros
 * The max length of a URI which can be collect and store in
 * @ref HttpServer_Message .
 */
#ifndef HTTPSERVER_MAX_URI_LENGTH
#define HTTPSERVER_MAX_URI_LENGTH           100
#endif
/**
 * @ingroup httpServer_macros
 * The max length of headers which can be collect and store in
 * @ref HttpServer_Message .
 */
#ifndef HTTPSERVER_HEADERS_MAX_LENGTH
#define HTTPSERVER_HEADERS_MAX_LENGTH       1023
#endif
/**
 * @ingroup httpServer_macros
 * The max length of the receive buffer for each @ref HttpServer_Client.
 */
#ifndef HTTPSERVER_RX_BUFFER_DIMENSION
#define HTTPSERVER_RX_BUFFER_DIMENSION      255
#endif
/**
 * @ingroup httpServer_macros
 * The max length of the trasmission buffer for each @ref HttpServer_Client.
 */
#ifndef HTTPSERVER_TX_BUFFER_DIMENSION
#define HTTPSERVER_TX_BUFFER_DIMENSION      255
#endif
/**
 * @ingroup httpServer_macros
 * Timeout .
 */
#ifndef HTTPSERVER_TIMEOUT
#define HTTPSERVER_TIMEOUT                  3000
#endif

/**
 * @ingroup httpServer_macros
 */
#define HTTPSERVER_STRING_REQUEST_GET        "GET"

/**
 * @ingroup httpServer_macros
 */
#define HTTPSERVER_STRING_REQUEST_POST       "POST"

/**
 * @ingroup httpServer_macros
 */
#define HTTPSERVER_STRING_REQUEST_PUT        "PUT"

/**
 * @ingroup httpServer_macros
 */
#define HTTPSERVER_STRING_REQUEST_OPTIONS    "OPTIONS"

/**
 * @ingroup httpServer_macros
 */
#define HTTPSERVER_STRING_REQUEST_HEAD       "HEAD"

/**
 * @ingroup httpServer_macros
 */
#define HTTPSERVER_STRING_REQUEST_DELETE     "DELETE"

/**
 * @ingroup httpServer_macros
 */
#define HTTPSERVER_STRING_REQUEST_TRACE      "TRACE"

/**
 * @ingroup httpServer_macros
 */
#define HTTPSERVER_STRING_REQUEST_CONNECT    "CONNECT"

/**
 * @ingroup httpServer_functions
 */
typedef enum
{
    ///GET request
    HTTPSERVER_REQUEST_GET,
    ///POST request
    HTTPSERVER_REQUEST_POST,
    ///PUT request
    HTTPSERVER_REQUEST_PUT,
    ///OPTIONS request
    HTTPSERVER_REQUEST_OPTIONS,
    ///HEAD request
    HTTPSERVER_REQUEST_HEAD,
    ///DELETE request
    HTTPSERVER_REQUEST_DELETE,
    ///TRACE request
    HTTPSERVER_REQUEST_TRACE,
    ///CONNECT request
    HTTPSERVER_REQUEST_CONNECT,

} HttpServer_Request;

/**
 * @ingroup httpServer_macros
 * Macro to improve firmware legibility
 */
#define HTTPSERVER_STRING_VERSION_1_0        "HTTP/1.0"

/**
 * @ingroup httpServer_macros
 * Macro to improve firmware legibility
 */
#define HTTPSERVER_STRING_VERSION_1_1        "HTTP/1.1"

/**
 * @ingroup httpServer_functions
 */
typedef enum
{
    ///Http version 1.0
    HTTPSERVER_VERSION_1_0,
    ///Http version 1.1
    HTTPSERVER_VERSION_1_1,

} HttpServer_Version;

typedef enum
{
    HTTPSERVER_RESPONSECODE_CONTINUE,                           // 100
    HTTPSERVER_RESPONSECODE_SWITCHINGPROTOCOLS,                 // 101
    HTTPSERVER_RESPONSECODE_OK,                                 // 200
    HTTPSERVER_RESPONSECODE_CREATED,                            // 201
    HTTPSERVER_RESPONSECODE_ACCEPTED,                           // 202
    HTTPSERVER_RESPONSECODE_NONAUTHORITATIVEINFORMATION,        // 203
    HTTPSERVER_RESPONSECODE_NOCONTENT,                          // 204
    HTTPSERVER_RESPONSECODE_RESETCONTENT,                       // 205
    HTTPSERVER_RESPONSECODE_PARTIALCONTENT,                     // 206
    HTTPSERVER_RESPONSECODE_MULTIPLECHOICES,                    // 300
    HTTPSERVER_RESPONSECODE_MOVEDPERMANENTLY,                   // 301
    HTTPSERVER_RESPONSECODE_FOUND,                              // 302
    HTTPSERVER_RESPONSECODE_SEEOTHER,                           // 303
    HTTPSERVER_RESPONSECODE_NOTMODIFIED,                        // 304
    HTTPSERVER_RESPONSECODE_USEPROXY,                           // 305
    HTTPSERVER_RESPONSECODE_TEMPORARYREDIRECT,                  // 307
    HTTPSERVER_RESPONSECODE_BADREQUEST,                         // 400
    HTTPSERVER_RESPONSECODE_UNAUTHORIZED,                       // 401
    HTTPSERVER_RESPONSECODE_PAYMENTREQUIRED,                    // 402
    HTTPSERVER_RESPONSECODE_FORBIDDEN,                          // 403
    HTTPSERVER_RESPONSECODE_NOTFOUND,                           // 404
    HTTPSERVER_RESPONSECODE_METHODNOTALLOWED,                   // 405
    HTTPSERVER_RESPONSECODE_NOTACCEPTABLE,                      // 406
    HTTPSERVER_RESPONSECODE_PROXYAUTHREQUIRED,                  // 407
    HTTPSERVER_RESPONSECODE_REQUESTTIMEOUT,                     // 408
    HTTPSERVER_RESPONSECODE_CONFLICT,                           // 409
    HTTPSERVER_RESPONSECODE_GONE,                               // 410
    HTTPSERVER_RESPONSECODE_LENGTHREQUIRED,                     // 411
    HTTPSERVER_RESPONSECODE_PRECONDITIONFAILED,                 // 412
    HTTPSERVER_RESPONSECODE_REQUESTENTITYTOOLARGE,              // 413
    HTTPSERVER_RESPONSECODE_REQUESTURITOOLARGE,                 // 414
    HTTPSERVER_RESPONSECODE_UNSUPPORTEDMEDIATYPE,               // 415
    HTTPSERVER_RESPONSECODE_REQUESTEDRANGENOTSATISFIABLE,       // 416
    HTTPSERVER_RESPONSECODE_EXPECTATIONFAILED,                  // 417
    HTTPSERVER_RESPONSECODE_INTERNALSERVERERROR,                // 500
    HTTPSERVER_RESPONSECODE_NOTIMPLEMENTED,                     // 501
    HTTPSERVER_RESPONSECODE_BADGATEWAY,                         // 502
    HTTPSERVER_RESPONSECODE_SERVICEUNAVAILABLE,                 // 503
    HTTPSERVER_RESPONSECODE_GATEWAYTIMEOUT,                     // 504
    HTTPSERVER_RESPONSECODE_HTTPVERSIONNOTSUPPORTED,            // 505
} HttpServer_ResponseCode;

typedef struct _HttpServer_Message
{
    ///Request type enum
    HttpServer_Request request;
    ///Request version enum
    HttpServer_Version version;

    ///Array of char where URI request is stored
    char uri[HTTPSERVER_MAX_URI_LENGTH+1];
    ///Array of char where headers of the request are stored
    char header[HTTPSERVER_HEADERS_MAX_LENGTH+1];

    ///Enum which contains the response code
    HttpServer_ResponseCode responseCode;

} HttpServer_Message, *HttpServer_MessageHandle;

typedef struct _HttpServer_Client
{
    ///Receive buffer where receiving data is stored
    uint8_t rxBuffer[HTTPSERVER_RX_BUFFER_DIMENSION+1];
    ///Trasmission buffer where sending data is store
    uint8_t txBuffer[HTTPSERVER_TX_BUFFER_DIMENSION+1];
    ///Receive buffer index
    uint16_t rxIndex;

    ///Incoming message are save as @ref HttpServer_Message
    HttpServer_Message message;

} HttpServer_Client, *HttpServer_ClientHandle;

typedef enum
{
    ///Everithing gone well
    HTTPSERVER_ERROR_OK,
    ///Everithing gone well but an empty line is arrived
    HTTPSERVER_ERROR_OK_EMPTYLINE,
    ///Wrong port
    HTTPSERVER_ERROR_WRONG_PORT,
    ///Wrong socket numer
    HTTPSERVER_ERROR_WRONG_SOCKET_NUMBER,
    ///Wrong client number
    HTTPSERVER_ERROR_WRONG_CLIENT_NUMBER,
    ///Server not started
    HTTPSERVER_ERROR_OPEN_FAIL,
    HTTPSERVER_ERROR_WRONG_PARAM,
    ///Timeout
    HTTPSERVER_ERROR_TIMEOUT,
    ///Wrong request format
    HTTPSERVER_ERROR_WRONG_REQUEST_FORMAT,
    ///URI too long
    HTTPSERVER_ERROR_URI_TOO_LONG,
} HttpServer_Error;

typedef struct _HttpServer_Device
{
    ///Port number.
    uint16_t port;
    ///Socket number.
    uint8_t socketNumber;
    ///The pointer to the EthernetConfig previously declared.
    EthernetSocket_Config* ethernetSocketConfig;
    ///Array of @ref HttpServer_Client .
    HttpServer_Client clients [ETHERNET_MAX_LISTEN_CLIENT];
    ///A void pointer which is going to pass to @ref performingCallback .
    void* appDevice;

    ///The callback function it will be call if a request arrived.
    HttpServer_Error (*performingCallback)(void* appDevice,HttpServer_MessageHandle message);

} HttpServer_Device, *HttpServer_DeviceHandle;


extern const char HttpServer_responseCode[40][36];

/**
 * @ingroup httpServer_functions
 * This function starts the HTTP server.
 * @param server The server which you want to start at the determined port, number and ethernet config.
 * @return HTTPSERVER_ERROR_OK if the socket was open and everything is ok, error otherwise.
 */
HttpServer_Error HttpServer_open (HttpServer_DeviceHandle dev);

/**
 * @ingroup httpServer_functions
 * This is a polling function which MUST be called in loop.
 * @param server The server pointer where you want to perform polling.
 */
void HttpServer_poll (HttpServer_DeviceHandle dev);

/**
 * @ingroup httpServer_functions
 * This function sends a HTTP response to the selected client.
 * @param dev The server pointer.
 * @param code The HTTP response code which it is going to send to the client.
 * @param[in] The char pointer to the heades string which it is going to send
 * to the client.
 * @param[in] The number of the client where the message it is going to send.
 */

void HttpServer_sendResponse(HttpServer_DeviceHandle dev,
                             HttpServer_ResponseCode code,
                             char* headers,
                             uint8_t client);
#endif // __OHILAB_HTTPSERVER_H
