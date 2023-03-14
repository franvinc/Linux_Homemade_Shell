
#include "header.h"

void message(){
    //On affiche le repertoire actuel
    char* shell_msg= calloc(1,MAX_MSG_SIZE);
    write(STDOUT_FILENO, strcat(strcat(strcat(shell_msg,"enseashell"),getcwd(NULL,0)),"$ "),MAX_MSG_SIZE);
}

char** break_to_pipe(char * str){
    //    //Les commandes user sont entrées sous formes de string
    //    //Puis elles sont divisées en utilisant le pipe comme délimiteur
    //    //La fonction resort alors les sous chaines correspondantes
    char ** pipe = (char **) malloc(100 * sizeof(char *));
    char *ptr;
    ptr = strtok(str,"|");
    int i;
    for (i = 0; ptr != NULL; i++) {
        pipe[i] = ptr;
        ptr = strtok(NULL,"|");
    }
    pipe[i] = NULL;
    return pipe;
}


char** break_to_line(char * str){//Decoupe un string est tableau de sous chaines en utilisant des espaces comme délimiteur
    char **res = (char **) malloc(100 * sizeof(char *));

    int i = 0;
    char *ptr;
    char *mystring = (char *)malloc(100);
    mystring = strcpy(mystring,str);
    ptr = strtok(mystring," \t\n");
    while(ptr!=NULL){
        res[i] = ptr;
        i++;
        ptr = strtok(NULL," \t\n");
    }
    res[i] = NULL;
    return res;
}

char *read_usr_input(void){
    char *cmd = (char*) malloc(MAX_CMD_SIZE*sizeof (char));

    if(!fgets(cmd,MAX_CMD_SIZE,stdin)){
        perror("Excessive command size");
        exit(EXIT_FAILURE);
    }
    //On vérifie que l'utilisateur a bien rentré une commande
    //Si ce n'est pas le cas on renvoie NULL

    if (strlen(cmd) == 0 || cmd[0] == '\0') {

        return NULL;  // l'utilisateur n'a rien rentré
    }
    int i;
    for (i = 0; cmd[i] != '\0'; i++) {
        if (!isspace(cmd[i])) {
            return cmd;
        }
    }

    return NULL;
}




int output_redirection(char * pipe){
    //On cherche un chevron <
    char** ptr = break_to_line(pipe);
    for (int i = 0; ptr[i] != NULL; i++) {
        if (strcmp(ptr[i], "<") == 0) {
            return 1;
        }
    }
    return 0;

}

int input_redirection(char* pipe) {
    //On cherche un chevron >
    char** ptr = break_to_line(pipe);
    for (int i = 0; ptr[i] != NULL; i++) {
        if (strcmp(ptr[i], ">") == 0) {
            return 1;
        }
    }
    return 0;
}

int pipes_size(char** pipes) {
    int k ;
    for ( k = 0; pipes[k] != NULL; k++);
    return k;
}

void run_usr_command ( char *pipe,int fd0, int fd1)
{   //Cette fonction exécute chaque sous commande dans un processus fils
    struct timespec start, end;//Ces structs vont nous servir à mesurer les temps d'execution de nos commandes

    char **ptr = break_to_line(pipe);
    int status;
    clock_gettime(CLOCK_MONOTONIC,&start);
    pid_t pid=fork();
    if(pid < 0){//On verifie que le fork se passe bien
        perror("error while forking");
        exit(EXIT_FAILURE);
    }
    //Ici on utilise dup2 pour rediriger les inputs/outputs standards vers nos fichiers
    if (pid == 0){//On vérifie que l'on est dans le processus fils
        if (fd0 != 0) {

            dup2 (fd0, 0);
            close (fd0);
        }

        if (fd1 != 1){
            dup2 (fd1, 1);
            close (fd1);
        }

        execvp(ptr[0],ptr);


    }
    else{


        wait(&status);//On attend l'execution du processus fils
        char str[100];//Ci-dessosu on récupère le code de sortie du processus fils
        sprintf(str,"exit:%d",WEXITSTATUS(status));
        //Ci-dessous on mesure le temps d'execution de la commande
        //Cette façon de faire à l'inconvénient de renvoyer le temps d'exec de chaque sous pipe
        write(STDOUT_FILENO, str,strlen(str));
        clock_gettime(CLOCK_MONOTONIC,&end);

        long time_ms = ((end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec)) / 1000000;
        char output[100];
        sprintf(output,"%ld ms ",time_ms);
        write(STDOUT_FILENO," ",strlen(" "));
        write(STDOUT_FILENO,output, strlen(output));
    }
}

char* remove_redirection_operator(char* pipe) {
    char** ptr = break_to_line(pipe);
    char* command = (char*)malloc(100);
    command[0] = '\0';
    for (int i = 0; ptr[i] != NULL; i++) {
        if (strcmp(ptr[i], "<") == 0 || strcmp(ptr[i], ">") == 0) {
            return command;
        }
        strcat(command, " ");
        strcat(command, ptr[i]);
    }
    return command;
}

char* get_my_offset(char* mypipe){
    char **ptr = break_to_line(mypipe);
    for (int i = 0; ptr[i] != NULL; i++) {
        if (strcmp(ptr[i], ">") == 0 || strcmp(ptr[i], "<") == 0) {
            return ptr[i+1];
        }
    }
    return NULL;
}

void cmd_handler(int size,char **mypipes){

    char** ptrs = break_to_line(mypipes[0]);
    int fd1,fd0=0;
    int pipeline[2];
    char * cmd,*input,*output;
    if(strcmp(ptrs[0],"cd") == 0){
        chdir(ptrs[1]);
        //Normalement , nous n'avons accès qu'à notre environnement immédiat
        //On est donc obligé d'implémenter la commande "cd" à la main en utilisant le syscall "chdir"

    }
    if(strcmp(ptrs[0],"exit") == 0) {
        exit(EXIT_SUCCESS);
    }
    else{
        for(int k = 0; k < size; k++){

            pipe(pipeline);
            fd1 = pipeline[1];

            cmd = mypipes[k];

            if(output_redirection(mypipes[k])){

                input= get_my_offset(mypipes[k]);
                cmd = remove_redirection_operator(cmd);
                fd0 = open(input,O_RDONLY);
            }
            if(input_redirection(mypipes[k])){

                output= get_my_offset(mypipes[k]);
                cmd = remove_redirection_operator(cmd);
                fd1 = open(output, O_CREAT | O_WRONLY | O_TRUNC,S_IRWXU);
            }
            if(k == size-1){
                if(!input_redirection(mypipes[k]) ){
                    fd1 = 1;
                }
                run_usr_command(cmd,fd0, fd1);
                close(pipeline[0]);
                close(pipeline[1]);

            }else{

                run_usr_command(cmd,fd0, fd1);
                close (pipeline[1]);

                fd0 = pipeline[0];
            }
        }
    }
}
