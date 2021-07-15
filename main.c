#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_LINE_LENGTH 1025
#define MAX_COMMAND_LENGTH 30
#define STATUS_SIZE_OFFSET 10
#define HISTORY_SIZE_OFFSET 1024

/**********************************************************
                        DATA TYPES
**********************************************************/

typedef struct status_node {
    char** status;
    int status_size;
    int count;
}status_node;

typedef struct instant_node{
    status_node* status_n;
    int valid_size;
    int start;
    int alloc;
}instant_node;

typedef enum { false, true } boolean;

/**********************************************************
                        GLOBAL VARIABLES
**********************************************************/

int alert = 0, time=-1, ur_offset = 0, history_size = 0, starting_line, finishing_line, num, mayday = 0, valid_alloc = 0;
instant_node* history;
char* tmp;
char command [MAX_COMMAND_LENGTH];

/**********************************************************
                        GENERIC FUNCTIONS
**********************************************************/

int max (int a, int b){
    if (a > b)
        return a;
    return b;
}

int min (int a, int b){
    if (a < b)
        return a;
    return b;
}

//Chiamo empty_future quando devo svuotare la pila della redo...in pratica svuoto il futuro della linea del tempo
void empty_future (){
    alert = 0;
    time += ur_offset;
    ur_offset = 0;
}

void history_memory_growth(){
    history_size += HISTORY_SIZE_OFFSET;
    history = realloc (history, sizeof(instant_node) * (history_size));
}


void handle_c (){
    int size, i, m;
    char last[3];
    valid_alloc = max(time, valid_alloc);
    if (alert)
        empty_future ();
    time++;
    if (time >= history_size){
        history_memory_growth();
    }

    //CASO 1: è la prima volta che scrivo qualcosa
    if (!mayday && (time == 0 || history[time-1].valid_size == 0)){
        size = (finishing_line / STATUS_SIZE_OFFSET + 1) * STATUS_SIZE_OFFSET;
        if (time > valid_alloc || history[time].alloc == 0) {
            history[time].status_n = malloc(sizeof(status_node));
            history[time].alloc = 1;
        }
        // Se il nodo non è ancora stato allocato, lo alloco
        if (time > valid_alloc || history[time].alloc == 0 || size > history[time].status_n->status_size) {
            history[time].status_n->status = malloc(sizeof(char **) * size);
            history[time].status_n->status_size = size;
        }
        for (i = starting_line; i <= finishing_line; i++)
        {
            if (fgets(tmp, MAX_LINE_LENGTH, stdin)==0)
                return;
            history[time].status_n->status[i] = malloc (sizeof(char) * (strlen(tmp)+1));
            strcpy (history[time].status_n->status[i], tmp);
        }
        history[time].valid_size = finishing_line + 1;
        history[time].start = 0;
        if (fgets (last, 4, stdin)==0)
            return;
        return;
    }
    //CASO 2: mayday = 1, dunque non posso più scrivere o sovrascrivere
    else if (mayday){
        m = max (finishing_line + 1, history[time-1].valid_size);
        size = (m / STATUS_SIZE_OFFSET + 1) * STATUS_SIZE_OFFSET;
        if (time > valid_alloc || history[time].alloc == 0) {
            history[time].status_n = malloc(sizeof(status_node));
            history[time].alloc = 1;
        }

        if (time > valid_alloc || history[time].alloc == 0 || size > history[time].status_n->status_size) {
            history[time].status_n->status = malloc(sizeof(char **) * size);
            history[time].status_n->status_size = size;
        }
        if (starting_line > 0){
            memcpy(history[time].status_n->status, history[time-1].status_n->status, starting_line * sizeof(char**));
        }
        for (i = starting_line; i <= finishing_line; i++){
            if (fgets(tmp, MAX_LINE_LENGTH, stdin)==0)
                return;
            history[time].status_n->status[i] = malloc (sizeof(char) * (strlen(tmp)+1));
            strcpy (history[time].status_n->status[i], tmp);
        }
        if (finishing_line + 1 < history[time-1].valid_size){
            memcpy(&history[time].status_n->status[finishing_line+1], &history[time-1].status_n->status[finishing_line+1], (history[time-1].valid_size - finishing_line - 1) * sizeof(char**));
        }
        history[time].valid_size = m;
        if (fgets (last, 4, stdin)==0)
            return;
        mayday = 0;
        history[time].start = 0;
        return;
    }

        //CASO 3: è una change di tipo 1 ma ho già scritto qualcosa in precedenza, mi occupo di allungare lo stato già creato prima
    else if (starting_line >= history[time-1].valid_size){
        history[time].alloc = 0;
        if (finishing_line + history[time-1].start >= history[time-1].status_n->status_size) {
            size = ((finishing_line + history[time-1].start)/ STATUS_SIZE_OFFSET + 1) * STATUS_SIZE_OFFSET;
            history[time - 1].status_n->status = realloc (history[time - 1].status_n->status, size * sizeof(char**));
            history[time - 1].status_n->status_size = size;
        }
        history[time].status_n = history[time-1].status_n;
        history[time].status_n->count++;
        for (i = starting_line + history[time-1].start; i <= history[time-1].start + finishing_line; i++){
            if (fgets(tmp, MAX_LINE_LENGTH, stdin)==0)
                return;
            history[time].status_n->status[i] = malloc (sizeof(char) * (strlen(tmp)+1));
            strcpy (history[time].status_n->status[i], tmp);
        }
        history[time].valid_size = finishing_line + 1;
        history[time].start = history[time-1].start;
        if (fgets (last, 4, stdin)==0)
            return;
        return;
    }

        //CASO 4: è una change di tipo 2, devo sovrascrivere, dunque risalvo l'intero stato
    else if (starting_line < history[time-1].valid_size){
        m = max (finishing_line + 1, history[time-1].valid_size);
        size = (m / STATUS_SIZE_OFFSET + 1) * STATUS_SIZE_OFFSET;
        if (time > valid_alloc || history[time].alloc == 0) {
            history[time].status_n = malloc(sizeof(status_node));
            history[time].alloc = 1;
        }
        // Se non ho allocato il nodo per lo stato o quello allocato precedentemente è troppo piccolo, lo alloco
        if (time > valid_alloc || history[time].alloc == 0 || size > history[time].status_n->status_size) {
            history[time].status_n->status = malloc(sizeof(char **) * size);
            history[time].status_n->status_size = size;
        }
        if (starting_line > 0){
            memcpy(history[time].status_n->status, &history[time-1].status_n->status[history[time-1].start], starting_line * sizeof(char**));
        }
        for (i = starting_line; i <= finishing_line; i++){
            if (fgets(tmp, MAX_LINE_LENGTH, stdin)==0)
                return;
            history[time].status_n->status[i] = malloc (sizeof(char) * (strlen(tmp)+1));
            strcpy (history[time].status_n->status[i], tmp);
        }
        if (finishing_line + 1 < history[time-1].valid_size){
            memcpy(&history[time].status_n->status[finishing_line+1], &history[time-1].status_n->status[history[time-1].start + finishing_line+1], (history[time-1].valid_size - finishing_line - 1) * sizeof(char**));
        }
        history[time].valid_size = m;
        history[time].start = 0;
        if (fgets (last, 4, stdin)==0)
            return;
    }

}

void handle_d(){
    int offset, m, size;
    valid_alloc = max(time, valid_alloc);

    if (alert)
        empty_future ();
    time++;
    if (time >= history_size)
        history_memory_growth();

    //CASO 1: non ho niente da cancellare perchè ci troviamo al primo stato o perchè lo stato precedente è vuoto
    if (time == 0 || history[time-1].valid_size == 0) {
        history[time].valid_size = 0;
        history[time].start = 0;
        if (time>valid_alloc)
            history[time].alloc = 0;
        return;
    }

    //calcolo la finishing_line effettiva
    finishing_line = min (finishing_line, history[time-1].valid_size - 1);

    //CASO 2: cancello tutto
    if (finishing_line == history[time-1].valid_size - 1 && starting_line == 0){
        history[time].valid_size = 0;
        history[time].start = 0;
        if (time>valid_alloc)
            history[time].alloc = 0;
        return;
    }

    //CASO 3: cancello le ultime celle, dunque accorcio la fine dello stato
    if (finishing_line == history[time-1].valid_size - 1){
        mayday = 1;
        history[time].status_n = history[time-1].status_n;
        history[time].valid_size = starting_line;
        history[time].start = 0;
        history[time].alloc = 0;
        return;
    }

    offset = finishing_line - starting_line +1;

    //CASO 4: cancello l'inizio dello stato, cambio la cella di inizio
    if (starting_line == 0){
        history[time].status_n = history[time-1].status_n;
        history[time].start = history[time-1].start + finishing_line + 1;
        history[time].valid_size = history[time-1].valid_size - offset;
        history[time].alloc = 0;
        return;
    }

    //CASO 5 (generale): devo cancellare qualcosa

    m = history[time-1].valid_size - offset;
    history[time].valid_size = m;
    size = (m / STATUS_SIZE_OFFSET + 1) * STATUS_SIZE_OFFSET;
    if (time > valid_alloc || history[time].alloc == 0) {
        history[time].status_n = malloc(sizeof(status_node));
        history[time].alloc = 1;
    }

    if (time > valid_alloc || history[time].alloc == 0 || size > history[time].status_n->status_size) {
        history[time].status_n->status = malloc(sizeof(char **) * size);
        history[time].status_n->status_size = size;
    }
    history[time].start = 0;
    if (starting_line > 0)
        memcpy (history[time].status_n->status, &history[time-1].status_n->status[history[time-1].start], sizeof(char**) * (starting_line));
    if (finishing_line + 1 < history[time-1].valid_size)
        memcpy (&history[time].status_n->status[starting_line], &history[time-1].status_n->status[history[time-1].start + finishing_line + 1], sizeof(char**) * (history[time-1].valid_size - finishing_line - 1));

}

void handle_p(){
    int i;
    if (history[time+ur_offset].valid_size == 0 || time+ur_offset == -1){
        for (i=starting_line; i<=finishing_line; i++)
            fputs(".\n", stdout);
        return;
    }
    for (i = history[time+ur_offset].start + starting_line; i <= history[time+ur_offset].start + finishing_line; i++){
        if ( i >= history[time+ur_offset].start && i < history[time+ur_offset].start + history[time+ur_offset].valid_size)
            fputs(history[time+ur_offset].status_n->status[i], stdout);
        else
            fputs(".\n", stdout);
    }
}


void handle_u (){
    alert = 1;
    ur_offset -= num;
    if (ur_offset < -time-1)
        ur_offset = -time-1;
}

void handle_r (){
    ur_offset += num;
    if (ur_offset > 0)
        ur_offset = 0;
}

boolean handle_command (){
    char command_chosen;
    if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL)
        return 0;
    command_chosen = command [strlen(command)-2];
    switch (command_chosen){
        case 'c':
            starting_line = atoi (strtok(command, ",")) - 1;
            finishing_line = atoi (strtok(NULL, "c")) - 1;
            handle_c();
            return true;
        case 'd':
            starting_line = atoi (strtok(command, ",")) - 1;
            finishing_line = atoi (strtok(NULL, "d")) - 1;
            handle_d();
            return true;
        case 'p':
            starting_line = atoi (strtok(command, ",")) - 1;
            finishing_line = atoi (strtok(NULL, "p")) - 1;
            handle_p();
            return true;
        case 'u':
            num = atoi (strtok(command, "u"));
            handle_u();
            return true;
        case 'r':
            num = atoi (strtok(command, "r"));
            handle_r();
            return true;
        default:
            return false;
    }
}

/**********************************************************
                             MAIN
**********************************************************/

int main (){
    int on;
    tmp = malloc(MAX_LINE_LENGTH * sizeof(char));
    do{
        on = handle_command();
    }while (on);
    free (tmp);
    return 0;
}
