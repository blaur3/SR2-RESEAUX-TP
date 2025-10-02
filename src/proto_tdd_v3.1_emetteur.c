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
#include <stdlib.h>
#include <string.h>

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */

#define TIMER 100
#define FENETRE 4


int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t tab_paquet[NUMEROTATION]; /* paquet utilisé par le protocole */
    paquet_t paquet_recu;
    unsigned int prochain_paquet = 0;
    unsigned int borne_inf = 0;
    int event = 4;
    int fenetre = 0;

    //Initialisation de la taille de la fenêtre
    if(argc > 1){
        // Utilise l'argument passé en ligne de commande
        fenetre = (atoi(argv[1])<16 && atoi(argv[1])>0) ? (unsigned int)strtol(argv[1], NULL, 10) : FENETRE;

    } else {
        //Utilise la taille de la fenêtre par défaut
        fenetre = FENETRE;
    }

    printf("Taille de la fenêtre : %u\n", fenetre);

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture des donnees suivantes de la couche application */
        de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 || (tab_paquet[prochain_paquet].num_seq != paquet_recu.num_seq) ) {

         //Si on a des paquets à envoyer
        if(dans_fenetre(borne_inf, prochain_paquet, fenetre)) //Si le prochain paquet est dans la fenêtre
        {
            
            /* construction paquet */
            
            for (int i=0; i<taille_msg; i++) {
                tab_paquet[prochain_paquet].info[i] = message[i];
            }
            tab_paquet[prochain_paquet].lg_info = taille_msg;
            tab_paquet[prochain_paquet].type = DATA;
            tab_paquet[prochain_paquet].num_seq = prochain_paquet;
            tab_paquet[prochain_paquet].somme_ctrl = somme_de_controle(&tab_paquet[prochain_paquet]);
            vers_reseau(&tab_paquet[prochain_paquet]);
            if(borne_inf == prochain_paquet){
                depart_temporisateur(TIMER);
            }
            prochain_paquet = inc(prochain_paquet, NUMEROTATION);
            if(taille_msg != 0)
            {    
             /* lecture des donnees suivantes de la couche application */
                de_application(message, &taille_msg);}

        }
        else{//On attend un acquittement
            event = attendre();
            if(event == PAQUET_RECU){
                de_reseau(&paquet_recu);
                printf("\nACK RECU\n");
                uint8_t s = somme_de_controle(&paquet_recu);
                if(verifier_controle(&paquet_recu, s) && dans_fenetre(borne_inf, paquet_recu.num_seq, fenetre)){
                    borne_inf = inc(paquet_recu.num_seq, NUMEROTATION);
                    if(borne_inf == prochain_paquet){
                        arret_temporisateur();
                    }
                    
                }   
            
            }

        else{
            printf("\nTEMPS ECROULE\n");
            int i = borne_inf;
            depart_temporisateur(TIMER);
            while(i != prochain_paquet){
                vers_reseau(&tab_paquet[i]);
                i = inc(i, NUMEROTATION);
        }
        }
    }

    
    }


    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
