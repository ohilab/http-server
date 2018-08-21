/*
 * A simple HTTP/RPC library
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

#include "http-server.h"
#include "utility.h"

#ifdef OHILAB_HTTPSERVER_DEBUG
#include "cli/cli.h"
#endif

const char HttpServer_responseCode[40][36] =
        {
                {'1','0','0',' ','C','o','n','t','i','n','u','e','\0'},
                {'1','0','1',' ','S','w','i','t','c','h','i','n','g',' ','P','r','o','t','o','c','o','l','s','\0'},
                {'2','0','0',' ','O','K','\0'},
                {'2','0','1',' ','C','r','e','a','t','e','d','\0'},
                {'2','0','2',' ','A','c','c','e','p','t','e','d','\0'},
                {'2','0','3',' ','N','o','n',' ','A','u','t','h','o','r','i','t','a','t','i','v','e',' ','I','n','f','o','r','m','a','t','i','o','n','\0'},
                {'2','0','4',' ','N','o',' ','C','o','n','t','e','n','t','\0'},
                {'2','0','5',' ','R','e','s','e','t',' ','c','o','n','t','e','n','t','\0'},
                {'2','0','6',' ','P','a','r','t','i','a','l',' ','C','o','n','t','e','n','t','\0'},
                {'3','0','0',' ','M','u','l','t','i','p','l','e',' ','C','h','o','i','c','e','s','\0'},
                {'3','0','1',' ','M','o','v','e','d',' ','P','e','r','m','a','n','e','n','t','l','y','\0'},
                {'3','0','2',' ','F','o','u','n','d','\0'},
                {'3','0','3',' ','S','e','e',' ','o','t','h','e','r','\0'},
                {'3','0','4',' ','N','o','t',' ','M','o','d','i','f','i','e','d','\0'},
                {'3','0','5',' ','U','s','e',' ','P','r','o','x','y','\0'},
                {'3','0','7',' ','T','e','m','p','o','r','a','r','y',' ','r','e','d','i','r','e','c','t','\0'},
                {'4','0','0',' ','B','a','d',' ','r','e','q','u','e','s','t','\0'},
                {'4','0','1',' ','U','n','a','u','t','h','o','r','i','z','e','d','\0'},
                {'4','0','2',' ','P','a','y','m','e','n','t',' ','R','e','q','u','i','r','e','d','\0'},
                {'4','0','3',' ','F','o','r','b','i','d','d','e','n','\0'},
                {'4','0','4',' ','N','o','t',' ','F','o','u','n','d','\0'},
                {'4','0','5',' ','M','e','t','h','o','d',' ','N','o','t',' ','A','l','l','o','w','e','d','\0'},
                {'4','0','6',' ','N','o','t',' ','a','c','c','e','p','t','a','b','l','e','\0'},
                {'4','0','7',' ','P','r','o','x','y',' ','A','u','t','h',' ','R','e','q','u','i','r','e','d','\0'},
                {'4','0','8',' ','R','e','q','u','e','s','t',' ','T','i','m','e','o','u','t','\0'},
                {'4','0','9',' ','C','o','n','f','l','i','c','t','\0'},
                {'4','1','0',' ','G','o','n','e','\0'},
                {'4','1','1',' ','L','e','n','g','t','h',' ','r','e','q','u','i','r','e','d','\0'},
                {'4','1','2',' ','P','r','e',' ','C','o','n','d','i','t','i','o','n',' ','F','a','i','l','e','d','\0'},
                {'4','1','3',' ','R','e','q','u','e','s','t','e','n','t',' ','E','n','t','i','t','y',' ','T','o','o',' ','L','a','r','g','e','\0'},
                {'4','1','4',' ','R','e','q','u','e','s','t',' ','U','R','I',' ','T','o','o',' ','L','a','r','g','e','\0'},
                {'4','1','5',' ','U','n','s','u','p','p','o','r','t','e','d',' ','M','e','d','i','a','T','y','p','e','\0'},
                {'4','1','6',' ','R','e','q','u','e','s','t','e','d',' ','R','a','n','g','e',' ','N','o','t',' ','S','a','t','i','s','f','i','a','b','l','e','\0'},
                {'4','1','7',' ','E','x','p','e','c','t','a','t','i','o','n',' ','F','a','i','l','e','d','\0'},
                {'5','0','0',' ','I','n','t','e','r','n','a','l','s','e','r','v','e','r','e','r','r','o','r','\0'},
                {'5','0','1',' ','N','o','t',' ','I','m','p','l','e','m','e','n','t','e','d','\0'},
                {'5','0','2',' ','B','a','d',' ','G','a','t','e','w','a','y','\0'},
                {'5','0','3',' ','S','e','r','v','i','c','e',' ','U','n','a','v','a','i','l','a','b','l','e','\0'},
                {'5','0','4',' ','G','a','t','e','w','a','y',' ','T','i','m','e','o','u','t','\0'},
                {'5','0','5',' ','H','T','T','P',' ','V','e','r','s','i','o','n',' ','N','o','t',' ','S','u','p','p','o','r','t','e','d','\0'}
        };

typedef uint32_t (*HttpServer_CurrentTick) (void);
typedef void (*HttpServer_Delay) (uint32_t);

static HttpServer_CurrentTick HttpServer_currentTick;
static HttpServer_Delay HttpServer_delay;

/**
 * @ingroup httpServer_functions
 * This function stores the incoming line
 *@param server The server pointer which you have previously definited
 *@param[out] buffer The buffer where the line is going to save
 *@param maxLength The maximum buffer length
 *@param timeout Timeout
 *@param client The number of the listened client
 *@param[out] The number of the character received including \r\n characters
 *@return HTTPSERRVER_ERROR_OK or HTTPSERVER_ERROR_OK_EMPTY_LINE if everything
  gone well other erros otherwise
 */
static HttpServer_Error HttpServer_getLine (HttpServer_DeviceHandle dev,
                                            char* buffer,
                                            uint16_t maxLength,
                                            uint16_t timeout,
                                            uint8_t client,
                                            int16_t* received);

/**
 * @ingroup httpServer_functions
 * This function parses the first line of the request.
 *@param server The server pointer which you have previously definited
 *@param buffer The char pointer of the string it is going to parse
 *@param length The length of the string
 *@param client The client number where parsed message it is going to save
 *@return HTTPSERVER_ERROR_OK if everythine gone well, other errors otherwise.
 */
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

void HttpServer_poll (HttpServer_DeviceHandle dev)
{
    uint8_t data;
    int16_t received = 0;
    uint16_t wrote = 0;
#ifdef OHILAB_HTTPSERVER_DEBUG
    char cliBuffer [64];
    uint8_t cliStringLength;
    char character [2];
#endif
    uint8_t j = 0;
    HttpServer_Error error = HTTPSERVER_ERROR_OK;

    for (uint8_t i = 0; i < ETHERNET_MAX_LISTEN_CLIENT; i++)
    {
#ifdef OHILAB_HTTPSERVER_DEBUG
        //converting uint8_t to char
        character[0] = i + '0';
        character[1] = '\0';
#endif

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
        dev->clients[i].rxIndex = 0;
        // Get first line
        error = HttpServer_getLine(dev,
                                   dev->clients[i].rxBuffer,
                                   HTTPSERVER_RX_BUFFER_DIMENSION,
                                   HTTPSERVER_TIMEOUT,
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
                                       HTTPSERVER_TIMEOUT,
                                       i,
                                       &received);

            if (error == HTTPSERVER_ERROR_OK)
            {
                if((received + dev->clients[i].rxIndex) < HTTPSERVER_HEADERS_MAX_LENGTH)
                {
                    strncpy(&dev->clients[i].message.header[dev->clients[i].rxIndex],
                            dev->clients[i].rxBuffer,
                            received);
                    dev->clients[i].rxIndex += received;
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

            //  If we received an empty line, this would indicate the end of the message

            if (received < 0)
            {
                // Performing the request
                dev->performingCallback(dev->appDevice, &dev->clients[i].message);
#ifdef OHILAB_HTTPSERVER_DEBUG
                Cli_sendMessage("HttpServer_poll:",
                                "performing the request",
                                CLI_MESSAGETYPE_INFO);
#endif
                // Just for test
                HttpServer_sendResponse(dev,
                                        dev->clients[i].message.responseCode,
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

    //Check if parameters make sense
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
    //Check if timeout occur or the received line is too long
    while ((nextTimeout > HttpServer_currentTick()) && (i < maxLength))
    {
        int16_t available = 0;
        EthernetServerSocket_available(dev->socketNumber,client, &available);
        if (available != 0)
        {
            EthernetServerSocket_read(dev->socketNumber,client,&buffer[i]);
            if (buffer[i] == '\n')
                {
                // \n character is not counted
                    i--;
                    break;
                }
            i++;
        }
    }

    // Check if timeout occur, if yes send the corrisponding error
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
    if ((i > 0) && (buffer[i] == '\r')) i--;

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

void HttpServer_sendResponse(HttpServer_DeviceHandle dev,
                             HttpServer_ResponseCode code,
                             char* headers,
                             uint8_t client)
{
    uint8_t bufferLength = 0;
    uint8_t bufferResponseCodeLenght = 0;
    uint16_t wrote = 0;
    //Add to the buffer the HTTP versionf
    sprintf(dev->clients[client].txBuffer,"HTTP/1.1 ");
    bufferLength = strlen(dev->clients[client].txBuffer);
    //Add to the Buffer the response Code
    bufferResponseCodeLenght = strlen(HttpServer_responseCode[code]);
    strncpy(&dev->clients[client].txBuffer[bufferLength],
            &HttpServer_responseCode[code][0],
            bufferResponseCodeLenght);
    //Add to the buffer the end line
    strncpy(&dev->clients[client].txBuffer[strlen(dev->clients[client].txBuffer)],"\r\n",2);
    //Add to the buffer the headers
    strncpy(&dev->clients[client].txBuffer[strlen(dev->clients[client].txBuffer)],headers,strlen(headers));
    EthernetServerSocket_writeBytes(dev->socketNumber,
                                    client,
                                    dev->clients[client].txBuffer,
                                    strlen(dev->clients[client].txBuffer),
                                    &wrote);
}
