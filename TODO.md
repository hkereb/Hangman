## TODO:

### Obsługa komend (strona serwera):
#### Klient -> Serwer:

- [x] 01\\[nick] - przypisanie nicku  
- [x] 02\\[nazwa] - utworzenie nowego pokoju o podanej nazwie 
- [x] 03\\[nazwa,haslo] - dołączenie do pokoju o podanej nazwie 
- [ ] 04\\[tutaj,ustawienia] - ustawienia pokoju
  - [x] 04.1\\ wpisanie ustawien
  - [ ] 04.2\\ obsluga zlych danych wejsciowych 
- [x] 05\\[] - start gry
- [x] 06\\[litera] - próba zgadnięcia litery
- [x] 07\\[pokój] - restart gry
- [x] 08\\[pokój] - podanie aktualnego stanu gry
- [x] 09\\[pokój] - opuszczenie pokoju przez gracza
- [x] 10\\[] - prośba o przesłanie listy pokoi


#### Serwer -> Klient:
- [x] 01\\[bool] - potwierdzenie ustawienia nicku (pętla while z podawaniem nicku do skutku powinna być po stronie klienta, więc potrzebna jest flaga), bool zapisany jako 0 lub 1
- [ ] 02\\[stan] - podanie stanu gry
- [ ] 03\\[bool] - wynik próby odgadnięcia litery
- [x] 04\\[nazwa1,..,nazwa99] - przesłanie listy nazw pokoi




HANIA (DO POPRAWY, wyjdzie w trakcie fixów w kodzie):
#### Klient -> Serwer:

- [x] 01\\[nick] - przypisanie nicku
- [x] 02\\[nazwa] - utworzenie nowego pokoju o podanej nazwie
- [x] 03\\[nazwa,haslo] - dołączenie do pokoju o podanej nazwie
- [ ] 04\\[tutaj,ustawienia] - ustawienia pokoju
    - [x] 04.1\\ wpisanie ustawien
    - [ ] 04.2\\ obsluga zlych danych wejsciowych
- [x] 10\\[] - prośba o przesłanie listy pokoi
- [x] 11\\[] - prośba o przesłanie listy nicków graczy obecnych w danym pokoju
- [x] 12\\[] - prośba o rozpoczęcie gry w pokoju gracza
- [ ] 99\\[litera] - próba zgadnięcia litery
- [ ] 99\\[pokój] - restart gry
- [ ] 99\\[pokój] - podanie aktualnego stanu gry
- [ ] 99\\[pokój] - opuszczenie pokoju przez gracza


#### Serwer -> Klient:
- [x] 01\\[bool] - potwierdzenie ustawienia nicku (pętla while z podawaniem nicku do skutku powinna być po stronie klienta, więc potrzebna jest flaga), bool zapisany jako 0 lub 1
- [x] 02\\[bool] - potwierdzenie stworzenia pokoj
- [x] 03\\[bool] - potwierdzenie dołączenia do pokoju
- [x] 05\\[nazwa1,..,nazwa99] - przesłanie listy nazw pokoi
- [x] 06\\[bool,nazwa1,..,nazwa99] - przesłanie listy nazw graczy w danym pokoju
- [x] 07\\[bool] - pozwolenie na rozpoczęcie gry (0 jeżeli gracze opuszczą waitroom jeszcze zanim zacznie się gra i jest <2)
- [x] 08\\[zaszyfrowane_hasło] - sygnał do przejścia do ekranu głównego gry
- [ ] 99\\[bool] - wynik próby odgadnięcia litery
- [ ] 99\\[stan] - podanie stanu gry
