#include <stdio.h>
#include "types.h"
#include "parser.h"
#include "report.h"
#include <pcap.h>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Podaj plik: %s <plik.pcap>\n", argv[0]);
        return 1;
    }

    Statystyki dane;

    if (wczytaj_pcap(argv[1], &dane) != 0)
    {
        printf("Blad wczytywania pliku\n");
        return 1;
    }


    if (zrob_wykresy(&dane) != 0) {
        printf("Blad generowania wykresow\n");
        return 1;
    }
    if (zrob_pdf(&dane, "raport.pdf") != 0) {
        printf("Blad generowania raportu\n");
        return 1;
    }


    return 0;
}