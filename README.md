Nume: Hodoboc-Velescu Tudor
Grupă: 335CC

# Tema 3 Loader de Executabile
# Este recomandat să folosiți diacritice

Organizare
-
Am plecat de la implementarea handler-ului din laboratorul 6. Ideea de baza a
fost ca in so_init_loader setez handler-ul nou ce va fi implementat. Apoi, in
functia so_execute, deschid un fd pentru a citi de fiecare data cand trebuie sa
copiez datele din fisier. In handler(segv_handler), se verifica daca semnalul
este cel care trebuie(SIGSEGV), in caz contrar se foloseste handler-ul default.
Dupa, se cauta pagina de la care s-a primit semnalul de eroare. Daca nu se
gaseste, se apeleaza handler-ul default. Cand se gaseste, se verifica daca pagina
respectiva are rezervata memorie(valorile din segment->data sunt diferite de 0).
Daca pagina este deja mapata, se foloseste handler default. Altfel, rezerva
memorie, se zeroizeaza, si se copiaza continutul din fisier daca este cazul.
Apoi se seteaza permisiunile, se marcheaza segmentul ca fiind mapat si se iese
din functie.
Tema a fost destul de utila in intelegerea memoriei.
Implementarea este relativ naiva.

Implementare
-
Tema a fost implementata in totalitate.

Cum se compilează și cum se rulează?
-
Se apeleaza make/make build pentru crearea bibliotecii care apoi trebuie linkata.

Bibliografie
-
Laboratorul 6, exercitul 5, implementarea handler-ului.
