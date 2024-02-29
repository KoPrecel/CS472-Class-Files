#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <ctype.h>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include "http.h"

//---------------------------------------------------------------------------------
// TODO:  Documentation
//
// Note that this module includes a number of helper functions to support this
// assignment.  YOU DO NOT NEED TO MODIFY ANY OF THIS CODE.  What you need to do
// is to appropriately document the socket_connect(), get_http_header_len(), and
// get_http_content_len() functions.
//
// NOTE:  I am not looking for a line-by-line set of comments.  I am looking for
//        a comment block at the top of each function that clearly highlights you
//        understanding about how the function works and that you researched the
//        function calls that I used.  You may (and likely should) add additional
//        comments within the function body itself highlighting key aspects of
//        what is going on.
//
// There is also an optional extra credit activity at the end of this function. If
// you partake, you need to rewrite the body of this function with a more optimal
// implementation. See the directions for this if you want to take on the extra
// credit.
//--------------------------------------------------------------------------------

char *strcasestr(const char *s, const char *find)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != 0)
    {
        c = tolower((unsigned char)c);
        len = strlen(find);
        do
        {
            do
            {
                if ((sc = *s++) == 0)
                    return (NULL);
            } while ((char)tolower((unsigned char)sc) != c);
        } while (strncasecmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}

char *strnstr(const char *s, const char *find, size_t slen)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0')
    {
        len = strlen(find);
        do
        {
            do
            {
                if ((sc = *s++) == '\0' || slen-- < 1)
                    return (NULL);
            } while (sc != c);
            if (len > slen)
                return (NULL);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}
//---------------------------------------------------------------------------------
// Documentation
// The function below takes host address and port to create a socket, connect, and
// return the socket for other functions to use.
//---------------------------------------------------------------------------------
int socket_connect(const char *host, uint16_t port)
{
    // hostent has host name, alises, host address type, length of address, and list of addresses from the name server
    struct hostent *hp;
    struct sockaddr_in addr;
    int sock;

    // tries to get information of the host by url on the internet. Store the pointer to hp when successful.
    if ((hp = gethostbyname(host)) == NULL)
    {
        herror("gethostbyname");
        return -2;
    }

    // copy the first address in hp to addr up to the length of address
    bcopy(hp->h_addr_list[0], &addr.sin_addr, hp->h_length);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    sock = socket(PF_INET, SOCK_STREAM, 0);

    // negative socket means it was unsuccessful to create a socket
    if (sock == -1)
    {
        perror("socket");
        return -1;
    }

    // attempt to connect on the socket to the addfress
    // if the connction fails, it's an error so return -1
    if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("connect");
        close(sock);
        return -1;
    }

    return sock;
}

//---------------------------------------------------------------------------------
// Documentation
// The function below takes http header data and buffer length to calculate
// the length of the http header.
//---------------------------------------------------------------------------------
int get_http_header_len(char *http_buff, int http_buff_len)
{
    char *end_ptr;
    int header_len = 0;
    // find the end of http header by locating a substring in the header
    // substring is the HTTP_HEADER_END constant
    // and save the pointer (start of the HTTP_HEADER_END) to end_ptr
    end_ptr = strnstr(http_buff, HTTP_HEADER_END, http_buff_len);

    if (end_ptr == NULL)
    {
        fprintf(stderr, "Could not find the end of the HTTP header\n");
        return -1;
    }

    // end_ptr points somewhere in the buffer and http_buff is a pointer to the start of the buffer
    // so subtracting the two pointers give the length of end_ptr
    // and add the constant length since end_ptr was before the constant

    header_len = (end_ptr - http_buff) + strlen(HTTP_HEADER_END);

    return header_len;
}

//---------------------------------------------------------------------------------
// Documentation
// The function below takes http header data and finds content length
//---------------------------------------------------------------------------------
int get_http_content_len(char *http_buff, int http_header_len)
{
    char header_line[MAX_HEADER_LINE];

    // first line is the beginning of the buffer
    char *next_header_line = http_buff;
    // end of buffer is the starting point + length
    char *end_header_buff = http_buff + http_header_len;

    // loop until buffer doesn't run out
    while (next_header_line < end_header_buff)
    {
        // header_line is a buffer we use each loop
        // so use bzero to clear it every loop
        bzero(header_line, sizeof(header_line));

        // read each line into header_line using regex
        // regex finds any non-linebreak characters
        // essentially putting all characters into header_line until linebreak
        sscanf(next_header_line, "%[^\r\n]s", header_line);

        char *isCLHeader2 = strcasecmp(header_line, CL_HEADER);
        // compares the line to see if the line starts with the constant, "Content-Length"
        // case-insensitive
        char *isCLHeader = strcasestr(header_line, CL_HEADER);

        if (isCLHeader != NULL)
        {
            // find where ":" is in the string
            char *header_value_start = strchr(header_line, HTTP_HEADER_DELIM);
            if (header_value_start != NULL)
            {
                // value is the pointer after ":"
                // then convert the value from string to int
                char *header_value = header_value_start + 1;
                int content_len = atoi(header_value);
                return content_len;
            }
        }

        // loop next line by adding pointer value of the line and linebreak
        next_header_line += strlen(header_line) + strlen(HTTP_HEADER_EOL);
    }
    fprintf(stderr, "Did not find content length\n");
    return 0;
}

// This function just prints the header, it might be helpful for your debugging
// You dont need to document this or do anything with it, its self explanitory. :-)
void print_header(char *http_buff, int http_header_len)
{
    fprintf(stdout, "%.*s\n", http_header_len, http_buff);
}

//--------------------------------------------------------------------------------------
// EXTRA CREDIT - 10 pts - READ BELOW
//
// Implement a function that processes the header in one pass to figure out BOTH the
// header length and the content length.  I provided an implementation below just to
// highlight what I DONT WANT, in that we are making 2 passes over the buffer to determine
// the header and content length.
//
// To get extra credit, you must process the buffer ONCE getting both the header and content
// length.  Note that you are also free to change the function signature, or use the one I have
// that is passing both of the values back via pointers.  If you change the interface dont forget
// to change the signature in the http.h header file :-).  You also need to update client-ka.c to
// use this function to get full extra credit.
//--------------------------------------------------------------------------------------
int process_http_header(char *http_buff, int http_buff_len, int *header_len, int *content_len)
{
    int h_len, c_len = 0;
    h_len = get_http_header_len(http_buff, http_buff_len);
    if (h_len < 0)
    {
        *header_len = 0;
        *content_len = 0;
        return -1;
    }
    c_len = get_http_content_len(http_buff, http_buff_len);
    if (c_len < 0)
    {
        *header_len = 0;
        *content_len = 0;
        return -1;
    }

    *header_len = h_len;
    *content_len = c_len;
    return 0; // success
}
