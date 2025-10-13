void eliminarProceso() {
    int opc;
    do {
        cleanSc();
        cout << "----Eliminar Proceso----\n\n";
        cout << "1. Eliminar por Cola (FIFO)\n";
        cout << "2. Eliminar por Pila (LIFO)\n";
        cout << "3. Regresar\n";
        cout << "Opcion: ";
        cin >> opc;

        switch (opc) {
            case 1: // FIFO
                if (!RAM.empty()) {
                    cout << "Proceso con PID " << RAM.front().pid << " eliminado de RAM.\n";
                    RAMUsada -= RAM.front().tam;
                    RAM.erase(RAM.begin());
                    moverVRAMaRAM();
                } else if (!VRAM.empty()) {
                    cout << "Proceso con PID " << VRAM.front().pid << " eliminado de VRAM.\n";
                    VRAMUsada -= VRAM.front().tam;
                    VRAM.erase(VRAM.begin());
                } else {
                    cout << "No hay procesos para eliminar.\n";
                }
                presionarEnter();
                break;
            case 2: // LIFO
                if (!RAM.empty()) {
                    cout << "Proceso con PID " << RAM.back().pid << " eliminado de RAM.\n";
                    RAMUsada -= RAM.back().tam;
                    RAM.pop_back();
                    moverVRAMaRAM();
                } else if (!VRAM.empty()) {
                    cout << "Proceso con PID " << VRAM.back().pid << " eliminado de VRAM.\n";
                    VRAMUsada -= VRAM.back().tam;
                    VRAM.pop_back();
                } else {
                    cout << "No hay procesos para eliminar.\n";
                }
                presionarEnter();
                break;
        }
    } while(opc != 3);
}