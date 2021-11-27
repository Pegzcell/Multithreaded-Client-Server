#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>

/////////////////////////////
#include <iostream>
#include <assert.h>
#include <tuple>
#include <queue>
#include <map>
#include <iterator>
using namespace std;
/////////////////////////////

//Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define ANSI_RESET "\x1b[0m"

typedef long long LL;

#define pb push_back
#define debug(x) cout << #x << " : " << x << endl
#define part cout << "-----------------------------------" << endl;

///////////////////////////////
#define MAX_CLIENTS 100
#define PORT_ARG 8001
#define KEYS 101
const int initial_msg_len = 256;

////////////////////////////////////

const LL buff_sz = 1048576;
///////////////////////////////////////////////////
pair<string, int> read_string_from_socket(const int &fd, int bytes)
{
    std::string output;
    output.resize(bytes);

    int bytes_received = read(fd, &output[0], bytes - 1);
    debug(bytes_received);
    if (bytes_received <= 0)
    {
        cerr << "Failed to read data from socket. \n";
    }

    output[bytes_received] = 0;
    output.resize(bytes_received);
    // debug(output);
    return {output, bytes_received};
}

int send_string_on_socket(int fd, const string &s)
{
    // debug(s.length());
    int bytes_sent = write(fd, s.c_str(), s.length());
    if (bytes_sent < 0)
    {
        cerr << "Failed to SEND DATA via socket.\n";
    }

    return bytes_sent;
}

///////////////////////////////
pthread_mutex_t cnt_lock, prnt_lock, dict_lock, worker_lock, key_locks[KEYS];
pthread_cond_t cccc = PTHREAD_COND_INITIALIZER;
int queue_size;
queue<string> client_requests;
map<int,string> dictionary;
int M;

typedef struct thread_details
{
    int idx;
} td;
////////////////////////////////////
int wel_socket_fd, client_socket_fds[MAX_CLIENTS], port_number; 
socklen_t clilen;
///////////////////////
int key(char *fn) {
    int r;
    if (!strcmp(fn, "insert"))
        r = 0;
    else if (!strcmp(fn, "delete"))
        r = 1;
    else if (!strcmp(fn, "update"))
        r = 2;
    else if (!strcmp(fn, "concat"))
        r = 3;
    else if (!strcmp(fn, "fetch"))
        r = 4;
    else
        r = -1;
    return r;
}

//////////////////////////////////// FUNCTIONS
string insert(char **y){
    int x = atoi(y[0]);
    if (x != 6){
        return "Incorrect number of arguments";
    }
    int k = atoi(y[3]);
    pthread_mutex_lock(&key_locks[k]);
    map<int, string>::iterator itr = dictionary.find(k);
    if ( itr == dictionary.end() ) {
        dictionary.insert(pair<int, string>(k,y[4]));
        pthread_mutex_unlock(&key_locks[k]);
        return "Insertion successful";
    } else {
        pthread_mutex_unlock(&key_locks[k]);
        return "Key already exists";
    }
}

string del(char **y){
    int x = atoi(y[0]);
    if (x != 5){
        return "Incorrect number of arguments";
    }
    int k = atoi(y[3]);    
    pthread_mutex_lock(&key_locks[k]);
    map<int, string>::iterator itr = dictionary.find(k);
    if ( itr != dictionary.end() ) {
        dictionary.erase(itr);
        pthread_mutex_unlock(&key_locks[k]);
        return "Deletion successful";
    } else {
        pthread_mutex_unlock(&key_locks[k]);
        return "No such key exists";
    }
}

string update(char **y){
    int x = atoi(y[0]);
    if (x != 6){
        return "Incorrect number of arguments";
    }
    int k = atoi(y[3]);
    pthread_mutex_lock(&key_locks[k]);
    map<int, string>::iterator itr = dictionary.find(k);
    if ( itr != dictionary.end() ) {
        dictionary[k] = y[4];
        pthread_mutex_unlock(&key_locks[k]);
        return y[4];
    } else {
        pthread_mutex_unlock(&key_locks[k]);
        return "Key does not exist";
    }
}

string concat(char **y){
    int x = atoi(y[0]);
    if (x != 6){
        return "Incorrect number of arguments";
    }
    int k1 = atoi(y[3]);
    int k2 = atoi(y[4]);
    string temp;
    pthread_mutex_lock(&key_locks[k1]);
    pthread_mutex_lock(&key_locks[k2]);
    map<int, string>::iterator itr1 = dictionary.find(k1);
    map<int, string>::iterator itr2 = dictionary.find(k2);
    if ( itr1 != dictionary.end() && itr2 != dictionary.end()) {
        temp = dictionary[k1];
        dictionary[k1].append(dictionary[k2]);
        dictionary[k2].append(temp); 
        temp = dictionary[k2];
        pthread_mutex_unlock(&key_locks[k1]);
        pthread_mutex_unlock(&key_locks[k2]);
        return temp;
    } else {
        pthread_mutex_unlock(&key_locks[k1]);
        pthread_mutex_unlock(&key_locks[k2]);
        return "Concat failed as at least one of the keys does not exist";
    }
}

string fetch(char **y){
    int x = atoi(y[0]);
    if (x != 5){
        return "Incorrect number of arguments";
    }
    int k = atoi(y[3]);   
    string temp; 
    pthread_mutex_lock(&key_locks[k]);
    map<int, string>::iterator itr = dictionary.find(k);
    if ( itr != dictionary.end() ) {
        temp = dictionary[k];
        pthread_mutex_unlock(&key_locks[k]);
        return temp;
    } else {
        pthread_mutex_unlock(&key_locks[k]);
        return "Key does not exist";
    }
}

string (*fn[])(char **) = {insert, del, update, concat, fetch};
////////////////////////////////////


void handle_connection(int client_socket_fd)
{
    // int client_socket_fd = *((int *)client_socket_fd_ptr);
    //####################################################
    /*
    pthread_mutex_lock(&sckt_lock);
    M = atoi(read_string_from_socket(client_socket_fd, buff_sz).first.c_str());
    pthread_mutex_unlock(&sckt_lock);
    if (M ==0){
        printf("Invalid number of users\n");
        return;
    }
    pthread_mutex_lock(&sckt_lock);
    send_string_on_socket(client_socket_fd, "hello");
    pthread_mutex_unlock(&sckt_lock);
    debug(M);
    */

    int received_num, sent_num;

    /* read message from client */
    int ret_val = 1;

    string cmd;
    tie(cmd, received_num) = read_string_from_socket(client_socket_fd, buff_sz);
    ret_val = received_num;
    // debug(ret_val);
    // printf("Read something\n");
    if (ret_val <= 0)
    {
        // perror("Error read()");
        printf("Server could not read msg sent from client\n");
        close(client_socket_fd);
        printf(BRED "Disconnected from client" ANSI_RESET "\n");
    }
    cout << "Client sent : " << cmd << endl;

    if (cmd == "exit")
    {
        cout << "Exit pressed by client" << endl;
        close(client_socket_fd);
        printf(BRED "Disconnected from client" ANSI_RESET "\n");
    }
    /*
    string msg_to_send_back = "Ack: " + cmd;

    ////////////////////////////////////////
    // "If the server write a message on the socket and then close it before the client's read. Will the client be able to read the message?"
    // Yes. The client will get the data that was sent before the FIN packet that closes the socket.

    int sent_to_client = send_string_on_socket(client_socket_fd, msg_to_send_back);

    // debug(sent_to_client);
    if (sent_to_client == -1)
    {
        perror("Error while writing to client. Seems socket has been closed");
        goto close_client_socket_ceremony;
    }
    */
    ////
    
    pthread_mutex_lock(&cnt_lock);
    //pthread_mutex_lock(&worker_lock);
    client_requests.push(cmd);
    //debug(client_requests.size());
    pthread_cond_signal(&cccc);
    pthread_mutex_unlock(&cnt_lock);
    //pthread_mutex_unlock(&worker_lock);
    
}
////////////////////////////////////
void *worker(void *inp)
{
    int thread_idx = ((struct thread_details *)inp)->idx;
    string to_send = "";
    int sent_to_client;
    //debug(thread_idx);
    while(1){
        pthread_mutex_lock(&cnt_lock);
        char* a;
        char ** words;
        //pthread_mutex_lock(&worker_lock);
        while (client_requests.size() == 0){
            pthread_cond_wait(&cccc, &cnt_lock);
        }
        //pthread_mutex_unlock(&worker_lock);
        //queue_size = client_requests.size();
        pthread_mutex_lock(&prnt_lock);
        cout << "---" << endl;
        debug(thread_idx);
        debug(queue_size);
        cout << "---" << endl;
        a =(char *)calloc(50,1);
        strcpy(a, client_requests.front().c_str());
        client_requests.pop();
        debug(a);
        pthread_mutex_unlock(&prnt_lock);
        //
        pthread_mutex_unlock(&cnt_lock);
        words = (char **) calloc(7, sizeof(char *));
        int x = 1;
        words[0] = (char*)calloc(3,1);
        words[x] = strtok(a, " ");
        while (words[x++] != NULL) {
            words[x] = strtok(NULL, " ");
        }
        if (x <= 5){
            cout<< "Incorrect input" << endl;
            continue;
        }

        strcpy(words[0],(to_string(x-1)).c_str());
        ///
        pthread_mutex_lock(&prnt_lock);
        debug(x);
        for(int i =0; i < x-1;i++){
            debug(words[i]);
        }
        int r = key(words[2]);
        debug(r);
        if (r != -1){ 
            to_send =words[x-2];
            to_send = BYEL + to_send + ":" + to_string(thread_idx) + ":" + fn[r](words) + ANSI_RESET + "\n";
            sent_to_client = send_string_on_socket(client_socket_fds[atoi(words[x-2])], to_send);
            if (sent_to_client == -1)
            {
                send_string_on_socket(client_socket_fds[atoi(words[x-2])], to_send);
                perror("Error while writing to client.");
            }
            close(client_socket_fds[atoi(words[x-2])]);
            printf(BRED "Disconnected from client: %s" ANSI_RESET "\n", words[x-2]);
            sleep(2);
        }
        pthread_mutex_unlock(&prnt_lock);
            ///
    }       
}

//////////////////////
int main(int argc, char *argv[])
{
    if (argc != 2){
        printf("Incorrect number of arguments");
        exit(0);
    }
    ///////////////////THREADS
    const int N = atoi(argv[1]);
    debug(N);
    pthread_t thread_ids_arr[N];

    for (int i = 0; i < N; i++)
    {
        pthread_t curr_tid;
        td *thread_input = (td *)(malloc(sizeof(td)));
        thread_input->idx = i;
        pthread_create(&curr_tid, NULL, worker, (void *)(thread_input));
        thread_ids_arr[i] = curr_tid;
    }
    
    ////////////////////LOCKS initiation
    pthread_mutex_init(&cnt_lock, NULL);
    pthread_mutex_init(&prnt_lock, NULL);
    pthread_mutex_init(&dict_lock, NULL);
    //pthread_mutex_init(&worker_lock, NULL);
    for(int i=0;i<KEYS;i++){
        pthread_mutex_init(&key_locks[i], NULL);
    }
    //////////////////////


    struct sockaddr_in serv_addr_obj, client_addr_obj;
    /////////////////////////////////////////////////////////////////////////
    /* create socket */
    /*
    The server program must have a special door—more precisely,
    a special socket—that welcomes some initial contact 
    from a client process running on an arbitrary host
    */
    //get welcoming socket
    //get ip,port
    /////////////////////////
    wel_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (wel_socket_fd < 0)
    {
        perror("ERROR creating welcoming socket");
        exit(-1);
    }

    //////////////////////////////////////////////////////////////////////
    /* IP address can be anything (INADDR_ANY) */
    bzero((char *)&serv_addr_obj, sizeof(serv_addr_obj));
    port_number = PORT_ARG;
    serv_addr_obj.sin_family = AF_INET;
    // On the server side I understand that INADDR_ANY will bind the port to all available interfaces,
    serv_addr_obj.sin_addr.s_addr = INADDR_ANY;
    serv_addr_obj.sin_port = htons(port_number); //process specifies port

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* bind socket to this port number on this machine */
    /*When a socket is created with socket(2), it exists in a name space
       (address family) but has no address assigned to it.  bind() assigns
       the address specified by addr to the socket referred to by the file
       descriptor wel_sock_fd.  addrlen specifies the size, in bytes, of the
       address structure pointed to by addr.  */

    //CHECK WHY THE CASTING IS REQUIRED
    if (bind(wel_socket_fd, (struct sockaddr *)&serv_addr_obj, sizeof(serv_addr_obj)) < 0)
    {
        perror("Error on bind on welcome socket: ");
        exit(-1);
    }
    //////////////////////////////////////////////////////////////////////////////////////

    /* listen for incoming connection requests */

    listen(wel_socket_fd, MAX_CLIENTS);
    cout << "Server has started listening on the LISTEN PORT" << endl;
    clilen = sizeof(client_addr_obj);

    int check;
    ////////////
    for(int i =0;i<MAX_CLIENTS;i++){
        /*
        cout<<"continue?";
        scanf("%d", &check);
        debug(check);
        if (check == 0) break;
        */
        printf("Waiting for a new client to request for a connection\n");
        client_socket_fds[i] = accept(wel_socket_fd, (struct sockaddr *)&client_addr_obj, &clilen);
        if (client_socket_fds[i] < 0)
        {
            perror("ERROR while accept() system call occurred in SERVER");
            exit(-1);
        }

        printf(BGRN "New client connected from port number %d and IP %s \n" ANSI_RESET, ntohs(client_addr_obj.sin_port), inet_ntoa(client_addr_obj.sin_addr));
        
        handle_connection(client_socket_fds[i]);
        /*
        // printing map dictionary
        map<int, string>::iterator itr;
        cout << "\nThe map dictionary is : \n";
        cout << "\tKEY\tELEMENT\n";
        for (itr = dictionary.begin(); itr != dictionary.end(); ++itr) {
            cout << '\t' << itr->first
            << '\t' << itr->second << '\n';
        }
        cout << endl;
        //
        */
    }
    

    /*
    for (int i = 0; i < N; i++)
    {
        pthread_join(thread_ids_arr[i], NULL);
    }
    */
    /////////////////////LOCKS destruction
    pthread_mutex_destroy(&cnt_lock);
    pthread_mutex_destroy(&prnt_lock);
    pthread_mutex_destroy(&dict_lock);
    //pthread_mutex_destroy(&worker_lock);
    for(int i=0;i<KEYS;i++){
        pthread_mutex_destroy(&key_locks[i]);
    }
    //////////////////////

    close(wel_socket_fd);
    return 0;
}