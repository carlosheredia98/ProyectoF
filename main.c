#include <stdio.h>
#include <string.h>
#include "funciones.h"

int main() {
    Zona zonas[ZONAS] = {0};
    Contaminantes predicciones[ZONAS];
    cargarLimites();

    char resp;
    printf("Desea cargar los datos historicos desde archivo? (s/n): ");
    scanf(" %c%*c", &resp);
    if (resp == 's' || resp == 'S') {
        cargarDatos(zonas);
        printf("Datos cargados desde archivo.\n");
    } else {
        const char *nombres[ZONAS] = {"Centro", "Norte", "Sur", "Este", "Oeste"};
        for (int i = 0; i < ZONAS; i++) {
            strncpy(zonas[i].nombre, nombres[i], sizeof(zonas[i].nombre) - 1);
            zonas[i].nombre[sizeof(zonas[i].nombre) - 1] = '\0';
        }
        printf("Se iniciara con datos vacios.\n");
    }

    int opcion;
    do {
        printf("\n===== MENU PRINCIPAL =====\n");
        printf("1. Ingresar datos actuales desde archivo\n");
        printf("2. Mostrar estado actual\n");
        printf("3. Predecir y emitir alertas\n");
        printf("4. Guardar reporte\n");
        printf("5. Cargar datos desde archivo\n");
        printf("6. Salir\n");
        printf("Seleccione una opcion: ");
        scanf("%d%*c", &opcion);

        switch (opcion) {
            case 1: {
                int tipoArchivo;
                printf("\nSeleccione el archivo de datos a cargar:\n");
                printf("1. valores_bajos.txt\n");
                printf("2. valores_medios.txt\n");
                printf("3. valores_altos.txt\n");
                printf("Seleccione una opcion: ");
                scanf("%d%*c", &tipoArchivo);

                const char *archivos[] = {"valores_bajos.txt", "valores_medios.txt", "valores_altos.txt"};

                if (tipoArchivo < 1 || tipoArchivo > 3) {
                    printf("Opcion invalida.\n");
                    break;
                }

                // Asignar límites según archivo
                switch (tipoArchivo) {
                    case 1:
                        limitesOMS[0] = 400;
                        limitesOMS[1] = 5;
                        limitesOMS[2] = 10;
                        limitesOMS[3] = 12;
                        break;
                    case 2:
                        limitesOMS[0] = 700;
                        limitesOMS[1] = 15;
                        limitesOMS[2] = 25;
                        limitesOMS[3] = 20;
                        break;
                    case 3:
                        limitesOMS[0] = 1000;
                        limitesOMS[1] = 20;
                        limitesOMS[2] = 40;
                        limitesOMS[3] = 25;
                        break;
                }

                FILE *archivo = fopen(archivos[tipoArchivo - 1], "r");
                if (!archivo) {
                    printf("No se pudo abrir el archivo %s\n", archivos[tipoArchivo - 1]);
                    break;
                }

                for (int i = 0; i < ZONAS; i++) {
                    char buffer[100];
                    fgets(buffer, sizeof(buffer), archivo); // Saltar línea encabezado
                    printf("\nDatos actuales para %s:\n", zonas[i].nombre);

                    fscanf(archivo, "  CO2: %f\n", &zonas[i].actual.CO2);
                    printf("  CO2: %.1f\n", zonas[i].actual.CO2);

                    fscanf(archivo, "  SO2: %f\n", &zonas[i].actual.SO2);
                    printf("  SO2: %.1f\n", zonas[i].actual.SO2);

                    fscanf(archivo, "  NO2: %f\n", &zonas[i].actual.NO2);
                    printf("  NO2: %.1f\n", zonas[i].actual.NO2);

                    fscanf(archivo, "  PM2.5: %f\n", &zonas[i].actual.PM25);
                    printf("  PM2.5: %.1f\n", zonas[i].actual.PM25);

                    fscanf(archivo, "  Temperatura (C): %f\n", &zonas[i].temperatura);
                    printf("  Temperatura (C): %.1f\n", zonas[i].temperatura);

                    fscanf(archivo, "  Viento (m/s): %f\n", &zonas[i].viento);
                    printf("  Viento (m/s): %.1f\n", zonas[i].viento);

                    fscanf(archivo, "  Humedad (%%): %f\n", &zonas[i].humedad);
                    printf("  Humedad (%%): %.1f\n", zonas[i].humedad);

                    fgets(buffer, sizeof(buffer), archivo); // Saltar línea en blanco
                }

                fclose(archivo);
                actualizarDatosHistoricos(zonas);
                printf("\nDatos cargados desde %s y limites ajustados automaticamente\n", archivos[tipoArchivo - 1]);
                break;
            }
            case 2:
                mostrarEstadoActual(zonas);
                break;
            case 3:
                predecirContaminacion(zonas, predicciones);
                emitirAlertas(zonas, predicciones);
                generarRecomendaciones(zonas, predicciones);
                break;
            case 4:
                guardarReporte(zonas);
                break;
            case 5:
                cargarDatos(zonas);
                printf("Datos recargados desde archivo.\n");
                break;
            case 6:
                printf("Saliendo del programa...\n");
                break;
            default:
                printf("Opcion no valida.\n");
        }
    } while (opcion != 6);

    return 0;
}
