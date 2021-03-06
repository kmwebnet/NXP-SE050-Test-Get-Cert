#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"


#include "sdkconfig.h" // generated by "make menuconfig"

#include "mbedtls/esp_config.h"

#define SDA2_PIN GPIO_NUM_18
#define SCL2_PIN GPIO_NUM_19

#define TAG "maintask"

#define I2C_MASTER_ACK 0
#define I2C_MASTER_NACK 1


void i2c_master_init()
{
	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = SDA2_PIN,
		.scl_io_num = SCL2_PIN,
		.sda_pullup_en = GPIO_PULLUP_DISABLE,
		.scl_pullup_en = GPIO_PULLUP_DISABLE,
		.master.clk_speed = 100000
		};
			
	i2c_param_config(I2C_NUM_0 , &i2c_config);
	i2c_driver_install(I2C_NUM_0 , I2C_MODE_MASTER, 0, 0, 0);


}
/* clang-format off */

#if defined(SSS_USE_FTR_FILE)
#include "fsl_sss_ftr.h"
#else
#include "fsl_sss_ftr_default.h"
#endif


#define mbedtls_time time
#define mbedtls_time_t time_t
#define mbedtls_printf printf
#define mbedtls_fprintf fprintf
#define mbedtls_snprintf snprintf

#include <assert.h>

#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "mbedtls/timing.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* +S */

#ifdef TGT_A71CH
#   include "sm_printf.h"
#endif

#if SSS_HAVE_ALT_SSS
#include "sss_mbedtls.h"
#endif

#include <ex_sss.h>
#include <ex_sss_boot.h>
#include <nxLog_App.h>
#include <se05x_APDU.h>

static ex_sss_boot_ctx_t gex_sss_demo_boot_ctx;
ex_sss_boot_ctx_t *pex_sss_demo_boot_ctx = &gex_sss_demo_boot_ctx;
static ex_sss_cloud_ctx_t gex_sss_demo_tls_ctx;
ex_sss_cloud_ctx_t *pex_sss_demo_tls_ctx = &gex_sss_demo_tls_ctx;

#define EX_SSS_BOOT_PCONTEXT (&gex_sss_demo_boot_ctx)

#define EX_SSS_BOOT_DO_ERASE 0
#define EX_SSS_BOOT_EXPOSE_ARGC_ARGV 1

#include <ex_sss_main_inc.h>

/*The size of the client certificate should be checked when script is used to store it in GP storage and updated here */
#define SIZE_CLIENT_CERTIFICATE 2048

#include "ex_sss_objid.h"

sss_status_t base64encode(
    const uint8_t*  data,         /**< [in] The input byte array that will be converted to base 64 encoded characters */
    size_t          data_size,    /**< [in] The length of the byte array */
    char*           encoded,      /**< [in] The output converted to base 64 encoded characters. */
    size_t*         encoded_size /**< [inout] Input: The size of the encoded buffer, Output: The length of the encoded base 64 character string */
    );



sss_status_t ex_sss_entry(ex_sss_boot_ctx_t *pCtx)
{

    int ret = 0;
    sss_status_t ret_code;
    uint8_t aclient_cer[SIZE_CLIENT_CERTIFICATE] = {0};

    mbedtls_x509_crt cacert;
    mbedtls_x509_crt clicert;
    mbedtls_x509_crt_init( &cacert );
    mbedtls_x509_crt_init( &clicert );

    uint8_t buf[4000];
    size_t buf_len = sizeof(buf);


        sss_status_t status;

    sss_se05x_session_t *pSession = (sss_se05x_session_t *)&pCtx->session;

        /* doc+:initialize-key-objs */

        /* pex_sss_demo_tls_ctx->obj will have the private key handle */
        status = sss_key_object_init(&pex_sss_demo_tls_ctx->obj, &pCtx->ks);
        if (status != kStatus_SSS_Success) {
            printf(" sss_key_object_init for keyPair Failed...\n");
            return kStatus_SSS_Fail;
        }

        status = sss_key_object_get_handle(
            &pex_sss_demo_tls_ctx->obj, EX_SSS_OBJID_TP_CERT_EC_D);
        if (status != kStatus_SSS_Success) {
            printf(" sss_key_object_get_handle  for keyPair Failed...\n");
            return kStatus_SSS_Fail;
        }

            /* doc+:load-certificate-from-se */
            size_t KeyBitLen = SIZE_CLIENT_CERTIFICATE * 8;
            size_t KeyByteLen = SIZE_CLIENT_CERTIFICATE;

            ret_code = sss_key_store_get_key(
                &pCtx->ks, &pex_sss_demo_tls_ctx->obj, aclient_cer, &KeyByteLen, &KeyBitLen);

            ret = mbedtls_x509_crt_parse_der(&clicert,
                (const unsigned char *)aclient_cer,
                sizeof(aclient_cer));
            if ((ret_code == kStatus_SSS_Success) && (ret == 0)) {
            }
            /* doc-:load-certificate-from-se */

    /* Convert to base 64 */
    base64encode(aclient_cer, (aclient_cer[2] * 256 + aclient_cer[3] + 4), (char *)buf, &buf_len);

    /* Add a null terminator */
    buf[buf_len] = 0;

    printf("cert object ID: %08x\n", EX_SSS_OBJID_TP_CERT_EC_D);

    /* Print out the key */
    printf("-----BEGIN CERTIFICATE-----\n%s\n-----END CERTIFICATE-----\n", buf);


    if (ret < 0)
        ret = kStatus_SSS_Fail;

    smStatus_t sw_status;
    SE05x_Result_t result = kSE05x_Result_NA;

    sw_status = Se05x_API_CheckObjectExists(
        &pSession->s_ctx, kSE05x_AppletResID_UNIQUE_ID, &result);
    if (SM_OK != sw_status) {
        LOG_E("Failed Se05x_API_CheckObjectExists");
        return kStatus_SSS_Fail;
    }
    uint8_t uid[SE050_MODULE_UNIQUE_ID_LEN];
    size_t uidLen = sizeof(uid);
    sw_status = Se05x_API_ReadObject(&pSession->s_ctx,
        kSE05x_AppletResID_UNIQUE_ID,
        0,
        (uint16_t)uidLen,
        uid,
        &uidLen);
    if (SM_OK != sw_status) {
        LOG_E("Failed Se05x_API_CheckObjectExists");
        return kStatus_SSS_Fail;
    }
    status = kStatus_SSS_Success;

    buf_len = sizeof(buf);
    memset (buf ,0 , buf_len);

    /* Convert to base 64 */
    base64encode(uid, uidLen, (char *)buf, &buf_len);

    /* Add a null terminator */
    buf[buf_len] = 0;

    /* Print out the pubkey */
    printf("-----BEGIN UID-----\n%s\n-----END UID-----\n", buf);

    return kStatus_SSS_Success;
}



void app_main(void)
{
	i2c_master_init();
    ESP_ERROR_CHECK( nvs_flash_init() );    
    main( 1 ,NULL);
}