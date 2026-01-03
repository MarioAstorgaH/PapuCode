/*
NOTA: para compilar (en Windows) debe estar instalado:
    - MSYS2 MINGW64
    - CMAKE

   > mingw32-make
*/

#include <iostream>
#include <fstream>
#include <vector>
#include "lexico.h"
#include "sintaxis.h"


using namespace std;


int main(int argc, char* argv[])
{
    vector<char>tCars;
    int error;
    string errToken;

    Lexico lex;

    char modoEjecucion = 'L';  // L : lexico; S : sintaxis;  M : semantica;  B : byte-code;   R : run-time


    if (argc < 2)
    {
        cout << "Uso: " << argv[0] << " <archivo.txt> <parametro>\n";
        cout << "   <parametro>:\n";
        cout << "      -L : (Lexico) para mostrar la lista de tokens\n";
        cout << "      -S : (Sintaxis) para hacer la revision sintactica\n";
        cout << "      -M : (seMantica) muestra la lista de identificadores declarados\n";
        cout << "      -B : (Byte-code) mostrar el byte-code generado\n";
        cout << "      -R : (Run-time) Ejecuta el codigo compilado\n";
        return 1;
    }

    ifstream archivo(argv[1], ios::in);
    if (!archivo)
    {
        cerr << "No se pudo abrir el archivo: " << argv[1] << "\n";
        return 1;
    }

    if (argc > 1)
    {
        string arg = argv[2];

        if (arg == "-L") modoEjecucion = 'L';
        else if (arg == "-S") modoEjecucion = 'S';
        else if (arg == "-M") modoEjecucion = 'M';
        else if (arg == "-B") modoEjecucion = 'B'; // Agregado
        else if (arg == "-R") modoEjecucion = 'R';
        else modoEjecucion = 'S';
    }


    //----------------------- leer todos los caracteres del archivo
    char c;
    while (archivo.get(c))
        tCars.push_back(c);
    tCars.push_back('\n');
    archivo.close();
    //-----------------------

    switch (modoEjecucion)
    {
        case 'L' :
            error = lex.generaLexico(tCars, errToken, false);
            if (error == ERR_NOERROR)
                lex.imprimir();
            else
                cout << "ERROR LEXICO: " << error << " :: " << errToken << "\n";
            cout << "Total de lineas procesadas: " << lex.getLineas() << "\n";
            break;

        case 'S' : // SOLO SINTAXIS
            error = lex.generaLexico(tCars, errToken, false);

            if (error == ERR_NOERROR)
            {
                lex.imprimir();
                cout << "=== INICIANDO ANALISIS SINTACTICO ===" << endl;

                // CORRECCION: 3 argumentos (Lexico, Semantica=OFF, Bytecode=OFF)
                Sintaxis sintax(lex, false, false);

                sintax.generaSintaxis();
                sintax.imprimirErrores();
            }
            else
                cout << "ERROR LEXICO FATAL: " << error << " :: " << errToken << "\n";
            break;

        case 'M' : // SINTAXIS + SEMANTICA
            error = lex.generaLexico(tCars, errToken, false);

            if (error == ERR_NOERROR)
            {
                cout << "=== INICIANDO ANALISIS SEMANTICO ===" << endl;

                // CORRECCION: 3 argumentos (Lexico, Semantica=ON, Bytecode=OFF)
                Sintaxis sintax(lex, true, false);

                sintax.generaSintaxis();
                sintax.imprimirErrores();
            }
            else
                cout << "ERROR LEXICO FATAL: " << error << " :: " << errToken << "\n";
            break;

        case 'B' : // BYTECODE
            error = lex.generaLexico(tCars, errToken, false);
            if (error == ERR_NOERROR)
            {
                cout << "=== GENERANDO BYTECODE ===" << endl;

                // CORRECCION: 3 argumentos (Lexico, Semantica=ON, Bytecode=ON)
                // Necesitamos semántica ON para saber los tipos de las variables al generar código
                Sintaxis sintax(lex, true, true);

                sintax.generaSintaxis();
                sintax.imprimirErrores();

                // Solo imprimimos el código si no hubo errores graves
                sintax.imprimirBytecode();
            }
            else
                cout << "ERROR LEXICO FATAL" << endl;
            break;

        case 'R' :
            cout << "El modo Run-Time aun no esta implementado." << endl;
            break;
    }

    return 0;
}