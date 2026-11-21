#ifndef TYPES_H
#define TYPES_H

typedef struct {
    char nazwa_pliku[128];
    double czas_trwania;

    long long wszystkie_pakiety;
    long long wszystkie_bajty;
    int tcp;
    int udp;
    int inne;

    int syn;
    int ack;
    int fin;
    int rst;

    int port_80;
    int port_443;
    int port_53;
    int port_inne;

    int ile_alertow;
    char alerty[20][128];
} Statystyki;

#endif