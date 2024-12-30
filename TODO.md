## TODO:

### Obsługa komend (strona serwera):
#### Klient -> Serwer:

- [x] 01\\[nick] - przypisanie nicku  
- [x] 02\\[pokój] - utworzenie nowego pokoju o podanej nazwie 
- [x] 03\\[pokój] - dołączenie do pokoju o podanej nazwie 
- [ ] 04\\[tutaj,ustawienia] - ustawienia pokoju
  - [x] 04.1\\ wpisanie ustawien
  - [ ] 04.2\\ obsluga zlych danych wejsciowych 
- [ ] 05\\[] - start gry
- [ ] 06\\[litera] - próba zgadnięcia litery
- [ ] 07\\[pokój] - restart gry
- [ ] 08\\[pokój] - podanie aktualnego stanu gry
- [ ] 09\\[pokój] - opuszczenie pokoju przez gracza


#### Serwer -> Klient:
- [x] 01\\[bool] - potwierdzenie ustawienia nicku (pętla while z podawaniem nicku do skutku powinna być po stronie klienta, więc potrzebna jest flaga), bool zapisany jako 0 lub 1
- [ ] 02\\[stan] - podanie stanu gry
- [ ] 03\\[bool] - wynik próby odgadnięcia litery
- [ ] 04\\[nazwa_pokoju] - (aktualizacja listy pokoi w gui) pokój został stworzony
- [ ] 05\\[nazwa_pokoju] - (aktualizacja listy pokoi w gui) pokój został usunięty
