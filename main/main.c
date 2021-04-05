/* 
    This is sample code for SHT20 testing.
*/
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "lib/SHT20.h"

#define SHT20_SDA GPIO_NUM_21
#define SHT20_SCL GPIO_NUM_22

void app_main(void)
{
    esp_err_t err = SHT20_init(SHT20_SDA, SHT20_SCL);
    if(err != ESP_OK){
        printf("SHT20 init failure!\n");
        return;
    }

    while(1) {
        printf("RH = %.2f(%%), T = %.2f(DegC)\n", read_humidity(), read_temperature());
        vTaskDelay(1000/ portTICK_RATE_MS);
    }
}