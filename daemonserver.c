#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include<arpa/inet.h> 
#include <stdlib.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/signal.h>
#include <wait.h>
#include<unistd.h> 

#define PID_PATH "/home/krishh1/daemon/un_server.pid"
#define LOG_PATH "/home/krishh1/daemon/server.log"

void fork_handler(int);
void handle_connection(int);
void handle_child(int sig);
void handle_args(int argc, char *argv[]);
void kill_daemon();
void daemonize();
void stop_server();
void init_socket();

int server_sockfd;
struct sockaddr_in remote;

int main(int argc, char *argv[])
{
        int client_sockfd;
        
        int t;

        if(argc > 1) {handle_args(argc, argv);}

        signal(SIGCHLD, handle_child);
        signal(SIGTERM, stop_server);

        init_socket();

        if(listen(server_sockfd, 5) == -1)
        {
                perror("listen");
                exit(1);
        }

        printf("Listening...\n");
        fflush(stdout);

        for(;;)
        {
                printf("Waiting for a connection\n");
                fflush(stdout);
                t = sizeof(remote);
                if((client_sockfd=accept(server_sockfd, (struct sockaddr *)&remote, (socklen_t*)&t)) == -1)
                {
                        perror("accept");
                        exit(1);
                }

                printf("Accepted connection\n");
                fflush(stdout);
                fork_handler(client_sockfd);
        }
}

void fork_handler(int client_sockfd)
{
       

        switch(fork())
        {
                case -1:
                        perror("Error forking connection handler");
                        break;
                case 0:
                        handle_connection(client_sockfd);
                        exit(0);
                default:
                        break;
        }
}

void handle_connection(int client_sockfd)
{
        char buff[1024];
        unsigned int len;

        printf("Handling connection\n");
        fflush(stdout);

        while(len = recv(client_sockfd, &buff, 1024, 0), len > 0)
                send(client_sockfd, &buff, len, 0);

        close(client_sockfd);
        printf("Done handling\n");
        fflush(stdout);
        exit(0);
}

void handle_child(int sig)
{
        int status;

        printf("Killing child\n");
        fflush(stdout);

        wait(&status);
}


void handle_args(int argc, char *argv[])
{
        if(strcmp(argv[1], "kill") == 0) {kill_daemon();}
        if(strcmp(argv[1], "-D") == 0 || strcmp(argv[1], "--daemon") == 0) {daemonize();}
}

void kill_daemon()
{
        FILE *pidfile;
        pid_t pid;
        if((pidfile=fopen(PID_PATH, "r")))
        {
                fscanf(pidfile, "%d", &pid);
                printf("Killing PID %d\n", pid);
                fflush(stdout);
                kill(pid, SIGTERM);
        }
        else
        {
                printf("un_server not running\n");
                fflush(stdout);
        }
        exit(0);
}

void daemonize()
{
        FILE *pidfile;
        pid_t pid;

        switch(pid = fork())
        {
                case 0:
                        freopen("/dev/null", "r", stdin);
                        freopen(LOG_PATH, "w", stdout);
                        freopen(LOG_PATH, "w", stderr);
                        setsid();
                        chdir("/");
                        break;
                case -1:
                        perror("Failed to fork daemon\n");
                        exit(1);
                default:
                        pidfile = fopen(PID_PATH, "w");
                        fprintf(pidfile, "%d", pid);
                        fflush(stdout);
                        fclose(pidfile);
                        exit(0);
        }
}

void stop_server()
{
        unlink(PID_PATH);
        kill(0, SIGKILL);
        exit(0);
}

void init_socket()
{
        struct sockaddr_in local;
        int len;

        if((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
                perror("Error creating server socket");
                exit(1);
        }

        local.sin_family = AF_INET;
        local.sin_addr.s_addr = INADDR_ANY;
        local.sin_port = htons(8000);
       
        len = sizeof(local);
        if(bind(server_sockfd, (struct sockaddr *)&local, len) == -1)
        {
                perror("binding");
                exit(1);
        }
}
