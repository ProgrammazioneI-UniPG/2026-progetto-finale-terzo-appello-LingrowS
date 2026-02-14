// gamelib.h

#include <stdbool.h>

// Definizioni Enum
typedef enum {
    BOSCO, SCUOLA, LABORATORIO, CAVERNA, STRADA, GIARDINO,
    SUPERMERCATO, CENTRALE_ELETTRICA, DEPOSITO_ABBANDONATO, STAZIONE_POLIZIA
} Tipo_zona;

typedef enum {
    NESSUN_NEMICO, BILLI, DEMOCANE, DEMOTORZONE
} Tipo_nemico;

typedef enum {
    NESSUN_OGGETTO, BICICLETTA, MAGLIETTA_FUOCOINFERNO, BUSSOLA, SCHITARRATA_METALLICA
} Tipo_oggetto;

typedef enum {
    VIVO, MORTO
} Stato_giocatore;

//Dichiarazione per i puntatori incrociati
struct Zona_soprasotto;
struct Zona_mondoreale;

// Struttura Zona Mondo Reale
struct Zona_mondoreale {
    Tipo_zona tipo;
    Tipo_nemico nemico;
    Tipo_oggetto oggetto;
    struct Zona_mondoreale* avanti;
    struct Zona_mondoreale* indietro;
    struct Zona_soprasotto* link_soprasotto;
};

// Struttura Zona Soprasotto
struct Zona_soprasotto {
    Tipo_zona tipo;
    Tipo_nemico nemico;
    //Nel soprasotto non ci sono oggetti
    struct Zona_soprasotto* avanti;
    struct Zona_soprasotto* indietro;
    struct Zona_mondoreale* link_mondoreale;
};

// Struttura Giocatore
struct Giocatore {
    char nome[50];
    int mondo; // 0 = Mondo Reale, 1 = Soprasotto
    struct Zona_mondoreale* pos_mondoreale;
    struct Zona_soprasotto* pos_soprasotto;
    unsigned char attacco_psichico; // Valore 1-20 (usiamo char per risparmiare memoria)
    unsigned char difesa_psichica; // Valore 1-20
    unsigned char fortuna; // Valore 1-20
    Tipo_oggetto zaino[3];
    Stato_giocatore stato;
};

//Dichiarazione delle funzioni pubbliche chiamate dal main
void imposta_gioco(void);
void gioca(void);
void termina_gioco(void);
void crediti(void);
