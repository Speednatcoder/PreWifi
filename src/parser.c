#include "parser.h"
#include <stdio.h>
#include <string.h>

static void sprawdz_anomalie(Statystyki* dane) {
    if (dane->syn > 50 && dane->syn > dane->ack && dane->ile_alertow < 20) {
        strcpy(dane->alerty[dane->ile_alertow], "Duzo pakietow SYN bez odpowiedzi");
        dane->ile_alertow++;
    }

    if (dane->rst > 10 && dane->ile_alertow < 20) {
        strcpy(dane->alerty[dane->ile_alertow], "Sporo zerwanych polaczen (RST)");
        dane->ile_alertow++;
    }
}

int wczytaj_pcap(const char* sciezka, Statystyki* dane) {
    memset(dane, 0, sizeof(Statystyki));
    strncpy(dane->nazwa_pliku, sciezka, 127);

    FILE* plik = fopen(sciezka, "rb");
    if (plik == NULL) {
        printf("Nie mozna otworzyc pliku: %s\n", sciezka);
        return -1;
    }

    /* miejsce na 
    odczyt pliku .pcap
    wczytac fread 24 bajty Global Header i sprawdzic czy linktype ==1(Ethernet)
    w petli while wczytywac koljne pakiety
    dla kazdego pakietu dodawac kolejne liczniki w strukturze dane (++)
    jak chcecie to zmienic dane sprawdz anomalie fukcje jak maja byc inne liczby
    jak chcecie dodac inne dane w strukturze types.h to musicie mi powiedziec bo musze wtedy zmienic report.c
    */

     //testowe dane
     dane->czas_trwania = 12.5;
     dane->tcp = 850;
     dane->udp = 320;
     dane->inne = 45;
     dane->wszystkie_pakiety = dane->tcp + dane->udp + dane->inne;
     dane->wszystkie_bajty = 984500;

     dane->syn = 140;
     dane->ack = 110;
     dane->fin = 60;
     dane->rst = 18;

     dane->port_80 = 120;
     dane->port_443 = 680;
     dane->port_53 = 290;
     dane->port_inne = 125;
     
     // koniec testowych danych
    fclose(plik);

    sprawdz_anomalie(dane);
    return 0;
}