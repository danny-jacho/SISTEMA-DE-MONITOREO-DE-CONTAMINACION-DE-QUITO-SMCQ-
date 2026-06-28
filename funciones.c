#include "funciones.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

long posZonas[ZONAS];
int contZonas = 1;

float limOMS [CONTAMINANTES] = {15,
                                25,
                                40,
                                100};

char nameContaminantes [CONTAMINANTES][10] =
{"PM25", "NO2", "SO2", "CO2"};

char recomContaminantes[CONTAMINANTES][100] =
    {"Evitar actividades al aire libre",                // PM2.5
    "Reducir tráfico vehicular",                        //NO2
    "Controlar emisiones industriales",                 //SO2
    "Reducir congestión y mejorar ventilación urbana"}; //CO2


void leerCadena(char *cadena, int n)
{
    int len;

    fgets(cadena, n, stdin);

    len = strlen(cadena);

    if (len > 0 && cadena[len - 1] == '\n')
    {
        cadena[len - 1] = '\0';
    }

    for (int i = 0; cadena[i] != '\0'; i++)
    {
        cadena[i] = toupper((unsigned char)cadena[i]);
    }

}

float validarFloatRango(float a, float b)
{
    float n;
    int aux;

    do
    {
        aux = scanf("%f", &n);

        while ((getchar()) != '\n');

        if (aux != 1 || n < a || n > b)
        {
            printf("Error: El valor ingresado es incorrecto\n");
            printf(">> ");
        }

    } while (aux != 1 || n < a || n > b);

    return n;
}

int menu()
{
    printf("\n----- SISTEMA DE MONITOREO DE CONTAMINACION DE QUITO (SMCQ) -----\n");
    printf("1. Registrar zona\n");
    printf("2. Predicciones \n");
    printf("3. Listar alertas \n");
    printf("4. Listar historicos \n");
    printf("5. Listar recomendaciones \n");
    printf("6. Configuración \n");
    printf("7. Generar reporte\n");
    printf("8. Salir\n");
    printf(">> ");

    int opc = (int) validarFloatRango(1, 8);

    return opc;
}
void crear()
{
    ZONA zona;
    int opc;

    printf("\n-------------------------------------------------------------------------------------\n");

    printf("NOMBBRE: ");
    leerCadena(zona.nombre,20);

    printf("REGISTRAR: \n");
    for (int c = 0; c < CONTAMINANTES; c++)
    {
        printf("%s: ", nameContaminantes[c]);
        zona.actual[c] = validarFloatRango(0, 50);

        if (zona.actual[c] > limOMS[c])
        {
            printf("%-15s", "¡ALERTA! NIVEL POR ENCIMA DEL LIMITE");
        }
    }

    printf("TEMPERATURA: ");
    zona.temperatura = validarFloatRango(0,50);

    printf("VIENTO: ");
    zona.viento = validarFloatRango(0,120);

    printf("HUMEDAD: ");
    zona.humedad = validarFloatRango(0,100);

    printf("\n-------------------------------------------------------------------------------------\n");

    printf("HISTORICOS: \n");
    for (int i = 0; i < DIAS; i++)
    {
        printf("DIA %d\n", i+1);

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            printf("%s: ", nameContaminantes[c]);
            zona.historicos[i][c] = validarFloatRango(0,50);
        }
        printf("\n-------------------------------------------------------------------------------------\n");

    }
    guardar(&zona);

    printf("¿Desea ver las recomendaciones? 1.SI/2.NO");
    opc = validarFloatRango(1,2);
    if (opc == 1)
    {
        generarRecomendaciones(1);
    }
    


}
void guardar(ZONA *zona)
{
    FILE *f = fopen("REGISTRO.dat", "ab");
    if (f == NULL)
    {
        printf("No se pudo abrir el archivo.\n");
        return;
    }

    long pos = ftell(f);
    fwrite(zona, sizeof(ZONA), 1, f);
    posZonas[contZonas -1] = pos;

    

    

    fclose(f);
    
}
void predicciones()
{
    float base ;
    float factor;
    long pos;
    ZONA zona;

    FILE *f = fopen("REGISTRO.dat", "rb+");
    if (f == NULL)
    {
        printf("No se pudo abrir el archivo.\n");
        return;

    }
    

    printf("\n-------------------------------------------------------------------------------------\n");
    printf("PREDICCIONES");
    printf("\n-------------------------------------------------------------------------------------\n");
    while (1)
    {
        pos = ftell(f);
        if (fread(&zona, sizeof(ZONA), 1, f) != 1)
        {
            break;
        }

        factor = calcularFactor(&zona);
        printf("ZONA: %s", zona.nombre);
        printf("\n-------------------------------------------------------------------------------------\n");

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            zona.promedio[c] = calcularPromedioPonderado(&zona, c);

            base = zona.actual[c] * 0.6 + zona.promedio[c] * 0.4;

            zona.prediccion[c] = base * factor;

            printf("%-10s %-10.2f", nameContaminantes[c], zona.prediccion[c]);

            if (zona.prediccion[c] > limOMS[c])
            {   
                strcpy(zona.estado[c], "¡EN ALERTA!");
                printf("%-15s", "¡ALERTA! NIVEL POR ENCIMA DEL LIMITE");
            }else{
                strcpy(zona.estado[c], "NORMAL");
            }
            printf("\n");
        }
        
        printf("\n-------------------------------------------------------------------------------------\n");

        fseek(f, pos, SEEK_SET);
        fwrite(&zona, sizeof(ZONA), 1, f);
        fflush(f);  // Sirve para guardar toda la información antes de usar nuevamente 'fread' en la siguiente iteración.
    }
    fclose(f);

    printf("¿Desea ver las recomendaciones? 1.SI/2.NO: ");
    int opc = validarFloatRango(1,2);
    if (opc == 1)
    {
        generarRecomendaciones(2);
    }

    
    printf("\n-------------------------------------------------------------------------------------\n");
}

float calcularFactor(ZONA *zona)
{
    float factor = 1.0;

    if (zona->viento < 2)
    {
        factor += 0.15;
    }

    if (zona->viento > 6)
    {
        factor -= 0.10;
    }

    if (zona->humedad > 70)
    {
        factor += 0.05;
    }

    if (zona->temperatura > 30)
    {
        factor += 0.05;
    }

    if (factor < 0.70)
    {
        factor = 0.70;
    }
    
    return factor;
}
float calcularPromedioPonderado(ZONA *zona, int contaminante)
{
    float suma = 0;
    float sumaPesos = 0;
    int peso;

    for (int i = 0; i < DIAS; i++)
    {
        peso = i + 1;

        suma = suma + zona->historicos[i][contaminante] * peso;
        sumaPesos = sumaPesos + peso;
    }

    return suma / sumaPesos;
}
void generarRecomendaciones(int n)
{
    FILE *f = fopen("REGISTRO.dat", "rb");
    if (f == NULL)
    {
        printf("No se pudo abrir el archivo.\n");
        return;
    }

    int aux;

    while (1)
    {   
        ZONA zona;
        aux = 0;
        if (fread(&zona, sizeof(ZONA), 1, f) != 1)
        {
            break;
        }

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            switch (n)
            {
            case 1:
                if (zona.actual[c] > limOMS[c])
                {   
                    aux += 1;
                    if (aux == 1)
                    {
                        printf("\n-------------------------------------------------------------------------------------\n");
                        printf("%s", zona.nombre);
                        printf("\n-------------------------------------------------------------------------------------\n");
                        printf("%-20s %-20s\n", "CONTAMINANTE", "RECOMENDACION");
                    }
                    
                    printf("%-20s %-20s\n", nameContaminantes[c], recomContaminantes[c]);

                }
                break;

            case 2:
                if (zona.prediccion[c] > limOMS[c])
                {
                    aux += 1;
                    if (aux == 1)
                    {
                        printf("\n-------------------------------------------------------------------------------------\n");
                        printf("%s", zona.nombre);
                        printf("\n-------------------------------------------------------------------------------------\n");
                        printf("%-20s %-20s\n", "CONTAMINANTE", "RECOMENDACION");
                    }
                    
                    printf("%-20s %-20s\n", nameContaminantes[c], recomContaminantes[c]);
                }
                break;
            default:
                break;
            }
        }
        aux = 0;
    }
    fclose(f);
}
void listarAlertas()
{
    FILE *f = fopen("REGISTRO.dat", "rb");
    if (f == NULL)
    {
        printf("No se pudo abrir el archivo.\n");
        return;
    }

    ZONA zona;
    int aux;

    while (1)
    {   
        aux = 0;
        if (fread(&zona, sizeof(ZONA), 1, f) != 1)
        {
            break;
        }



        for (int c = 0; c < CONTAMINANTES; c++)
        {
            if (zona.prediccion[c] > limOMS[c])
            {   
                aux += 1;

                if (aux == 1)
                {
                    printf("\n-------------------------------------------------------------------------------------\n");
                    printf("%s\n", zona.nombre);
                    printf("-------------------------------------------------------------------------------------\n");
                    printf("%-15s %-15s\n", "CONTAMINANTE", "ESTADO");
                }

                printf("%-15s %-15s\n", nameContaminantes[c], zona.estado[c]);

            }
        }
    }



    printf("\n-------------------------------------------------------------------------------------\n");

    fclose(f);
}
void listarRecomendacion()
{
    FILE *f = fopen("REGISTRO.dat", "rb");
    if (f == NULL)
    {
        printf("No se pudo abrir el archivo.\n");
        return;
    }

    ZONA zona;
    int aux;
    while (1)
    {   
        aux = 0;
        if (fread(&zona, sizeof(ZONA), 1, f) != 1)
        {
            break;
        }

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            if (zona.prediccion[c] > limOMS[c])
            {
                aux += 1;
                if (aux == 1)
                {
                    printf("\n-------------------------------------------------------------------------------------\n");
                    printf("%s\n", zona.nombre);
                    printf("-------------------------------------------------------------------------------------\n");
                    printf("%-15s %-50s\n", "CONTAMINANTE", "RECOMENDACION");
                }

                printf("%-15s %-50s\n", nameContaminantes[c], recomContaminantes[c]);

            }
        }
    }


    printf("\n-------------------------------------------------------------------------------------\n");

    fclose(f);
}

void configuracion()
{

    FILE *f = fopen("REGISTRO.dat", "rb");
    if (f == NULL)
    {
        printf("No se pudo abrir el archivo.\n");
        return;
    }
    ZONA zona;
    fread(&zona, sizeof(ZONA), 1, f);
    
    printf("\n-------------------------------------------------------------------------------------\n");
    printf("1. Nombres de contaminantes\n");
    printf("2. Limites directrices de la OMS\n");
    printf("3. Recomendaciones por contaminante\n");
    printf("4. Salir\n");
    
    printf("Escoga una opcion: ");
    int opc2 = validarFloatRango(1,4);
    
    switch (opc2)
    {
    case 1:
        for (int c = 0; c < CONTAMINANTES; c++)
        {
            printf("NOMBRE ACTUAL %d: %s", c+1, nameContaminantes[c]);
            printf("NUEVO NOMBRE: ");
            leerCadena(nameContaminantes[c],10);
        }
        break;
    case 2:
        for (int c = 0; c < CONTAMINANTES; c++)
        {
            printf("LIMITE ACTUAL DE %s",nameContaminantes[c]);
            printf("LIMITE NUEVO: ");
            limOMS[c] = validarFloatRango(0, 100);
        }
        break;
    case 3:
        for (int c = 0; c < CONTAMINANTES; c++)
        {
            printf("RECOMENDACION ACTUAL DE %s", recomContaminantes[c]);
            printf("NUEVA RECOMENDACION: ");
            leerCadena(recomContaminantes[c], 50);
        }
            break;
    case 4:
        fclose(f);
        return;
        break;

    default:
        break;
        
    
    }
    fclose(f);
}
void listarHistoricos()
{
    FILE *f = fopen("REGISTRO.dat", "rb");
    if (f == NULL)
    {
        printf("No se pudo abrir el archivo.\n");
        return;
    }
    ZONA zona;

    while (1)
    {
        if (fread(&zona, sizeof(ZONA), 1, f) != 1)
        {
            break;
        }

        printf("\n-------------------------------------------------------------------------------------\n");
        printf("%s", zona.nombre);
        printf("\n-------------------------------------------------------------------------------------\n");

        printf("%-15s", "DIA");
        
        for (int c = 0; c < CONTAMINANTES; c++)
        {
            printf("%-15s ", nameContaminantes[c]);
        }

      
        for (int d = 0; d < DIAS; d++)
        {
            printf("\n%-15d", d+1);

            for (int c = 0; c < CONTAMINANTES; c++)
            {
                printf("%-15.2f ", zona.historicos[d][c]);
            }
        }
    }
    printf("\n-------------------------------------------------------------------------------------\n");
    fclose(f);
}
void generarReporte()
{   
    FILE *r = fopen("REGISTRO.dat", "rb");  // r de REGISTRO   
    if (r == NULL)
    {
        printf("No se pudo abrir el archivo.\n");
        return;
    }

    FILE *f = fopen("REPORTE.txt", "w");
    if (f == NULL)
    {
        printf("No se pudo abrir el archivo.\n");
        return;
    }
    ZONA zona;
    int aux;
    int flag;

    fprintf(f, "REPORTE DEL SISTEMA DE MONITOREO DE CONTAMINACION DE QUITO");
    fprintf(f, "\n-------------------------------------------------------------------------------------\n");

    fprintf(f, "LIMITES USADOS\n");

    for (int c = 0; c < CONTAMINANTES; c++)
    {
        fprintf(f, "%s: %.2f\n", nameContaminantes[c], limOMS[c]);
    }

    while (1)
    {   
        aux = 0;
        flag = 0;

        if (fread(&zona, sizeof(ZONA), 1, r) != 1)
        {
            break;
        }

        fprintf(f, "\n-------------------------------------------------------------------------------------\n");
        fprintf(f, "ZONA: %s", zona.nombre);
        fprintf(f, "\n-------------------------------------------------------------------------------------\n");

        fprintf(f, "DATOS CLIMATICOS\n");
        fprintf(f, "Temperatura: %.2f\n", zona.temperatura);
        fprintf(f, "Viento: %.2f\n", zona.viento);
        fprintf(f, "Humedad: %.2f\n", zona.humedad);

        fprintf(f, "\n");

        fprintf(f, "CONTAMINACION ACTUAL\n");
        fprintf(f, "%-20s %-20s %-20s %-20s\n", "CONTAMINANTE", "ACTUAL", "LIMITE", "ESTADO");

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            fprintf(f, "%-20s %-20.2f %-20.2f %-20s \n", nameContaminantes[c],
                    zona.actual[c],
                    limOMS[c],
                    zona.estado[c]);
        }

        fprintf(f, "\n");

        fprintf(f, "PROMEDIOS Y PREDICCIONES\n");
        fprintf(f, "%-20s %-20s %-20s %-20s %-20s\n", "CONTAMINANTE", "PROMEDIO", "PREDICCION", "LIMITE", "ESTADO");

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            fprintf(f, "%-20s %-20.2f %-20.2f %-20.2f %-20s\n", nameContaminantes[c],
                    zona.promedio[c],
                    zona.prediccion[c],
                    limOMS[c],
                    zona.estado[c]);
        }

        fprintf(f, "\n");

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            
            if ((zona.prediccion[c] > limOMS[c]) || (zona.promedio[c] > limOMS[c]))
            {
                aux += 1;
                flag = 1;

                if (aux == 1)
                {
                    fprintf(f, "RECOMENDACIONES\n");
                }

                fprintf(f, "%s: %s\n", nameContaminantes[c], recomContaminantes[c]);
            }
        }
        if (flag != 1)
        {
            fprintf(f, "SIN RECOMENDACIONES\n");
        }
        
    }
    fclose(f);
    fclose(r);
}   