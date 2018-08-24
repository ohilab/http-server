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
 * This library is developed in order to manage a simple
 * HTTP server using the @a libohiboard, @a timer, @ethernet-serversocket libraries.
 *
 * @section changelog ChangeLog
 *
 * @li v1.0.0 of 2018/08/24 - First release
 *
 * @section library External Library
 *
 * The library use the following external library
 * @li libohiboard https://github.com/ohilab/libohiboard a C framework for
 * NXP Kinetis microcontroller
 * @li timer https://github.com/warcomeb/timer a C library to
 * create a timer based on PIT module of libohiboard
 * @li KSX8081RNA https://github.com/Loccioni-Electronic/KSZ8081RNA
 * @li CLI https://github.com/Loccioni-Electronic/cli
 * @li ethernet-socketserver https://github.com/ohilab/ethernet-socket a C
 * library to manage server socket
 *
 * @section Example
 * before starting with the example, which consist only of
 * the main.c file you MUST create <BR>
 * a board.h file in ~/http_example/Includes/board.h
 * which contains all macros in order to improve
 * code legibility.
 * <BR>It could be something
 * like this:
 *
 * @code
 * #include "libohiboard.h"
 *
 *  //macros for timer module
 *  #define WARCOMEB_TIMER_NUMBER        0
 *  #define WARCOMEB_TIMER_FREQUENCY     1000
 *  #define WARCOMEB_TIMER_CALLBACK      5
 *
 *  //macros for ethernet-serversocket module
 *  #define ETHERNET_MAX_LISTEN_CLIENT 5
 *  #define ETHERNET_MAX_SOCKET_BUFFER 1023
 *  #define ETHERNET_MAX_SOCKET_CLIENT 5
 *  #define ETHERNET_MAX_SOCKET_SERVER 5
 *
 *  //macros for http-server module
 *  #define HTTPSERVER_MAX_URI_LENGTH           99
 *  #define HTTPSERVER_HEADERS_MAX_LENGTH       1023
 *  #define HTTPSERVER_BODY_MAX_LENGTH          127
 *  #define HTTPSERVER_RX_BUFFER_DIMENSION      255
 *  #define HTTPSERVER_TX_BUFFER_DIMENSION      255
 *  #define HTTPSERVER_TIMEOUT                  3000
 *  #define OHILAB_HTTPSERVER_MODULE_TEST       1
 *
 *  //macros for CLI module
 *  #define PROJECT_NAME "iot-node_frdmK64"
 *  #define PROJECT_COPYRIGTH "Copyright (C) 2018 AEA s.r.l. Loccioni Group - Elctronic Design Dept."
 *  #define FW_TIME_VERSION 0
 *  #define FW_VERSION_STRING "0.3"
 *  #define PCB_VERSION_STRING "1.0"
 *  #define LOCCIONI_CLI_DEV        OB_UART0
 *  #define LOCCIONI_CLI_RX_PIN     UART_PINS_PTB16
 *  #define LOCCIONI_CLI_TX_PIN     UART_PINS_PTB17
 *  #define LOCCIONI_CLI_BAUDRATE   115200
 *  #define LOCCIONI_CLI_ETHERNET   0
 *
 * @endcode
 * <BR>
 *
 * While the main.c is developed only to test this library so every request
 * have a 400 Bad Request error reply.<BR>
 * It could be something like this
 *
 * @code
 *  //Including all needed libraries
 *  #include "libohiboard.h"
 *  #include "timer/timer.h"
 *  #include "KSZ8081RNA/KSZ8081RNA.h"
 *  #include "cli/cli.h"
 *  #include "http-server/http.server.h"
 *  #include "ethernet-socket/ethernet-serversocket.h"
 *
 *  //declare netif struct type
 *  struct netif nettest;
 *
 *  int main(void)
 *  {
 *      uint32_t fout;
 *      uint32_t foutBus;
 *      Clock_State clockState;
 *
 *      //Declaring EthernetSocket_Config struct
 *      EthernetSocket_Config ethernetSocketConfig=
 *      {
 *          .timeout = 3000,
 *          .delay = Timer_delay,
 *          .currentTick = Timer_currentTick,
 *      };
 *
 *      //Declaring HttpServer_Device
 *      HttpServer_Device httpServer=
 *      {
 *          .port = 80,
 *          .socketNumber = 0,
 *          .ethernetSocketConfig = &ethernetSocketConfig,
 *      };
 *
 *      //Declaring ClockConfig struct
 *      Clock_Config clockConfig =
 *      {
 *          .source         = CLOCK_EXTERNAL,
 *          .fext           = 50000000,
 *          .foutSys        = 120000000,
 *          .busDivider     = 2,
 *          .flexbusDivider = 4,
 *          .flashDivider   = 6,
 *      };
 *
 *      // Enable Clock
 *      System_Errors  error = Clock_Init(&clockConfig);
 *
 *      // JUST FOR DEBUG
 *      clockState = Clock_getCurrentState();
 *      fout = Clock_getFrequency (CLOCK_SYSTEM);
 *      foutBus = Clock_getFrequency (CLOCK_BUS);
 *
 *      // Enable all ports
 *      SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK |
 *                   SIM_SCGC5_PORTB_MASK |
 *                   SIM_SCGC5_PORTC_MASK |
 *                   SIM_SCGC5_PORTD_MASK |
 *                   SIM_SCGC5_PORTE_MASK;
 *
 *      // Network configurations
 *      Ethernet_NetworkConfig netConfig;
 *
 *      // fill net config
 *      IP4_ADDR(&netConfig.ip,192,168,1,6);
 *      IP4_ADDR(&netConfig.mask,255,255,255,0);
 *      IP4_ADDR(&netConfig.gateway,192,168,1,1);
 *      ETHERNET_MAC_ADDR(&netConfig.mac,0x00,0xCF,0x52,0x35,0x00,0x01);
 *
 *      netConfig.phyCallback           = KSZ8081RNA_init;
 *      netConfig.timerCallback         = Timer_currentTick;
 *      netConfig.netif_link_callback   = 0;
 *      netConfig.netif_status_callback = 0;
 *
 *      //Just for test
 *      Gpio_config(GPIO_PINS_PTB22,GPIO_PINS_OUTPUT);
 *      Gpio_config(GPIO_PINS_PTE26,GPIO_PINS_OUTPUT);
 *      Gpio_config(GPIO_PINS_PTB21,GPIO_PINS_OUTPUT);
 *
 *      //The default value is clear, in this way RGB led
 *      //will be OFF
 *      Gpio_set(GPIO_PINS_PTB22);
 *      Gpio_set(GPIO_PINS_PTE26);
 *      Gpio_set(GPIO_PINS_PTB21);
 *
 *      //timer initialization
 *      Timer_init();
 *
 *      //Ethernet server socket initialization
 *      Ethernet_networkConfig(&nettest, &netConfig);
 *
 *      //Http server initialization
 *      HttpServer_open(&httpServer);
 *
 *      //Turn the red LED on, now we can send some HTTP request
 *      Gpio_clear(GPIO_PINS_PTB22);
 *
 *     while(1)
 *     {
 *         sys_check_timeouts();
 *
 *         //A small delay
 *         Timer_delay(100);
 *
 *         //CLI polling function
 *         Cli_check();
 *
 *         //HTTP server polling function to detect request
 *         HttpServer_poll(&httpServer);
 *
 *      }
 *      return 0;
 *  }
 * @endcode
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
#define OHILAB_HTTPSERVER_LIBRARY_TIME        1535124886

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
 * The max length of body which can be send to the client
 */
#ifndef HTTPSERVER_BODY_MAX_LENGTH
#define HTTPSERVER_BODY_MAX_LENGTH          127
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
    ///Array of char where body of the response are stored
    char body[HTTPSERVER_BODY_MAX_LENGTH+1];

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
    ///Wrong port
    HTTPSERVER_ERROR_WRONG_PORT,
    ///Wrong socket numer
    HTTPSERVER_ERROR_WRONG_SOCKET_NUMBER,
    ///Wrong client number
    HTTPSERVER_ERROR_WRONG_CLIENT_NUMBER,
    ///Server not started
    HTTPSERVER_ERROR_OPEN_FAIL,
    ///Timeout
    HTTPSERVER_ERROR_TIMEOUT,
    ///Wrong request format
    HTTPSERVER_ERROR_WRONG_REQUEST_FORMAT,
    ///Everithing gone well but an empty line is arrived
    HTTPSERVER_ERROR_OK_EMPTYLINE,
    ///URI too long
    HTTPSERVER_ERROR_URI_TOO_LONG,
    HTTPSERVER_ERROR_WRONG_PARAM,

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
    HttpServer_Error (*performingCallback)(void* appDevice,
                                           HttpServer_MessageHandle message,
                                           uint8_t clientNumber);

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
 * @param[in] The char pointer to the headers string which it is going to send
 * to the client.
 * @param[in] The char pointer to the body string which is going to send
 * to the client.
 * @param[in] The number of the client where the message it is going to send.
 */

void HttpServer_sendResponse(HttpServer_DeviceHandle dev,
                             HttpServer_ResponseCode code,
                             char* headers,
                             char* body,
                             uint8_t client);
#endif // __OHILAB_HTTPSERVER_H
