/*******************************************************************************
 * A simple HTTP/SERVER library
 * Copyright (C) 2018 A. C. Open Hardware Ideas Lab
 *
 * Authors:
 *  Marco Giammarini <m.giammarini@warcomeb.it>
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
 ******************************************************************************/


#ifndef __OHILAB_HTTPSERVER_H
#define __OHILAB_HTTPSERVER_H

#define OHILAB_HTTPSERVER_LIBRARY_VERSION     "1.0.0"
#define OHILAB_HTTPSERVER_LIBRARY_VERSION_M   1
#define OHILAB_HTTPSERVER_LIBRARY_VERSION_m   0
#define OHILAB_HTTPSERVER_LIBRARY_VERSION_bug 0
#define OHILAB_HTTPSERVER_LIBRARY_TIME        0

#include "libohiboard.h"

#include "ethernet-socket/ethernet-socket.h"
#include "ethernet-socket/ethernet-serversocket.h"

/*
 * The user must define these label... TODO
 */
#ifndef __NO_BOARD_H
#include "board.h"
#endif

#ifndef HTTPSERVER_MAX_URI_LENGTH
#define HTTPSERVER_MAX_URI_LENGTH               100
#endif
#ifndef HTTPSERVER_BUFFER_DIMENSION
#define HTTPSERVER_BUFFER_DIMENSION             255
#endif
#ifndef HTTPSERVER_HEADERS_MAX_LENGTH
#define HTTPSERVER_HEADERS_MAX_LENGTH           1023
#endif
#ifndef HTTPSERVER_BODY_MESSAGE_MAX_LENGTH
#define HTTPSERVER_BODY_MESSAGE_MAX_LENGTH      1023
#endif


#define HTTPSERVER_STRING_REQUEST_GET        "GET"
#define HTTPSERVER_STRING_REQUEST_POST       "POST"
#define HTTPSERVER_STRING_REQUEST_PUT        "PUT"
#define HTTPSERVER_STRING_REQUEST_OPTIONS    "OPTIONS"
#define HTTPSERVER_STRING_REQUEST_HEAD       "HEAD"
#define HTTPSERVER_STRING_REQUEST_DELETE     "DELETE"
#define HTTPSERVER_STRING_REQUEST_TRACE      "TRACE"
#define HTTPSERVER_STRING_REQUEST_CONNECT    "CONNECT"

typedef enum
{
    HTTPSERVER_REQUEST_GET,
    HTTPSERVER_REQUEST_POST,
    HTTPSERVER_REQUEST_PUT,
    HTTPSERVER_REQUEST_OPTIONS,
    HTTPSERVER_REQUEST_HEAD,
    HTTPSERVER_REQUEST_DELETE,
    HTTPSERVER_REQUEST_TRACE,
    HTTPSERVER_REQUEST_CONNECT,

} HttpServer_Request;

#define HTTPSERVER_STRING_VERSION_1_0        "HTTP/1.0"
#define HTTPSERVER_STRING_VERSION_1_1        "HTTP/1.1"

typedef enum
{
    HTTPSERVER_VERSION_1_0,
    HTTPSERVER_VERSION_1_1,

} HttpServer_Version;

typedef struct _HttpServer_Message
{
    HttpServer_Request request;     /**< Specifies the request type received */
    HttpServer_Version version;/**< Contains the version requested by client */

    char uri[HTTPSERVER_MAX_URI_LENGTH];/**< The uri associated with the request */
    char header[HTTPSERVER_HEADERS_MAX_LENGTH];
    char body[HTTPSERVER_BODY_MESSAGE_MAX_LENGTH];

} HttpServer_Message, *HttpServer_MessageHandle;

typedef struct _HttpServer_Client
{
    uint8_t rxBuffer[HTTPSERVER_BUFFER_DIMENSION+1];
    uint16_t rxIndex;

    HttpServer_Message message;

} HttpServer_Client, *HttpServer_ClientHandle;

typedef enum
{
    HTTPSERVER_ERROR_OK,
    HTTPSERVER_ERROR_OK_EMPTYLINE,
    HTTPSERVER_ERROR_WRONG_PORT,
    HTTPSERVER_ERROR_WRONG_SOCKET_NUMBER,
    HTTPSERVER_ERROR_WRONG_CLIENT_NUMBER,
    HTTPSERVER_ERROR_OPEN_FAIL,
    HTTPSERVER_ERROR_WRONG_PARAM,
    HTTPSERVER_ERROR_TIMEOUT,
    HTTPSERVER_ERROR_WRONG_REQUEST_FORMAT,
    HTTPSERVER_ERROR_URI_TOO_LONG,
} HttpServer_Error;

typedef struct _HttpServer_Device
{
    uint16_t port;
    uint8_t socketNumber;
    EthernetSocket_Config* ethernetSocketConfig;
    HttpServer_Client clients [ETHERNET_MAX_LISTEN_CLIENT];

    HttpServer_Error (*performingCallback)(HttpServer_Message message);

} HttpServer_Device, *HttpServer_DeviceHandle;

typedef enum
{
    HTTPSERVER_RESPONSECODE_CONTINUE                       = 100,  // 100
    HTTPSERVER_RESPONSECODE_SWITCHINGPROTOCOLS             = 101,  // 101
    HTTPSERVER_RESPONSECODE_OK                             = 200,  // 200
    HTTPSERVER_RESPONSECODE_CREATED                        = 201,  // 201
    HTTPSERVER_RESPONSECODE_ACCEPTED                       = 202,  // 202
    HTTPSERVER_RESPONSECODE_NONAUTHORITATIVEINFORMATION    = 203,  // 203
    HTTPSERVER_RESPONSECODE_NOCONTENT                      = 204,  // 204
    HTTPSERVER_RESPONSECODE_RESETCONTENT                   = 205,  // 205
    HTTPSERVER_RESPONSECODE_PARTIALCONTENT                 = 206,  // 206
    HTTPSERVER_RESPONSECODE_MULTIPLECHOICES                = 300,  // 300
    HTTPSERVER_RESPONSECODE_MOVEDPERMANENTLY               = 301,  // 301
    HTTPSERVER_RESPONSECODE_FOUND                          = 302,  // 302
    HTTPSERVER_RESPONSECODE_SEEOTHER                       = 303,  // 303
    HTTPSERVER_RESPONSECODE_NOTMODIFIED                    = 304,  // 304
    HTTPSERVER_RESPONSECODE_USEPROXY                       = 305,  // 305
    HTTPSERVER_RESPONSECODE_TEMPORARYREDIRECT              = 307,  // 307
    HTTPSERVER_RESPONSECODE_BADREQUEST                     = 400,  // 400
    HTTPSERVER_RESPONSECODE_UNAUTHORIZED                   = 401,  // 401
    HTTPSERVER_RESPONSECODE_PAYMENTREQUIRED                = 402,  // 402
    HTTPSERVER_RESPONSECODE_FORBIDDEN                      = 403,  // 403
    HTTPSERVER_RESPONSECODE_NOTFOUND                       = 404,  // 404
    HTTPSERVER_RESPONSECODE_METHODNOTALLOWED               = 405,  // 405
    HTTPSERVER_RESPONSECODE_NOTACCEPTABLE                  = 406,  // 406
    HTTPSERVER_RESPONSECODE_PROXYAUTHREQUIRED              = 407,  // 407
    HTTPSERVER_RESPONSECODE_REQUESTTIMEOUT                 = 408,  // 408
    HTTPSERVER_RESPONSECODE_CONFLICT                       = 409,  // 409
    HTTPSERVER_RESPONSECODE_GONE                           = 410,  // 410
    HTTPSERVER_RESPONSECODE_LENGTHREQUIRED                 = 411,  // 411
    HTTPSERVER_RESPONSECODE_PRECONDITIONFAILED             = 412,  // 412
    HTTPSERVER_RESPONSECODE_REQUESTENTITYTOOLARGE          = 413,  // 413
    HTTPSERVER_RESPONSECODE_REQUESTURITOOLARGE             = 414,  // 414
    HTTPSERVER_RESPONSECODE_UNSUPPORTEDMEDIATYPE           = 415,  // 415
    HTTPSERVER_RESPONSECODE_REQUESTEDRANGENOTSATISFIABLE   = 416,  // 416
    HTTPSERVER_RESPONSECODE_EXPECTATIONFAILED              = 417,  // 417
    HTTPSERVER_RESPONSECODE_INTERNALSERVERERROR            = 500,  // 500
    HTTPSERVER_RESPONSECODE_NOTIMPLEMENTED                 = 501,  // 501
    HTTPSERVER_RESPONSECODE_BADGATEWAY                     = 502,  // 502
    HTTPSERVER_RESPONSECODE_SERVICEUNAVAILABLE             = 503,  // 503
    HTTPSERVER_RESPONSECODE_GATEWAYTIMEOUT                 = 504,  // 504
    HTTPSERVER_RESPONSECODE_HTTPVERSIONNOTSUPPORTED        = 505,  // 505
} HttpServer_ResponseCode;


/**
 * @param server The server which you want to start at the determined port, number and ethernet config
 * @param clients The array of clients which are to be cleared
 * @return HTTPSERVER_ERROR_OK if the socket was open and everything is ok, error otherwise
 */
HttpServer_Error HttpServer_open (HttpServer_DeviceHandle dev);

/**
 *@param[in] server The server pointer which you have previously definited
 *@param[out] clients The array clients which is going to be filled
 */
void HttpServer_poll (HttpServer_DeviceHandle dev, uint16_t timeout);


void HttpServer_sendError (HttpServer_DeviceHandle dev,
                           HttpServer_ResponseCode code,
                           uint8_t client);

#endif // __OHILAB_HTTPSERVER_H
