#include <stdio.h>
#include <string.h>
#include <time.h>
#include "funciones.h"

float limitesOMS[CONTAMINANTES];

const char* nivelContaminacion(float valor, float limite) {
    if (valor > 1.5 * limite) return "Alta";
    else if (valor > limite) return "Moderada";
    else return "Buena";
}

void cargarLimites() {
    FILE *archivo = fopen("config.txt", "r");
    if (!archivo) {
        float valoresDefault[CONTAMINANTES] = {1000, 20, 40, 25};
        for (int i = 0; i < CONTAMINANTES; i++)
            limitesOMS[i] = valoresDefault[i];
        return;
    }
    for (int i = 0; i < CONTAMINANTES; i++) {
        fscanf(archivo, "%f", &limitesOMS[i]);
    }
    fclose(archivo);
}

void cargarDatos(Zona zonas[]) {
    // Abre el archivo de datos históricos para lectura
    FILE *archivo = fopen("datos.txt", "r");
    if (!archivo) {
        // Si no existe el archivo, inicializa los valores vacíos
        printf("No se encontro datos.txt. Se iniciaran valores vacios.\n");
        return;
    }

    char linea[256]; // Buffer para leer cada línea del archivo
    int zonaIndex = -1; // Índice de la zona actual
    int diaIndex = 0;   // Índice del día histórico actual

    // Lee el archivo línea por línea
    while (fgets(linea, sizeof(linea), archivo)) {
        // Ignora líneas de comentarios o vacías
        if (linea[0] == '#' || linea[0] == '\n') continue;

        // Si la línea contiene solo el nombre de la zona, actualiza zonaIndex
        if (sscanf(linea, "%49s", zonas[zonaIndex + 1].nombre) == 1 &&
            strchr(linea, ' ') == NULL && strchr(linea, '\t') == NULL) {
            zonaIndex++;
            diaIndex = 0; // Reinicia el contador de días para la nueva zona
            if (zonaIndex >= ZONAS) break; // Si ya se leyeron todas las zonas, termina
            continue;
        }

        // Si la línea contiene datos, los asigna al histórico de la zona
        if (zonaIndex >= 0 && zonaIndex < ZONAS && diaIndex < DIAS_HISTORICOS) {
            if (sscanf(linea, "%f %f %f %f %f %f %f",
                       &zonas[zonaIndex].historico[diaIndex].CO2, // CO2
                       &zonas[zonaIndex].historico[diaIndex].SO2, // SO2
                       &zonas[zonaIndex].historico[diaIndex].NO2, // NO2
                       &zonas[zonaIndex].historico[diaIndex].PM25, // PM2.5
                       &zonas[zonaIndex].temperatura, // Temperatura
                       &zonas[zonaIndex].viento,      // Viento
                       &zonas[zonaIndex].humedad) == 7) { // Humedad
                diaIndex++; // Avanza al siguiente día histórico
            }
        }
    }

    // Cierra el archivo
    fclose(archivo);
}

void actualizarDatosHistoricos(Zona zonas[]) {
    for (int i = 0; i < ZONAS; i++) {
        for (int j = 0; j < DIAS_HISTORICOS - 1; j++) {
            zonas[i].historico[j] = zonas[i].historico[j + 1];
        }
        zonas[i].historico[DIAS_HISTORICOS - 1] = zonas[i].actual;
    }

    FILE *archivo = fopen("datos.txt", "w");
    fprintf(archivo, "# CO2 SO2 NO2 PM2.5 Temp Viento Humedad\n");

    for (int i = 0; i < ZONAS; i++) {
        fprintf(archivo, "%s\n", zonas[i].nombre);
        for (int j = 0; j < DIAS_HISTORICOS; j++) {
            fprintf(archivo, "%.2f %.2f %.2f %.2f %.2f %.2f %.2f\n",
                    zonas[i].historico[j].CO2,
                    zonas[i].historico[j].SO2,
                    zonas[i].historico[j].NO2,
                    zonas[i].historico[j].PM25,
                    zonas[i].temperatura,
                    zonas[i].viento,
                    zonas[i].humedad);
        }
    }

    fclose(archivo);
    printf("Archivo datos.txt actualizado correctamente.\n");
}

int leerFloatSegura(const char *mensaje, float *valor) {
    char buffer[100];
    int leidos;
    do {
        printf("%s", mensaje);
        if (!fgets(buffer, sizeof(buffer), stdin)) continue;
        leidos = sscanf(buffer, "%f", valor);
        if (leidos != 1)
            printf("Entrada invalida. Intente de nuevo.\n");
    } while (leidos != 1);
    return 1;
}

void guardarReporte(Zona zonas[]) {
    FILE *archivo = fopen("reporte.txt", "a");
    if (!archivo) {
        printf("Error al abrir reporte.txt\n");
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(archivo, "\n==============================\n");
    fprintf(archivo, "Reporte generado el %04d-%02d-%02d %02d:%02d\n",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min);
    fprintf(archivo, "==============================\n");

    for (int i = 0; i < ZONAS; i++) {
        fprintf(archivo, "Zona: %s\n", zonas[i].nombre);
        fprintf(archivo, "  CO2=%.2f ppm SO2=%.2f ug/m3 NO2=%.2f ug/m3 PM2.5=%.2f ug/m3\n",
                zonas[i].actual.CO2, zonas[i].actual.SO2,
                zonas[i].actual.NO2, zonas[i].actual.PM25);
        fprintf(archivo, "  Temp=%.1f C Viento=%.1f m/s Humedad=%.1f %%\n",
                zonas[i].temperatura, zonas[i].viento, zonas[i].humedad);
        Contaminantes prom = calcularPromedio(zonas[i]);
        fprintf(archivo, "  Promedio 30 dias: CO2=%.2f SO2=%.2f NO2=%.2f PM2.5=%.2f\n\n",
                prom.CO2, prom.SO2, prom.NO2, prom.PM25);
    }

    fclose(archivo);
    printf("Reporte guardado en reporte.txt\n");
}

void mostrarEstadoActual(Zona zonas[]) {
    printf("\nZona         | CO2           | SO2           | NO2           | PM2.5         | Temp | Viento | Humedad\n");
    for (int i = 0; i < ZONAS; i++) {
        Contaminantes c = zonas[i].actual;
        if (c.CO2 == 0 && c.SO2 == 0 && c.NO2 == 0 && c.PM25 == 0) {
            c = zonas[i].historico[DIAS_HISTORICOS - 1];
            printf("Mostrando datos anteriores para %s\n", zonas[i].nombre);
        }

        printf("%-12s | %.2f (%s) | %.2f (%s) | %.2f (%s) | %.2f (%s) | %.1f | %.1f | %.1f\n",
               zonas[i].nombre,
               c.CO2, nivelContaminacion(c.CO2, limitesOMS[0]),
               c.SO2, nivelContaminacion(c.SO2, limitesOMS[1]),
               c.NO2, nivelContaminacion(c.NO2, limitesOMS[2]),
               c.PM25, nivelContaminacion(c.PM25, limitesOMS[3]),
               zonas[i].temperatura,
               zonas[i].viento,
               zonas[i].humedad);
    }
}

Contaminantes calcularPromedio(Zona zona) {
    Contaminantes p = {0};
    for (int i = 0; i < DIAS_HISTORICOS; i++) {
        p.CO2 += zona.historico[i].CO2;
        p.SO2 += zona.historico[i].SO2;
        p.NO2 += zona.historico[i].NO2;
        p.PM25 += zona.historico[i].PM25;
    }
    p.CO2 /= DIAS_HISTORICOS;
    p.SO2 /= DIAS_HISTORICOS;
    p.NO2 /= DIAS_HISTORICOS;
    p.PM25 /= DIAS_HISTORICOS;
    return p;
}

Contaminantes promedioPonderado(Zona zona) {
    Contaminantes p = {0};
    float pesos[] = {5, 4, 3, 2, 1};
    float total = 15.0;
    for (int i = 0; i < 5; i++) {
        int idx = DIAS_HISTORICOS - 1 - i;
        p.CO2 += zona.historico[idx].CO2 * pesos[i];
        p.SO2 += zona.historico[idx].SO2 * pesos[i];
        p.NO2 += zona.historico[idx].NO2 * pesos[i];
        p.PM25 += zona.historico[idx].PM25 * pesos[i];
    }
    p.CO2 /= total;
    p.SO2 /= total;
    p.NO2 /= total;
    p.PM25 /= total;

    // Ajuste por variables climáticas
    float vientoFactor = (zona.viento > 10.0) ? 0.85 : (zona.viento < 3.0 ? 1.10 : 1.0); // Viento alto reduce, bajo aumenta
    float humedadFactor = (zona.humedad > 60.0) ? 1.15 : (zona.humedad < 30.0 ? 0.95 : 1.0); // Humedad alta aumenta
    float tempFactor = (zona.temperatura > 30.0) ? 1.10 : (zona.temperatura < 10.0 ? 0.95 : 1.0); // Temp alta aumenta

    p.CO2 = p.CO2 * vientoFactor * humedadFactor * tempFactor;
    p.SO2 = p.SO2 * vientoFactor * humedadFactor * tempFactor;
    p.NO2 = p.NO2 * vientoFactor * humedadFactor * tempFactor;
    p.PM25 = p.PM25 * vientoFactor * humedadFactor * tempFactor;

    return p;
}

void predecirContaminacion(Zona zonas[], Contaminantes predicciones[]) {
    for (int i = 0; i < ZONAS; i++) {
        predicciones[i] = promedioPonderado(zonas[i]);
    }
}

void emitirAlertas(Zona zonas[], Contaminantes predicciones[]) {
    printf("\nPrediccion para manana:\n");
    printf("Zona         | CO2   | SO2   | NO2   | PM2.5\n");

    for (int i = 0; i < ZONAS; i++) {
        Contaminantes p = predicciones[i];
        printf("----------------------------------------------\n");
        printf("%-12s | %.2f | %.2f | %.2f | %.2f\n",
               zonas[i].nombre, p.CO2, p.SO2, p.NO2, p.PM25);
        if (p.CO2 > limitesOMS[0]) {
            printf("ALERTA: %s supera el limite de CO2 (%.2f ppm)\n", zonas[i].nombre, limitesOMS[0]);
        }
        if (p.SO2 > limitesOMS[1]) {
            printf("ALERTA: %s supera el limite de SO2 (%.2f ug/m3)\n", zonas[i].nombre, limitesOMS[1]);
        }
        if (p.NO2 > limitesOMS[2]) {
            printf("ALERTA: %s supera el limite de NO2 (%.2f ug/m3)\n", zonas[i].nombre, limitesOMS[2]);
        }
        if (p.PM25 > limitesOMS[3]) {
            printf("ALERTA: %s supera el limite de PM2.5 (%.2f ug/m3)\n", zonas[i].nombre, limitesOMS[3]);
        }
        if (p.CO2 > limitesOMS[0] || p.SO2 > limitesOMS[1] || p.NO2 > limitesOMS[2] || p.PM25 > limitesOMS[3]) {
            printf("Recomendacion: Evitar actividades al aire libre en %s.\n", zonas[i].nombre);
        } else {
            printf("Estado normal en %s. No se requieren acciones.\n", zonas[i].nombre);
        }
    }
}

void generarRecomendaciones(Zona zonas[], Contaminantes predicciones[]) {
    printf("\nRecomendaciones basadas en la prediccion:\n");
    for (int i = 0; i < ZONAS; i++) {
        Contaminantes p = predicciones[i];
        int huboRecomendacion = 0;

        printf("------------------------------\n");
        printf("Zona: %s\n", zonas[i].nombre);

        if (p.CO2 > limitesOMS[0]) {
            printf("  - CO2 alto: Reducir el uso de vehículos motorizados.\n");
            huboRecomendacion = 1;
        }
        if (p.SO2 > limitesOMS[1]) {
            printf("  - SO2 alto: Controlar emisiones industriales y usar filtros.\n");
            huboRecomendacion = 1;
        }
        if (p.NO2 > limitesOMS[2]) {
            printf("  - NO2 alto: Limitar el uso de combustibles fósiles y calefacción a gas.\n");
            huboRecomendacion = 1;
        }
        if (p.PM25 > limitesOMS[3]) {
            printf("  - PM2.5 alto: Evitar quemas al aire libre y controlar polvo en obras.\n");
            huboRecomendacion = 1;
        }

        if (!huboRecomendacion) {
            printf("  - Estado normal: No se requieren recomendaciones especiales.\n");
        }
    }
}

void ingresarDatosManual(Zona zonas[]) {
    printf("\nIngreso manual de datos actuales por zona:\n");
    for (int i = 0; i < ZONAS; i++) {
        printf("\nZona: %s\n", zonas[i].nombre);
        leerFloatSegura("  CO2: ", &zonas[i].actual.CO2);
        leerFloatSegura("  SO2: ", &zonas[i].actual.SO2);
        leerFloatSegura("  NO2: ", &zonas[i].actual.NO2);
        leerFloatSegura("  PM2.5: ", &zonas[i].actual.PM25);
        leerFloatSegura("  Temperatura (C): ", &zonas[i].temperatura);
        leerFloatSegura("  Viento (m/s): ", &zonas[i].viento);
        leerFloatSegura("  Humedad (%%): ", &zonas[i].humedad);
    }
    actualizarDatosHistoricos(zonas);
    printf("\nDatos ingresados manualmente y actualizados en datos.txt\n");
}
