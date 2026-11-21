#include <stdio.h>
#include "types.h"
#include "parser.h"
#include "report.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Podaj plik: %s <plik.pcap>\n", argv[0]);
        return 1;
    }

    Statystyki dane;

    if (wczytaj_pcap(argv[1], &dane) != 0) {
        printf("Blad wczytywania pliku\n");
        return 1;
    }

    zrob_wykres(&dane, "wykres.png");
    zrob_pdf(&dane, "raport.pdf");

    printf("Gotowe.\n");
    return 0;
}