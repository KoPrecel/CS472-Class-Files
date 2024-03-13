#pragma once

#define PROG_MD_CLI 0
#define PROG_MD_SVR 1
#define DEF_PORT_NO 2080
#define FNAME_SZ 150
#define PROG_DEF_FNAME "test.c"
#define PROG_DEF_SVR_ADDR "127.0.0.1"

typedef struct prog_config
{
    int prog_mode;
    int port_number;
    uint32_t proto : 4,
        ver : 4,
        status : 8,
        error : 8;
    char svr_ip_addr[16];
    char file_name[128];
} prog_config;

/*
 * Drexel FTP DPU
 */
#define DF_VER 1

#define DF_STATUS_OK 1
#define DF_STATUS_IDLE 2
#define DF_STATUS_PROCESSING 4
#define DF_STATUS_FAIL 8
#define DF_STATUS_CLOSE 16

#define DF_NO_ERROR 0
#define DF_ERROR_GENERAL -1
#define DF_ERROR_NOT_CONNECTED -2
#define DF_ERROR_BAD_PORT -4
#define DF_ERROR_FILE_READ -8
#define DF_ERROR_FILE_WRITE -16
#define DF_ERROR_PROGRAM_MODE -32
