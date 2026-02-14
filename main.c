// main.c

#include <stdio.h>
#include <stdlib.h>
#include "gamelib.h"

//Funzione main, tutto ha inizio da qui
int main(void) {
    int scelta = 0;

    printf("\n--- BENVENUTI A OCCHINZ ---\n");

    do {
        printf("MENU PRINCIPALE:\n");
        printf("1) Imposta gioco\n");
        printf("2) Gioca\n");
        printf("3) Termina gioco\n");
        printf("4) Crediti\n");
        printf("Inserisci la tua scelta: ");

        //scanf salta automaticamente tab, spazi e /n
        if (scanf("%d", &scelta) != 1) { //scanf con %d ritorna 1 quando legge un intero, torna 0 se non può convertire ad intero (es: CIAO, 12Az)
            // In questa zona gestiamo l'input non numerico
            scelta = 0;//impostiamo la scelta ad un valore che non ci fa proseguire(0 in questo caso non corrisponde a nessun input utile)
        }
        
        //Puliamo il buffer da eventuali residui
        while (getchar() != '\n');

        switch (scelta) {
            case 1:
                system("clear");
                imposta_gioco();
                break;
            case 2:
                system("clear");
                gioca();
                break;
            case 3:
                system("clear");
                termina_gioco();
                break;
            case 4:
                system("clear");
                crediti();
                break;
            default:
                system("clear");
                printf("\n[ERRORE] Scelta non valida. Riprova.\n");
                break;
        }

    } while (scelta != 3);

    return 0;
}
