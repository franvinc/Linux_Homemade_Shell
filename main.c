
#include "header.h"


void main(){

    char* mycommand;

while(1){
    //On sort de la boucle avec la commande exit
    message();
    mycommand=read_usr_input();
    if(mycommand!=NULL){
        char** pipes=break_to_pipe(mycommand);

        cmd_handler(pipes_size(pipes),pipes);

    }
}


}
