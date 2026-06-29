#define ZONAS 5
#define MAX_DIAS 30
#define CONTAMINANTES 4


typedef struct 
{
    char nombre[20];

    int dias;

    float historicos[MAX_DIAS][CONTAMINANTES];
    float promedio [CONTAMINANTES];
    float actual [CONTAMINANTES];
    float prediccion [CONTAMINANTES];

    float temperatura;
    float viento;
    float humedad;

    char estado[CONTAMINANTES][20]; 
    
}ZONA;


int menu();
float validarFloatRango(float a, float b);
void leerCadena(char *cadena, int n);
void crear();
void guardar(ZONA *zona);
void predicciones();
float calcularFactor(ZONA *zona);
float calcularPromedioPonderado(ZONA *zona, int contaminante);
void generarRecomendaciones(int n);
void listarAlertas();
void listarRecomendacion();
void editar();
void listarHistoricos();
void generarReporte();


