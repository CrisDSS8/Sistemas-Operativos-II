#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

int main() {
    pid_t h10, h5, subh2, subh4, subh9, subh1, subh3, subh6, subh7, subh8;

    cout << "\n" << setw(25) << "ARBOL DE PROCESOS PID\n" << endl;

    if ((h5 = fork()) == 0) {  //Creación de NODO 5
        if ((subh2 = fork()) == 0) { //Creación de NODO 2
            if ((subh1 = fork()) == 0) { //Creación de NODO 1
                cout << "[1] -> " << " PID: " << getpid() << " PPID: " << getppid() << "\n";
            } else { //Impresión del SUBPADRE 2
                waitpid(subh2, NULL, 0);
                waitpid(subh1, NULL, 0);
                cout << "[2] -> " << " PID: " << getpid() << " PPID: " << getppid() << "\n";
            }
        } else {
            waitpid(subh2, NULL, 0);
            if((subh4 = fork()) == 0) { //Creación de NODO 4
                if ((subh3 = fork()) == 0) { //Creación de NODO 3
                    cout << "[3] -> " << " PID: " << getpid() << " PPID: " << getppid() << "\n";
                } else { //Impresión del SUBPADRE 4
                    waitpid(subh3, NULL, 0);
                    cout << "[4] -> " << " PID: " << getpid() << " PPID: " << getppid() << "\n";
                }
            } else { //Impresión del SUBPADRE 5
                waitpid(subh4, NULL, 0);
                cout << "[5] -> " << " PID: " << getpid() << " PPID: " << getppid() << "\n";
            }
        }
    } else {
        waitpid(h5, NULL, 0);
        if((h10 = fork()) == 0) { //Creación de NODO 10
            if ((subh9 = fork()) == 0) { //Creación de NODO 9
                if ((subh6 = fork()) == 0) { //Creación de NODO 6
                   cout << "[6] -> " << " PID: " << getpid() << " PPID: " << getppid() << "\n"; 
                } else if ((subh7 = fork()) == 0) { //Creación de NODO 7
                    waitpid(subh6, NULL, 0);
                    cout << "[7] -> " << " PID: " << getpid() << " PPID: " << getppid() << "\n";
                } else {
                    if ((subh8 = fork()) == 0) { //Creación de NODO 8
                        waitpid(subh7, NULL, 0);
                        cout << "[8] -> " << " PID: " << getpid() << " PPID: " << getppid() << "\n";
                    } else { //Impresión de SUBPADRE 9
                        waitpid(subh8, NULL, 0);
                        cout << "[9] -> " << " PID: " << getpid() << " PPID: " << getppid() << "\n";
                    }
                }
            } else {  //Impresión de SUBPADRE 10
                waitpid(subh9, NULL, 0);
                cout << "[10] -> " << " PID: " << getpid() << " PPID: " << getppid() << "\n";
            }
        } else { //Impresión del PADRE GENERAL 11
            waitpid(h10, NULL, 0);
            cout << "[11] -> " << " PID: " << getpid() << " PPID: " << getppid() << "\n" << endl;
        }
    }
}