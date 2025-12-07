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
    vector<tToken> lst;
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

        if (arg == "-L")
            modoEjecucion = 'L';
        else if (arg == "-S")
            modoEjecucion = 'S';
        else if (arg == "-M")
            modoEjecucion = 'M';
        else if (arg == "-B")
            modoEjecucion = 'B';
        else if (arg == "-R")
            modoEjecucion = 'R';
        else
            modoEjecucion = 'S';
    }

 
            //----------------------- leer todos los caracteres del archivo. Meterlos a una lista
    char c; 
    while (archivo.get(c)) 
        tCars.push_back(c);
    tCars.push_back('\n');
    archivo.close();
    //-----------------------

    //-------------------- control. Imprimir el contenido de la lista para ver si entraron todos los caracteres
    //for (auto c = begin(tCars); c != end(tCars); c++)
    //    cout << *c;
    //--------------------

    switch (modoEjecucion)
    {
        case 'L' :
            error = lex.generaLexico(tCars, errToken, true);

            if (error == ERR_NOERROR)
                lex.imprimir();
            else 
                cout << "ERROR: " << error << " :: " << errToken << "\n";
            cout << "Total de lineas procesadas: " << lex.getLineas() << "\n";
            break;
        
        case 'S' :
            error = lex.generaLexico(tCars, errToken, false);

            if (error == ERR_NOERROR)
            {
                //lex.imprimir();
                Sintaxis sintax(lex);  
                error = sintax.generaSintaxis();
                string msgErrorSintax = sintax.mensajeError(error);
                cout << "ERROR DE SINTAXIS: " << error << " :: " << msgErrorSintax << "\n";
            }
            else 
                cout << "ERROR: " << error << " :: " << errToken << "\n";

            cout << "Total de lineas procesadas: " << lex.getLineas() << "\n";
            break;

        case 'M' : 
            break;

        case 'B' :
            break;

        case 'R' :
            break;
    }

    return 0;
}
