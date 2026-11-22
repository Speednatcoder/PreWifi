# PreWifi — Analiza ruchu TCP/UDP z plików PCAP

Program generuje plik PDF z kompleksową analizą ruchu TCP i UDP wraz z wykresami na podstawie plików PCAP.



## 📚 Spis treści
- [Opis projektu](#opis-projektu)
- [Użyte technologie](#użyte-technologie)
- [Instalacja i uruchomienie](#instalacja-i-uruchomienie)
- [Rozwiązania zastosowane w projekcie](#rozwiązania-zastosowane-w-projekcie)

## 📝 Opis projektu
Projekt powstał w ramach testu eliminacyjnego na hackaton organizowany przez grupę BEST. 
Program napisany w języku C analizuje komunikację TCP i UDP na podstawie dostarczonych plików pcap, generuje raport PDF oraz zestaw wykresów PNG, przedstawiających: 
- statystyki ruchu,
- charakterystykę komunikacji,
- potencjalne nieprawidłowości. 


## 🧩 Użyte technologie
- **C** - główny język
- **libpcap** - biblioteka do analizy plików binarnych pcap
- **Cairo PDF i PNG** - biblioteka graficzna pozwalająca tworzyć wykresy w plikach png i generować pdf
  standardowe biblioteki systemu Linux przeznaczone do programowania sieciowego,


## ⚙️ Instalacja i uruchomienie
Projekt został przygotowany dla systemu Linux (testowano na Ubuntu).
  Wymagane pakiety:
  - GCC
  - CMake
  - libpcap
  - Cairo
<details>
<summary><strong>🔧 Instalacja krok po kroku</strong></summary>

  
```bash
# Klonowanie repozytorium
git clone https://github.com/Speednatcoder/PreWifi.git
cd prewifi

# Instalacja zależności:
sudo apt update
sudo apt install build-essential cmake libpcap-dev libcairo2-dev

# Kompilacja
## Przejście do katalogu projektu:
cd analizator_pcap

## Utworzenie katalogu build:
mkdir build
cd build

## Generowanie plików projektu:
cmake ..

## Kompilacja:
cmake --build .

# Uruchomienie
./analizator_pcap test.pcap

gdzie:
- analizator_pcap – skompilowany program,
- test.pcap – plik przeznaczony do analizy.

```
</details>


## 🧠 Rozwiązania zastosowane w projekcie

Podczas analizy ruchu sieciowego program koncentruje się głównie na pakietach zapisanych w formacie Ethernet (LinkType = Ethernet), ponieważ jest to najczęściej spotykany typ ramek w plikach PCAP wykorzystywanych do analizy ruchu TCP/IP. Program umożliwia również analizę ruchu pochodzącego z sieci bezprzewodowych WiFi, o ile pakiety zostały zapisane w obsługiwanym formacie.

W projekcie wykorzystano bibliotekę **libpcap**, która umożliwia odczyt oraz analizę pakietów zapisanych w plikach PCAP. Na jej podstawie realizowane jest pobieranie informacji o protokołach sieciowych, portach, adresach IP oraz czasie przechwycenia ruchu.

Do generowania raportów PDF oraz wykresów PNG wykorzystano bibliotekę **Cairo**, odpowiadającą za rysowanie tekstu, wykresów oraz tworzenie końcowego raportu w formacie PDF.

W projekcie wykorzystano także standardowe biblioteki systemu Linux przeznaczone do programowania sieciowego, między innymi **arpa/inet.h**, która umożliwia konwersję adresów IPv4 z postaci binarnej do czytelnego formatu kropkowanego, np.124.158.124.42. Dzięki temu adresy prezentowane w generowanym raporcie są bardziej czytelne dla użytkownika.

Dodatkowo wykorzystano standardowe nagłówki sieciowe systemu Linux, takie jak :
- arpa/inet.h,
- netinet/ip.h,
- netinet/tcp.h,
- netinet/udp.h.

Dostarczają one gotowe struktury danych opisujące nagłówki protokołów IPv4, TCP i UDP, dzięki czemu możliwa była bezpośrednia analiza pakietów bez konieczności samodzielnego implementowania struktur tych protokołów. Biblioteka arpa/inet.h została dodatkowo wykorzystana do prezentowania adresów IPv4 w czytelnej postaci kropkowanej.


Zastosowany zestaw bibliotek pozwolił na stworzenie lekkiego analizatora ruchu sieciowego generującego zarówno raport PDF, jak i graficzne podsumowanie analizowanych danych w postaci wykresów PNG.

