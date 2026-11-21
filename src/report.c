#include "report.h"
#include <cairo.h>
#include <cairo-pdf.h>
#include <stdio.h>

void zrob_wykres(const Statystyki* dane, const char* plik_wyjsciowy) {
    int szer = 600;
    int wys = 400;

    cairo_surface_t* kartka = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, szer, wys);
    cairo_t* cr = cairo_create(kartka);

    // biale tlo
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    // tytul wykresu
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20);
    cairo_move_to(cr, 40, 45);
    cairo_show_text(cr, "Rozklad protokolow (liczba pakietow)");

    // dane do slupkow
    int wartosci[3] = { dane->tcp, dane->udp, dane->inne };
    const char* nazwy[3] = { "TCP", "UDP", "Inne" };

    // szukamy najwiekszej wartosci, zeby dobrze przeskalowac wysokosc slupkow
    int maks = 1;
    for (int i = 0; i < 3; i++) {
        if (wartosci[i] > maks) {
            maks = wartosci[i];
        }
    }

    // rysowanie 3 slupkow
    int start_x = 90;
    int dol_y = 330;
    int maks_wysokosc = 230;
    int szer_slupka = 90;
    int odstep = 150;

    cairo_set_font_size(cr, 14);
    for (int i = 0; i < 3; i++) {
        double wys_slupka = ((double)wartosci[i] / maks) * maks_wysokosc;
        int x = start_x + i * odstep;
        int y = dol_y - (int)wys_slupka;

        // slupek (niebieski)
        cairo_set_source_rgb(cr, 0.2, 0.5, 0.8);
        cairo_rectangle(cr, x, y, szer_slupka, wys_slupka);
        cairo_fill(cr);

        // liczba nad slupkiem
        char tekst[32];
        sprintf(tekst, "%d", wartosci[i]);
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, x + 25, y - 10);
        cairo_show_text(cr, tekst);

        // podpis pod slupkiem
        cairo_move_to(cr, x + 25, dol_y + 25);
        cairo_show_text(cr, nazwy[i]);
    }

    cairo_surface_write_to_png(kartka, plik_wyjsciowy);
    cairo_destroy(cr);
    cairo_surface_destroy(kartka);
}

void zrob_pdf(const Statystyki* dane, const char* plik_wyjsciowy) {
    // format A4 w punktach: 595 x 842
    cairo_surface_t* kartka = cairo_pdf_surface_create(plik_wyjsciowy, 595, 842);
    cairo_t* cr = cairo_create(kartka);

    char linia[256];
    int y = 60;

    // naglowek
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20);
    cairo_move_to(cr, 50, y);
    cairo_show_text(cr, "Raport z analizy pliku PCAP");

    // statystyki ogolne
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);

    y += 40;
    sprintf(linia, "Plik: %s", dane->nazwa_pliku);
    cairo_move_to(cr, 50, y);
    cairo_show_text(cr, linia);

    y += 22;
    sprintf(linia, "Wszystkie pakiety: %lld (bajtow: %lld)", dane->wszystkie_pakiety, dane->wszystkie_bajty);
    cairo_move_to(cr, 50, y);
    cairo_show_text(cr, linia);

    y += 22;
    sprintf(linia, "TCP: %d   |   UDP: %d   |   Inne: %d", dane->tcp, dane->udp, dane->inne);
    cairo_move_to(cr, 50, y);
    cairo_show_text(cr, linia);

    // flagi TCP
    y += 40;
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_move_to(cr, 50, y);
    cairo_show_text(cr, "Flagi TCP:");

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    y += 22;
    sprintf(linia, "SYN: %d   |   ACK: %d   |   FIN: %d   |   RST: %d", dane->syn, dane->ack, dane->fin, dane->rst);
    cairo_move_to(cr, 50, y);
    cairo_show_text(cr, linia);

    // porty
    y += 40;
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_move_to(cr, 50, y);
    cairo_show_text(cr, "Najczestsze porty:");

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    y += 22;
    sprintf(linia, "80 (HTTP): %d  |  443 (HTTPS): %d  |  53 (DNS): %d  |  Inne: %d",
        dane->port_80, dane->port_443, dane->port_53, dane->port_inne);
    cairo_move_to(cr, 50, y);
    cairo_show_text(cr, linia);

    // nieprawidlowosci
    y += 45;
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_move_to(cr, 50, y);
    cairo_show_text(cr, "Wykryte nieprawidlowosci:");

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    if (dane->ile_alertow == 0) {
        y += 22;
        cairo_move_to(cr, 50, y);
        cairo_show_text(cr, "Brak wykrytych anomalii.");
    }
    else {
        for (int i = 0; i < dane->ile_alertow; i++) {
            y += 22;
            sprintf(linia, "- %s", dane->alerty[i]);
            cairo_move_to(cr, 50, y);
            cairo_show_text(cr, linia);
        }
    }

    cairo_show_page(cr);
    cairo_destroy(cr);
    cairo_surface_destroy(kartka);
}