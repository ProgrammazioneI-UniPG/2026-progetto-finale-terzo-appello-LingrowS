// gamelib.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "gamelib.h"

// --- Variabili Globali Statiche (Visibili solo in questo file) ---
static struct Giocatore* giocatori = NULL;
static int numero_giocatori = 0;
static bool gioco_impostato = false;
static bool mappa_creata = false;

// Puntatori alle liste della mappa
static struct Zona_mondoreale* testa_reale = NULL;
static struct Zona_soprasotto* testa_sotto = NULL;
static struct Zona_mondoreale* coda_reale = NULL;
static struct Zona_soprasotto* coda_sotto = NULL;
static int contatore_zone = 0;

// Stringhe per stampa
static const char* nomi_zone[] = {
    "Bosco", "Scuola", "Laboratorio", "Caverna", "Strada", 
    "Giardino", "Supermercato", "Centrale Elettrica", 
    "Deposito Abbandonato", "Stazione Polizia"
};
static const char* nomi_nemici[] = {
    "Nessuno", "Billi", "Democane", "Demotorzone"
};
static const char* nomi_oggetti[] = {
    "Vuoto", "Bicicletta", "Maglietta Fuocoinferno", "Bussola", "Schitarrata Metallica"
};

// --- unzioni Interne (Static) ---
static int lancia_dado(int facce);
static void svuota_buffer(void);
static void crea_giocatori(void);
static void menu_creazione_mappa(void);
static void genera_mappa(void);
static void inserisci_zona(void);
static void cancella_zona(void);
static void stampa_mappa(void);
static void chiudi_mappa(void);
static void dealloca_mappa(void);
static void turno_giocatore(int indice_g);
static void stampa_giocatore(int indice_g);
static void stampa_zona_corrente(int indice_g);
static bool avanza(int indice_g, bool mossa_fatta);
static bool indietreggia(int indice_g, bool mossa_fatta);
static bool cambia_mondo(int indice_g, bool mossa_fatta);
static void combatti(int indice_g);
static void raccogli_oggetto(int indice_g);
static void utilizza_oggetto(int indice_g);
static bool verifica_vittoria(void);
static bool verifica_tutti_morti(void);
//static void funzione(void);









// --- IMPOSTA GIOCO ---

void imposta_gioco(void) {

    if (giocatori != NULL) {    //Se il gioco è stato precedentemente impostato, chiediamo all'utente se vuole impostarlo nuovamente o se vuole continuare con l'impostazione precedente
        printf("Il gioco e' gia' stato impostato. Vuoi resettarlo? ([1] = Si, [Qualunque altro valore] = No): ");
        int r;
        scanf("%d", &r); 
        svuota_buffer();
        if (r != 1) return; //Se l'utente inserisce un qualunque altro valore, allora non vuole resettare
        termina_gioco(); //L'utente ha inserito 1 e vuole dunque impostare nuovamente il gioco
    }
    //Vediamo questa parte solo alla prima impostazione del gioco, oppure se abbiamo premuto 1 in precedenza
    printf("\n--- IMPOSTAZIONE GIOCO ---\n");
    crea_giocatori();
    menu_creazione_mappa();
    gioco_impostato = true;
}










// --- GIOCA ---

void gioca(void) {
    if (!gioco_impostato || !mappa_creata) {
        printf("\n[ERRORE] Devi prima impostare il gioco e creare la mappa!\n");
        return;
    }

    printf("\n--- INIZIO PARTITA ---\n");
    
    // Posizionamento iniziale dei giocatori
    for (int i = 0; i < numero_giocatori; i++) {
        giocatori[i].mondo = 0; // Tutti partono nel mondo reale
        giocatori[i].pos_mondoreale = testa_reale;
        giocatori[i].pos_soprasotto = NULL;
        giocatori[i].stato = VIVO;
        printf("%s entra a Occhinz nel %s.\n", giocatori[i].nome, nomi_zone[testa_reale->tipo]);
    }

    bool fine_partita = false;
    while (!fine_partita) {
        // Ordine casuale dei turni: mescoliamo un array di indici
        int ordine[4] = {0, 1, 2, 3}; // Max 4 giocatori
        for (int i = 0; i < numero_giocatori; i++) {
            int r = rand() % numero_giocatori;
            int temp = ordine[i];
            ordine[i] = ordine[r];
            ordine[r] = temp;
        }

        for (int i = 0; i < numero_giocatori; i++) {
            int idx = ordine[i];
            if (giocatori[idx].stato == VIVO) {
                turno_giocatore(idx);
                
                if (verifica_vittoria()) {
                    printf("\n*** VITTORIA! IL DEMOTORZONE E' STATO SCONFITTO! ***\n");
                    fine_partita = true;
                    break;
                }
            }
        }

        if (!fine_partita && verifica_tutti_morti()) {
            printf("\n*** GAME OVER! Nessun sopravvissuto a Occhinz. ***\n");
            fine_partita = true;
        }
    }
    
    // Reset automatico alla fine
    termina_gioco();
}










// --- TERMINA GIOCO ---

void termina_gioco(void) {
    printf("\nPulizia memoria in corso...\n");
    if (giocatori != NULL) {
        free(giocatori);
        giocatori = NULL;
    }
    dealloca_mappa();
    numero_giocatori = 0;
    gioco_impostato = false;
    mappa_creata = false;
    printf("Gioco terminato.\n");
}










// --- CREDITI ---

void crediti(void) {
    printf("\n--- CREDITI ---\n");
    printf("Sviluppato da: Rowlings Omordia\n");
    printf("Ispirato a: Cosestrane (Stranger Things)\n");
}










// --- Implementazione Funzioni Interne (Static) ---

static int lancia_dado(int facce) {
    return (rand() % facce) + 1;
}

static void svuota_buffer(void) {
    while (getchar() != '\n');
}

static void crea_giocatori(void) {

    do { //ciclo da fare sempre almeno una volta, quindi do...while
        printf("Inserisci il numero di giocatori che prenderanno parte alla partita (MIN 1, MAX 4): ");
        scanf("%d", &numero_giocatori);
        svuota_buffer();

        if((numero_giocatori < 1 || numero_giocatori > 4)){//se il numero inserito non è valido, allora stampo un messaggio di valore non valido, e non esco dal ciclo
            system("clear");
            printf("--- IMPOSTAZIONE GIOCO ---\n");
            printf("\nNumero inserito non valido. \n");
        }
    } while (numero_giocatori < 1 || numero_giocatori > 4);
    //se sono uscito dal ciclo, allora il numero di giocatori selezionato è giusto

    giocatori = (struct Giocatore*)malloc(numero_giocatori * sizeof(struct Giocatore));//Creazione dei giocatori in memoria dinamica
    if (giocatori == NULL) {//Gestiamo eventuali errori di allocazione
        perror("Errore malloc giocatori");
        exit(1);
    }

    srand(time(NULL));
    bool limitatore_via_poteri = true;
    for (int i = 0; i < numero_giocatori; i++) {
        printf("\nGiocatore %d, inserisci il nome: ", i + 1);
        scanf("%49s", giocatori[i].nome);//leggiamo una stringa (s) di massimo 49 caratteri
        svuota_buffer();

        //generiamo le caratteristiche del giocatore, e lo facciamo per tutti i giocatori
        giocatori[i].attacco_psichico = lancia_dado(20);
        giocatori[i].difesa_psichica = lancia_dado(20);
        giocatori[i].fortuna = lancia_dado(20);
        
        //inizializziamo lo zaino come vuoto
        for(int k=0; k<3; k++) giocatori[i].zaino[k] = NESSUN_OGGETTO;
        giocatori[i].stato = VIVO;//impostiamo il giocatore come vivo, per permettergli di giocare

        printf("Statistiche tirate: ATK: %d, DEF: %d, LUCK: %d\n", 
               giocatori[i].attacco_psichico, giocatori[i].difesa_psichica, giocatori[i].fortuna);

        do{
            printf("Scegli classe:\n");
            printf("1) Normale (Modifica manuale)\n");
            printf("2) UndiciVirgolaCinque (Diventa speciale)\n");
            int scelta_classe=0;
            scanf("%d", &scelta_classe);
            svuota_buffer();

            if ((scelta_classe == 2) && limitatore_via_poteri){
                printf("Hai scelto la via dei poteri!\n");
                int a = giocatori[i].attacco_psichico + 4;
                int d = giocatori[i].difesa_psichica + 4;
                int f = giocatori[i].fortuna - 7;
                giocatori[i].attacco_psichico = (a > 20) ? 20 : a;
                giocatori[i].difesa_psichica = (d > 20) ? 20 : d;
                giocatori[i].fortuna = (f < 1) ? 1 : f;
                strcat(giocatori[i].nome, " (UVC)"); //concateniamo, tramite strcat, UVC al nome di questo giocatore
                limitatore_via_poteri = false;//se un giocatore sceglie la via dei poteri, non può sceglierla nessun altro
                break;
            } else if (scelta_classe<1 || scelta_classe>2){
                system("clear");
                printf("Si prega di inserire un valore valido per la classe (inserire [1] oppure [2])\n");
            } else {
                if(!limitatore_via_poteri){
                    printf("La classe UndiciVirgolaCinque ha già raggiunto il suo massimo di giocatori disponibili, si verrà reinderizzati alla classe normale!\n");
                }
                printf("Vuoi bilanciare le statistiche?\n");
                printf("1) +3 ATK / -3 DEF\n");
                printf("2) -3 ATK / +3 DEF\n");
                printf("Qualunque altro valore) Nessuna modifica\n");
                int mod;
                scanf("%d", &mod);
                svuota_buffer();
                if (mod == 1) {
                    giocatori[i].attacco_psichico += 3;
                    if(giocatori[i].difesa_psichica > 3) giocatori[i].difesa_psichica -= 3; else giocatori[i].difesa_psichica=1;
                } else if (mod == 2) {
                    if(giocatori[i].attacco_psichico > 3) giocatori[i].attacco_psichico -= 3; else giocatori[i].attacco_psichico=1;
                    giocatori[i].difesa_psichica += 3;
                }
                break;
            }   
        } while(1); //restiamo nel ciclo finché non raggiungiamo un break
        
    }
}



















// --- Gestione Mappa ---

static void menu_creazione_mappa(void) {
    int scelta;
    do {
        printf("\n--- CREAZIONE MAPPA (Zone attuali: %d) ---\n", contatore_zone);
        printf("1) Genera mappa casuale (sovrascrive tutto)\n");
        printf("2) Inserisci zona manualmente\n");
        printf("3) Cancella zona\n");
        printf("4) Stampa mappa\n");
        printf("5) Chiudi mappa e termina creazione\n");
        //printf("6) xxx\n");
        printf("Scelta: ");
        scanf("%d", &scelta);
        svuota_buffer();

        switch (scelta) {
            case 1: genera_mappa(); break;
            case 2: inserisci_zona(); break;
            case 3: cancella_zona(); break;
            case 4: stampa_mappa(); break;
            case 5: chiudi_mappa(); break;
            //case 6: finzione(); break;
            default: printf("Scelta errata.\n");
        }
    } while (!mappa_creata);
}

//static void funzione(){}





// funzione che, una volta generati i parametri, crea la singola zona aggiungendola in coda
static void append_zone(Tipo_zona t, Tipo_nemico n_r, Tipo_nemico n_s, Tipo_oggetto o_r) {
    struct Zona_mondoreale* nuova_r = (struct Zona_mondoreale*)malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto* nuova_s = (struct Zona_soprasotto*)malloc(sizeof(struct Zona_soprasotto));

    if (!nuova_r || !nuova_s) { perror("Memoria esaurita"); exit(1); }//errore nella creazione di memoria dinamica

    //Inizializzazione zona reale
    nuova_r->tipo = t;
    nuova_r->nemico = n_r;
    nuova_r->oggetto = o_r;
    nuova_r->avanti = NULL;
    nuova_r->indietro = coda_reale;
    nuova_r->link_soprasotto = nuova_s;

    //Inizializzazione zona sottosopra
    nuova_s->tipo = t; // Stesso tipo
    nuova_s->nemico = n_s;
    nuova_s->avanti = NULL;
    nuova_s->indietro = coda_sotto;
    nuova_s->link_mondoreale = nuova_r;

    // Collegamenti lista
    if (coda_reale) coda_reale->avanti = nuova_r;
    else testa_reale = nuova_r;
    coda_reale = nuova_r;

    if (coda_sotto) coda_sotto->avanti = nuova_s;
    else testa_sotto = nuova_s;
    coda_sotto = nuova_s;

    contatore_zone++;
}

static void dealloca_mappa(void) {
    struct Zona_mondoreale* curr_r = testa_reale;
    struct Zona_soprasotto* curr_s = testa_sotto;
    
    while (curr_r != NULL) {
        struct Zona_mondoreale* temp = curr_r;
        curr_r = curr_r->avanti;
        free(temp);
    }
    while (curr_s != NULL) {
        struct Zona_soprasotto* temp = curr_s;
        curr_s = curr_s->avanti;
        free(temp);
    }
    testa_reale = coda_reale = NULL;
    testa_sotto = coda_sotto = NULL;
    contatore_zone = 0;
}

static void genera_mappa(void) {
    dealloca_mappa(); //Puliamo la mappa, se in precedenza avevamo creato delle zone
    printf("Generazione 15 zone...\n");
    
    bool demotorzone_piazzato = false;

    for (int i = 0; i < 15; i++) {
        Tipo_zona t = rand() % 10;
        
        // Nemico reale, 10% di possibilità
        Tipo_nemico nr = NESSUN_NEMICO;
        if ((rand() % 100) < 10) nr = (rand() % 2) ? BILLI : DEMOCANE; // Solo Billi o Democane
        
        // Nemico soprasotto, 10% di possibilità
        Tipo_nemico ns = NESSUN_NEMICO;
        if ((rand() % 100) < 10) ns = DEMOCANE;

        // Oggetto reale, 10% di possibilità
        Tipo_oggetto or = NESSUN_OGGETTO;
        if ((rand() % 100) < 10) or = (rand() % 4) + 1;

        // Forza Demotorzone nell'ultima zona se non piazzato, o a caso nel Soprasotto
        if (i == 14 && !demotorzone_piazzato) {
            ns = DEMOTORZONE;
            demotorzone_piazzato = true;
        } else if (!demotorzone_piazzato && (rand() % 15 == 0)) {
            ns = DEMOTORZONE;
            demotorzone_piazzato = true;
        }

        append_zone(t, nr, ns, or);
    }
}
/* if(testa_reale == NULL){
  printf("non ci stanno zone");
  return;
 }

  if(testa_reale->avanti == NULL){
  printf("non ci sta una seconda zona");
  retun;
  }

   int pos = 2;
   // 4. Allocazione Memoria
    struct Zona_mondoreale* nuova_r = (struct Zona_mondoreale*)malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto* nuova_s = (struct Zona_soprasotto*)malloc(sizeof(struct Zona_soprasotto));
        
    // Inizializzazione dati
    nuova_r->link_soprasotto = nuova_s->avanti;
    nuova_r->avanti->link_soprasotto = nuova_s;

    nuova_s->link_mondoreale = nuova_r->avanti;
    nuova_s->avanti->link_mondoreale = nuova_r;

*/

/*if(testa_reale == NULL){
 printf("non ci stanno zone");
 }

 if(testa_reale->avanti == NULL){
 printf("seconda zone non presente")
 }
 
 if(testa_reale->avanti->tipo == NESSUN_OGGETTO){
 testa_reale->avanti>tipo == BICICLETTA;
  }
*/

static void inserisci_zona(void) {
    printf("\n--- INSERIMENTO MANUALE ZONA ---\n");

    // 1. Calcoliamo quante zone ci sono attualmente
    int max_zone = 0;
    struct Zona_mondoreale* temp = testa_reale;
    while (temp != NULL) {
        max_zone++;
        temp = temp->avanti;
    }

    // 2. Chiediamo la posizione desiderata
    int pos;
    do {
        printf("Inserisci la posizione in cui inserire la zona (0 - %d): ", max_zone);
        // Nota: max_zone è una posizione valida (equivale ad aggiungere in coda)
        scanf("%d", &pos);
        svuota_buffer();
        if (pos < 0 || pos > max_zone) {
            printf("[ERRORE] Posizione non valida.\n");
        }
    } while (pos < 0 || pos > max_zone);

    // 3. Chiediamo i dati della zona
    int t, nr, ns, or;
    do {
        printf("Tipo zona (0-9): "); 
        scanf("%d", &t); svuota_buffer();
        if (t < 0 || t > 9) {
            printf("[ERRORE] Zona non valida.\n");
        }
    } while (t < 0 || t > 9);
    
    do {
        printf("Nemico Reale (0=Nessuno, 1=Billi, 2=Democane): "); 
        scanf("%d", &nr); svuota_buffer();
        if (nr < 0 || nr > 2) {
            printf("[ERRORE] Valore inserito non valido.\n");
        }
    } while (nr < 0 || nr > 2);

    do {
        printf("Nemico Soprasotto (0=Nessuno, 2=Democane, 3=Demotorzone): "); 
        scanf("%d", &ns); svuota_buffer();        
        
        if (ns < 0 || ns > 3 || ns==1) {
            printf("[ERRORE] Valore inserito non valido.\n");
        }
    } while (ns < 0 || ns > 3 || ns==1);

    do {
    printf("Oggetto Reale (0=Vuoto, 1=Bici, 2=Maglietta, 3=Bussola, 4=Schitarra): "); 
    scanf("%d", &or); svuota_buffer();      
        
        if (or < 0 || or > 4) {
            printf("[ERRORE] Valore inserito non valido.\n");
        }
    } while (or < 0 || or > 4);    


    // Validazione base input
    if (t < 0 || t > 9) t = 0;
    if (nr == 3) nr = 0; // Demotorzone non può stare nel reale
    if (ns == 1) ns = 0; // Billi non può stare nel soprasotto

    // 4. Allocazione Memoria
    struct Zona_mondoreale* nuova_r = (struct Zona_mondoreale*)malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto* nuova_s = (struct Zona_soprasotto*)malloc(sizeof(struct Zona_soprasotto));

    if (!nuova_r || !nuova_s) {
        perror("Errore malloc inserimento zona");
        exit(1);
    }

    // Inizializzazione dati
    nuova_r->tipo = (Tipo_zona)t;
    nuova_r->nemico = (Tipo_nemico)nr;
    nuova_r->oggetto = (Tipo_oggetto)or;
    nuova_r->link_soprasotto = nuova_s;

    nuova_s->tipo = (Tipo_zona)t;
    nuova_s->nemico = (Tipo_nemico)ns;
    nuova_s->link_mondoreale = nuova_r;

    // 5. Collegamento nella lista (Logica dei puntatori)
    
    // CASO A: Lista Vuota
    if (testa_reale == NULL) {
        nuova_r->avanti = NULL;
        nuova_r->indietro = NULL;
        nuova_s->avanti = NULL;
        nuova_s->indietro = NULL;
        
        testa_reale = coda_reale = nuova_r;
        testa_sotto = coda_sotto = nuova_s;
    }
    // CASO B: Inserimento in Testa (Posizione 0)
    else if (pos == 0) {
        nuova_r->avanti = testa_reale;
        nuova_r->indietro = NULL;
        testa_reale->indietro = nuova_r;
        testa_reale = nuova_r;

        nuova_s->avanti = testa_sotto;
        nuova_s->indietro = NULL;
        testa_sotto->indietro = nuova_s;
        testa_sotto = nuova_s;
    }
    // CASO C: Inserimento in Coda (Posizione == max_zone)
    else if (pos == max_zone) {
        nuova_r->avanti = NULL;
        nuova_r->indietro = coda_reale;
        coda_reale->avanti = nuova_r;
        coda_reale = nuova_r;

        nuova_s->avanti = NULL;
        nuova_s->indietro = coda_sotto;
        coda_sotto->avanti = nuova_s;
        coda_sotto = nuova_s;
    }
    // CASO D: Inserimento nel Mezzo
    else {
        // Scorriamo fino alla zona che attualmente occupa quella posizione
        struct Zona_mondoreale* corr_r = testa_reale;
        struct Zona_soprasotto* corr_s = testa_sotto;
        
        for (int i = 0; i < pos; i++) {
            corr_r = corr_r->avanti;
            corr_s = corr_s->avanti;
        }

        // Inseriamo PRIMA di corr_r/corr_s
        struct Zona_mondoreale* prev_r = corr_r->indietro;
        struct Zona_soprasotto* prev_s = corr_s->indietro;

        // Collega il nuovo nodo al precedente
        prev_r->avanti = nuova_r;
        nuova_r->indietro = prev_r;
        
        prev_s->avanti = nuova_s;
        nuova_s->indietro = prev_s;

        // Collega il nuovo nodo al successivo (corrente)
        nuova_r->avanti = corr_r;
        corr_r->indietro = nuova_r;

        nuova_s->avanti = corr_s;
        corr_s->indietro = nuova_s;
    }

    contatore_zone++;
    printf("Zona inserita con successo in posizione %d.\n", pos);
}


/*if (testa_reale == NULL) {
 prinf("\n[ERRORE] la mappa è vuota, nessuna zona da cancellare.\n")
 return;
 }

 if (testa_reale->avanti == NULL){
   printf("non ci sta la seconda zona da cancellare.\n")
   return;
 }

 
 int pos = 2;

 // 3. Raggiungo la zona da eliminare
    struct Zona_mondoreale* to_del_r = testa_reale;
    struct Zona_soprasotto* to_del_s = testa_sotto;

 // Scorro la lista fino alla posizione indicata
    for (int i = 0; i < pos; i++) {
        to_del_r = to_del_r->avanti;
        to_del_s = to_del_s->avanti;
    }
    printf("Cancellazione della zona %d in corso...\n", pos);

    // Gestione Mondo Reale
        to_del_r->indietro->avanti = to_del_r->avanti;
        
        if (to_del_r->avanti != NULL) {
        to_del_r->avanti->indietro = to_del_r->indietro;
        } else {
           coda_reale = to_del_r->indietro; 
        }

        // Gestione Soprasotto (speculare al reale)
        to_del_s->indietro->avanti = to_del_s->avanti;

    if (to_del_s->avanti != NULL) {
        to_del_s->avanti->indietro = to_del_s->indietro;
    } else {
        coda_sotto = to_del_s->indietro;
    }
    
    // 5. Deallocazione memoria e aggiornamento contatore
    free(to_del_r);
    free(to_del_s);
    contatore_zone--;



*/

static void cancella_zona(void) {
    
    // 1. Controllo se la mappa è vuota
    if (testa_reale == NULL) { 
        printf("\n[ERRORE] La mappa è vuota, nessuna zona da cancellare.\n"); 
        return; 
    }

    printf("\n--- CANCELLAZIONE ZONA ---\n");

    // 2. Chiedo all'utente quale zona cancellare
    int max_index = contatore_zone - 1;
    int pos;

    do {
        printf("Scegli la zona da eliminare (0 - %d): ", max_index);
        scanf("%d", &pos);
        svuota_buffer();

        if (pos < 0 || pos > max_index) {
            printf("[ERRORE] Posizione non valida.\n");
        }
    } while (pos < 0 || pos > max_index);

    // 3. Raggiungo la zona da eliminare
    struct Zona_mondoreale* to_del_r = testa_reale;
    struct Zona_soprasotto* to_del_s = testa_sotto;

    // Scorro la lista fino alla posizione indicata
    for (int i = 0; i < pos; i++) {
        to_del_r = to_del_r->avanti;
        to_del_s = to_del_s->avanti;
    }

    printf("Cancellazione della zona %d in corso...\n", pos);

    // 4. Aggiornamento dei puntatori (Scollegamento nodo)
 
    // Gestione Mondo Reale
    if (to_del_r->indietro != NULL) {
        // Se NON è la testa, collego il precedente al successivo
        to_del_r->indietro->avanti = to_del_r->avanti;
    } else {
        // Se È la testa, sposto il puntatore globale testa
        testa_reale = to_del_r->avanti;
    }

    if (to_del_r->avanti != NULL) {
        // Se NON è la coda, collego il successivo al precedente
        to_del_r->avanti->indietro = to_del_r->indietro;
    } else {
        // Se È la coda, sposto il puntatore globale coda
        coda_reale = to_del_r->indietro;
    }

    // Gestione Soprasotto (speculare al reale)
    if (to_del_s->indietro != NULL) {
        to_del_s->indietro->avanti = to_del_s->avanti;
    } else {
        testa_sotto = to_del_s->avanti;
    }

    if (to_del_s->avanti != NULL) {
        to_del_s->avanti->indietro = to_del_s->indietro;
    } else {
        coda_sotto = to_del_s->indietro;
    }

    // 5. Deallocazione memoria e aggiornamento contatore
    free(to_del_r);
    free(to_del_s);
    contatore_zone--;

    printf("Zona eliminata con successo. \n");
}

static void stampa_mappa(void) {
    // Controllo preventivo se la mappa esiste
    if (testa_reale == NULL) {
        printf("\n[ERRORE] La mappa non e' stata ancora creata.\n");
        return;
    }

    int scelta_stampa = 0;
    
    do{
        // Menu di scelta per la modalità di stampa
        printf("\nSeleziona modalita' di stampa:\n");
        printf("1) Mappa Completa (Mondo Reale + Soprasotto)\n");
        printf("2) Solo Mondo Reale\n");
        printf("3) Solo Soprasotto\n");
        printf("Scelta: ");
        scanf("%d", &scelta_stampa);
        svuota_buffer(); 
        
        if(scelta_stampa < 1 || scelta_stampa > 3){
            system("clear");
            printf("Valore inserito non valido\n");
        }
    } while(scelta_stampa < 1 || scelta_stampa > 3);
    system("clear");
    printf("\n--- VISUALIZZAZIONE MAPPA ---\n");

    if(scelta_stampa==1){
        printf("\nStampa dell'intera mappa di gioco:\n");
    } else if(scelta_stampa==2){
        printf("\nStampa del Mondo Reale:\n");
    }else {
        printf("\nStampa del Soprasotto:\n");
    }

    struct Zona_mondoreale* r = testa_reale;
    struct Zona_soprasotto* s = testa_sotto;
    int i = 0;

    while (r != NULL) {
        // Stampiamo il nome della zona (comune a entrambi i mondi)
        printf("\n[Zona %d]: %s", i++, nomi_zone[r->tipo]);

        // Se l'utente vuole tutto (1) o solo Reale (2)
        if (scelta_stampa == 1 || scelta_stampa == 2) {
            printf("\n  [Reale] Nemico: %-12s | Oggetto: %s", 
                   nomi_nemici[r->nemico], 
                   nomi_oggetti[r->oggetto]);
        }

        // Se l'utente vuole tutto (1) o solo Soprasotto (3)
        if (scelta_stampa == 1 || scelta_stampa == 3) {
            // Controllo di sicurezza su 's', anche se le liste dovrebbero essere parallele
            if (s != NULL) {
                printf("\n  [Sopra] Nemico: %s", nomi_nemici[s->nemico]);
            }
        }
        
        printf("\n"); // Spaziatura tra le zone

        // Avanziamo con entrambi i puntatori per mantenere l'allineamento
        r = r->avanti;
        if (s != NULL) s = s->avanti;
    }
    printf("-----------------------------\n");
}

static void chiudi_mappa(void) {
    if (contatore_zone < 15) {
        printf("Errore: La mappa deve avere almeno 15 zone. Attuali: %d\n", contatore_zone);
        return;
    }
    
    // Verifica Demotorzone
    struct Zona_soprasotto* curr = testa_sotto;
    int count_demo = 0;
    while(curr) {
        if (curr->nemico == DEMOTORZONE) count_demo++;
        curr = curr->avanti;
    }

    if (count_demo != 1) {
        printf("Errore: Deve esserci esattamente 1 Demotorzone nel Soprasotto. Trovati: %d\n", count_demo);
        return;
    }

    mappa_creata = true;
    printf("Mappa validata e chiusa.\n");
}





















// --- Logica di Gioco ---

static bool verifica_vittoria(void) {
    // Si vince se il Demotorzone non è più presente nella mappa
    struct Zona_soprasotto* curr = testa_sotto;
    while(curr) {
        if (curr->nemico == DEMOTORZONE) return false;
        curr = curr->avanti;
    }
    return true;
}

static bool verifica_tutti_morti(void) {
    for (int i=0; i<numero_giocatori; i++) {
        if (giocatori[i].stato == VIVO) return false;
    }
    return true;
}

static void turno_giocatore(int indice_g) {
    struct Giocatore* g = &giocatori[indice_g];
    int azione=0;
    bool turno_finito=false;
    bool ha_mosso = false;

    printf("\n>>> TURNO DI %s (Mondo: %s) <<<\n", g->nome, g->mondo == 0 ? "Reale" : "Soprasotto");

    while (!turno_finito && g->stato == VIVO) {
        printf("\nScegli azione:\n");
        printf("1) Avanza\n2) Indietreggia\n3) Cambia Mondo\n4) Combatti\n5) Raccogli Oggetto\n6) Utilizza Oggetto\n7) Stampa Stato\n8) Esamina Zona\n9) Passa Turno\n");
        scanf("%d", &azione);
        svuota_buffer();

        switch (azione) {
            case 1: ha_mosso=avanza(indice_g, ha_mosso); break;
            case 2: ha_mosso=indietreggia(indice_g, ha_mosso); break;
            case 3: ha_mosso=cambia_mondo(indice_g, ha_mosso); break;
            case 4: combatti(indice_g); break; 
            case 5: raccogli_oggetto(indice_g); break;
            case 6: utilizza_oggetto(indice_g); break;
            case 7: stampa_giocatore(indice_g); break;
            case 8: stampa_zona_corrente(indice_g); break;
            case 9: printf("Passi il turno.\n"); turno_finito = true; break;
            default: printf("Azione non valida.\n");
        }

        // Se l'azione ha provocato la morte o la vittoria, esci
        if (g->stato == MORTO || verifica_vittoria()) turno_finito = true;
    }
}

static bool avanza(int indice_g, bool mossa_fatta) {
    //se abbiamo, in precedenza, già fatto un'azione che si può fare una sola volta per turno, allora non possiamo fare altro
    if(mossa_fatta){
        printf("Impossibile avanzare, azione già effettuata in questo turno\n");
        return true;
    }

    struct Giocatore* g = &giocatori[indice_g];
    Tipo_nemico nemico_presente = NESSUN_NEMICO;

    if (g->mondo == 0) {
        if (g->pos_mondoreale->nemico != NESSUN_NEMICO) nemico_presente = g->pos_mondoreale->nemico;
    } else {
        if (g->pos_soprasotto->nemico != NESSUN_NEMICO) nemico_presente = g->pos_soprasotto->nemico;
    }

    if (nemico_presente != NESSUN_NEMICO) {
        printf("Non puoi avanzare! C'e' un nemico (%s) che ti blocca. Devi combattere o scappare cambiando mondo.\n", nomi_nemici[nemico_presente]);
        return false;//se non avanziamo, allora l'azione si può ancora fare
    }

    if (g->mondo == 0) {//logica avanzamento mondo reale
        if (g->pos_mondoreale->avanti != NULL) {
            g->pos_mondoreale = g->pos_mondoreale->avanti;
            printf("Avanzato a: %s\n", nomi_zone[g->pos_mondoreale->tipo]);
            return true;
        } else {
            printf("Sei all'inizio della mappa.\n");
            return false; }
    } else {
        if (g->pos_soprasotto->avanti != NULL) {
            g->pos_soprasotto = g->pos_soprasotto->avanti;
            printf("Avanzato a: %s\n", nomi_zone[g->pos_soprasotto->tipo]);
            return true;
        } else {
            printf("Sei all'inizio della mappa.\n");
            return false; }
    }
    return true;//non vediamo mai questo return, lo mettiamo per sicurezza
}

static bool indietreggia(int indice_g, bool mossa_fatta) {
    // Logica identica ad avanza per il blocco nemici

    //se abbiamo, in precedenza, già fatto un'azione che si può fare una sola volta per turno, allora non possiamo fare altro
    if(mossa_fatta){
        printf("Impossibile indietreggiare, azione già effettuata in questo turno\n");
        return true;
    }

    struct Giocatore* g = &giocatori[indice_g];
    Tipo_nemico nemico_presente = NESSUN_NEMICO;
    if (g->mondo == 0) { if (g->pos_mondoreale->nemico != NESSUN_NEMICO) nemico_presente = g->pos_mondoreale->nemico; }
    else { if (g->pos_soprasotto->nemico != NESSUN_NEMICO) nemico_presente = g->pos_soprasotto->nemico; }

    if (nemico_presente != NESSUN_NEMICO) {
        printf("Nemico presente! Non puoi indietreggiare.\n");
        return false;
    }

    if (g->mondo == 0) {
        if (g->pos_mondoreale->indietro != NULL) {
            g->pos_mondoreale = g->pos_mondoreale->indietro;
            printf("Tornato a: %s\n", nomi_zone[g->pos_mondoreale->tipo]);
            return true;
        } else {
            printf("Sei all'inizio della mappa.\n");
            return false; }
    } else {
        if (g->pos_soprasotto->indietro != NULL) {
            g->pos_soprasotto = g->pos_soprasotto->indietro;
            printf("Tornato a: %s\n", nomi_zone[g->pos_soprasotto->tipo]);
            return true;
        } else {
            printf("Sei all'inizio della mappa.\n");
            return false; }
    }
    return true; //non vediamo mai questo return, lo mettiamo per sicurezza
}

static bool cambia_mondo(int indice_g, bool mossa_fatta) {
    struct Giocatore* g = &giocatori[indice_g];

    //cntrollo se il giocatore ha già effettuato un movimento in questo turno
    if (mossa_fatta) {
        printf("\n[ERRORE] Hai gia' effettuato un movimento (avanza/indietreggia) in questo turno.\n");
        printf("L'energia per attraversare le dimensioni e' esaurita per ora.\n");
        return true;
    }

    if (g->mondo == 0) {
        // --- MONDO REALE ---
        //Non si può cambiare mondo se c'è un nemico nella zona
        if (g->pos_mondoreale->nemico != NESSUN_NEMICO) {
            printf("\n[BLOCCATO] Un nemico (%s) ti sbarra la strada verso il portale!\n", 
                   nomi_nemici[g->pos_mondoreale->nemico]);
            printf("Devi sconfiggerlo o scappare (indietreggiare) prima di poter cambiare dimensione.\n");
            return false; 
        } else {
        // Se non c'è il nemico, il passaggio è libero (nessun dado richiesto da Reale a Soprasotto nelle tue specifiche)
        printf("\nIl varco dimensionale e' aperto. Ti lasci cadere nel vuoto...\n");
        g->mondo = 1;
        g->pos_soprasotto = g->pos_mondoreale->link_soprasotto;
        g->pos_mondoreale = NULL; // Per sicurezza mettiamo a NULL il puntatore dell'altro mondo
        printf("Sei entrato nel SOPRASOTTO!\n");
        return true;
        }
    } else {
        // --- SOPRASOTTO ---
        // Vincolo: Nessun blocco sui nemici, ma serve un tiro di Fortuna per scappare
        
        int lancio = lancia_dado(20);
        printf("\nCerchi disperatamente un'uscita verso la realta'...\n");
        printf("Tiro Dado: %d (La tua Fortuna: %d)\n", lancio, g->fortuna);

        // La specifica richiede un punteggio INFERIORE alla fortuna
        if (lancio < g->fortuna) {
            printf("Successo! Trovi uno squarcio nella materia oscura e ci salti dentro.\n");
            g->mondo = 0;
            g->pos_mondoreale = g->pos_soprasotto->link_mondoreale;
            g->pos_soprasotto = NULL;
            printf("Sei tornato nel MONDO REALE!\n");
            return true;
        } else {
            printf("Fallimento! Il Demotorzone distorce la realta' e l'uscita si chiude davanti ai tuoi occhi.\n");
            printf("Sei rimasto bloccato nel Soprasotto.\n");
            return true;//mettiamo true perché abbiamo comunque usato il nostro tentativo
        }
    }
    return true;//non vediamo mai questo return, lo mettiamo per sicurezza
}

static void combatti(int indice_g) {

    
    struct Giocatore* g = &giocatori[indice_g];
    Tipo_nemico nemico;
    
    // Identifica nemico
    if (g->mondo == 0) nemico = g->pos_mondoreale->nemico;
    else nemico = g->pos_soprasotto->nemico;

    if (nemico == NESSUN_NEMICO) {
        printf("Nessun nemico qui. Ti stai battendo con l'aria?\n");
        return;
    }

    printf("Combattimento contro %s!\n", nomi_nemici[nemico]);
    
    //valori per i nemici
    int forza_nemico = 0;
    switch(nemico) {
        case BILLI: forza_nemico = 15; break;
        case DEMOCANE: forza_nemico = 20; break;
        case DEMOTORZONE: forza_nemico = 30; break;
        default: break;
    }

    int tiro_attacco = lancia_dado(20);
    int potenza_totale = tiro_attacco + g->attacco_psichico;
    
    printf("Tiro dado (%d) + Attacco (%d) = %d VS Forza Nemico (%d)\n", 
            tiro_attacco, g->attacco_psichico, potenza_totale, forza_nemico);

    if (potenza_totale >= forza_nemico) {
        if(nemico == DEMOTORZONE){
            printf("Hai sconfitto il demortozone, ed hai vinto, complimenti per essere riuscito in questa impresa");
            return;
        }
        printf("Hai sconfitto %s!\n", nomi_nemici[nemico]);
        // 50% probabilità che scompaia
        if (lancia_dado(100) <= 50) {
            printf("Il nemico si dissolve nel nulla.\n");
            if (g->mondo == 0) g->pos_mondoreale->nemico = NESSUN_NEMICO;
            else g->pos_soprasotto->nemico = NESSUN_NEMICO;
        } else {
            printf("Il nemico è a terra ma respira ancora (rimane nella zona).\n");
        }
    } else {
        // Calcolo danno subito
        int difesa = lancia_dado(20) + g->difesa_psichica;
        printf("Il nemico contrattacca! Tua difesa tot: %d\n", difesa);
        if (difesa < forza_nemico) {
            printf("Colpo mortale! %s muore.\n", g->nome);
            g->stato = MORTO;
        } else {
            printf("Sei riuscito a parare il colpo all'ultimo!\n");
        }
    }
}

static void raccogli_oggetto(int indice_g) {
    struct Giocatore* g = &giocatori[indice_g];
    
    if (g->mondo == 1) {
        printf("Non ci sono oggetti nel Soprasotto.\n");
        return;
    }

    if (g->pos_mondoreale->oggetto == NESSUN_OGGETTO) {
        printf("Nessun oggetto qui.\n");
        return;
    }

    // Cerca spazio
    int slot = -1;
    for (int i=0; i<3; i++) {
        if (g->zaino[i] == NESSUN_OGGETTO) {
            slot = i;
            break;
        }
    }

    if (slot != -1) {
        g->zaino[slot] = g->pos_mondoreale->oggetto;
        printf("Hai raccolto: %s\n", nomi_oggetti[g->pos_mondoreale->oggetto]);
        g->pos_mondoreale->oggetto = NESSUN_OGGETTO; // Rimosso dalla terra
    } else {
        printf("Zaino pieno!\n");
    }
}

static void utilizza_oggetto(int indice_g) {
    struct Giocatore* g = &giocatori[indice_g];
    printf("Zaino:\n");
    for (int i=0; i<3; i++) printf("[%d] %s\n", i+1, nomi_oggetti[g->zaino[i]]);
    
    printf("Quale slot usare? (1-3, 0 annulla): ");
    int s; scanf("%d", &s); svuota_buffer();
    if (s<1 || s>3 || g->zaino[s-1] == NESSUN_OGGETTO) {
        printf("non succede nulla\n");
        return;}

    Tipo_oggetto obj = g->zaino[s-1];
    
    //effetti degli oggetti
    switch(obj) {
        case BICICLETTA:
            printf("Pedali veloce! Ottieni +5 Fortuna.\n");
            g->fortuna += 5;
            break;
        case MAGLIETTA_FUOCOINFERNO:
            printf("Stile aggressivo! +5 Attacco, -1 Difesa.\n");
            g->attacco_psichico += 5;
            if(g->difesa_psichica > 1) g->difesa_psichica--;
            break;
        case BUSSOLA:
            printf("Ti orienti meglio. Recuperi lucidità, +5 difesa.\n");
            g->difesa_psichica +=5 ;
            break;
        case SCHITARRATA_METALLICA:
            printf("I nemici tremano. (Uccide nemici minori istantaneamente).\n");
            if (g->mondo == 0 && g->pos_mondoreale->nemico != DEMOTORZONE) 
                g->pos_mondoreale->nemico = NESSUN_NEMICO;
            else if (g->mondo == 1 && g->pos_soprasotto->nemico != DEMOTORZONE)
                g->pos_soprasotto->nemico = NESSUN_NEMICO;
            break;
        default: break;
    }

    g->zaino[s-1] = NESSUN_OGGETTO; // Consumato
}

static void stampa_giocatore(int indice_g) {
    struct Giocatore* g = &giocatori[indice_g];
    printf("--- %s ---\n", g->nome);
    printf("Stato: %s\n", g->stato == VIVO ? "VIVO" : "MORTO");
    printf("Dimensione: %s\n", g->mondo == 0 ? "Reale" : "Soprasotto");
    printf("ATK: %d | DEF: %d | LUCK: %d\n", g->attacco_psichico, g->difesa_psichica, g->fortuna);
    printf("Zaino: [ %s, %s, %s ]\n", nomi_oggetti[g->zaino[0]], nomi_oggetti[g->zaino[1]], nomi_oggetti[g->zaino[2]]);
}

static void stampa_zona_corrente(int indice_g) {
    struct Giocatore* g = &giocatori[indice_g];
    Tipo_zona tipo;
    Tipo_nemico nem;
    Tipo_oggetto ogg = NESSUN_OGGETTO;

    if (g->mondo == 0) {
        tipo = g->pos_mondoreale->tipo;
        nem = g->pos_mondoreale->nemico;
        ogg = g->pos_mondoreale->oggetto;
    } else {
        tipo = g->pos_soprasotto->tipo;
        nem = g->pos_soprasotto->nemico;
    }

    printf("\nTi trovi in: %s\n", nomi_zone[tipo]);
    printf("Nemico: %s\n", nomi_nemici[nem]);
    if (g->mondo == 0) printf("Oggetto a terra: %s\n", nomi_oggetti[ogg]);
}
