#ifndef TYPES_H
#define TYPES_H

#define MAX_ALERTOW 30
#define MAX_PAR_IP 10
#define MAX_SEKUND 1200  //20 minut do wykresu ruchu 
typedef struct {
    int src_ip; //IP hosta zrodlowego
    int dst_ip; //IP hosta do którego dociera

    int pakiety;
    long long bajty;

} ParaIP;

typedef struct {
    char nazwa_pliku[128];
    int linktype; //protokoly warstwy łacza danych - Ethernet, WiFi itp.
    double czas_trwania;

    //Do statystyk ogolnych
    long long wszystkie_pakiety;
    long long wszystkie_bajty;

    //TCP vs UDP i inne protokoly
    long long tcp_pakiety;
    long long udp_pakiety;
    long long inne_pakiety;

    long long tcp_bajty;
    long long udp_bajty;
    long long inne_bajty;

    //Flagi
    int syn;
    int ack;
    int fin;
    int rst;

    //Porty
    int porty[65536];

    //Pary IP - najbardziej aktuwne
    ParaIP pary_ip[MAX_PAR_IP];
    int liczba_par_ip;

    //Ruch w czasie
    long long bajty_na_sekunde[MAX_SEKUND];
    long long pakiety_na_sekunde[MAX_SEKUND];

    //Anomalie 
    int ile_alertow;
    char alerty[MAX_ALERTOW][128];
} Statystyki;

#endif

/*
Wartosci wyliczane podczas generowania raportu:

procentowy udział TCP, UDP, innych; 

przepustowosc

top_porty

*/