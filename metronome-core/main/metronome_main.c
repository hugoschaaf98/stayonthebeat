// /* standart */
#include <stdio.h>
// #include <string.h>
#include <assert.h>

/* FreeRTOs */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// /* network */
// #include "esp_netif.h"
// #include "esp_event.h"
// #include "mdns.h" /* local dns */


/* general */
#include "nvs_flash.h"
#include "esp_log.h"
#include "driver/timer.h"

/* metronome */
#include "m_init.h"
#include "m_http_server.h"
#include "m_beat.h"

#define TIMEOUT_TICKS_VALUE pdMS_TO_TICKS(1000) /* timeout queue receive after 1 second */
#define MAX_TIMEOUT_COUNT 10    /* allow to timeout 10 times consecutively */


static const char* TAG = "metronome_main";

static uint8_t sound_buf[1024];

void metronome_init(BeatMachine_t* bm);
void metronome_run(BeatMachine_t* bm);
void metronome_quit(BeatMachine_t* bm);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* MAIN                                                                    */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void app_main(void)
{
    BeatMachine_t metronome;
    while(1){
        metronome_init(&metronome);
        ESP_LOGV(TAG, "<metronome_run()> metronome main loop...");
        metronome_run(&metronome);
        ESP_LOGV(TAG, "<metronome_quit()> metronome shutdown...");
        metronome_quit(&metronome);
    }
}


void metronome_init(BeatMachine_t* bm)
{
    ESP_LOGV(TAG, "<%s> : metronome initialization...", __ASSERT_FUNC);

    /* Initialize NVS */
    ESP_LOGI(TAG, "NVS initialization...");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* FILE SYSTEM */
    ESP_LOGI(TAG, "File System initialization...");
    ESP_ERROR_CHECK(fs_init());

    /* WIFI */
    /*ESP_LOGV(TAG, "Wifi initialization...");
    ESP_ERROR_CHECK(wifi_ap_init());


    ESP_LOGV(TAG, "Starting http server...");
    ESP_ERROR_CHECK(http_server_start(CONFIG_M_WEB_MOUNT_POINT));
    */

    ESP_LOGI(TAG, "Initializing beat machine...");
    ESP_ERROR_CHECK(beat_init(bm));
    ESP_ERROR_CHECK(beat_register_timer(bm, TIMER_GROUP_0, TIMER_1, true));
    ESP_ERROR_CHECK(beat_register_sound(bm, sound_buf, sizeof(sound_buf)*sizeof(uint8_t)));
    ESP_ERROR_CHECK(beat_set_bpm(bm,50));

}

void metronome_run(BeatMachine_t* bm)
{
    ESP_LOGV(TAG, "<%s> : metronome running...", __ASSERT_FUNC);

    bpm_t bpm = bm->bpm;
    uint8_t timeout_count = 0;
    BaseType_t res;
    char cmd;
    const char help_msg[]="List of available commands :\ns : start the metronome\n \
                                        p : pause the metronome\n + : increment bpm by 1\n \
                                        - : decrement bpm by 1\n h : for help";
    
    /* run the metronome main code */
    puts(help_msg);

    beat_start(bm);

    while(1)
    {
        /* queue les data du serveur ou alors de la console
        ou du bluetooth ou jsais pas d'ou encore */

        if((cmd = getchar()))
        //if(xQueueReceive(/*machin bidule et timeout*/) == pdPASS)
        {
            /* check si on revient d'un ou plusieurs timeout reset le count*/
            if(timeout_count) timeout_count = 0;

            /* si pas de timeout */
            if(cmd == 's') /* si il faut demarrer le metronome */
            {
                ESP_LOGI(TAG, "metronome started !");
                beat_start(bm);
            }
            else if(cmd == 'p') /* ou le stopper */
            {
                ESP_LOGI(TAG, "metronome stopped !");
                beat_stop(bm);
            }
            if(cmd == '+') /* ou changer le tempo à la fois */
            {
                beat_set_bpm(bm, bpm++);
                ESP_LOGI(TAG, "metronome bpm updated ! bpm : %d", bm->bpm);
            }
            if(cmd == '-') /* ou changer le tempo à la fois */
            {
                beat_set_bpm(bm, bpm--);
                ESP_LOGI(TAG, "metronome bpm updated ! bpm : %d", bm->bpm);
            }

            if(cmd == 'h') /* ou changer le tempo à la fois */
            {
                puts(help_msg);
            }

        }
        else if(timeout_count++ >= MAX_TIMEOUT_COUNT)
        {
            ESP_LOGE(TAG, "Reached maximum timeout to attempt data from user !");
        }
        vTaskDelay(10);
    }
}

void metronome_quit(BeatMachine_t* bm)
{
    ESP_LOGV(TAG, "<%s> : stopping metronome...", __ASSERT_FUNC);
    /* free all resources and shutdown */
}