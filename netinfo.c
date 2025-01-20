#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8888
#define MEM_DYD 1024 //1kB

char *skait(){  
    FILE *failas = fopen("/proc/net/dev", "r");
    char *buferis = malloc(MEM_DYD*4);
    
    size_t baitai = fread(buferis, 1, MEM_DYD*4-1, failas);
    buferis[baitai] = '\0';

    fclose(failas);
    return buferis;
}  


enum MHD_Result answer_to_connection(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
    
    char *info = skait();
    if (!info) {
        const char *klaid_pus = "<html><body>Kažką pridirbau :D</body></html>";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(klaid_pus), (void *)klaid_pus, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return ret;
    }

    char page[MEM_DYD*16];
    snprintf(page, sizeof(page), "<html><head>Internetinio srauto statistikos</head><body><pre>%s</pre></body>", info); //pre, kad išlaikyti esančius \t ir \n /proc/net/dev išvesty
    free(info);

    struct MHD_Response *response;
    enum MHD_Result ret;

    response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
    if (!response)
        return MHD_NO;

    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

int main(){
    struct MHD_Daemon *daemon;
    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_END);

    if (NULL == daemon)
    {
        fprintf(stderr, "Nepavyko paleisti daemono.\n");
        return 1;
    }

    printf("Serveris veikia porte %d. Paspausti ENTER, kad atšaukti\n", PORT);
    getchar();
    MHD_stop_daemon(daemon);

    return 0;
}


