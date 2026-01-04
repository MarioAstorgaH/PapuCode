#include <iostream>
#include <fstream>
#include <vector>
#include "lexico.h"
#include "sintaxis.h"
#include "runtime.h"

using namespace std;

int main(int argc, char* argv[])
{
    vector<char>tCars;
    int error;
    string errToken;

    Lexico lex;
    char modoEjecucion = 'L';

    if (argc < 2) {
        cout << "Uso: " << argv[0] << " <archivo.txt> <parametro>\n";
        return 1;
    }

    ifstream archivo(argv[1], ios::in);
    if (!archivo) {
        cerr << "No se pudo abrir el archivo: " << argv[1] << "\n";
        return 1;
    }

    if (argc > 1) {
        string arg = argv[2];
        if (arg == "-L") modoEjecucion = 'L';
        else if (arg == "-S") modoEjecucion = 'S';
        else if (arg == "-M") modoEjecucion = 'M';
        else if (arg == "-B") modoEjecucion = 'B';
        else if (arg == "-R") modoEjecucion = 'R';
        else modoEjecucion = 'S';
    }

    char c;
    while (archivo.get(c)) tCars.push_back(c);
    tCars.push_back('\n');
    archivo.close();

    switch (modoEjecucion)
    {
        case 'L' :
            error = lex.generaLexico(tCars, errToken, false);
            if (error == ERR_NOERROR) lex.imprimir();
            else cout << "ERROR LEXICO: " << error << " :: " << errToken << "\n";
            break;

        case 'S' :
            error = lex.generaLexico(tCars, errToken, false);
            if (error == ERR_NOERROR) {
                Sintaxis sintax(lex, false, false);
                sintax.generaSintaxis();
                sintax.imprimirErrores();
            }
            break;

        case 'M' :
            error = lex.generaLexico(tCars, errToken, false);
            if (error == ERR_NOERROR) {
                Sintaxis sintax(lex, true, false);
                sintax.generaSintaxis();
                sintax.imprimirErrores();
            }
            break;

        case 'B' :
            error = lex.generaLexico(tCars, errToken, false);
            if (error == ERR_NOERROR) {
                Sintaxis sintax(lex, true, true);
                sintax.generaSintaxis();
                sintax.imprimirErrores();
                sintax.imprimirBytecode(); // Solo si quieres ver el bytecode incluso con errores
            }
            break;

        case 'R' : // RUNTIME (AQUÍ ESTABA EL FALLO)
            error = lex.generaLexico(tCars, errToken, false);
            if (error == ERR_NOERROR)
            {
                // 1. Compilar (Semantica + Bytecode)
                Sintaxis sintax(lex, true, true);

                // Guardamos el resultado del analisis
                int resultadoAnalisis = sintax.generaSintaxis();

                // 2. VERIFICACIÓN ESTRICTA
                // Si el resultado NO es ERR_NO_SINTAX_ERROR (0), significa que hubo errores (101, etc)
                if (resultadoAnalisis == ERR_NO_SINTAX_ERROR) {

                    // Solo si no hubo errores, obtenemos el código y ejecutamos
                    vector<tInstruccion> codigo = sintax.getBytecodeGenerado();
                    Runtime vm(codigo);
                    vm.run();

                } else {
                    // Si hubo errores, los imprimimos y ABORTAMOS
                    sintax.imprimirErrores();
                    cout << "No se puede ejecutar debido a errores de compilacion." << endl;
                }
            }
            else
            {
                 cout << "ERROR LEXICO FATAL" << endl;
            }
            break;
    }

    return 0;
}