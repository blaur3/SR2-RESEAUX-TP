/*************************************************************
* proto_tdd_v0 -  émetteur                                   *
* TRANSFERT DE DONNEES  v1                                   *
*                                                            *
* Protocole Selective Repeat         *
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

#define TIMER 150
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
    int ack_recu[NUMEROTATION] = {0};

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
    while ( taille_msg != 0 || borne_inf != prochain_paquet ) {

        
         //Si on a des paquets à envoyer
        if(dans_fenetre(borne_inf, prochain_paquet, fenetre) && taille_msg > 0) //Si le prochain paquet est dans la fenêtre
        {
            
            /* Construction paquet */
            
            for (int i=0; i<taille_msg; i++) {
                tab_paquet[prochain_paquet].info[i] = message[i];
            }
            tab_paquet[prochain_paquet].lg_info = taille_msg;
            tab_paquet[prochain_paquet].type = DATA;
            tab_paquet[prochain_paquet].num_seq = prochain_paquet;
            tab_paquet[prochain_paquet].somme_ctrl = somme_de_controle(&tab_paquet[prochain_paquet]);

            /* Depart temporisateur */
            depart_temporisateur_num(prochain_paquet, TIMER);

            /* Envoi du paquet */
            printf("\nEnvoi du paquet %d, borne_inf = %d\n", prochain_paquet, borne_inf);
            vers_reseau(&tab_paquet[prochain_paquet]);

            /* Incrementation du numéro de prochain paquet */
            prochain_paquet = inc(prochain_paquet, NUMEROTATION);

            /* lecture des donnees suivantes de la couche application */
            de_application(message, &taille_msg);

        }
        else{//On attend un acquittement
            event = attendre();

            // ACK recu
            if(event == PAQUET_RECU){
                /* Reception du paquet d'acquittement */
                de_reseau(&paquet_recu);

                /* Si il n'y a pas d'erreur dans le paquet d'acquittement et qu'il est dans la fenêtre*/
                if(verifier_controle(&paquet_recu) && dans_fenetre(borne_inf, paquet_recu.num_seq, fenetre)){

                    ack_recu[paquet_recu.num_seq] = 1;

                    /* Arrêt du temporisateur */
                    arret_temporisateur_num(paquet_recu.num_seq);

                     while (ack_recu[borne_inf]) {
                        ack_recu[borne_inf] = 0; // on réinitialise (optionnel)
                        borne_inf = inc(borne_inf, NUMEROTATION);
                    }
                }
            }

            // Temps écoulé
            else{
                /* On redémarre le temporisateur */
                depart_temporisateur_num(event, TIMER);
                printf("\nTemps ecoule pour le paquet %d, retransmission\n", event);
                /* On renvoie le paquet non acquitté */
                vers_reseau(&tab_paquet[event]);
            }
        }

    }



    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
