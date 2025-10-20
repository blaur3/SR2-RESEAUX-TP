/*************************************************************
 * proto_tdd_v0 -  récepteur                                  *
 * TRANSFERT DE DONNEES  v0                                   *
 *                                                            *
 * Protocole Selective Repeat  *
 *                                                            *
 *      *
 **************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

#define TIMER 500

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char *argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t paquet; 
    paquet_t paquets_recu[NUMEROTATION];                /* paquet utilisé par le protocole */
    paquet_t pack;
    int paquet_attendu = 0;
    int fin = 0; /* condition d'arrêt */
    int borne_inf = 0;
    

    for (int i = 0; i < NUMEROTATION; i++)
        paquets_recu[i].lg_info = 0;

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");




    /* tant que le récepteur reçoit des données */
    while (!fin)
    {

        // attendre(); /* optionnel ici car de_reseau() fct bloquante */
        de_reseau(&paquet);
        printf("\nPaquet %d recu, paquet attendu : %d ***********\n", paquet.num_seq, paquet_attendu);
        
        //Si erreur dans le paquet
        if ((!verifier_controle(&paquet)))
        {
            /* On ne renvoie rien */
        }

        //Pas d'erreur dans le paquet
        else{
            if(dans_fenetre(borne_inf, paquet.num_seq, NUMEROTATION/2)){
                /*Historisation du paquet reçu*/
                if(paquets_recu[paquet.num_seq].lg_info == 0){
                    paquets_recu[paquet.num_seq] = paquet;
                }
            }
            //Si le paquet est hors séquence    
            if(!(paquet_attendu == paquet.num_seq))
            {
                printf("Le paquet est hors séquence, num seq = %d\n", paquet.num_seq);
                
                
            }
            //Si le paquet est en séquence
            else{

                /*Traitement de tous les paquets en séquence*/
                for(int i = 0; i < NUMEROTATION; i++){

                    //Si le paquet est en séquence
                    if(paquets_recu[paquet_attendu].lg_info > 0){

                        /* extraction des données du paquet recu */
                        for(int j = 0; j < paquets_recu[paquet_attendu].lg_info; j++)
                            message[j] = paquets_recu[paquet_attendu].info[j];

                        /* Envoi des données à l'application */
                        fin = vers_application(message, paquets_recu[paquet_attendu].lg_info);

                        /*Déhistorisation du paquet de données retransmises*/
                        paquets_recu[paquet_attendu].lg_info = 0;

                        /*Incrementation du numéro de paquet attendu*/
                        paquet_attendu = inc(paquet_attendu, NUMEROTATION);

                        /*Decalage de la fenetre*/
                        borne_inf = inc(borne_inf, NUMEROTATION);
                        
                        
                        
                    }
                }
                
            }

                /* Construction d'un paquet d'acquittement*/
                pack.type = ACK;
                pack.num_seq = paquet.num_seq;
                pack.lg_info = 0;
                pack.somme_ctrl = somme_de_controle(&pack);
                
                /* envoi de l'acquittement */
                vers_reseau(&pack);
        
        }
    }

    //En cas de perte du dernier ACK
    depart_temporisateur(TIMER);
    while(attendre() == PAQUET_RECU){
        arret_temporisateur();
        de_reseau(&paquet);
        vers_reseau(&pack);
        depart_temporisateur(TIMER);
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}