#include "report.h"
#include <cairo.h>
#include <cairo-pdf.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>


#define PAGE_WIDTH 595
#define PAGE_HEIGHT 842

#define MARGIN_LEFT 50
#define MARGIN_RIGHT 50
#define CONTENT_WIDTH (PAGE_WIDTH - MARGIN_LEFT - MARGIN_RIGHT)

#define COLOR_NAVY_R 0.12
#define COLOR_NAVY_G 0.20
#define COLOR_NAVY_B 0.32

#define COLOR_BLUE_R 0.25
#define COLOR_BLUE_G 0.48
#define COLOR_BLUE_B 0.72

#define COLOR_TEXT_R 0.20
#define COLOR_TEXT_G 0.23
#define COLOR_TEXT_B 0.28

#define COLOR_MUTED_R 0.48
#define COLOR_MUTED_G 0.52
#define COLOR_MUTED_B 0.57

#define COLOR_LIGHT_R 0.93
#define COLOR_LIGHT_G 0.95
#define COLOR_LIGHT_B 0.97

#define MAX_TEXT 512

#define BOTTOM_LIMIT 780

typedef struct {
    cairo_t* cr;
    int y;
    int page_number;
} RaportPDF;


/* Nazwa typu lacza danych */
const char* linktype_nazwa(int linktype)
{
    switch (linktype) {
    case 1:
        return "Ethernet";

    case 105:
        return "IEEE 802.11 (WiFi)";


    default:
        return "Nieznany";
    }
}


/* Ustawienie koloru tekstu */
static void ustaw_kolor_tekstu(cairo_t* cr)
{
    cairo_set_source_rgb(
        cr,
        COLOR_TEXT_R,
        COLOR_TEXT_G,
        COLOR_TEXT_B
    );
}


/* Pocz¹tek nowej strony */
static void rozpocznij_strone(RaportPDF* pdf)
{
    cairo_t* cr = pdf->cr;

    pdf->page_number++;

    /* Biale tlo */
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    /* Pasek naglowka */
    cairo_set_source_rgb(
        cr,
        COLOR_NAVY_R,
        COLOR_NAVY_G,
        COLOR_NAVY_B
    );

    cairo_rectangle(cr, 0, 0, PAGE_WIDTH, 65);
    cairo_fill(cr);

    /* Tytul */
    cairo_select_font_face(
        cr,
        "Sans",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_BOLD
    );

    cairo_set_font_size(cr, 19);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);

    cairo_move_to(cr, MARGIN_LEFT, 40);
    cairo_show_text(cr, "Raport z analizy ruchu sieciowego");

    pdf->y = 105;
}


/* Stopka i zakonczenie strony */
static void zakoncz_strone(RaportPDF* pdf)
{
    cairo_t* cr = pdf->cr;

    cairo_set_source_rgb(
        cr,
        COLOR_LIGHT_R,
        COLOR_LIGHT_G,
        COLOR_LIGHT_B
    );

    cairo_rectangle(
        cr,
        MARGIN_LEFT,
        800,
        CONTENT_WIDTH,
        1
    );

    cairo_fill(cr);

    cairo_select_font_face(
        cr,
        "Sans",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_NORMAL
    );

    cairo_set_font_size(cr, 9);

    cairo_set_source_rgb(
        cr,
        COLOR_MUTED_R,
        COLOR_MUTED_G,
        COLOR_MUTED_B
    );

    cairo_move_to(cr, MARGIN_LEFT, 820);
    cairo_show_text(cr, "Analiza ruchu sieciowego");

    char numer[32];

    snprintf(
        numer,
        sizeof(numer),
        "Strona %d",
        pdf->page_number
    );

    cairo_text_extents_t extents;
    cairo_text_extents(cr, numer, &extents);

    cairo_move_to(
        cr,
        PAGE_WIDTH - MARGIN_RIGHT - extents.width,
        820
    );

    cairo_show_text(cr, numer);

    cairo_show_page(cr);
}


/* Sprawdzenie, czy pozostalo miejsce na tekst */
static void sprawdz_miejsce(RaportPDF* pdf, int wysokosc)
{
    if (pdf->y + wysokosc > BOTTOM_LIMIT) {
        zakoncz_strone(pdf);
        rozpocznij_strone(pdf);
    }
}


/* Rysowanie pojedynczej linii */
static void rysuj_linie(RaportPDF* pdf, const char* tekst)
{
    sprawdz_miejsce(pdf, 20);

    cairo_t* cr = pdf->cr;

    cairo_select_font_face(
        cr,
        "Sans",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_NORMAL
    );

    cairo_set_font_size(cr, 11);
    ustaw_kolor_tekstu(cr);

    cairo_move_to(cr, MARGIN_LEFT, pdf->y);
    cairo_show_text(cr, tekst);

    pdf->y += 19;
}


/* Rysowanie tekstu z automatycznym zawijaniem */
/* Rysowanie tekstu z automatycznym zawijaniem */
static void rysuj_tekst(RaportPDF* pdf, const char* tekst)
{
    if (tekst == NULL || tekst[0] == '\0') {
        return;
    }

    char kopia[MAX_TEXT];

    snprintf(kopia, sizeof(kopia), "%s", tekst);

    char linia[MAX_TEXT] = "";

    /*
     * Maksymalny tekst:
     * 511 znakow + spacja + 511 znakow + znak konca.
     */
    char kandydat[MAX_TEXT * 2];

    cairo_t* cr = pdf->cr;

    cairo_select_font_face(
        cr,
        "Sans",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_NORMAL
    );

    cairo_set_font_size(cr, 11);

    char* slowo = strtok(kopia, " \t\r\n");

    while (slowo != NULL) {

        size_t dlugosc_linii = strlen(linia);
        size_t dlugosc_slowa = strlen(slowo);

        /*
         * Budowanie nowego tekstu bez ryzyka
         * przekroczenia rozmiaru bufora.
         */
        if (dlugosc_linii == 0) {

            snprintf(
                kandydat,
                sizeof(kandydat),
                "%s",
                slowo
            );
        }
        else {

            memcpy(kandydat, linia, dlugosc_linii);

            kandydat[dlugosc_linii] = ' ';

            memcpy(
                kandydat + dlugosc_linii + 1,
                slowo,
                dlugosc_slowa + 1
            );
        }

        cairo_text_extents_t extents;

        cairo_text_extents(
            cr,
            kandydat,
            &extents
        );

        /*
         * Sprawdzamy, czy tekst zmiesci sie
         * w szerokosci strony.
         */
        if (extents.x_advance <= CONTENT_WIDTH) {

            snprintf(
                linia,
                sizeof(linia),
                "%s",
                kandydat
            );
        }
        else {

            /*
             * Poprzednia linia jest juz pelna,
             * wiec rysujemy ja i przechodzimy nizej.
             */
            if (linia[0] != '\0') {
                rysuj_linie(pdf, linia);
            }

            /*
             * Rozpoczynamy nowa linie od aktualnego slowa.
             */
            snprintf(
                linia,
                sizeof(linia),
                "%s",
                slowo
            );
        }

        slowo = strtok(NULL, " \t\r\n");
    }

    /*
     * Rysujemy ostatnia linie.
     */
    if (linia[0] != '\0') {
        rysuj_linie(pdf, linia);
    }
}

/* Naglowek sekcji */
static void rysuj_sekcje(RaportPDF* pdf, const char* tytul)
{
    sprawdz_miejsce(pdf, 45);

    cairo_t* cr = pdf->cr;

    pdf->y += 12;

    /* Delikatne niebieskie tlo */
    cairo_set_source_rgb(
        cr,
        COLOR_LIGHT_R,
        COLOR_LIGHT_G,
        COLOR_LIGHT_B
    );

    cairo_rectangle(
        cr,
        MARGIN_LEFT - 8,
        pdf->y - 19,
        CONTENT_WIDTH + 16,
        29
    );

    cairo_fill(cr);

    /* Niebieski pasek z lewej */
    cairo_set_source_rgb(
        cr,
        COLOR_BLUE_R,
        COLOR_BLUE_G,
        COLOR_BLUE_B
    );

    cairo_rectangle(
        cr,
        MARGIN_LEFT - 8,
        pdf->y - 19,
        3,
        29
    );

    cairo_fill(cr);

    cairo_select_font_face(
        cr,
        "Sans",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_BOLD
    );

    cairo_set_font_size(cr, 12);

    ustaw_kolor_tekstu(cr);

    cairo_move_to(cr, MARGIN_LEFT + 5, pdf->y);
    cairo_show_text(cr, tytul);

    pdf->y += 24;
}

/* Wstawianie wykresu PNG do dokumentu PDF */
static int wstaw_wykres(
    RaportPDF* pdf,
    const char* nazwa_pliku,
    double x,
    double y,
    double szerokosc,
    double wysokosc
)
{
    cairo_surface_t* obraz =
        cairo_image_surface_create_from_png(nazwa_pliku);

    if (cairo_surface_status(obraz) != CAIRO_STATUS_SUCCESS) {
        fprintf(stderr, "Nie mozna otworzyc wykresu: %s\n", nazwa_pliku);
        cairo_surface_destroy(obraz);
        return -1;
    }

    double szer = cairo_image_surface_get_width(obraz);
    double wys = cairo_image_surface_get_height(obraz);

    double skala_x = szerokosc / szer;
    double skala_y = wysokosc / wys;

    double skala = skala_x < skala_y ? skala_x : skala_y;

    double nowa_szer = szer * skala;
    double nowa_wys = wys * skala;

    cairo_save(pdf->cr);

    cairo_translate(
        pdf->cr,
        x + (szerokosc - nowa_szer) / 2.0,
        y + (wysokosc - nowa_wys) / 2.0
    );

    cairo_scale(pdf->cr, skala, skala);
    cairo_set_source_surface(pdf->cr, obraz, 0, 0);
    cairo_paint(pdf->cr);

    cairo_restore(pdf->cr);

    cairo_surface_destroy(obraz);

    return 0;
}

/* Dodawanie dwóch wykresow na jednej stronie PDF */
static void dodaj_strone_wykresow(
    RaportPDF* pdf,
    const char* plik1,
    const char* tytul1,
    const char* plik2,
    const char* tytul2
)
{
    rozpocznij_strone(pdf);

    rysuj_sekcje(pdf, tytul1);

    wstaw_wykres(
        pdf,
        plik1,
        MARGIN_LEFT,
        pdf->y,
        CONTENT_WIDTH,
        275
    );

    pdf->y += 300;

    rysuj_sekcje(pdf, tytul2);

    wstaw_wykres(
        pdf,
        plik2,
        MARGIN_LEFT,
        pdf->y,
        CONTENT_WIDTH,
        275
    );

    zakoncz_strone(pdf);
}


/* Dodawanie wszystkich czterech wykresow */
static void dodaj_wykresy_do_pdf(RaportPDF* pdf)
{
    dodaj_strone_wykresow(
        pdf,
        "protokoly.png",
        "Rozklad protokolow",
        "flagi_tcp.png",
        "Charakterystyka flag TCP"
    );

    dodaj_strone_wykresow(
        pdf,
        "top_porty.png",
        "Top 10 najaktywniejszych portow",
        "ruch_w_czasie.png",
        "Intensywnosc ruchu w czasie"
    );
}


/* Generowanie raportu PDF */
int zrob_pdf(const Statystyki* dane, const char* plik_wyjsciowy)
{
    if (dane == NULL || plik_wyjsciowy == NULL) {
        fprintf(stderr, "Nieprawidlowe dane raportu.\n");
        return;
    }

    cairo_surface_t* surface =
        cairo_pdf_surface_create(
            plik_wyjsciowy,
            PAGE_WIDTH,
            PAGE_HEIGHT
        );

    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        fprintf(stderr, "Nie mozna utworzyc pliku PDF.\n");
        cairo_surface_destroy(surface);
        return;
    }

    cairo_t* cr = cairo_create(surface);

    RaportPDF pdf;

    pdf.cr = cr;
    pdf.y = 0;
    pdf.page_number = 0;

    rozpocznij_strone(&pdf);

    char linia[MAX_TEXT];

    /* =====================================
       INFORMACJE O PLIKU
       ===================================== */

    rysuj_sekcje(&pdf, "Informacje o pliku");

    snprintf(
        linia,
        sizeof(linia),
        "Plik: %s",
        dane->nazwa_pliku
    );

    rysuj_tekst(&pdf, linia);

    snprintf(
        linia,
        sizeof(linia),
        "Typ lacza danych: %s",
        linktype_nazwa(dane->linktype)
    );

    rysuj_tekst(&pdf, linia);

    snprintf(
        linia,
        sizeof(linia),
        "Czas przechwytywania: %.2f s",
        dane->czas_trwania
    );

    rysuj_tekst(&pdf, linia);


    /* =====================================
       PODSUMOWANIE RUCHU
       ===================================== */

    rysuj_sekcje(&pdf, "Podsumowanie ruchu");

    snprintf(
        linia,
        sizeof(linia),
        "Liczba pakietow: %lld",
        dane->wszystkie_pakiety
    );

    rysuj_tekst(&pdf, linia);

    snprintf(
        linia,
        sizeof(linia),
        "Laczny transfer: %lld B",
        dane->wszystkie_bajty
    );

    rysuj_tekst(&pdf, linia);


    /* =====================================
       STATYSTYKI PROTOKOLOW
       ===================================== */

    double tcp_proc = 0;
    double udp_proc = 0;
    double inne_proc = 0;

    if (dane->wszystkie_pakiety > 0) {

        tcp_proc =
            100.0 * dane->tcp_pakiety /
            dane->wszystkie_pakiety;

        udp_proc =
            100.0 * dane->udp_pakiety /
            dane->wszystkie_pakiety;

        inne_proc =
            100.0 * dane->inne_pakiety /
            dane->wszystkie_pakiety;
    }

    rysuj_sekcje(&pdf, "Statystyki protokolow");

    snprintf(
        linia,
        sizeof(linia),
        "TCP: %lld pakietow, %lld B (%.1f%%)",
        dane->tcp_pakiety,
        dane->tcp_bajty,
        tcp_proc
    );

    rysuj_tekst(&pdf, linia);

    snprintf(
        linia,
        sizeof(linia),
        "UDP: %lld pakietow, %lld B (%.1f%%)",
        dane->udp_pakiety,
        dane->udp_bajty,
        udp_proc
    );

    rysuj_tekst(&pdf, linia);

    snprintf(
        linia,
        sizeof(linia),
        "Inne: %lld pakietow, %lld B (%.1f%%)",
        dane->inne_pakiety,
        dane->inne_bajty,
        inne_proc
    );

    rysuj_tekst(&pdf, linia);


    /* =====================================
       FLAGI TCP
       ===================================== */

    rysuj_sekcje(&pdf, "Flagi TCP");

    snprintf(linia, sizeof(linia), "SYN: %d", dane->syn);
    rysuj_tekst(&pdf, linia);

    snprintf(linia, sizeof(linia), "ACK: %d", dane->ack);
    rysuj_tekst(&pdf, linia);

    snprintf(linia, sizeof(linia), "FIN: %d", dane->fin);
    rysuj_tekst(&pdf, linia);

    snprintf(linia, sizeof(linia), "RST: %d", dane->rst);
    rysuj_tekst(&pdf, linia);

    pdf.y += 5;

    rysuj_tekst(
        &pdf,
        "SYN - rozpoczynanie nowych polaczen TCP."
    );

    rysuj_tekst(
        &pdf,
        "ACK - potwierdzenie odebrania danych."
    );

    rysuj_tekst(
        &pdf,
        "FIN - poprawne zakonczenie polaczenia."
    );

    rysuj_tekst(
        &pdf,
        "RST - natychmiastowe zerwanie polaczenia."
    );


    /* =====================================
       CHARAKTERYSTYKA KOMUNIKACJI
       ===================================== */

    rysuj_sekcje(&pdf, "Charakterystyka komunikacji");

    if (dane->tcp_pakiety > dane->udp_pakiety &&
        dane->tcp_pakiety > dane->inne_pakiety) {

        rysuj_tekst(
            &pdf,
            "Dominujacym protokolem byl TCP."
        );
    }
    else if (dane->udp_pakiety > dane->tcp_pakiety &&
        dane->udp_pakiety > dane->inne_pakiety) {

        rysuj_tekst(
            &pdf,
            "Dominujacym protokolem byl UDP."
        );
    }
    else if (dane->inne_pakiety > dane->tcp_pakiety &&
        dane->inne_pakiety > dane->udp_pakiety) {

        rysuj_tekst(
            &pdf,
            "Najwiekszy udzial mialy pozostale protokoly."
        );
    }
    else {
        rysuj_tekst(
            &pdf,
            "Nie stwierdzono jednego dominujacego protokolu."
        );
    }


    /* Przepustowosc */

    double przepustowosc = 0;

    if (dane->czas_trwania > 0) {
        przepustowosc =
            (dane->wszystkie_bajty * 8.0) /
            dane->czas_trwania;
    }

    if (przepustowosc >= 1000000.0) {

        snprintf(
            linia,
            sizeof(linia),
            "Srednia przepustowosc: %.2f Mbps",
            przepustowosc / 1000000.0
        );
    }
    else if (przepustowosc >= 1000.0) {

        snprintf(
            linia,
            sizeof(linia),
            "Srednia przepustowosc: %.2f Kbps",
            przepustowosc / 1000.0
        );
    }
    else {

        snprintf(
            linia,
            sizeof(linia),
            "Srednia przepustowosc: %.2f bps",
            przepustowosc
        );
    }

    rysuj_tekst(&pdf, linia);


    /* =====================================
       NAJAKTYWNIEJSZE PORTY
       ===================================== */
    //opis najpopularniejszych portów wraz z opisem
    const char* opis_portu(int port)
    {
        switch (port)
        {
        case 80:
            return "HTTP - ruch stron WWW";

        case 443:
            return "HTTPS - szyfrowany ruch WWW";

        case 53:
            return "DNS - tlumaczenie nazw domen";

        case 22:
            return "SSH - zdalny dostep";

        case 25:
            return "SMTP - wysylanie poczty";

        case 110:
            return "POP3 - odbior poczty";

        case 143:
            return "IMAP - synchronizacja poczty";

        case 123:
            return "NTP - synchronizacja czasu";

        default:
            return NULL;
        }
    }

    rysuj_sekcje(&pdf, "Najaktywniejsze porty");

    int porty_kopia[65536];

    for (int i = 0; i < 65536; i++) {
        porty_kopia[i] = dane->porty[i];
    }

    int znalezione_porty = 0;

    for (int i = 0; i < 10; i++) {

        int max_port = -1;
        int max_wartosc = 0;

        for (int port = 0; port < 65536; port++) {

            if (porty_kopia[port] > max_wartosc) {
                max_wartosc = porty_kopia[port];
                max_port = port;
            }
        }

        if (max_port == -1 || max_wartosc <= 0) {
            break;
        }

        const char* opis = opis_portu(max_port);

        if (opis != NULL) {

            snprintf(
                linia,
                sizeof(linia),
                "Port %d (%s) - %d wystapien",
                max_port,
                opis,
                max_wartosc
            );
        }
        else {

            snprintf(
                linia,
                sizeof(linia),
                "Port %d - %d wystapien",
                max_port,
                max_wartosc
            );
        }

        rysuj_tekst(&pdf, linia);

        porty_kopia[max_port] = -1;
        znalezione_porty++;
    }

    if (znalezione_porty == 0) {
        rysuj_tekst(
            &pdf,
            "Brak danych o portach."
        );
    }


    /* =====================================
       NAJAKTYWNIEJSZE PARY IP
       ===================================== */

    rysuj_sekcje(&pdf, "Najaktywniejsze pary IP");

    int liczba_par = dane->liczba_par_ip;

    if (liczba_par < 0) {
        liczba_par = 0;
    }

    if (liczba_par > MAX_PAR_IP) {
        liczba_par = MAX_PAR_IP;
    }

    if (liczba_par == 0) {
        rysuj_tekst(
            &pdf,
            "Brak danych o parach adresow IP."
        );
    }

    for (int i = 0; i < liczba_par; i++) {

        struct in_addr src;
        struct in_addr dst;

        src.s_addr = dane->pary_ip[i].src_ip;
        dst.s_addr = dane->pary_ip[i].dst_ip;

        char src_ip[32];
        char dst_ip[32];

        strcpy(src_ip, inet_ntoa(src));
        strcpy(dst_ip, inet_ntoa(dst));

        sprintf(
            linia,
            "%d. %s -> %s (%d pakietow, %lld B)",
            i + 1,
            src_ip,
            dst_ip,
            dane->pary_ip[i].pakiety,
            dane->pary_ip[i].bajty);

        rysuj_tekst(&pdf, linia);
    }

    /* =====================================
       WYKRYTE ANOMALIE
       ===================================== */

    rysuj_sekcje(&pdf, "Wykryte anomalie");

    int liczba_alertow = dane->ile_alertow;

    if (liczba_alertow < 0) {
        liczba_alertow = 0;
    }

    if (liczba_alertow > MAX_ALERTOW) {
        liczba_alertow = MAX_ALERTOW;
    }

    if (liczba_alertow == 0) {
        rysuj_tekst(
            &pdf,
            "Nie wykryto anomalii na podstawie zastosowanych regul."
        );
    }

    for (int i = 0; i < liczba_alertow; i++) {

        snprintf(
            linia,
            sizeof(linia),
            "- %s",
            dane->alerty[i]
        );

        rysuj_tekst(&pdf, linia);
    }


    /* =====================================
       ZAKONCZENIE RAPORTU
       ===================================== */

    zakoncz_strone(&pdf);

    /* Dodanie stron zawierajacych wykresy */
    dodaj_wykresy_do_pdf(&pdf);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    printf("Wygenerowano raport PDF. \n");

    return 0;
}


