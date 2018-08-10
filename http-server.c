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

const char HttpServer_responseCode[40][32] =
        {
                {'C','o','n','t','i','n','u','e','\0'},
                {'S','w','i','t','c','h','i','n','g',' ','P','r','o','t','o','c','o','l','s','\0'},
                {'O','K','\0'},
                {'C','r','e','a','t','e','d','\0'},
                {'A','c','c','e','p','t','e','d','\0'},
                {'N','o','n',' ','A','u','t','h','o','r','i','t','a','t','i','v','e',' ','I','n','f','o','r','m','a','t','i','o','n','\0'},
                {'N','o',' ','C','o','n','t','e','n','t','\0'},
                {'R','e','s','e','t',' ','c','o','n','t','e','n','t','\0'},
                {'P','a','r','t','i','a','l',' ','C','o','n','t','e','n','t','\0'},
                {'M','u','l','t','i','p','l','e',' ','C','h','o','i','c','e','s','\0'},
                {'M','o','v','e','d',' ','P','e','r','m','a','n','e','n','t','l','y','\0'},
                {'F','o','u','n','d','\0'},
                {'S','e','e',' ','o','t','h','e','r','\0'},
                {'N','o','t',' ','M','o','d','i','f','i','e','d','\0'},
                {'U','s','e',' ','P','r','o','x','y','\0'},
                {'T','e','m','p','o','r','a','r','y',' ','r','e','d','i','r','e','c','t','\0'},
                {'B','a','d',' ','r','e','q','u','e','s','t','\0'},
                {'U','n','a','u','t','h','o','r','i','z','e','d','\0'},
                {'P','a','y','m','e','n','t',' ','R','e','q','u','i','r','e','d','\0'},
                {'F','o','r','b','i','d','d','e','n','\0'},
                {'N','o','t',' ','F','o','u','n','d','\0'},
                {'M','e','t','h','o','d',' ','N','o','t',' ','A','l','l','o','w','e','d','\0'},
                {'N','o','t',' ','a','c','c','e','p','t','a','b','l','e','\0'},
                {'P','r','o','x','y',' ','A','u','t','h',' ','R','e','q','u','i','r','e','d','\0'},
                {'R','e','q','u','e','s','t',' ','T','i','m','e','o','u','t','\0'},
                {'C','o','n','f','l','i','c','t','\0'},
                {'G','o','n','e','\0'},
                {'L','e','n','g','t','h',' ','r','e','q','u','i','r','e','d','\0'},
                {'P','r','e',' ','C','o','n','d','i','t','i','o','n',' ','F','a','i','l','e','d','\0'},
                {'R','e','q','u','e','s','t','e','n','t',' ','E','n','t','i','t','y',' ','T','o','o',' ','L','a','r','g','e','\0'},
                {'R','e','q','u','e','s','t',' ','U','R','I',' ','T','o','o',' ','L','a','r','g','e','\0'},
                {'U','n','s','u','p','p','o','r','t','e','d',' ','M','e','d','i','a','T','y','p','e','\0'},
                {'R','e','q','u','e','s','t','e','d',' ','R','a','n','g','e',' ','N','o','t',' ','S','a','t','i','s','f','i','a','b','l','e','\0'},
                {'E','x','p','e','c','t','a','t','i','o','n',' ','F','a','i','l','e','d','\0'},
                {'I','n','t','e','r','n','a','l','s','e','r','v','e','r','e','r','r','o','r','\0'},
                {'N','o','t',' ','I','m','p','l','e','m','e','n','t','e','d','\0'},
                {'B','a','d',' ','G','a','t','e','w','a','y','\0'},
                {'S','e','r','v','i','c','e',' ','U','n','a','v','a','i','l','a','b','l','e','\0'},
                {'G','a','t','e','w','a','y',' ','T','i','m','e','o','u','t','\0'},
                {'H','T','T','P',' ','V','e','r','s','i','o','n',' ','N','o','t',' ','S','u','p','p','o','r','t','e','d','\0'}
        };

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

static HttpServer_Error HttpServer_parseRequest (HttpServer_DeviceHandle dev,
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
#ifdef OHILAB_HTTPSERVER_DEBUG
    char cliBuffer [64];
    uint8_t cliStringLength;
#endif
    uint8_t j = 0;
    char character [2];


    uint8_t nextTimeout = HttpServer_currentTick() + timeout;

    HttpServer_Error error = HTTPSERVER_ERROR_OK;

    for (uint8_t i = 0; i < ETHERNET_MAX_LISTEN_CLIENT; ++i)
    {
        //converting uint8_t to char
        character[0] = i + '0';
        character[1] = '\0';
        // When a client is not connected, jump to the next
        if (!EthernetServerSocket_isConnected(dev->socketNumber,i))
        {
            continue;
        }
#ifdef OHILAB_HTTPSERVER_DEBUG
        strncpy(cliBuffer, "new client #",12);
        strncpy(&cliBuffer[12],character,1);
        strncpy(&cliBuffer[13], " is connected",13);
        strncpy(&cliBuffer[26], '\0',1);
        Cli_sendMessage("HttpServer_poll:",cliBuffer, CLI_MESSAGETYPE_INFO);

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
        // When there isn't message, jump to the next client
        if (received < 0)
        {
            //close the connection for this client
            EthernetServerSocket_disconnectClient(dev->socketNumber,
                                                  i);
#ifdef OHILAB_HTTPSERVER_DEBUG
            strncpy(cliBuffer, "client #",8);
            strncpy(&cliBuffer[7],character,1);
            strncpy(&cliBuffer[8], " is disconnected",15);
            strncpy(&cliBuffer[24], '\0',1);
            Cli_sendMessage("HttpServer_poll:",cliBuffer, CLI_MESSAGETYPE_INFO);
#endif

            continue;
        }
        // Parse the first line of the request
        error = HttpServer_parseRequest(dev,
                                        dev->clients[i].rxBuffer,
                                        received,
                                        i);
        if (error != HTTPSERVER_ERROR_OK)
        {
#ifdef OHILAB_HTTPSERVER_DEBUG
            strncpy(cliBuffer, "client #",8);
            strncpy(&cliBuffer[7],character,1);
            strncpy(&cliBuffer[8], " is disconnected",16);
            strncpy(&cliBuffer[25], '\0',1);
            Cli_sendMessage("HttpServer_poll:",cliBuffer, CLI_MESSAGETYPE_INFO);
            Cli_sendMessage("",character,CLI_MESSAGETYPE_INFO);

#endif
            //Send bad request and disconnect the client!
            HttpServer_sendResponse(dev, HTTPSERVER_RESPONSECODE_BADREQUEST,
                                    "Content-Length: 0\r\nServer: OHILab\r\n\n\r",
                                    i);
            EthernetServerSocket_disconnectClient(dev->socketNumber,
                                                  i);
            // Jump to the next client
            continue;
        }

#ifdef OHILAB_HTTPSERVER_DEBUG
        Cli_sendMessage("HttpServer_poll: ",
                        "First line is parsed",
                        CLI_MESSAGETYPE_INFO);
#endif
        // The request was received
        do
        {

            //put every headers in headerBuffer
            error = HttpServer_getLine(dev,
                                       dev->clients[i].rxBuffer,
                                       HTTPSERVER_RX_BUFFER_DIMENSION,
                                       3000,
                                       i,
                                       &received);

            if (error == HTTPSERVER_ERROR_OK)
            {
                    if((received + j) < HTTPSERVER_HEADERS_MAX_LENGTH)
                    {
                        strncpy(&dev->clients[i].message.header[j],
                                dev->clients[i].rxBuffer,
                                received);
                        j += received;
                    }
                    else
                    {
#ifdef OHILAB_HTTPSERVER_DEBUG
                        Cli_sendMessage("HttpServer_poll:",
                                        "header buffer too short",
                                        CLI_MESSAGETYPE_INFO);
#endif
                    }
                }
#if 0
//                    received += 3;
                    {

                        switch(emptyLineCounter)
                        {
                        case 0:
                            received += 3;
                            if((received + j) < HTTPSERVER_HEADERS_MAX_LENGTH)
                            {
                                strncpy(&dev->clients[i].message.header[j],
                                        dev->clients[i].rxBuffer,
                                        received);
                                j += received;
                            }
                            else
                            {
#ifdef OHILAB_HTTPSERVER_DEBUG
                                Cli_sendMessage("HttpServer_poll:",
                                                "header buffer too short",
                                                CLI_MESSAGETYPE_INFO);
#endif
                            }

                            break;
                        case 1:
                            //received += 2;
                            if ((received + k) < HTTPSERVER_BODY_MESSAGE_MAX_LENGTH)
                            {
                                strncpy(&dev->clients[i].message.body[k],
                                    dev->clients[i].rxBuffer,
                                    received);
                                k += received;
                            }
                            else
                            {
#ifdef OHILAB_HTTPSERVER_DEBUG
                                Cli_sendMessage("HttpServer_poll:",
                                                "body buffer too short",
                                                CLI_MESSAGETYPE_INFO);
#endif
                            }
                        }

                    }
#endif
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
                    HttpServer_sendResponse(dev,HTTPSERVER_RESPONSECODE_NOTFOUND,
                                            "Content-Length: 0\r\nServer: OHILab\r\n\n\r",
                                            i);
                    EthernetServerSocket_disconnectClient(dev->socketNumber,
                                                          i);
#ifdef OHILAB_HTTPSERVER_DEBUG
                    strncpy(cliBuffer, "client #",8);
                    strncpy(&cliBuffer[7],character,1);
                    strncpy(&cliBuffer[8], " is disconnected",15);
                    strncpy(&cliBuffer[24], '\0',1);
                    Cli_sendMessage("HttpServer_poll:",cliBuffer, CLI_MESSAGETYPE_INFO);
#endif
                }
        }while (received > 0);
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
                    i--;
                    break;
                }

            i++;
//            if (buffer[i] == '\n')
//            {
//                // Clear last char
//                buffer[i] = '\0';
//                --i;
//                break;
//            }
//            i++;
        }
    }

    // Check if timeout occur
    if (nextTimeout <= HttpServer_currentTick())
    {
        *received = -1;
#ifdef OHILAB_HTTPSERVER_DEBUG
            Cli_sendMessage("HttpServer_getLine: ",
                            "timeout",
                            CLI_MESSAGETYPE_INFO);
#endif
        return HTTPSERVER_ERROR_TIMEOUT;
    }
//    Clear '\r'
    if ((i > 0) && (buffer[i] == '\r'))
    {
//        buffer[i] = '\0';
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

static HttpServer_Error HttpServer_parseRequest (HttpServer_DeviceHandle dev,
                                                   char* buffer,
                                                   uint16_t length,
                                                   uint8_t client)
{
    char tmp[256];
    uint8_t numArgs = 0;
    uint16_t argCounter = 0;

    // Add end string to \r character
    buffer[length+1] = '\0';
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
                    Cli_sendMessage("HttpServer_parseFirstLine: ",
                                    "URI too long",
                                    CLI_MESSAGETYPE_INFO);

#endif
                    HttpServer_sendResponse(dev,
                                          HTTPSERVER_RESPONSECODE_REQUESTURITOOLARGE,
                                          "Content-Length: 0\r\nServer: OHILab\r\n\n\r",
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
    sprintf(dev->clients[client].txBuffer,
            "HTTP/1.1 %d Error\r\nContent-Length: 0\r\nServer: OHILab\r\n\n\r",
            code);
    EthernetServerSocket_writeBytes(dev->socketNumber,
                                    client,
                                    dev->clients[client].txBuffer,
                                    strlen(dev->clients[client].txBuffer),
                                    &wrote);
}

void HttpServer_sendResponse(HttpServer_DeviceHandle dev,
                             HttpServer_ResponseCode code,
                             char* headers,
                             uint8_t client)
{
    uint8_t bufferLength = 0;
    uint8_t bufferResponseCodeLenght = 0;
    uint16_t wrote = 0;
    sprintf(dev->clients[client].txBuffer,
            "HTTP/1.1 %d ",
            code);
    bufferLength = strlen(dev->clients[client].txBuffer);
    bufferResponseCodeLenght = strlen(HttpServer_responseCode[code]);
    strncpy(&dev->clients[client].txBuffer[bufferLength],
            &HttpServer_responseCode[code][0],
            bufferResponseCodeLenght);

    strncpy(&dev->clients[client].txBuffer[strlen(dev->clients[client].txBuffer)],"\r\n",2);

    strncpy(&dev->clients[client].txBuffer[strlen(dev->clients[client].txBuffer)],headers,strlen(headers));
    EthernetServerSocket_writeBytes(dev->socketNumber,
                                    client,
                                    dev->clients[client].txBuffer,
                                    strlen(dev->clients[client].txBuffer),
                                    &wrote);
}
