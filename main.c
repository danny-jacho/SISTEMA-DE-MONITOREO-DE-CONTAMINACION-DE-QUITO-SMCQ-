#include <stdio.h>
#include "funciones.h"

int main (int argc, char *argv[]) {

    int opc, opc2;

    do 
    {
        opc = menu();

        switch (opc)
        {
        case 1:
            crear();
            break;

        case 2:
            predicciones();
            break;

        case 3:
            listarAlertas();
            break;
        case 4:
            listarHistoricos();
            break;
        case 5:
            generarRecomendaciones();
            break;
        case 6:
            editar();
            break;
        case 7:
            generarReporte();
            break;
        case 8:
            return 0;
            break;

        default:
            break;
        }
        
        printf("¿Desea continuar? 1.SI/2.NO: ");
        opc2 = validarFloatRango(1, 2);
    }while( opc2 == 1);

    return 0;
}

