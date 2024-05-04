
/*
 * This version of the project "CercaPercorso" exploits Dijkstra's algorithm to find the optimal path
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#define MAX_DIM_PARCO 512
#define MIN_DIM_AUTOSTRADA 64
#define DIM_REALLOC 64
#define UINT_MAX 4294967295
#define DIM_REALLOC_STACK 64
#define DIM_STACK_INIT MIN_DIM_AUTOSTRADA
typedef struct Stazione Stazione;
typedef struct Vertice Vertice;
typedef struct Node Node;

struct Stazione{
    unsigned int distanza;
    unsigned int* autonomie;
};

struct Vertice{
    unsigned int my_index;          //Indice del vertice nell'autostrada
    unsigned int dist_from_src;     //Distanza dal vertice sorgente
    int prec_index;        //Indice del vertice precedente nell'autostrada
    char subset;                    //'q' o 's' a seconda che sia in Q o in S
};

struct Node{
    unsigned int value;
    struct Node* next;
};

Stazione* autostrada = NULL;        //Array di stazioni
unsigned int numeroStazioni = 0;    //Numero di stazioni
unsigned int autostradaAllocata = 0;   //Dimensione effettiva dell'array autostrada
Vertice* Q = NULL;                  //Array di vertici
unsigned int QSize = 0;             //Numero di vertici in Q
unsigned int cardQ = 0;             //Numero di elementi effettivamente in Q (numero di 'q')


/*
 * Restituisce l'indice della stazione che si trova a distanza target
 */
int binary_search(Stazione* autostrada, int startIndex, int endIndex, unsigned int target){
    if(startIndex>endIndex){
        return -1;
    }
    if(autostrada == NULL){         //È inutile perché se autostrada == NULL allora startIndex > endIndex
        return -1;
    }
    int m = (startIndex+endIndex)/2;
    if(autostrada[m].distanza == target){
        return m;
    }else if(autostrada[m].distanza > target){
        return binary_search(autostrada,startIndex,m-1,target);
    }else{
        return binary_search(autostrada,m+1,endIndex,target);
    }
}
/*
 * Restituisce l'indice della stazione dopo la quale va inserita la nuova stazione che si trova a distanza target
 * Restituisce -2 se l'indirizzo è già presente (Potremmo non avere l'indirizzo di tutte le stazioni)
 */
int binary_getInsertPosition(Stazione* autostrada, int startIndex, int endIndex, unsigned int target){
    if(startIndex>endIndex){
        return endIndex; //Perché potrebbe essere startIndex = DIM_AUTOSTRADA oppure endIndex = -1
    }
    int m = (startIndex+endIndex)/2;
    //NOTA: autostrada[i] != NULL per ogni i
    if(autostrada[m].distanza == target) {
        //Esiste già una stazione a distanza target
        return -2;
    }else if(autostrada[m].distanza > target) {
        if (m == 0) {
            return -1; //La stazione va inserita prima della prima stazione, cioè va inserita in indice 0
        } else if (autostrada[m - 1].distanza < target) { //separato dal caso precedente solo per evitare IOB
            return (m-1); //La stazione va inserita tra la stazione m-1 e la stazione m
        } else {
            return binary_getInsertPosition(autostrada, startIndex, m - 1, target);
        }
    }else{
        if(m==endIndex){
            return endIndex; //La stazione va inserita dopo l'ultima stazione
        }else if(autostrada[m+1].distanza > target){ //separato dal caso precedente solo per evitare IOB
            return m; //La stazione va inserita tra la stazione m e la stazione m+1
        }else{
            return binary_getInsertPosition(autostrada,m+1,endIndex,target);
        }
    }
}

/*
 * Restituisce l'indice dell'array di autonomie in cui si trova il valore target,
 * dove l'array autonomie è un array ordinato in ordine decrescente
 */
int binary_searchBackwards(unsigned int* autonomie, int startIndex, int endIndex, unsigned int target){
    if(startIndex>endIndex){
        return -1;
    }
    int m = (startIndex+endIndex)/2;
    if(autonomie[m] == target){
        return m;
    }else if(autonomie[m] > target){
        return binary_searchBackwards(autonomie,m+1,endIndex,target);
    }else{
        return binary_searchBackwards(autonomie,startIndex,m-1,target);
    }
}

/*
 * Restituisce l'indice prima del quale va inserito il nuovo valore effettuando una ricerca binaria,
 * ricevendo come parametro l'array autonomie che è un array ordinato in ordine decrescente
 * Il valore va inserito anche se già presente.
 * Restituisce -2 se l'array autonomie è NULL
 */
int binary_getInsertPositionBackwards(unsigned int* autonomie, int startIndex, int endIndex, unsigned int target){
    if(autonomie == NULL){
        return -2;
    }
    if(startIndex>endIndex){ //Non dovrebbe mai capitare, forse
        //printf("ERRORE: non dovrebbe mai capitare [riga 101] (FORSE)\n"
        return startIndex; //Perché potrebbe essere startIndex = MAX_DIM_PARCO oppure endIndex = -1
    }
    int m = (startIndex+endIndex)/2;
    if(autonomie[m] == target) {
        //Abbiamo già una copia, dobbiamo comunque inserire il nuovo valore
        //Va bene inserirlo qui dove c'è la sua copia, che verrà spostata in avanti
        return m;
    }
    if(autonomie[m] > target) {
        if (m == endIndex) {
            /*
                //Se l'ultimo elemento dell'array, cioè il più piccolo, è maggiore del target allora
                // o il target è un valore illegal e o è un valore di troppo
                printf("ERRORE: Valore di autonomia troppo piccolo o eccedente i %d valori ammessi (vedi consegna)\n", MAX_DIM_PARCO);
            */
            return (endIndex+1); //Trovata una copia, inseriamo comunque il nuovo valore
        } else if (autonomie[m + 1] < target) {
            return (m+1); //Il valore va inserito tra la posizione m e la posizione m+1
        } else {
            return binary_getInsertPositionBackwards(autonomie, m + 1, endIndex, target);
        }
    }else{
        if(m==startIndex){
            /*
                //Se il primo elemento dell'array, cioè il più grande, è minore del target allora
                // o il target è un valore illegale o è un valore di troppo
                printf("ERRORE: Valore di autonomia troppo grande o eccedente i %d valori ammessi (vedi consegna)\n", MAX_DIM_PARCO);
            */
            return startIndex; //Trovata una copia, inseriamo comunque il nuovo valore
        }else if(autonomie[m-1] > target){
            return m; //Il valore va inserito tra la posizione m-1 e la posizione m
        }else{
            return binary_getInsertPositionBackwards(autonomie,startIndex,m-1,target);
        }
    }

}

void aggiungi_auto(unsigned int stazione, unsigned int newAutonomia, char flag){
    //printf(" A: newAutonomia: %d \n", newAutonomia);
    int indice = binary_search(autostrada,0,numeroStazioni-1,stazione);
    //printf(" B: indice = %d e numeroStazioni = %d\n", indice, numeroStazioni);
    if(indice == -1){
        //La stazione non esiste
        if(flag == 't'){
            printf("non aggiunta\n");
        }
        return;
    }
    //La stazione esiste
    unsigned int** autonomie = &(autostrada[indice].autonomie);
    if(*autonomie == NULL){
        //Non abbiamo ancora inserito autonomie per questa stazione
        //autonomie = malloc(sizeof(unsigned int)*MIN_DIM_AUTONOMIE);
        //printf(" C \n");
        autostrada[indice].autonomie = calloc(MAX_DIM_PARCO,sizeof(unsigned int));
        autostrada[indice].autonomie[0] = newAutonomia;
        if(flag == 't'){
            printf("aggiunta\n");
        }
        //printf(" D \n");
    }else {
        //Prima di tutto dobbiamo controllare di non aver già occupato tutti gli spazi (MAX_DIM_PARCO)
        //printf(" E \n");
        if ((*autonomie)[MAX_DIM_PARCO - 1] > 0) {
            //printf(" F \n");
            //Abbiamo già occupato tutti gli spazi
            printf("[Errore]: Stai tentando di aggiungere più di %d autonomie per la stazione %d\n", MAX_DIM_PARCO,
                   stazione);
            return;
        }
        //printf(" G \n");
        //La dimensione è esattamente MAX_DIM_PARCO (per questa prima implementazione)
        //So dal testo che non mi verrà richiesto di inserire più di MAX_DIM_PARCO autonomie per stazione
        int indiceAutonomia = binary_getInsertPositionBackwards((*autonomie), 0, MAX_DIM_PARCO - 1, newAutonomia);
        assert(indiceAutonomia != -2);
        if (indiceAutonomia <= 0) { //TODO: Controlla
            //Quello da inserire è il valore massimo fra tutti quelli inseriti
            //printf(" H \n");
            for (unsigned int i = MAX_DIM_PARCO - 1; i > indiceAutonomia && i > 0; i--) {
                (*autonomie)[i] = (*autonomie)[i - 1];
            }
            //printf(" I \n");
            (*autonomie)[0] = newAutonomia;
            if (flag == 't') {
                printf("aggiunta\n");
            }
            return;
        }
        //indiceAutonomia è l'indice prima del quale va inserito il nuovo valore
        //printf(" J \n");
        for (unsigned int i = MAX_DIM_PARCO - 1; i > indiceAutonomia; i--) {
            (*autonomie)[i] = (*autonomie)[i - 1];
        }
        //printf(" K \n");
        (*autonomie)[indiceAutonomia] = newAutonomia;
        //printf(" L \n");
        if (flag == 't') {
            printf("aggiunta\n");
        }
    }
}

/*
 * Trova l'indice dell'auto nell'array di autonomie della stazione a distanza stazione e lo elimina, spostando di
 * conseguenza tutti gli altri valori
 */
void rottama_auto(unsigned int stazione, unsigned int autonomia_da_rottamare){
    int indice = binary_search(autostrada,0,numeroStazioni-1,stazione);
    if(indice == -1){
        //La stazione non esiste
        printf("non rottamata\n");
        return;
    }
    //La stazione esiste
    unsigned int* autonomie = autostrada[indice].autonomie;
    if(autonomie == NULL){
        //Non abbiamo ancora inserito autonomie per questa stazione
        printf("non rottamata\n");
        return;
    }
    int indiceAutonomia = binary_searchBackwards(autonomie,0,MAX_DIM_PARCO-1,autonomia_da_rottamare);
    if(indiceAutonomia==-1){
        //Non abbiamo trovato l'autonomia da rottamare
        printf("non rottamata\n");
        return;
    }
    //Abbiamo trovato l'autonomia da rottamare
    for(int i=indiceAutonomia;i<MAX_DIM_PARCO-1;i++){
        autonomie[i] = autonomie[i+1];
    }
    autonomie[MAX_DIM_PARCO-1] = 0;
    printf("rottamata\n");
}
/*
 * Produce sempre un array compatto per consentire la binary search
 * Non necessariamente alloca la minima memoria necessaria
 */
char aggiungi_stazione(Stazione s){
    if(autostrada == NULL){
        assert(numeroStazioni==0); //Se autostrada è NULL, numeroStazioni deve essere 0 (non abbiamo stazioni)
        autostrada = malloc(sizeof(Stazione)*MIN_DIM_AUTOSTRADA);
        autostradaAllocata = MIN_DIM_AUTOSTRADA;
        autostrada[0] = s;
        numeroStazioni++;
        printf("aggiunta\n");
        return 'a';
    }else{
        int indice;
        if(numeroStazioni==autostradaAllocata){
            //Già che ci sono alloco un po' di memoria in più, ma non raddoppio
            autostrada = realloc(autostrada,sizeof(Stazione)*(autostradaAllocata+DIM_REALLOC));
            autostradaAllocata += DIM_REALLOC;
        }
        //Il seguente if potrebbe non servire, ma lo lascio per sicurezza
        if(numeroStazioni<autostradaAllocata){
            //non darà IOB
            autostrada[numeroStazioni].distanza = -1;
            autostrada[numeroStazioni].autonomie = NULL;
        }
        //numeroStazioni++;
        //indice = Da definizione metodo: indice prima di posizione di inserimento
        indice = binary_getInsertPosition(autostrada,0,numeroStazioni-1,s.distanza);
        if(indice==-2) {
            //La stazione è già presente
            //numeroStazioni--;
            printf("non aggiunta\n");
            return 'n';
        }else{
            //La stazione non è presente, la inserisco
            numeroStazioni++;
            for(int i = numeroStazioni-1;i>(indice+1);i--){
                autostrada[i] = autostrada[i-1];
            }
            autostrada[(indice+1)] = s;
            printf("aggiunta\n");
            //Il seguente if potrebbe non servire, ma lo metto per "bloccare" l'accesso a celle allocate ma non scritte.
            if(numeroStazioni<autostradaAllocata){
                //non darà IOB
                autostrada[numeroStazioni].distanza = -1;
                autostrada[numeroStazioni].autonomie = NULL;
            }
            //printf("fine aggiungi stazione\n");
            return 'a';
        }
    }
}

/*
 * così come l'aggiunta produce sempre un array compatto, per consentire la binary search
 */
void demolisci_stazione(unsigned int distanza){
    int indice = binary_search(autostrada,0,numeroStazioni-1,distanza);
    if(indice!=-1){
        //La stazione esiste, la demoliamo
        free(autostrada[indice].autonomie);
        autostrada[indice].autonomie = NULL;
        for(int i = indice;i<numeroStazioni-1;i++){
            autostrada[i].distanza = autostrada[i+1].distanza;
            autostrada[i].autonomie = autostrada[i+1].autonomie;
        }
        autostrada[numeroStazioni-1].distanza = -1;
        autostrada[numeroStazioni-1].autonomie = NULL; //ATTENZIONE: Non devo fare free di autonomie perché ho copiato solo il puntatore perderei il dato
        numeroStazioni--;
        printf("demolita\n");
        return;
    }else{
        //indice==-1, la stazione non esiste
        printf("non demolita\n");
        return;
    }
}

/*
 * Restituisce l'indice di Q a cui si trova il nodo con distanza dalla sorgente src minima e minima distanza dall'inizio
 * dell'autostrada
 */
unsigned int extract_min(){
    unsigned int dist_from_src_min = UINT_MAX;
    unsigned int dist_from_start_min = UINT_MAX;
    unsigned int indice_min = 0;
    for(int i=0;i<QSize;i++){
        if(Q[i].subset == 'q'){
            if(Q[i].dist_from_src < dist_from_src_min){
                dist_from_src_min = Q[i].dist_from_src;
                dist_from_start_min = autostrada[Q[i].my_index].distanza;
                indice_min = i;
            }else if(Q[i].dist_from_src == dist_from_src_min){
                if(autostrada[Q[i].my_index].distanza < dist_from_start_min){
                    dist_from_src_min = Q[i].dist_from_src;
                    dist_from_start_min = autostrada[Q[i].my_index].distanza;
                    indice_min = i;
                }
            }
        }
    }
    return indice_min;
}

/*
 * Ripercorre a ritroso il percorso seguendo gl indici prec_index e stampa il percorso.
 */
void stampa_soluzione(unsigned int indiceSrc, unsigned int indiceDst){
    if(Q[QSize-1].dist_from_src == UINT_MAX || Q[QSize-1].prec_index == -1){
        printf("nessun percorso\n");
        return;
    }
    //Stampa l'array Q pere DEBUG
    //printf("Dentro stampa soluzione: indiceSrc = %d, indiceDST = %d\n", indiceSrc, indiceDst);
    /*for(int j = 0;j < QSize;j++){
        printf("Q[%d].my_index = %u   ",j,Q[j].my_index);
        printf("Q[%d].dist_from_src = %u   ",j,Q[j].dist_from_src);
        printf("Q[%d].prec_index = %d   ",j,Q[j].prec_index);
        printf("Q[%d].subset = %c   ",j,Q[j].subset);
        printf("\n");
    }*/
    Node* head = NULL;
    //La dst è l'ultimo elemento di Q
    //La src è il primo elemento di Q
    int i = QSize-1;
    //printf("QSize: %d\n",QSize);
    while(i>0){
        Node* new = calloc(1, sizeof(Node));
        //printf("autostrada[Q[%d].my_index].distanza = %u   ",i,autostrada[(Q[i].my_index)].distanza);
        new->value = autostrada[(Q[i].my_index)].distanza;
        //printf("i= %d and new->value: %u\n",i, new->value);
        new->next = head;
        //new->prev = NULL;
        //head->prev = new;
        head = new;
        //prec_index è l'indice in autostrada, devo convertirlo in indice in Q
        if(indiceSrc < indiceDst){  //src < dst
            i = Q[i].prec_index - indiceSrc;
        }else{
            i = indiceSrc - Q[i].prec_index;
        }
        //Stampa l'array Q pere DEBUG
        /*for(int j = 0;j < QSize;j++){
            printf("Q[%d].my_index = %u   ",j,Q[j].my_index);
            printf("Q[%d].dist_from_src = %u   ",j,Q[j].dist_from_src);
            printf("Q[%d].prec_index = %d   ",j,Q[j].prec_index);
            printf("Q[%d].subset = %c   ",j,Q[j].subset);
            printf("\n");
        }
        printf("i: %d\n",i);*/
    }
    //Esce quando i, quindi ho raggiunto la src, devo aggiungere la src in testa alla lista
    Node* new = malloc(sizeof(Node));
    new->value = autostrada[Q[0].my_index].distanza;
    new->next = head;
    head = new;
    //Stampa l'array Q pere DEBUG
    /*for(int i = 0;i < QSize;i++){
        printf("Q[%d].my_index = %u   ",i,Q[i].my_index);
        printf("Q[%d].dist_from_src = %u   ",i,Q[i].dist_from_src);
        printf("Q[%d].prec_index = %d   ",i,Q[i].prec_index);
        printf("Q[%d].subset = %c   ",i,Q[i].subset);
        printf("\n");
    }*/
    //new->prev = NULL;
    //head->prev = new;
    //Stampo la lista
    Node* tmp = head;
    while(tmp!=NULL){
        if(tmp->next == NULL){
            printf("%d",tmp->value);
        }else{
            printf("%d ",tmp->value);
        }
        tmp = tmp->next;
    }
    printf("\n");
    //Faccio la free di tutti i puntatori ai nodi
    tmp = head;
    while(tmp!=NULL){
        Node* tmp2 = tmp;
        tmp = tmp->next;
        free(tmp2);
    }
}

void pianifica_percorso(unsigned int src, unsigned int dst){
    int indiceSrc = binary_search(autostrada,0,numeroStazioni-1,src);
    int indiceDst = binary_search(autostrada,0,numeroStazioni-1,dst);
    //printf("HELLO\n");
    //printf("Dentro pianifica percorso: src = %u, dst = %u, indiceSrc = %u, indiceDST = %u\n",src, dst, indiceSrc, indiceDst);
    if(indiceSrc == -1 || indiceDst == -1){
        printf("nessun percorso\n");
        return;
    }
    if(src==dst){
        if(indiceSrc !=-1 && indiceDst !=-1) {
            //Se sia src che dst esistono, allora...
            printf("%d\n", src);
        }else{
            printf("nessun percorso\n");
        }
        return;
    }
    //Devo inizializzare la struttura dati Q
    if(src < dst){
        //indiceSrc < indiceDst
        QSize = indiceDst-indiceSrc+1;
    }else{
        //indiceSrc > indiceDst
        QSize = indiceSrc-indiceDst+1;
    }
    Q = calloc(QSize, sizeof(Vertice));
    if(src < dst){
        for(int i = indiceSrc;i <= indiceDst;i++){
            Q[i-indiceSrc].my_index = i;
            if(i == indiceSrc) {
                Q[i - indiceSrc].dist_from_src = 0;
            }else{
                Q[i - indiceSrc].dist_from_src = UINT_MAX;
            }
            Q[i-indiceSrc].prec_index = -1;
            Q[i-indiceSrc].subset = 'q';
        }
    }else{
        //printf("CIAO1\n");
        for(int i = indiceSrc; i >= 0 && i >= indiceDst;i--){ //i >= 0 && i >= indiceDst perché unsigned
            //printf("CIAO i: %d\n",i);
            Q[indiceSrc-i].my_index = i;
            if(i == indiceSrc) {
                Q[indiceSrc - i].dist_from_src = 0;
            }else{
                Q[indiceSrc - i].dist_from_src = UINT_MAX;
            }
            Q[indiceSrc - i].prec_index = -1;
            Q[indiceSrc - i].subset = 'q';
        }
    }
    //printf("CIAO2\n");
    cardQ = QSize;
    //Inizializzazione Q completata
    //Stampa l'array Q pere DEBUG
    /*for(int i = 0;i < QSize;i++){
        printf("Q[%d].my_index = %u   ",i,Q[i].my_index);
        printf("Q[%d].dist_from_src = %u   ",i,Q[i].dist_from_src);
        printf("Q[%d].prec_index = %d   ",i,Q[i].prec_index);
        printf("Q[%d].subset = %c   ",i,Q[i].subset);
        printf("\n");
    }
    printf("\n");*/
    char flag = 'n'; //Flag per vedere se ho trovato vicini della sorgente
    while(cardQ > 0){
        unsigned int q_index = extract_min();                  //estrarre il minimo da Q
        Q[q_index].subset = 's';                               //inserire il minimo in S
        cardQ--;
        if(Q[q_index].my_index == indiceDst){
            //Se il minimo è il nodo di destinazione, allora ho finito
            stampa_soluzione(indiceSrc, indiceDst);
            break;
        }
        int distanza_v = autostrada[Q[q_index].my_index].distanza; //Il vertice si chiamerà v, i vicini u
        for(unsigned int u = 0; u < QSize; u++){
            if(Q[u].subset == 'q'){
                int distanza_u = autostrada[Q[u].my_index].distanza;
                int distanza_v_u = abs(distanza_u - distanza_v);
                //printf("distanza_v_u = %d\n",distanza_v_u);
                if(q_index < u){ //Allora è fra me e la destinazione, non è me ma può essere la dest.(appartiene a (me,dest])
                    //printf("Ciao1\n");
                    if(autostrada[Q[q_index].my_index].autonomie[0] >= distanza_v_u){   //È un mio vicino
                        if(Q[q_index].dist_from_src < UINT_MAX){
                            unsigned int alt = Q[q_index].dist_from_src + 1;
                            if(alt < Q[u].dist_from_src){
                                if(q_index == 0){
                                    flag = 's';
                                }
                                Q[u].dist_from_src = alt;
                                Q[u].prec_index = Q[q_index].my_index;
                            }
                        }
                    }
                }
            }
        }
        if(q_index==0 && flag == 'n'){
            //Non ho trovato vicini della sorgente
            printf("nessun percorso\n");
            break;
        }
        /*for(int i = 0;i < QSize;i++){
            printf("Q[%d].my_index = %u   ",i,Q[i].my_index);
            printf("Q[%d].dist_from_src = %u   ",i,Q[i].dist_from_src);
            printf("Q[%d].prec_index = %d   ",i,Q[i].prec_index);
            printf("Q[%d].subset = %c   ",i,Q[i].subset);
            printf("\n");
        }
        printf("\n");*/
    }
    //Faccio la free di Q
    free(Q);
    Q = NULL;
}

/*
 * DEBUG
 */
void stampa_auto(unsigned int distanza_stazione){
    int indice = binary_search(autostrada,0,numeroStazioni-1,distanza_stazione);
    if(indice==-1){
        //La stazione non esiste
        printf("[DEBUG]: La stazione non esiste\n");
        return;
    }
    printf("A%d: ",distanza_stazione);
    for(int i=0;i<MAX_DIM_PARCO && autostrada[indice].autonomie[i]>=0;i++){
        printf("%d ",autostrada[indice].autonomie[i]);
    }
    printf("\n");
}

/*
 * DEBUG
 */
void stampa_autostrada(){
    printf("Numero stazioni: %d\n", numeroStazioni);
    for(int i=0;i<numeroStazioni;i++){
        printf("A%d: ",autostrada[i].distanza);
        if(autostrada[i].autonomie == NULL){
            printf("Non ci sono auto");
        }else{
            for(int j=0;j<MAX_DIM_PARCO && autostrada[i].autonomie[j]>0;j++){
                printf("%d ",autostrada[i].autonomie[j]);
            }
        }
        printf("\n");
    }
}

void manageInput(char* line){
    char* command = strtok(line," ");
    if(strcmp(command,"aggiungi-stazione")==0) {
        //Aggiungo una stazione
        unsigned int distanza = atoi(strtok(NULL, " "));
        unsigned int numeroAutonomie = atoi(strtok(NULL, " "));
        Stazione* newStazione = malloc(sizeof(Stazione));
        newStazione->distanza = distanza;
        newStazione->autonomie = NULL;
        char esito = aggiungi_stazione(*newStazione);
        //newStazione->autonomie = calloc(MAX_DIM_PARCO, sizeof(unsigned int));
        for (int i = 0; i < numeroAutonomie; i++) {
            //autonomie[i] = atoi(strtok(NULL, " "));
            unsigned int autonomia = atoi(strtok(NULL, " "));
            if(esito == 'a'){
                aggiungi_auto(newStazione->distanza, autonomia, 'f');
            }
        }
    }else if(strcmp(command,"demolisci-stazione")==0) {
        //Demolisco una stazione
        unsigned int distanza = atoi(strtok(NULL, " "));
        demolisci_stazione(distanza);
    }else if(strcmp(command,"aggiungi-auto")==0) {
        //Aggiungo un'auto
        unsigned int distanza_stazione = atoi(strtok(NULL, " "));
        unsigned int autonomia = atoi(strtok(NULL, " "));
        aggiungi_auto(distanza_stazione,autonomia, 't');
    }else if(strcmp(command,"rottama-auto")==0) {
        //Rottamo un'auto
        unsigned int distanza_stazione = atoi(strtok(NULL, " "));
        unsigned int autonomia = atoi(strtok(NULL, " "));
        rottama_auto(distanza_stazione,autonomia);
    }else if(strcmp(command,"pianifica-percorso")==0) {
        //Stampo il percorso
        unsigned int src = atoi(strtok(NULL, " "));
        unsigned int dst = atoi(strtok(NULL, " "));
        pianifica_percorso(src,dst);
    }else if(strcmp(command,"stampa-autostrada")==0) {          //DEBUG
        //Stampo l'autostrada
        stampa_autostrada();
    }else if(strcmp(command,"stampa-auto")==0) {                 //DEBUG
        //Stampo le auto
        unsigned int distanza_stazione = atoi(strtok(NULL, " "));
        stampa_auto(distanza_stazione);
    }else if(strcmp(command,"fine")==0) {
        //Termino il programma
        exit(0);
    }

}

int main(int argc, char* argv[]){
    //Leggo l'input da stdin
    char* line = NULL;
    size_t len = 0;

    /*while(1){
        getline(&line,&len,stdin);
        if(line != NULL){
            manageInput(line);
        }
    }*/
    while(getline(&line, &len, stdin) != -1){
        if(line != NULL){
            manageInput(line);
        }
    }
    //Faccio la free dell'array autostrada
    for(int i=0;i<numeroStazioni;i++){
        if(autostrada[i].autonomie != NULL){
            free(autostrada[i].autonomie);
            autostrada[i].autonomie = NULL;
        }
    }
    free(autostrada);
    autostrada = NULL;
}