
#include "charts.h"

#include <cairo.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define W 900
#define H 500

/* Kolory */
#define NAVY_R 0.08
#define NAVY_G 0.16
#define NAVY_B 0.28

#define BLUE_R 0.15
#define BLUE_G 0.42
#define BLUE_B 0.75

#define TEXT_R 0.12
#define TEXT_G 0.14
#define TEXT_B 0.18

#define GRID_R 0.87
#define GRID_G 0.89
#define GRID_B 0.92

/* Naglowek wykresu */
static void naglowek(cairo_t* cr, const char* tytul)
{
    /* Biale tlo */
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    /* Granatowy pasek */
    cairo_set_source_rgb(cr, NAVY_R, NAVY_G, NAVY_B);
    cairo_rectangle(cr, 0, 0, W, 70);
    cairo_fill(cr);

    /* Bialy tytul */
    cairo_select_font_face(
        cr,
        "Sans",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_BOLD
    );

    cairo_set_font_size(cr, 25);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_move_to(cr, 35, 45);
    cairo_show_text(cr, tytul);
}

/* Rysowanie tekstu */
static void tekst(
    cairo_t* cr,
    double x,
    double y,
    const char* napis,
    double rozmiar)
{
    cairo_select_font_face(
        cr,
        "Sans",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_NORMAL
    );

    cairo_set_font_size(cr, rozmiar);
    cairo_set_source_rgb(cr, TEXT_R, TEXT_G, TEXT_B);

    cairo_move_to(cr, x, y);
    cairo_show_text(cr, napis);
}

/* Linie pomocnicze */
static void linie_siatki(cairo_t* cr)
{
    cairo_set_source_rgb(cr, GRID_R, GRID_G, GRID_B);
    cairo_set_line_width(cr, 1);

    for (int y = 120; y <= 390; y += 54) {
        cairo_move_to(cr, 70, y);
        cairo_line_to(cr, 850, y);
    }

    cairo_stroke(cr);
}

/* Zapis wykresu PNG */
static int zapisz_png(
    cairo_surface_t* surface,
    const char* nazwa)
{
    cairo_status_t status =
        cairo_surface_write_to_png(surface, nazwa);

    cairo_surface_destroy(surface);

    if (status != CAIRO_STATUS_SUCCESS) {
        fprintf(stderr, "Blad zapisu wykresu: %s\n", nazwa);
        return -1;
    }

    return 0;
}

/* ==========================================
   WYKRES 1 - ROZKLAD PROTOKOLOW
   ========================================== */

static int wykres_protokolow(const Statystyki* dane)
{
    cairo_surface_t* surface =
        cairo_image_surface_create(
            CAIRO_FORMAT_ARGB32,
            W,
            H
        );

    cairo_t* cr = cairo_create(surface);

    naglowek(cr, "Rozklad protokolow");
    linie_siatki(cr);

    long long wartosci[] = {
        dane->tcp_pakiety,
        dane->udp_pakiety,
        dane->inne_pakiety
    };

    const char* nazwy[] = {
        "TCP",
        "UDP",
        "Inne"
    };

    long long maksimum = 1;

    for (int i = 0; i < 3; i++) {
        if (wartosci[i] > maksimum)
            maksimum = wartosci[i];
    }

    for (int i = 0; i < 3; i++) {

        double x = 180 + i * 220;
        double wysokosc =
            270.0 * wartosci[i] / maksimum;

        cairo_set_source_rgb(
            cr,
            BLUE_R,
            BLUE_G,
            BLUE_B
        );

        cairo_rectangle(
            cr,
            x,
            390 - wysokosc,
            110,
            wysokosc
        );

        cairo_fill(cr);

        char liczba[64];

        snprintf(
            liczba,
            sizeof(liczba),
            "%lld",
            wartosci[i]
        );

        tekst(
            cr,
            x + 25,
            420,
            nazwy[i],
            18
        );

        tekst(
            cr,
            x + 25,
            380 - wysokosc,
            liczba,
            16
        );
    }

    cairo_destroy(cr);

    return zapisz_png(surface, "protokoly.png");
}

/* ==========================================
   WYKRES 2 - FLAGI TCP
   ========================================== */

static int wykres_flag_tcp(const Statystyki* dane)
{
    cairo_surface_t* surface =
        cairo_image_surface_create(
            CAIRO_FORMAT_ARGB32,
            W,
            H
        );

    cairo_t* cr = cairo_create(surface);

    naglowek(cr, "Charakterystyka flag TCP");
    linie_siatki(cr);

    int wartosci[] = {
        dane->syn,
        dane->ack,
        dane->fin,
        dane->rst
    };

    const char* nazwy[] = {
        "SYN",
        "ACK",
        "FIN",
        "RST"
    };

    int maksimum = 1;

    for (int i = 0; i < 4; i++) {
        if (wartosci[i] > maksimum)
            maksimum = wartosci[i];
    }

    for (int i = 0; i < 4; i++) {

        double x = 100 + i * 190;

        double wysokosc =
            270.0 * wartosci[i] / maksimum;

        cairo_set_source_rgb(
            cr,
            BLUE_R,
            BLUE_G,
            BLUE_B
        );

        cairo_rectangle(
            cr,
            x,
            390 - wysokosc,
            100,
            wysokosc
        );

        cairo_fill(cr);

        char liczba[64];

        snprintf(
            liczba,
            sizeof(liczba),
            "%d",
            wartosci[i]
        );

        tekst(
            cr,
            x + 25,
            420,
            nazwy[i],
            18
        );

        tekst(
            cr,
            x + 25,
            380 - wysokosc,
            liczba,
            16
        );
    }

    cairo_destroy(cr);

    return zapisz_png(surface, "flagi_tcp.png");
}

/* ==========================================
   WYKRES 3 - TOP 10 PORTOW
   ========================================== */

static int wykres_top_portow(const Statystyki* dane)
{
    cairo_surface_t* surface =
        cairo_image_surface_create(
            CAIRO_FORMAT_ARGB32,
            W,
            H
        );

    cairo_t* cr = cairo_create(surface);

    naglowek(cr, "Top 10 najaktywniejszych portow");

    int kopia[65536];

    memcpy(
        kopia,
        dane->porty,
        sizeof(kopia)
    );

    int maksimum = 1;

    for (int i = 0; i < 65536; i++) {
        if (kopia[i] > maksimum)
            maksimum = kopia[i];
    }

    for (int i = 0; i < 10; i++) {

        int port = -1;
        int wartosc = 0;

        for (int j = 0; j < 65536; j++) {

            if (kopia[j] > wartosc) {
                wartosc = kopia[j];
                port = j;
            }
        }

        if (port == -1)
            break;

        double y = 90 + i * 37;

        double szerokosc =
            550.0 * wartosc / maksimum;

        char etykieta[64];

        snprintf(
            etykieta,
            sizeof(etykieta),
            "%d",
            port
        );

        tekst(
            cr,
            75,
            y + 18,
            etykieta,
            15
        );

        cairo_set_source_rgb(
            cr,
            BLUE_R,
            BLUE_G,
            BLUE_B
        );

        cairo_rectangle(
            cr,
            160,
            y,
            szerokosc,
            23
        );

        cairo_fill(cr);

        snprintf(
            etykieta,
            sizeof(etykieta),
            "%d",
            wartosc
        );

        tekst(
            cr,
            170 + szerokosc,
            y + 18,
            etykieta,
            14
        );

        kopia[port] = -1;
    }

    cairo_destroy(cr);

    return zapisz_png(surface, "top_porty.png");
}

/* ==========================================
   WYKRES 4 - RUCH SIECIOWY W CZASIE
   ========================================== */

static int wykres_ruchu(const Statystyki* dane)
{
    cairo_surface_t* surface =
        cairo_image_surface_create(
            CAIRO_FORMAT_ARGB32,
            W,
            H
        );

    cairo_t* cr = cairo_create(surface);

    naglowek(cr, "Intensywnosc ruchu w czasie");

    linie_siatki(cr);

    int liczba = (int)dane->czas_trwania;

    if (liczba < 1)
        liczba = 1;

    if (liczba > MAX_SEKUND)
        liczba = MAX_SEKUND;

    long long maksimum = 1;

    for (int i = 0; i < liczba; i++) {

        if (dane->bajty_na_sekunde[i] > maksimum)
            maksimum = dane->bajty_na_sekunde[i];
    }

    cairo_set_source_rgb(
        cr,
        BLUE_R,
        BLUE_G,
        BLUE_B
    );

    cairo_set_line_width(cr, 3);

    for (int i = 0; i < liczba; i++) {

        double x =
            70 + 780.0 * i / (liczba > 1 ? liczba - 1 : 1);

        double y =
            390 - 270.0 *
            dane->bajty_na_sekunde[i] / maksimum;

        if (i == 0)
            cairo_move_to(cr, x, y);
        else
            cairo_line_to(cr, x, y);
    }

    cairo_stroke(cr);

    tekst(
        cr,
        400,
        455,
        "Czas (sekundy)",
        16
    );

    tekst(
        cr,
        75,
        105,
        "Bajty / sekunde",
        14
    );

    cairo_destroy(cr);

    return zapisz_png(surface, "ruch_w_czasie.png");
}

/* ==========================================
   GLOWNA FUNKCJA GENERUJACA WYKRESY
   ========================================== */

int zrob_wykresy(const Statystyki* dane)
{
    if (dane == NULL) {

        fprintf(
            stderr,
            "Brak danych do wygenerowania wykresow.\n"
        );

        return -1;
    }

    if (wykres_protokolow(dane) != 0)
        return -1;

    if (wykres_flag_tcp(dane) != 0)
        return -1;

    if (wykres_top_portow(dane) != 0)
        return -1;

    if (wykres_ruchu(dane) != 0)
        return -1;

    printf("Wygenerowano cztery wykresy PNG.\n");

    return 0;
}
