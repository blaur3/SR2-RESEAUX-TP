#include <stdio.h>
#include "couche_transport.h"
#include "services_reseau.h"
#include "application.h"

/* ************************************************************************** */
/* *************** Fonctions utilitaires couche transport ******************* */
/* ************************************************************************** */

// RAJOUTER VOS FONCTIONS DANS CE FICHIER...



/*--------------------------------------*/
/* Fonction d'inclusion dans la fenetre */
/*--------------------------------------*/
int dans_fenetre(unsigned int inf, unsigned int pointeur, int taille) {

    unsigned int sup = (inf+taille-1) % SEQ_NUM_SIZE;

    return
        /* inf <= pointeur <= sup */
        ( inf <= sup && pointeur >= inf && pointeur <= sup ) ||
        /* sup < inf <= pointeur */
        ( sup < inf && pointeur >= inf) ||
        /* pointeur <= sup < inf */
        ( sup < inf && pointeur <= sup);
}


uint8_t somme_de_controle(paquet_t * p){
    uint8_t s = p->type ^ p->num_seq ^ p->lg_info;
    for(int i = 0; i<p->lg_info; i++){
        s = s ^ p->info[i];
    }
    return s;
}

int verifier_controle(paquet_t* p){
    uint8_t s = somme_de_controle(p);
    return p->somme_ctrl == s;
}


int inc(int num, int modulo){
    return (num+1)%modulo;
}



