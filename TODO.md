## TODO:

### Obsługa komend (strona serwera):
#### Klient -> Serwer:

- [x] 01\[nick] - przypisanie nicku  
- [x] 02\[pokój] - utworzenie nowego pokoju o podanej nazwie 
- [x] 03\[pokój] - dołączenie do pokoju o podanej nazwie
  - [x] 03.1 zabezpieczenie przed ponownym polaczeniem do tego samego pokoju
- [x] 04\[tutaj,ustawienia] - ustawienia pokoju
  - [x] 04.1 wpisanie ustawien
  - [x] 04.2 obsluga zlych danych wejsciowych 
- [ ] 05\[] - start gry
  - [ ] 05.1 inicjalizowanie listy slow
    - [x] 05.1.1 wstepna prosta lista ustawiona na sztywno
    - [ ] 05.1.2 lista wybierana z api przez ustawienia trudnosci
- [ ] 06\[litera] - próba zgadnięcia litery
  - [ ] 06.1 obsluga litery
    - [x] 06.1.1 wpisanie litery
    - [ ] 06.1.2 sprawdzanie litery z haslem
    - [ ] 06.1.3 walidacja litery i aktualizowanie stanu wisielca
- [ ] 07\[pokój] - restart gry
- [ ] 08\[pokój] - podanie aktualnego stanu gry
- [ ] 09\[pokój] - opuszczenie pokoju przez gracza
- [x] 10\[] - wypisanie stanu pokoju (pomocnicze na czas testowania serwera)


#### Serwer -> Klient:
- [x] 01\[bool] - potwierdzenie ustawienia nicku (pętla while z podawaniem nicku do skutku powinna być po stronie klienta, więc potrzebna jest flaga), bool zapisany jako 0 lub 1
- [ ] 02\[] - wyswietlanie listy serwerow
- [ ] 03\[stan] - podanie stanu gry
- [ ] 04\[..] - wynik próby odgadnięcia litery 

