#ifndef WEB_SERVER_H
#define WEB_SERVER_H

// Etapa 4: WebSocket de solo lectura. Se ignora todo mensaje entrante.

void wsError(AsyncWebSocketClient *c, uint32_t id, const char *msg)
{
    char b[96];
    snprintf(b, sizeof(b), "{\"tipo\":\"err\",\"id\":%lu,\"msg\":\"%s\"}",
             (unsigned long)id, msg);
    c->text(b);
}

void wsRecibir(AsyncWebSocketClient *c, void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (!(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)) return;
    if (len == 0 || len > 160) { wsError(c, 0, "Mensaje invalido"); return; }

    char m[161];
    memcpy(m, data, len);
    m[len] = 0;

    float f;
    uint32_t id = (jsonNum(m, "id", f) && f >= 0) ? (uint32_t)f : 0;

    if (strstr(m, "\"cmd\":\"paro\""))
    {
        paroId = id;
        paroSolicitado = true;           // el loop lo atiende primero
        return;
    }

    if (strstr(m, "\"cmd\":\"params\""))
    {
        Comando k = { CMD_PARAMS, id, 0, 0, 0, 0, 0 };
        if (!jsonNum(m, "kp", k.kp) || !jsonNum(m, "ki", k.ki) ||
            !jsonNum(m, "kd", k.kd) || !jsonNum(m, "sp", k.sp))
        { wsError(c, id, "Faltan parametros"); return; }

        const char *e = validarParams(k.kp, k.ki, k.kd, k.sp);
        if (e) { wsError(c, id, e); return; }
        if (xQueueSend(colaCmd, &k, 0) != pdTRUE) wsError(c, id, "Robot ocupado");
        return;
    }

    if (strstr(m, "\"cmd\":\"motor\""))
    {
        Comando k = { CMD_MOTOR, id, 0, 0, 0, 0, 0 };
        if      (strstr(m, "\"on\":true"))  k.on = 1;
        else if (strstr(m, "\"on\":false")) k.on = 0;
        else { wsError(c, id, "Comando invalido"); return; }
        if (xQueueSend(colaCmd, &k, 0) != pdTRUE) wsError(c, id, "Robot ocupado");
        return;
    }

    wsError(c, id, "Comando desconocido");
}

void wsEvento(AsyncWebSocket *s, AsyncWebSocketClient *c, AwsEventType tipo,
              void *arg, uint8_t *data, size_t len)
{
    if (tipo == WS_EVT_CONNECT)
    {
        char b[160];
        snprintf(b, sizeof(b),
            "{\"tipo\":\"hola\",\"kp\":%.2f,\"ki\":%.2f,\"kd\":%.2f,\"sp\":%.2f}",
            equilibrioKp, equilibrioKi, equilibrioKd, equilibrioSetpoint);
        c->text(b);
    }
    else if (tipo == WS_EVT_DATA)
    {
        wsRecibir(c, arg, data, len);
    }
}

void tareaTelemetria(void *)
{
    MuestraTel m;
    Respuesta r;
    char b[240];
    uint8_t ciclo = 0;

    for (;;)
    {
        bool hay = (xQueueReceive(colaTel, &m, pdMS_TO_TICKS(100)) == pdTRUE);

        telClientes = ws.count();
        if (++ciclo >= 20) { ciclo = 0; ws.cleanupClients(2); }

        while (xQueueReceive(colaResp, &r, 0) == pdTRUE)
        {
            int k = r.ok
                ? snprintf(b, sizeof(b),
                    "{\"tipo\":\"ack\",\"id\":%lu,\"kp\":%.2f,\"ki\":%.2f,\"kd\":%.2f,\"sp\":%.2f}",
                    (unsigned long)r.id, r.kp, r.ki, r.kd, r.sp)
                : snprintf(b, sizeof(b),
                    "{\"tipo\":\"err\",\"id\":%lu,\"msg\":\"%s\"}", (unsigned long)r.id, r.msg);
            if (ws.availableForWriteAll()) ws.textAll(b, k);
        }

        if (hay && telClientes > 0 && ws.availableForWriteAll())
        {
          int n = snprintf(b, sizeof(b),
                "{\"tipo\":\"tel\",\"t\":%lu,\"ang\":%.2f,\"sp\":%.2f,\"err\":%.2f,"
                "\"om\":%.1f,\"u\":%.1f,\"pl\":%d,\"pr\":%d,\"rl\":%.0f,\"rr\":%.0f,"
                "\"hz\":%.1f,\"mo\":%d,\"ar\":%d}",
                (unsigned long)m.t_ms, m.ang, m.sp, m.err, m.om, m.u,
                m.pl, m.pr, m.rl, m.rr, m.hz, m.motor, m.arm);
            ws.textAll(b, n);
        }
    }
}

void wsBegin()
{
    ws.onEvent(wsEvento);
    xTaskCreatePinnedToCore(tareaTelemetria, "tel", 4096, nullptr, 1, nullptr, 0);
}

#endif