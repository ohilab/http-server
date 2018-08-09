/*******************************************************************************
 * A simple HTTP/RPC library
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

#include "http-server.h"
#include "utility.h"

#ifdef OHILAB_HTTPSERVER_DEBUG
#include "cli/cli.h"
#endif

//#define HTTPSERVER_BUFFER_DIMENSION 0x3FF

typedef uint32_t (*HttpServer_CurrentTick) (void);
typedef void (*HttpServer_Delay) (uint32_t);

static HttpServer_CurrentTick HttpServer_currentTick;
static HttpServer_Delay HttpServer_delay;
static HttpServer_Error HttpServer_getLine (HttpServer_DeviceHandle dev,
                                            char* buffer,
                                            uint16_t maxLength,
                                            uint16_t timeout,
                                            uint8_t client,
                                            int16_t* received);

static HttpServer_Error HttpServer_parseHeader (HttpServer_DeviceHandle dev,
                                                char* buffer,
                                                uint16_t length,
                                                uint8_t client);

HttpServer_Error HttpServer_open (HttpServer_DeviceHandle dev)
{
    // Check if the port is valid
    if (dev->port == 0)
        return HTTPSERVER_ERROR_WRONG_PORT;

    // Check if the socket exist
    if (dev->socketNumber >= ETHERNET_MAX_SOCKET_SERVER)
        return HTTPSERVER_ERROR_WRONG_SOCKET_NUMBER;

    // Check if user want configure the server
    if (dev->ethernetSocketConfig != 0)
    {
        EthernetServerSocket_init(dev->ethernetSocketConfig);
        // Save callback functions for timing
        HttpServer_currentTick = dev->ethernetSocketConfig->currentTick;
        HttpServer_delay = dev->ethernetSocketConfig->delay;
    }

    // Open the server socket
    if (EthernetServerSocket_connect(dev->socketNumber,dev->port) != ETHERNETSOCKET_ERROR_OK)
    {
#ifdef OHILAB_HTTPSERVER_DEBUG
        Cli_sendMessage("HttpServer_open:",
                        "opening server fail",
                        CLI_MESSAGETYPE_INFO);
#endif
        return HTTPSERVER_ERROR_OPEN_FAIL;
    }

    // Reset all buffer
    for (uint8_t i = 0; i < ETHERNET_MAX_LISTEN_CLIENT; ++i)
            dev->clients[i].rxIndex = 0;

#ifdef OHILAB_HTTPSERVER_DEBUG
    Cli_sendMessage("HttpServer_open:",
                    "server is started",
                    CLI_MESSAGETYPE_INFO);
#endif
    return HTTPSERVER_ERROR_OK;
}

void HttpServer_poll (HttpServer_DeviceHandle dev, uint16_t timeout)
{
    uint8_t data;
    int16_t received = 0;
    uint16_t wrote = 0;
    uint16_t stringLength;

    uint8_t j = 0;
//    uint16_t k = 0;
    char character [2];

    uint8_t nextTimeout = HttpServer_currentTick() + timeout;

    HttpServer_Error error = HTTPSERVER_ERROR_OK;

    for (uint8_t i = 0; i < ETHERNET_MAX_LISTEN_CLIENT; ++i)
    {
        character[0] = i + '0';
        character[1] = '\0';
        // When a client is not connected, jump to the next
        if (!EthernetServerSocket_isConnected(dev->socketNumber,i))
        {
            continue;
        }


#ifdef OHILAB_HTTPSERVER_DEBUG
        Cli_sendMessage("HttpServer_poll:",
                        "new client #",
                        CLI_MESSAGETYPE_INFO);
        Cli_sendMessage("",character,CLI_MESSAGETYPE_INFO);
        Cli_sendMessage("is connected","",CLI_MESSAGETYPE_INFO);
#endif
        // Clear index
        received = 0;
        // Get first line
        error = HttpServer_getLine(dev,
                                   dev->clients[i].rxBuffer,
                                   255,
                                   3000,
                                   i,
                                   &received);
        // When there isn't message, jump to the next
        if (received < 0)
        {
            //clear buffer
            //close connection?
            continue;
        }

        // Parse the header
        error = HttpServer_parseHeader(dev,
                                       dev->clients[i].rxBuffer,
                                       strlen(dev->clients[i].rxBuffer),
                                       i);
        if (error != HTTPSERVER_ERROR_OK)
        {
#ifdef OHILAB_HTTPSERVER_DEBUG
            Cli_sendMessage("HttpServer_poll:",
                            "URI too long",
                            CLI_MESSAGETYPE_INFO);
            Cli_sendMessage("HttpServer_poll:",
                            "disconnecting client #",
                            CLI_MESSAGETYPE_INFO);
            Cli_sendMessage("",character,CLI_MESSAGETYPE_INFO);

#endif
            EthernetServerSocket_disconnectClient(dev->socketNumber,
                                                  i);
            // clear buffer
            // Jump to the next client
            continue;
        }

#ifdef OHILAB_HTTPSERVER_DEBUG
        Cli_sendMessage("HttpServer_poll: ",
                        "parsing the request body...",
                        CLI_MESSAGETYPE_INFO);
#endif
        // The request header was received

        do
        {

            //put everthing in message
            error = HttpServer_getLine(dev,
                                       dev->clients[i].rxBuffer,
                                       HTTPSERVER_BUFFER_DIMENSION,
                                       3000,
                                       i,
                                       &received);

//            if(j < HTTPSERVER_BODY_MESSAGE_LENGTH)
//            {
                stringLength = strlen(dev->clients[i].rxBuffer);
                if((error == HTTPSERVER_ERROR_OK) || (error = HTTPSERVER_ERROR_OK))
                {
                    if( ((stringLength + j) < HTTPSERVER_BODY_MESSAGE_LENGTH) )

                    {
                        strncpy(&dev->clients[i].message.bodyMessage[j],
                                dev->clients[i].rxBuffer,
                                stringLength);
                        j += stringLength + 1;
                    }

                    else
                    {
#ifdef OHILAB_HTTPSERVER_DEBUG
                        Cli_sendMessage("HttpServer_poll:",
                                        "body message buffer too short",
                                        CLI_MESSAGETYPE_INFO);
#endif
                    }
                }
                else
                {
#ifdef OHILAB_HTTPSERVER_DEBUG
                    Cli_sendMessage("HttpServer_poll:",
                                    "body message, timeout occur or wrong parameter",
                                    CLI_MESSAGETYPE_INFO);
#endif

                }


//            }
            //  If we received an empty line, this would indicate the end of the message
                if (received < 0)
                {
            	//checking if timeout occur
//                if(nextTimeout < HttpServer_currentTick())
//                {
//                    EthernetServerSocket_disconnectClient(server->socketNumber, i);
//                    break;
//                }

                // Performing the request
            	//server->performingCallback(server->clients[i].message);

#ifdef OHILAB_HTTPSERVER_DEBUG
                    Cli_sendMessage("HttpServer_poll:",
                                    "performing the request",
                                    CLI_MESSAGETYPE_INFO);
#endif
                    // Just for test
                    HttpServer_sendError(dev,404,i);
                    EthernetServerSocket_disconnectClient(dev->socketNumber,
                                                          i);

#ifdef OHILAB_HTTPSERVER_DEBUG
                    Cli_sendMessage("HttpServer_poll:",
                                    "client disconnected",
                                    CLI_MESSAGETYPE_INFO);
#endif
//                for (k=0; k<257; k++)
//                {
//                    server->clients[i].rxBuffer[k] = 0;
//                }
//
//                break;
                }

            //j++;
            // Otherwise parse the message params

        } while (received > 0);
    }
}

static HttpServer_Error HttpServer_getLine (HttpServer_DeviceHandle dev,
                                            char* buffer,
                                            uint16_t maxLength,
                                            uint16_t timeout,
                                            uint8_t client,
                                            int16_t* received)
{
    int16_t i = 0, count = 0;
    *received = -1; // Default

    if ((buffer == NULL) || (maxLength == 0))
    {

#ifdef OHILAB_HTTPSERVER_DEBUG
        Cli_sendMessage("HttpServer_getLine:",
                        "wrong parameter",
                        CLI_MESSAGETYPE_INFO);
#endif
        return HTTPSERVER_ERROR_WRONG_PARAM;
    }


    uint32_t nextTimeout = HttpServer_currentTick() + timeout;
    while ((nextTimeout > HttpServer_currentTick()) && (i < maxLength))
    {
        if (EthernetServerSocket_available(dev->socketNumber,client) != 0)
        {
            EthernetServerSocket_read(dev->socketNumber,client,&buffer[i]);

            if (buffer[i] == '\n')
            {
                // Clear last char
                buffer[i] == '\0';
                --i;
                break;
            }
            i++;
        }
    }

    // Check if timeout occur
    if (nextTimeout < HttpServer_currentTick())
    {
        *received = -1;
#ifdef OHILAB_HTTPSERVER_DEBUG
            Cli_sendMessage("HttpServer_getLine: ",
                            "timeout",
                            CLI_MESSAGETYPE_INFO);
#endif
        return HTTPSERVER_ERROR_TIMEOUT;
    }

    // Clear '\r'
    if ((i > 0) && (buffer[i] == '\r'))
    {
        buffer[i] = '\0';
        i--;
    }

    // Empty line
    if (i == 0)
    {
        *received = -2;
#ifdef OHILAB_HTTPSERVER_DEBUG
            Cli_sendMessage("HttpServer_getLine: ",
                            "empty line arrived",
                            CLI_MESSAGETYPE_INFO);
#endif
        return HTTPSERVER_ERROR_OK_EMPTYLINE;
    }
    *received = i;
#ifdef OHILAB_HTTPSERVER_DEBUG
            Cli_sendMessage("HttpServer_getLine: ","OK",CLI_MESSAGETYPE_INFO);
#endif
    return HTTPSERVER_ERROR_OK;

}

static HttpServer_Error HttpServer_parseHeader (HttpServer_DeviceHandle dev,
                                                char* buffer,
                                                uint16_t length,
                                                uint8_t client)
{
    char tmp[256];
    uint8_t numArgs = 0;
    uint16_t argCounter = 0;

    // Add end string to the last char
    buffer[length] = '\0';
    // Increase the length to detect and parse the last char
    length++;

    if (client >= ETHERNET_MAX_LISTEN_CLIENT)
        return HTTPSERVER_ERROR_WRONG_CLIENT_NUMBER;

    for (uint16_t i = 0; i < length; ++i)
    {
        if ((buffer[i] == ' ') || (buffer[i] == 0))
        {
            tmp[argCounter] = 0;
            if (numArgs == 0) // Choose request type
            {
                if (strcmp(tmp,HTTPSERVER_STRING_REQUEST_GET) == 0)
                {
                    dev->clients[client].message.request = HTTPSERVER_REQUEST_GET;
                }
                else if (strcmp(tmp,HTTPSERVER_STRING_REQUEST_POST) == 0)
                {
                    dev->clients[client].message.request = HTTPSERVER_REQUEST_POST;
                }
                else
                {
                    return HTTPSERVER_ERROR_WRONG_REQUEST_FORMAT;
                }
            }
            else if (numArgs == 1) // Uri
            {
                if( strlen(tmp) < HTTPSERVER_MAX_URI_LENGTH)
                {
                    strcpy(dev->clients[client].message.uri,tmp);
                }

                else
                {
#ifdef OHILAB_HTTPSERVER_DEBUG
                    Cli_sendMessage("HttpServer_parseHeader: ",
                                    "URI too long",
                                    CLI_MESSAGETYPE_INFO);

#endif
                    HttpServer_sendError (dev,
                                          HTTPSERVER_RESPONSECODE_REQUESTURITOOLARGE,
                                          client);
                    return HTTPSERVER_ERROR_URI_TOO_LONG;

                }
            }
            else if (numArgs == 2) // HTTP Version
            {
                if (strcmp(tmp,HTTPSERVER_STRING_VERSION_1_0) == 0)
                {
                	dev->clients[client].message.version = HTTPSERVER_VERSION_1_0;
                }
                else if (strcmp(tmp,HTTPSERVER_STRING_VERSION_1_1) == 0)
                {
                	dev->clients[client].message.version = HTTPSERVER_VERSION_1_1;
                }
                else
                {
                    return HTTPSERVER_ERROR_WRONG_REQUEST_FORMAT;
                }
            }
            else if (numArgs > 2)
            {
                return HTTPSERVER_ERROR_WRONG_REQUEST_FORMAT;
            }
            // Reset counter for the next argument
            argCounter = 0;
            // Increment argument numbers
            numArgs++;
            continue;
        }
        tmp[argCounter] = buffer[i];
        argCounter++;
    }
    return HTTPSERVER_ERROR_OK;
}

void HttpServer_sendError (HttpServer_DeviceHandle dev,
                           HttpServer_ResponseCode code,
                           uint8_t client)
{
    uint16_t wrote = 0;
    sprintf(dev->clients[client].rxBuffer,
            "HTTP/1.1 %d Error\r\nContent-Length: 0\r\nServer: OHILab\r\n\n\r",
            code);
    EthernetServerSocket_writeBytes(dev->socketNumber,
                                    client,
                                    dev->clients[client].rxBuffer,
                                    strlen(dev->clients[client].rxBuffer),
                                    &wrote);
}
