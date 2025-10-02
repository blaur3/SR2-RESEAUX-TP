/*************************************************************
* proto_tdd_v0 -  récepteur                                  *
* TRANSFERT DE DONNEES  v0                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t paquet; /* paquet utilisé par le protocole */
    paquet_t pack;
    int paquet_attendu = 0;
    int fin = 0; /* condition d'arrêt */
    int dernier_paquet=0;

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        // attendre(); /* optionnel ici car de_reseau() fct bloquante */
        de_reseau(&paquet);

        // verification des eventuelles erreurs */
        uint8_t s = somme_de_controle(&paquet);
        printf("\nPaquet %d recu, paquet attendu : %d, dernier paquet : %d ***********\n", paquet.num_seq, paquet_attendu, dernier_paquet);
        
        if( (!verifier_controle(&paquet, s)) || !(paquet_attendu == paquet.num_seq) ){
            printf("s : %d,  s_recu : %d", s, verifier_controle(&paquet, s) );
            pack.num_seq = dernier_paquet;
        }
        else{

                printf("s : %d,  s_recu : %d", s, verifier_controle(&paquet, s) );
                printf("Le paquet n'a pas d'erreur\n");            
                /* extraction des donnees du paquet recu */
                for (int i=0; i<paquet.lg_info; i++) {
                    message[i] = paquet.info[i];
                }
                /* remise des données à la couche application */
                dernier_paquet = paquet.num_seq;
                printf("L'incrementation devrait etre %d \n", inc(paquet_attendu, NUMEROTATION));
                paquet_attendu = inc(paquet_attendu, NUMEROTATION);
                fin = vers_application(message, paquet.lg_info);
            }
            pack.type = ACK;
            pack.lg_info = 0;
            pack.num_seq = dernier_paquet;
            pack.somme_ctrl = somme_de_controle(&pack);
                // envoi de l'acquittement */
            vers_reseau(&pack);
          }

        
     
       
    

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}