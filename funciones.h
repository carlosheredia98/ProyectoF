#ifndef FUNCIONES_H
#define FUNCIONES_H

#define ZONAS 5
#define DIAS_HISTORICOS 30
#define CONTAMINANTES 4

typedef struct {
    float CO2;
    float SO2;
    float NO2;
    float PM25;
} Contaminantes;

typedef struct {
    char nombre[50];
    Contaminantes historico[DIAS_HISTORICOS];
    Contaminantes actual;
    float temperatura;
    float viento;
    float humedad;
} Zona;

extern float limitesOMS[CONTAMINANTES];

// Declaración de funciones
void cargarLimites();
void cargarDatos(Zona zonas[]);
void actualizarDatosHistoricos(Zona zonas[]);
void guardarReporte(Zona zonas[]);
void mostrarEstadoActual(Zona zonas[]);
void predecirContaminacion(Zona zonas[], Contaminantes predicciones[]);
void emitirAlertas(Zona zonas[], Contaminantes predicciones[]);
void generarRecomendaciones(Zona zonas[], Contaminantes predicciones[]);
Contaminantes calcularPromedio(Zona zona);
Contaminantes promedioPonderado(Zona zona);
int leerFloatSegura(const char *mensaje, float *valor);
const char* nivelContaminacion(float valor, float limite);

#endif
