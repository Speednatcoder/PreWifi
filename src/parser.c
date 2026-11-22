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

static void sprawdz_anomalie(Statystyki* dane) {
    if (dane->syn > 50 && dane->syn > dane->ack && dane->ile_alertow < MAX_ALERTOW)
    {
        strcpy(dane->alerty[dane->ile_alertow],"Duza liczba pakietow SYN bez odpowiedzi ACK" );
        dane->ile_alertow++;
    }

    if (dane->rst > 10 && dane->ile_alertow < MAX_ALERTOW)
    {
        strcpy(dane->alerty[dane->ile_alertow],"Duza liczba pakietow RST");

        dane->ile_alertow++;
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
