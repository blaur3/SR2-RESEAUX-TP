/*************************************************************
* proto_tdd_v0 -  émetteur                                   *
* TRANSFERT DE DONNEES  v1                                   *
*                                                            *
* Protocole Stop-and-Wait avec acquittement négatif          *
*                                                            *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */

#define TIMER 100

int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t paquet; /* paquet utilisé par le protocole */
    paquet_t paquet_recu;
    paquet.num_seq = 0;
    int event;

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 ) {

        /* construction paquet */
        for (int i=0; i<taille_msg; i++) {
            paquet.info[i] = message[i];
        }
        paquet.lg_info = taille_msg;
        paquet.type = DATA;
        paquet.somme_ctrl = somme_de_controle(&paquet);

        /* remise à la couche reseau et départ du temporisateur */
        vers_reseau(&paquet);
        depart_temporisateur(TIMER);

        /* retransmission du paquet corrompu ou*/        
        while((event=attendre()) != PAQUET_RECU){
            vers_reseau(&paquet);
            depart_temporisateur(TIMER);
        } 
        arret_temporisateur(TIMER);
        de_reseau(&paquet_recu);
        paquet.num_seq=inc(paquet.num_seq,2);


    
    /* lecture des donnees suivantes de la couche application */
        de_application(message, &taille_msg);
}

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}