# QViaggiaTreno

QViaggiaTreno è un front-end grafico a ViaggiaTreno, il servizio web di Trenitalia con informazioni in tempo reale sulla circolazione dei treni passeggeri.

È scritto utilizzando le librerie Qt 6 (>= 6.5)  e può essere eseguito sotto Linux, Windows e Mac

## Compilazione

Dipendenze: CMake, un compilatore C++17 e Qt 6 (>= 6.5) con i moduli Core, Gui, Widgets, Network, Xml e Test.

* Debian/Ubuntu: `sudo apt install cmake g++ qt6-base-dev qt6-base-dev-tools`
* Arch: `sudo pacman -S cmake gcc qt6-base`
* Fedora: `sudo dnf install cmake gcc-c++ qt6-qtbase-devel`

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
sudo cmake --install build
```

I test usano QtTest; in assenza di display impostare `QT_QPA_PLATFORM=offscreen`.

## Ringraziamenti

Voglio ringraziare tutte le persone che hanno provato le varie versioni beta che ho messo a disposizione, segnalandomi bug o 
facendo suggerimenti. In particolare, senza togliere niente a nessuno, ringrazio:

* Eurocity, per aver verificato che qviaggiatreno si compila perfettamente anche sotto Mac OS X e per la compilazione di pacchetti per Mac
* Jack_A, per aver verificato che qviaggiatreno si compila sotto debian lenny
* msr.cooper per aver suggerito di usare due tonalità di giallo nella tabella delle partenze
* danny1984, trambvs, xyz per aver segnalato bug o difetti
* Il Max per avermi segnalato immediatamente l'esistenza di treni soppressi o parzialmente soppressi, permettendomi di individuare alcuni bug insidiosi

## Original author
fra74 (https://github.com/fra74)
## Original repo
http://svn.xp-dev.com/svn/qviaggiatreno/trunk/
