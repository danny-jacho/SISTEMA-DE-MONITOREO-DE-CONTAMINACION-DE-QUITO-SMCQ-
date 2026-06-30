#include "funciones.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

int contZonas = 0;

float limOMS [CONTAMINANTES] = {15,
                                25,
                                40,
                                40};

char nameContaminantes [CONTAMINANTES][10] =
{"PM25", "NO2", "SO2", "CO2"};

char recomContaminantes[CONTAMINANTES][100] =
    {"Evitar actividades al aire libre",                //PM2.5
    "Reducir trafico vehicular",                        //NO2
    "Controlar emisiones industriales",                 //SO2
    "Reducir congestion y mejorar ventilacion urbana"}; //CO2


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
            printf("Error: El valor ingresado esta fuera del rango de %.2f a %.2f\n", a, b);
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
    printf("6. Editar \n");
    printf("7. Generar reporte\n");
    printf("8. Salir\n");
    printf(">> ");

    int opc = (int) validarFloatRango(1, 8);

    return opc;
}
void crear()
{
    if (contZonas >= 5)
    {
        printf("Limite de zonas alcanzado.\n");
        return;
    }
    
    ZONA zona;
    int opc;
    int flagR = 0;

    printf("\n-------------------------------------------------------------------------------------\n");

    printf("NOMBBRE: ");
    leerCadena(zona.nombre,20);

    printf("REGISTRAR: \n");
    for (int c = 0; c < CONTAMINANTES; c++)
    {
        printf("%s (ug/m^3): ", nameContaminantes[c]);
        zona.actual[c] = validarFloatRango(0, 50);

        if (zona.actual[c] > limOMS[c])
        {
            printf("%-15s", "¡ALERTA! NIVEL POR ENCIMA DEL LIMITE\n");
            flagR = 1;
        }
    }

    printf("TEMPERATURA (C): ");
    zona.temperatura = validarFloatRango(0,35);

    printf("VIENTO (km/h): ");
    zona.viento = validarFloatRango(0,60);

    printf("HUMEDAD (%%): ");
    zona.humedad = validarFloatRango(0,100);

    printf("\n-------------------------------------------------------------------------------------\n");

    printf("HISTORICOS: \n");
    printf("CANTIDAD DE DIAS A REGISTRAR: ");
    zona.dias = validarFloatRango(1, MAX_DIAS);

    for (int i = 0; i < zona.dias; i++)
    {
        printf("DIA %d\n", i+1);

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            printf("%s (ug/m^3): ", nameContaminantes[c]);
            zona.historicos[i][c] = validarFloatRango(0,50);
        }
        printf("\n-------------------------------------------------------------------------------------\n");

    }
    
    for (int c = 0; c < CONTAMINANTES; c++)
    {
        zona.promedio[c] = 0;
        zona.prediccion[c] = 0;
        strcpy(zona.estado[c], "PENDIENTE");
    }
    guardar(&zona);

    if (flagR == 1)
    {
        printf("¿Desea ver las recomendaciones? 1.SI/2.NO: ");
        opc = validarFloatRango(1,2);
        if (opc == 1)
        {
            generarRecomendaciones();
        }
        
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

    fwrite(zona, sizeof(ZONA), 1, f);
    fclose(f);

    contZonas++;
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
    
    int opc2;

    printf("\n-------------------------------------------------------------------------------------\n");
    printf("PREDICCIONES DENTRO DE LAS PROXIMAS 24 HORAS");
    printf("\n-------------------------------------------------------------------------------------\n");

    printf("\n1. Ver todas\n");
    printf("2. Ver por zona\n");
    printf(">> ");
    opc2 = validarFloatRango(1,2);
    if (opc2 == 1)
    {  
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
    
            printf("%-20s %-20s %-20s\n", "CONTAMINANTE", "NIVEL", "LIMITE");
    
            for (int c = 0; c < CONTAMINANTES; c++)
            {
                zona.promedio[c] = calcularPromedioPonderado(&zona, c);
    
                base = zona.actual[c] * 0.6 + zona.promedio[c] * 0.4;
    
                zona.prediccion[c] = base * factor;
    
                printf("%-20s %-20.2f %-20.2f", nameContaminantes[c], zona.prediccion[c], limOMS[c]);
    
                if (zona.prediccion[c] > limOMS[c])
                {   
                    strcpy(zona.estado[c], "¡EN ALERTA!");
                    printf("%-15s", "¡ALERTA! NIVEL POR ENCIMA DEL LIMITE");
                }else{
                    strcpy(zona.estado[c], "NORMAL");
                    printf("%-15s", "NORMAL");
                }
                printf("\n");
            }
            
            printf("\n-------------------------------------------------------------------------------------\n");
    
            fseek(f, pos, SEEK_SET);
            fwrite(&zona, sizeof(ZONA), 1, f);
            fflush(f);  // Sirve para guardar toda la información antes de usar nuevamente 'fread' en la siguiente iteración.
        }
    }

    if (opc2 == 2)
    {
        long pos = seleccionarZona(f);
        fseek(f, pos, SEEK_SET);
        fread(&zona, sizeof(ZONA), 1, f);

        factor = calcularFactor(&zona);
        printf("ZONA: %s", zona.nombre);
        printf("\n-------------------------------------------------------------------------------------\n");

        printf("%-20s %-20s %-20s\n", "CONTAMINANTE", "NIVEL", "LIMITE");

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            zona.promedio[c] = calcularPromedioPonderado(&zona, c);

            base = zona.actual[c] * 0.6 + zona.promedio[c] * 0.4;

            zona.prediccion[c] = base * factor;

            printf("%-20s %-20.2f %-20.2f", nameContaminantes[c], zona.prediccion[c], limOMS[c]);

            if (zona.prediccion[c] > limOMS[c])
            {
                strcpy(zona.estado[c], "¡EN ALERTA!");
                printf("%-15s", "¡ALERTA! NIVEL POR ENCIMA DEL LIMITE");
            }
            else
            {
                strcpy(zona.estado[c], "NORMAL");
                printf("%-15s", "NORMAL");
            }
            printf("\n");
        }

        printf("\n-------------------------------------------------------------------------------------\n");

        fseek(f, pos, SEEK_SET);
        fwrite(&zona, sizeof(ZONA), 1, f);
        fflush(f); // Sirve para guardar toda la información antes de usar nuevamente 'fread' en la siguiente iteración.
    }
    
    
    fclose(f);

    printf("¿Desea ver las recomendaciones? 1.SI/2.NO: ");
    int opc = validarFloatRango(1,2);
    if (opc == 1)
    {
        generarRecomendaciones();
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

    for (int i = 0; i < zona->dias; i++)
    {
        peso = i + 1;

        suma = suma + zona->historicos[i][contaminante] * peso;
        sumaPesos = sumaPesos + peso;
    }

    return suma / sumaPesos;
}
void generarRecomendaciones()
{
    FILE *f = fopen("REGISTRO.dat", "rb");
    if (f == NULL)
    {
        printf("No se pudo abrir el archivo.\n");
        return;
    }

    int aux;
    int opc;
    int opc2;
    int flag;
    ZONA zona;

    printf("1. Ver recomendaciones actuales\n");
    printf("2. Ver recomendaciones promedio\n");
    printf("3. Ver recomendaciones predictivas\n");
    printf(">> ");
    opc = validarFloatRango(1,3);

    printf("\n1. Ver todas\n");
    printf("2. Ver por zona\n");
    printf(">> ");
    opc2 = validarFloatRango(1,2);

    switch (opc)
    {
    case 1:
        aux = 0;
        printf("\n-------------------------------------------------------------------------------------\n");
        printf("RECOMENDACIONES ACTUALES");
        printf("\n-------------------------------------------------------------------------------------\n");

        if (opc2 == 1)
        {
            while (1)
            {
                if (fread(&zona, sizeof(ZONA), 1, f) != 1)
                {
                    break;
                }
                for (int c = 0; c < CONTAMINANTES; c++)
                {
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
                }
            }
        }
        if (opc2 == 2)
        {
            long pos;
            
            pos = seleccionarZona(f);

            fseek(f, pos, SEEK_SET);
            fread(&zona, sizeof(ZONA), 1, f);

            for (int c = 0; c < CONTAMINANTES; c++)
            {
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
            }
            
        }
        break;

    case 2:
        aux = 0;
        printf("\n-------------------------------------------------------------------------------------\n");
        printf("RECOMENDACIONES PROMEDIO");
        printf("\n-------------------------------------------------------------------------------------\n");

        if (opc2 == 1)
        {
            while (1)
            {
                if (fread(&zona, sizeof(ZONA), 1, f) != 1)
                {
                    break;
                }
                for (int c = 0; c < CONTAMINANTES; c++)
                {
                    if (zona.promedio[c] > limOMS[c])
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
                }
            }
        }
        if (opc2 == 2)
        {
            long pos;

            pos = seleccionarZona(f);

            fseek(f, pos, SEEK_SET);
            fread(&zona, sizeof(ZONA), 1, f);

            for (int c = 0; c < CONTAMINANTES; c++)
            {
                if (zona.promedio[c] > limOMS[c])
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
            }
        }
        break;
    
    case 3:
        aux = 0;
        printf("\n-------------------------------------------------------------------------------------\n");
        printf("RECOMENDACIONES PREDICTIVAS");
        printf("\n-------------------------------------------------------------------------------------\n");

        if (opc2 == 1)
        {   
            flag = 0;
            while (1)
            {
                if (fread(&zona, sizeof(ZONA), 1, f) != 1)
                {
                    break;
                }
                flag = 1;
                for (int c = 0; c < CONTAMINANTES; c++)
                {
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
                }

                if (flag == 0)
                {
                    printf("\nSin recomendaciones..\n");
                    printf("Ejecute la función predicciones.\n");
                }
                
            }
        }
        if (opc2 == 2)
        {
            long pos;
            flag = 0;
            pos = seleccionarZona(f);

            fseek(f, pos, SEEK_SET);
            fread(&zona, sizeof(ZONA), 1, f);
  

            for (int c = 0; c < CONTAMINANTES; c++)
            {
                if (strcmp(zona.estado[c], "PENDIENTE") == 0)
                {
                    printf("\nSin recomendaciones..\n");
                    printf("Ejecute la función predicciones.\n");
                }
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
            }
        }
        break;
    default:
        break;
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

    int aux;
    int opc;
    int opc2;
    int flag;
    ZONA zona;

    printf("1. Ver alertas actuales\n");
    printf("2. Ver alertas promedio\n");
    printf("3. Ver alertas predictivas\n");
    printf(">> ");
    opc = validarFloatRango(1,3);

    printf("\n1. Ver todas\n");
    printf("2. Ver por zona\n");
    printf(">>");
    opc2 = validarFloatRango(1,2);

    switch (opc)
    {
    case 1:
        aux = 0;
        printf("\n-------------------------------------------------------------------------------------\n");
        printf("ALERTAS ACTUALES");
        printf("\n-------------------------------------------------------------------------------------\n");

        if (opc2 == 1)
        {
            while (1)
            {
                if (fread(&zona, sizeof(ZONA), 1, f) != 1)
                {
                    break;
                }
                for (int c = 0; c < CONTAMINANTES; c++)
                {
                    if (zona.actual[c] > limOMS[c])
                    {
                        aux += 1;
                        if (aux == 1)
                        {
                            printf("\n-------------------------------------------------------------------------------------\n");
                            printf("%s", zona.nombre);
                            printf("\n-------------------------------------------------------------------------------------\n");
                            printf("%-20s \n", "CONTAMINANTE");
                        }

                        printf("%-20s %-20s\n", nameContaminantes[c], "ALERTA");
                    }
                }
            }
        }
        if (opc2 == 2)
        {
            long pos;
            flag = 0;

            pos = seleccionarZona(f);

            fseek(f, pos, SEEK_SET);
            fread(&zona, sizeof(ZONA), 1, f);

            for (int c = 0; c < CONTAMINANTES; c++)
            {
                if (zona.actual[c] > limOMS[c])
                {   
                    flag = 1;
                    aux += 1;
                    if (aux == 1)
                    {
                        printf("\n-------------------------------------------------------------------------------------\n");
                        printf("%s", zona.nombre);
                        printf("\n-------------------------------------------------------------------------------------\n");
                        printf("%-20s \n", "CONTAMINANTE");
                    }
    
                    printf("%-20s %-20s\n", nameContaminantes[c], "ALERTA");
                }
            }
            if (flag == 0)
            {
                printf("Sin alertas...\n");
            }
            
        }
        break;

    case 2:
        printf("\n-------------------------------------------------------------------------------------\n");
        printf("ALERTAS PROMEDIO");
        printf("\n-------------------------------------------------------------------------------------\n");

        if (opc2 == 1)
        {
            while (1)
            {
                if (fread(&zona, sizeof(ZONA), 1, f) != 1)
                {
                    break;
                }
                for (int c = 0; c < CONTAMINANTES; c++)
                {
                    if (zona.promedio[c] > limOMS[c])
                    {
                        aux += 1;
                        if (aux == 1)
                        {
                            printf("\n-------------------------------------------------------------------------------------\n");
                            printf("%s", zona.nombre);
                            printf("\n-------------------------------------------------------------------------------------\n");
                            printf("%-20s \n", "CONTAMINANTE");
                        }

                        printf("%-20s %-20s\n", nameContaminantes[c], "ALERTA");
                    }
                }
            }
        }
        if (opc2 == 2)
        {
            long pos;

            pos = seleccionarZona(f);

            fseek(f, pos, SEEK_SET);
            fread(&zona, sizeof(ZONA), 1, f);

            for (int c = 0; c < CONTAMINANTES; c++)
            {
                if (zona.promedio[c] > limOMS[c])
                {
                    aux += 1;
                    if (aux == 1)
                    {
                        printf("\n-------------------------------------------------------------------------------------\n");
                        printf("%s", zona.nombre);
                        printf("\n-------------------------------------------------------------------------------------\n");
                        printf("%-20s \n", "CONTAMINANTE");
                    }

                    printf("%-20s %-20s\n", nameContaminantes[c], "ALERTA");
                }
            }
        }
        break;
    
    case 3:
        printf("\n-------------------------------------------------------------------------------------\n");
        printf("ALERTAS PREDICTIVAS");
        printf("\n-------------------------------------------------------------------------------------\n");

        if (opc2 == 1)
        {   
            flag = 0;
            while (1)
            {
                if (fread(&zona, sizeof(ZONA), 1, f) != 1)
                {
                    break;
                }

                flag = 1;
                for (int c = 0; c < CONTAMINANTES; c++)
                {
                    if (zona.prediccion[c] > limOMS[c])
                    {
                        aux += 1;
                        if (aux == 1)
                        {
                            printf("\n-------------------------------------------------------------------------------------\n");
                            printf("%s", zona.nombre);
                            printf("\n-------------------------------------------------------------------------------------\n");
                            printf("%-20s \n", "CONTAMINANTE");
                        }

                        printf("%-20s %-20s\n", nameContaminantes[c], "ALERTA");
                    }
                }

                if (flag == 0)
                {
                    printf("\nSin alertas..\n");
                }
                
            }
        }
        if (opc2 == 2)
        {
            long pos;
            flag = 0;
            pos = seleccionarZona(f);

            fseek(f, pos, SEEK_SET);
            fread(&zona, sizeof(ZONA), 1, f);          

            for (int c = 0; c < CONTAMINANTES; c++)
            {
                if (strcmp(zona.estado[c], "PENDIENTE") == 0)
                {
                    printf("\nSin alertas..\n");
                }
                if (zona.prediccion[c] > limOMS[c])
                {
                    aux += 1;
                    if (aux == 1)
                    {
                        printf("\n-------------------------------------------------------------------------------------\n");
                        printf("%s", zona.nombre);
                        printf("\n-------------------------------------------------------------------------------------\n");
                        printf("%-20s \n", "CONTAMINANTE");
                    }

                    printf("%-20s %-20s\n", nameContaminantes[c], "ALERTA");
                }
            }
        }
        break;
    default:
        break;
    }
    
    fclose(f);
}

void editar()
{
    int opc2;

    printf("\n-------------------------------------------------------------------------------------\n");
    printf("1. Contaminacion actual\n");
    printf("2. Historicos\n");
    printf("3. Nombres de contaminantes\n");
    printf("4. Limites directrices de la OMS\n");
    printf("5. Recomendaciones por contaminante\n");
    printf("6. Salir\n");

    printf("Escoga una opcion: ");
    opc2 = validarFloatRango(1, 6);

    switch (opc2)
    {
    case 1:
    {
        FILE *f = fopen("REGISTRO.dat", "rb+");

        if (f == NULL)
        {
            printf("No se pudo abrir el archivo.\n");
            return;
        }

        ZONA zona;
        long pos;

        pos = seleccionarZona(f);

        fseek(f, pos, SEEK_SET);
        fread(&zona, sizeof(ZONA), 1, f);

        printf("\nZONA SELECCIONADA: %s\n", zona.nombre);
        printf("REGISTRAR NUEVOS DATOS\n");

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            printf("%s (ug/m^3): ", nameContaminantes[c]);
            zona.actual[c] = validarFloatRango(0, 50);
        }

        printf("TEMPERATURA (C): ");
        zona.temperatura = validarFloatRango(0, 35);

        printf("VIENTO (km/h): ");
        zona.viento = validarFloatRango(0, 60);

        printf("HUMEDAD (%%): ");
        zona.humedad = validarFloatRango(0, 100);

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            zona.promedio[c] = 0;
            zona.prediccion[c] = 0;
            strcpy(zona.estado[c], "PENDIENTE");
        }

        fseek(f, pos, SEEK_SET);
        fwrite(&zona, sizeof(ZONA), 1, f);

        fclose(f);

        printf("Datos actualizados.\n");
        printf("Ejecute nuevamente las predicciones.\n");

        break;
    }

    case 2:
    {
        FILE *f = fopen("REGISTRO.dat", "rb+");

        if (f == NULL)
        {
            printf("No se pudo abrir el archivo.\n");
            return;
        }

        ZONA zona;
        long pos;


        pos = seleccionarZona(f);


        fseek(f, pos, SEEK_SET);
        fread(&zona, sizeof(ZONA), 1, f);

        printf("\nZONA SELECCIONADA: %s\n", zona.nombre);

        printf("%-15s", "DIA");

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            printf("%-15s ", nameContaminantes[c]);
        }

        for (int d = 0; d < zona.dias; d++)
        {
            printf("\n%-15d", d + 1);

            for (int c = 0; c < CONTAMINANTES; c++)
            {
                printf("%-15.2f ", zona.historicos[d][c]);
            }
        }

        printf("\nEscoga el dia: ");
        int dia = validarFloatRango(1, zona.dias);

        printf("EDITAR HISTORICOS DEL DIA %d\n", dia);

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            printf("%s (ug/m^3): ", nameContaminantes[c]);
            zona.historicos[dia - 1][c] = validarFloatRango(0, 50);
        }

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            zona.promedio[c] = 0;
            zona.prediccion[c] = 0;
            strcpy(zona.estado[c], "PENDIENTE");
        }

        fseek(f, pos, SEEK_SET);
        fwrite(&zona, sizeof(ZONA), 1, f);

        fclose(f);

        printf("Historico actualizados.\n");
        printf("Ejecute nuevamente las predicciones.\n");
        break;
    }

    case 3:
    {
        for (int c = 0; c < CONTAMINANTES; c++)
        {
            printf("NOMBRE ACTUAL %d: %s\n", c + 1, nameContaminantes[c]);
            printf("NUEVO NOMBRE: ");
            leerCadena(nameContaminantes[c], 10);
        }

        break;
    }

    case 4:
    {
        for (int c = 0; c < CONTAMINANTES; c++)
        {
            printf("LIMITE ACTUAL DE %s (ug/m^3): %.2f\n", nameContaminantes[c], limOMS[c]);
            printf("LIMITE NUEVO (ug/m^3): ");
            limOMS[c] = validarFloatRango(0, 100);
        }

        break;
    }

    case 5:
    {
        for (int c = 0; c < CONTAMINANTES; c++)
        {
            printf("RECOMENDACION ACTUAL DE %s: %s\n", nameContaminantes[c], recomContaminantes[c]);
            printf("NUEVA RECOMENDACION: ");
            leerCadena(recomContaminantes[c], 100);
        }

        break;
    }

    case 6:
        return;

    default:
        break;
    }
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
    int opc2;

    printf("\n-------------------------------------------------------------------------------------\n");
    printf("HISTORICOS");
    printf("\n-------------------------------------------------------------------------------------\n");

    printf("\n1. Ver todos\n");
    printf("2. Ver por zona\n");
    opc2 = validarFloatRango(1,2);

    if (opc2 == 1)
    {
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
    
            for (int d = 0; d < zona.dias; d++)
            {
                printf("\n%-15d", d + 1);
    
                for (int c = 0; c < CONTAMINANTES; c++)
                {
                    printf("%-15.2f ", zona.historicos[d][c]);
                }
            }
        }
    
        printf("\n-------------------------------------------------------------------------------------\n");
    }
    if (opc2 == 2)
    {
        long pos = seleccionarZona(f);

        fseek(f, pos, SEEK_SET);
        fread(&zona, sizeof(ZONA), 1, f);

        printf("\n-------------------------------------------------------------------------------------\n");
        printf("%s", zona.nombre);
        printf("\n-------------------------------------------------------------------------------------\n");

        printf("%-15s", "DIA");

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            printf("%-15s ", nameContaminantes[c]);
        }

        for (int d = 0; d < zona.dias; d++)
        {
            printf("\n%-15d", d + 1);

            for (int c = 0; c < CONTAMINANTES; c++)
            {
                printf("%-15.2f ", zona.historicos[d][c]);
            }
        }
    }
    fclose(f);
}
void generarReporte()
{
    FILE *r = fopen("REGISTRO.dat", "rb");  // Archivo de registros

    if (r == NULL)
    {
        printf("No se pudo abrir el archivo REGISTRO.dat.\n");
        return;
    }

    FILE *f = fopen("REPORTE.txt", "w");    // Archivo de reporte

    if (f == NULL)
    {
        printf("No se pudo crear el archivo REPORTE.txt.\n");
        fclose(r);
        return;
    }

    ZONA zona;
    int aux;
    int totalZonas = 0;

    fprintf(f, "REPORTE DEL SISTEMA DE MONITOREO DE CONTAMINACION DE QUITO\n");
    fprintf(f, "-------------------------------------------------------------------------------------\n");

    fprintf(f, "LIMITES USADOS\n");

    for (int c = 0; c < CONTAMINANTES; c++)
    {
        fprintf(f, "%-10s %-10.2f (ug/m^3)\n", nameContaminantes[c], limOMS[c]);
    }

    while (1)
    {
        if (fread(&zona, sizeof(ZONA), 1, r) != 1)
        {
            break;
        }

        totalZonas++;

        fprintf(f, "\n-------------------------------------------------------------------------------------\n");
        fprintf(f, "ZONA: %s\n", zona.nombre);
        fprintf(f, "-------------------------------------------------------------------------------------\n");

        fprintf(f, "DATOS CLIMATICOS\n");
        fprintf(f, "%-15s %.2f C\n","Temperatura", zona.temperatura);
        fprintf(f, "%-15s %.2f km/h\n","Viento", zona.viento);
        fprintf(f, "%-15s %.2f %%\n","Humedad", zona.humedad);

        fprintf(f, "\n");

        fprintf(f, "CONTAMINACION ACTUAL\n");
        fprintf(f, "%-20s %-20s %-20s %-20s\n", 
                "CONTAMINANTE", "ACTUAL", "LIMITE", "ESTADO");

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            fprintf(f, "%-20s %-20.2f %-20.2f ",
                    nameContaminantes[c],
                    zona.actual[c],
                    limOMS[c]);

            if (zona.actual[c] > limOMS[c])
            {
                fprintf(f, "%-20s\n", "ALERTA");
            }
            else
            {
                fprintf(f, "%-20s\n", "NORMAL");
            }
        }

        fprintf(f, "\n");

        fprintf(f, "PROMEDIOS Y PREDICCIONES\n");
        fprintf(f, "%-20s %-20s %-20s %-20s %-20s\n",
                "CONTAMINANTE", "PROMEDIO", "PREDICCION", "LIMITE", "ESTADO");

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            fprintf(f, "%-20s %-20.2f %-20.2f %-20.2f %-20s\n",
                    nameContaminantes[c],
                    zona.promedio[c],
                    zona.prediccion[c],
                    limOMS[c],
                    zona.estado[c]);
        }

        fprintf(f, "\n");

        aux = 0;

        for (int c = 0; c < CONTAMINANTES; c++)
        {
            if (zona.prediccion[c] > limOMS[c])
            {
                aux++;

                if (aux == 1)
                {
                    fprintf(f, "RECOMENDACIONES\n");
                }

                fprintf(f, "%s: %s\n",
                        nameContaminantes[c],
                        recomContaminantes[c]);
            }
        }

        if (aux == 0)
        {
            fprintf(f, "SIN RECOMENDACIONES\n");
        }
    }

    fprintf(f, "\n-------------------------------------------------------------------------------------\n");
    fprintf(f, "TOTAL DE ZONAS REGISTRADAS: %d\n", totalZonas);
    fprintf(f, "-------------------------------------------------------------------------------------\n");

    fclose(f);
    fclose(r);

    printf("Reporte generado con exito.\n");
}
long seleccionarZona(FILE *f)
{
    ZONA zona;
    long posiciones[ZONAS];
    long pos;
    int cont = 0;
    int opc;

    rewind(f);

    while (1)
    {
        pos = ftell(f);

        if (fread(&zona, sizeof(ZONA), 1, f) != 1)
        {
            break;
        }

        posiciones[cont] = pos;

        printf("ZONA #%d: %s\n", cont + 1, zona.nombre);

        cont++;
    }

    if (cont == 0)
    {
        printf("No hay zonas registradas.\n");
        return -1;
    }

    printf("Escoga una zona (#): ");
    opc = validarFloatRango(1, cont);

    return posiciones[opc - 1];
}