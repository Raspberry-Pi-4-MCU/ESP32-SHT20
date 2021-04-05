#include <stdio.h>

#include "SHT20.h"

#define I2C_PORT    I2C_NUM_0
#define I2C_WRITE_MODE      0
#define I2C_READ_MODE       1
#define I2C_MASTER_TX_BUF_DISABLE   0   /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0   /*!< I2C master doesn't need buffer */
#define ACK     0                       /*!< I2C ack value */
#define NACK    1                       /*!< I2C nack value */
#define ACK_CHECK_ENABLE    1
#define ACK_CHECK_DISABLE   0

#define SHT20_ADDRESS       0x40
#define SHT20_HUMIDITY_WAIT_TICK    20
#define SHT20_TEMPERATURE_WAIT_TICK 40

#define SHT20_T_HOLD        0xE3
#define SHT20_RH_HOLD       0xE5
#define SHT20_T_NO_HOLD     0xF3
#define SHT20_RH_NO_HOLD    0xF5
#define SHT20_SOFT_RESET    0xFE

esp_err_t SHT20_init(int SDA, int SCL){
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA,              /* select GPIO specific to your project */
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = SCL,              /* select GPIO specific to your project */
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,     /* select frequency specific to your project */
        // .clk_flags = 0,              /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };

    esp_err_t err = i2c_param_config(I2C_PORT, &conf);
    if(err != ESP_OK){
        return err;
    }

    return i2c_driver_install(I2C_PORT, conf.mode, I2C_MASTER_RX_BUF_DISABLE ,I2C_MASTER_TX_BUF_DISABLE, 0);
}

float read_humidity(void)
{
    uint8_t humidity[3] = {0};
    uint8_t *ptr = humidity;
    i2c_cmd_handle_t cmd;
    uint16_t result = 0;

    /* I2C Write */
    cmd = i2c_cmd_link_create();                                                        /* Create an I2C command */
    i2c_master_start(cmd);                                                              /* Add I2C start bit to the command */
    i2c_master_write_byte(cmd, SHT20_ADDRESS << 1 | I2C_WRITE_MODE, ACK_CHECK_ENABLE);  /* Add the SHT20 address and WRITE mode to the command, and then check the ACK */
    i2c_master_write_byte(cmd, SHT20_RH_NO_HOLD, ACK_CHECK_ENABLE);                     /* Add a SHT20 RH_NO_HOLD command and ACK to the command */
    i2c_master_stop(cmd);                                                               /* Add I2C stop bit to the command */
    i2c_master_cmd_begin(I2C_PORT, cmd, 1 / portTICK_RATE_MS);                          /* Send command and wait 1 ms */
    i2c_cmd_link_delete(cmd);                                                           /* Free I2C command */

    /* Measuring and I2C Read */
    vTaskDelay(SHT20_HUMIDITY_WAIT_TICK / portTICK_RATE_MS);
    cmd = i2c_cmd_link_create();                                                        /* Create an I2C command */
    i2c_master_start(cmd);                                                              /* Add I2C start bit to the command */
    i2c_master_write_byte(cmd, SHT20_ADDRESS << 1 | I2C_READ_MODE, ACK_CHECK_DISABLE);  /* Add the SHT20 address and READ mode to the command, and then do not check the ACK */
    i2c_master_stop(cmd);                                                               /* Add I2C stop bit to the command */
    i2c_master_cmd_begin(I2C_PORT, cmd, 0);                                             /* Send command */
    i2c_cmd_link_delete(cmd);                                                           /* Free I2C command */

    /* Continue measuring and I2C read  */
    vTaskDelay(SHT20_HUMIDITY_WAIT_TICK / portTICK_RATE_MS);
    cmd = i2c_cmd_link_create();                                                        /* Create an I2C command */
    i2c_master_start(cmd);                                                              /* Add an I2C start bit into command */
    i2c_master_write_byte(cmd, SHT20_ADDRESS << 1 | I2C_READ_MODE, ACK_CHECK_ENABLE);   /* Add the SHT20 address and READ mode to the command, and then do not check the ACK */
    i2c_master_read_byte(cmd, ptr, ACK);                                                /* MSB from 15 to 8 bit */
    i2c_master_read_byte(cmd, ptr+1, ACK);                                              /* LSB from 7 to 2 bit. Status is bit 1 and 0 */
    i2c_master_read_byte(cmd, ptr+2, NACK);                                             /* Checksum */
    i2c_master_stop(cmd);                                                               /* Add I2C stop bit to the command */
    i2c_master_cmd_begin(I2C_PORT, cmd, 0);                                             /* Send command */
    i2c_cmd_link_delete(cmd);                                                           /* Free I2C command */

    result = ((uint16_t)humidity[0] << 8) + ((uint16_t)(humidity[1] & 0xFC));

    /* Check the measurement type by bit 1 (temperature=0, humidity=1)*/
    if(((humidity[1]>>1) & 0x01) != 1){
        return 0;
    }        

    return (125.0*(result/65536.0)-6.0);
}

float read_temperature(void)
{
    uint8_t temperature[3] = {0};
    uint8_t *ptr = temperature;
    i2c_cmd_handle_t cmd;
    uint16_t result = 0;

    /* I2C Write */
    cmd = i2c_cmd_link_create();                                                        /* Create an I2C command */
    i2c_master_start(cmd);                                                              /* Add I2C start bit to the command */
    i2c_master_write_byte(cmd, SHT20_ADDRESS << 1 | I2C_WRITE_MODE, ACK_CHECK_ENABLE);  /* Add the SHT20 address and WRITE mode to the command, and then check the ACK */
    i2c_master_write_byte(cmd, SHT20_T_NO_HOLD, ACK_CHECK_ENABLE);                      /* Add a SHT20 RH_NO_HOLD command and ACK to the command */
    i2c_master_stop(cmd);                                                               /* Add I2C stop bit to the command */
    i2c_master_cmd_begin(I2C_PORT, cmd, 1 / portTICK_RATE_MS);                          /* Send command and wait 1 ms */
    i2c_cmd_link_delete(cmd);                                                           /* Free I2C command */

    /* Measuring and I2C Read */
    vTaskDelay(SHT20_TEMPERATURE_WAIT_TICK / portTICK_RATE_MS);
    cmd = i2c_cmd_link_create();                                                        /* Create an I2C command */
    i2c_master_start(cmd);                                                              /* Add I2C start bit to the command */
    i2c_master_write_byte(cmd, SHT20_ADDRESS << 1 | I2C_READ_MODE, ACK_CHECK_DISABLE);  /* Add the SHT20 address and READ mode to the command, and then do not check the ACK */
    i2c_master_stop(cmd);                                                               /* Add I2C stop bit to the command */
    i2c_master_cmd_begin(I2C_PORT, cmd, 0);                                             /* Send command */
    i2c_cmd_link_delete(cmd);                                                           /* Free I2C command */

    /* Continue measuring and I2C read  */
    vTaskDelay(SHT20_TEMPERATURE_WAIT_TICK / portTICK_RATE_MS);
    cmd = i2c_cmd_link_create();                                                        /* Create an I2C command */
    i2c_master_start(cmd);                                                              /* Add an I2C start bit into command */
    i2c_master_write_byte(cmd, SHT20_ADDRESS << 1 | I2C_READ_MODE, ACK_CHECK_ENABLE);   /* Add the SHT20 address and READ mode to the command, and then do not check the ACK */
    i2c_master_read_byte(cmd, ptr, ACK);                                                /* MSB from 15 to 8 bit */
    i2c_master_read_byte(cmd, ptr+1, ACK);                                              /* LSB from 7 to 2 bit. Status is bit 1 and 0 */
    i2c_master_read_byte(cmd, ptr+2, NACK);                                             /* Checksum */
    i2c_master_stop(cmd);                                                               /* Add I2C stop bit to the command */
    i2c_master_cmd_begin(I2C_PORT, cmd, 0);                                             /* Send command */
    i2c_cmd_link_delete(cmd);                                                           /* Free I2C command */

    result = ((uint16_t)temperature[0] << 8) + ((uint16_t)(temperature[1] & 0xFC));

    /* Check the measurement type by bit 1 (temperature=0, humidity=1)*/
    if(((temperature[1]>>1) & 0x00) != 0){
        return 0;
    }
    return (175.72*(result/65536.0)-46.85);
}