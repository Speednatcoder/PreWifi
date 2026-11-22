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
- **Cairo 2D Graphics Library** - biblioteka do tworzenia wykresów w plikach png i generowania pdf: https://www.cairographics.org/

## ⚙️ Instalacja i uruchomienie

<details>
<summary><strong>🔧 Instalacja krok po kroku</strong></summary>

```bash
# Klonowanie repozytorium
git clone https://github.com/Speednatcoder/PreWifi.git
cd prewifi

# Kompilacja
make

# Uruchomienie
./prefiwi input.pcap
```
</details>


## 🧠 Rozwiązania zastosowane w projekcie

