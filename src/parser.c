#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <pcap.h> //bilbioteka do czytania plików pcap
//gotowe struktury ip, tcp i udp 
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
// funkcje ntohs(), ntohl() do konwersji danych sieciowych
#include <arpa/inet.h>

//anomalie


static void dodaj_alert(Statystyki* dane, const char* komunikat)
{
    if (dane->ile_alertow >= MAX_ALERTOW)
        return;

    strcpy(dane->alerty[dane->ile_alertow], komunikat);
    dane->ile_alertow++;
}

static void sprawdz_anomalie(Statystyki* dane)
{

    /* =====================================
       SYN bez ACK
       ===================================== */

    if (dane->syn > 50 &&
        dane->syn > dane->ack)
    {
        dodaj_alert(
            dane,
            "Duza liczba pakietow SYN bez odpowiedzi ACK (mozliwe nieudane polaczenia lub SYN Flood).");
    }

    /* =====================================
       Duzo RST
       ===================================== */

    if (dane->rst > 10)
    {
        dodaj_alert(
            dane,
            "Duza liczba pakietow RST (mozliwe nagle zrywanie polaczen lub bledy komunikacji).");
    }

    /* =====================================
       Nietypowo duzo FIN
       ===================================== */

    if (dane->tcp_pakiety > 0 &&
        dane->fin > 20 &&
        dane->fin > dane->tcp_pakiety * 0.7)
    {
        dodaj_alert(
            dane,
            "Nietypowo duzy udzial flag FIN (duza liczba zamykanych polaczen TCP).");
    }
    /* =====================================
   ACK < SYN flagi
   ===================================== */

    if (dane->syn > 0 &&
        dane->ack < dane->syn * 0.3)
    {
        dodaj_alert(
            dane,
            "Liczba ACK jest znacznie mniejsza od SYN (czesc polaczen moze nie byc ustanawiana poprawnie).");
    }

    /* =====================================
       Ponad 95% ruchu to TCP
       ===================================== */

    if (dane->wszystkie_pakiety > 0)
    {
        double tcp_proc =
            100.0 *
            dane->tcp_pakiety /
            dane->wszystkie_pakiety;

        double udp_proc =
            100.0 *
            dane->udp_pakiety /
            dane->wszystkie_pakiety;

        if (tcp_proc > 95.0)
        {
            dodaj_alert(
                dane,
                "Ponad 95% ruchu stanowil protokol TCP");
        }
        if (udp_proc > 95.0) {
            dodaj_alert(dane,
                "Ponad 95% ruchu stanowil protokol UDP");
        }

        /* =====================================
           Dominacja jednego portu
           ===================================== */

        long long suma_portow = 0;
        int max_port_ruch = 0;

        for (int i = 0; i < 65536; i++)
        {
            suma_portow += dane->porty[i];

            if (dane->porty[i] > max_port_ruch)
            {
                max_port_ruch = dane->porty[i];
            }
        }

        if (suma_portow > 0 &&
            max_port_ruch >= 20 &&
            max_port_ruch >= suma_portow * 0.6)
        {
            dodaj_alert(
                dane,
                "Jeden port dominuje w analizowanym ruchu sieciowym (mozliwa koncentracja komunikacji na pojedynczej usludze)");
        }

        /* =====================================
           Dominacja jednego adresu IP
           ===================================== */

        struct
        {
            int ip;
            long long pakiety;
        }
        stat_ip[MAX_PAR_IP * 2];

        int liczba_ip = 0;

        long long suma_ip = 0;
        long long max_ip = 0;

        for (int i = 0; i < dane->liczba_par_ip; i++)
        {
            ParaIP para = dane->pary_ip[i];

            suma_ip += 2LL * para.pakiety;

            int znaleziono = 0;

            for (int j = 0; j < liczba_ip; j++)
            {
                if (stat_ip[j].ip == para.src_ip)
                {
                    stat_ip[j].pakiety += para.pakiety;
                    znaleziono = 1;
                    break;
                }
            }

            if (!znaleziono &&
                liczba_ip < MAX_PAR_IP * 2)
            {
                stat_ip[liczba_ip].ip = para.src_ip;
                stat_ip[liczba_ip].pakiety = para.pakiety;
                liczba_ip++;
            }

            znaleziono = 0;

            for (int j = 0; j < liczba_ip; j++)
            {
                if (stat_ip[j].ip == para.dst_ip)
                {
                    stat_ip[j].pakiety += para.pakiety;
                    znaleziono = 1;
                    break;
                }
            }

            if (!znaleziono &&
                liczba_ip < MAX_PAR_IP * 2)
            {
                stat_ip[liczba_ip].ip = para.dst_ip;
                stat_ip[liczba_ip].pakiety = para.pakiety;
                liczba_ip++;
            }
        }

        for (int i = 0; i < liczba_ip; i++)
        {
            if (stat_ip[i].pakiety > max_ip)
            {
                max_ip = stat_ip[i].pakiety;
            }
        }

        if (suma_ip > 0 &&
            max_ip >= 20 &&
            max_ip >= suma_ip * 0.6)
        {
            dodaj_alert(
                dane,
                "Jeden adres IP dominuje w analizowanym ruchu.");
        }

        /* =====================================
           Nagly skok ruchu
           ===================================== */

        int liczba_sekund =
            (int)dane->czas_trwania;

        if (liczba_sekund < 1)
            liczba_sekund = 1;

        if (liczba_sekund > MAX_SEKUND)
            liczba_sekund = MAX_SEKUND;

        long long suma_pakietow = 0;
        long long max_sekundowy = 0;

        for (int i = 0; i < liczba_sekund; i++)
        {
            suma_pakietow +=
                dane->pakiety_na_sekunde[i];

            if (dane->pakiety_na_sekunde[i] >
                max_sekundowy)
            {
                max_sekundowy =
                    dane->pakiety_na_sekunde[i];
            }
        }

        double srednia =
            (double)suma_pakietow /
            liczba_sekund;

        if (srednia > 0 &&
            max_sekundowy >= 20 &&
            max_sekundowy > srednia * 5)
        {
            dodaj_alert(
                dane,
                "Wykryto nagly skok liczby pakietow w czasie (mozliwa intensywna transmisja lub nietypowe zdarzenie sieciowe).");
        }
    }


    

        /* =====================================
           Brak anomalii
           ===================================== */

        if (dane->ile_alertow == 0)
        {
            dodaj_alert(
                dane,
                "Nie wykryto istotnych anomalii");
        }
}


//Parsoawnie pliku
static void analizuj_pcap(pcap_t* pcap, Statystyki* dane)
{
    //sprawdzenie typu ³acza danych
    dane->linktype = pcap_datalink(pcap);
    int offset_ip = -1;
    if (dane->linktype == 1)      // Ethernet
    {
        offset_ip = 14;
    }
    else if (dane->linktype == 105) // WiFi (IEEE 802.11)
    {
        offset_ip = 24; //upraszamy - dziala tylko dla WiFi, gdzie ramka ma 24 bajty
    }
    else
    {
        printf("Nieobslugiwany typ lacza danych\n");
        return;
    }

    struct pcap_pkthdr* header;
    const u_char* packet;

    long start_sec = 0;
    long end_sec = 0;

    int pierwszy_pakiet = 1;

    // liczenie pakietów i czasu trwania przechwytwania
    while (pcap_next_ex(pcap, &header, &packet) >= 0)
    {
        dane->wszystkie_pakiety++;
        dane->wszystkie_bajty += header->len;

        if (pierwszy_pakiet)
        {
            start_sec = header->ts.tv_sec;
            pierwszy_pakiet = 0;
        }

        end_sec = header->ts.tv_sec;

        //Ruch w czasie
        int sekunda = (int)(header->ts.tv_sec - start_sec);

        if (sekunda >= 0 && sekunda < MAX_SEKUND)
        {
            dane->pakiety_na_sekunde[sekunda]++;
            dane->bajty_na_sekunde[sekunda] += header->len;
        }

        //nthos() zamienia z formatu sieciowego na taki ok dla komputera

       // pakiet za krótki ¿eby zawiera³ IPv4
        if (header->len <= offset_ip)
        {
            continue;
        }

        // struktura IP, bo TCP i UDP zakopane pod IP
        struct ip* ip_hdr = (struct ip*)(packet + offset_ip);

        //IPv4 musi miec min 20 bajtów
        if (ip_hdr->ip_hl < 5)
        {
            continue;
        }
        //Protokoly
        // TCP
        if (ip_hdr->ip_p == IPPROTO_TCP)
        {
            dane->tcp_pakiety++;
            dane->tcp_bajty += header->len;

            struct tcphdr* tcp_hdr = (struct tcphdr*)((unsigned char*)ip_hdr + ip_hdr->ip_hl * 4);
            // ip moze miec opcje dodatkowe, dlatego korzystamy z rzeczywistej dlugosci naglowka ip_hl

               //flagi TCP

            if (tcp_hdr->th_flags & TH_SYN)
                dane->syn++;

            if (tcp_hdr->th_flags & TH_ACK)
                dane->ack++;

            if (tcp_hdr->th_flags & TH_FIN)
                dane->fin++;

            if (tcp_hdr->th_flags & TH_RST)
                dane->rst++;

            // porty TCP

            int src_port = ntohs(tcp_hdr->th_sport);

            int dst_port = ntohs(tcp_hdr->th_dport);

            dane->porty[src_port]++;
            dane->porty[dst_port]++;
        }

        // UDP
        else if (ip_hdr->ip_p == IPPROTO_UDP)
        {
            dane->udp_pakiety++;
            dane->udp_bajty += header->len;

            struct udphdr* udp_hdr = (struct udphdr*)((unsigned char*)ip_hdr + ip_hdr->ip_hl * 4);

            int src_port = ntohs(udp_hdr->uh_sport);

            int dst_port = ntohs(udp_hdr->uh_dport);

            dane->porty[src_port]++;
            dane->porty[dst_port]++;
        }

        // inne protokoly
        else
        {
            dane->inne_pakiety++;
            dane->inne_bajty += header->len;
        }

        //Pary IP
        int src_ip = ip_hdr->ip_src.s_addr;
        int dst_ip = ip_hdr->ip_dst.s_addr;
        int znaleziono = 0;

        for (int i = 0; i < dane->liczba_par_ip; i++)
        {
            if (dane->pary_ip[i].src_ip == src_ip && dane->pary_ip[i].dst_ip == dst_ip)
            {
                dane->pary_ip[i].pakiety++;
                dane->pary_ip[i].bajty += header->len;

                znaleziono = 1;
                break;
            }
        }
        if (!znaleziono && dane->liczba_par_ip < MAX_PAR_IP)
        {
            int idx = dane->liczba_par_ip;
            dane->pary_ip[idx].src_ip = src_ip;
            dane->pary_ip[idx].dst_ip = dst_ip;

            dane->pary_ip[idx].pakiety = 1;
            dane->pary_ip[idx].bajty = header->len;
            dane->liczba_par_ip++;
        }
    }
    if (!pierwszy_pakiet) {
        dane->czas_trwania = (double)(end_sec - start_sec);
    }
    sprawdz_anomalie(dane);
}

//Wczytanie pliku
int wczytaj_pcap(const char* sciezka, Statystyki* dane)
{
    memset(dane, 0, sizeof(Statystyki));

    strncpy(dane->nazwa_pliku, sciezka, sizeof(dane->nazwa_pliku) - 1);

    char errbuf[PCAP_ERRBUF_SIZE];

    pcap_t* pcap = pcap_open_offline(sciezka, errbuf);

    if (pcap == NULL)
    {
        printf("Blad otwarcia PCAP: %s\n", errbuf);
        return -1;
    }
    analizuj_pcap(pcap, dane);
    pcap_close(pcap);
    return 0;
}
