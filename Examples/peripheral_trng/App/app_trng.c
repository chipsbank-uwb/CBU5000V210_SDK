/**
 * @file app_trng.c
 * @brief Implementation file for example usages of the Trng driver.
 * @details 3 examples:
 *          Get an RN with hardware entropy (Noise)
 *          Get an RN with user-provided seed (Nonce)
 *          Run healthcheck on hardware entropy source (KAT)
 * @author Chipsbank
 * @date 2024
 */
//-------------------------------
// INCLUDE
//-------------------------------
#include "app_trng.h"
#include "CB_trng.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_CPU_TRNG_UARTPRINT_ENABLE APP_TRUE

#if (APP_CPU_TRNG_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_trng_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_trng_print(...)
#endif
//-------------------------------
// DEFINE
//-------------------------------
//-------------------------------
// ENUM
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Generate an RN using noise-seeded TRNG core.
 * @details DRBG runs on AES-256, with no additional input and with prediction resistance disabled.
 */
void app_trng_gen_with_noise(void) 
{
    uint32_t rn = 0;
    uint32_t rn_arr[4] = {0};
    uint32_t rn_arr2[8] = {0};
    enTrngErrCode err = EN_CB_TRNG_OK;

    app_trng_print("noise-seeded:\n");

    stTrngConfig config = 
    {
        .Alg = EN_CB_TRNG_AES256,
        .UseAddIn = APP_FALSE,
    };

    CB_TRNG_Init();

    /********************************************
     * Seeding: Expensive, do it once at startup
     ********************************************/
    CB_TRNG_Snoise(&config);

    // Generate 1 4-byte RN
    err = CB_TRNG_GetRng(&rn, 1);
    if (err != EN_CB_TRNG_OK) 
    {
        app_trng_print("[app_trng] Request error: %d\n", err);
    }
    app_trng_print("[app_trng] Generated: %x\n", rn);

    // Generate 4-byte RN array
    err = CB_TRNG_GetRng(rn_arr, sizeof(rn_arr) / sizeof(rn_arr[0]));
    if (err != EN_CB_TRNG_OK) 
    {
        app_trng_print("[app_trng] Request error: %d\n", err);
    }

    app_trng_print("[app_trng] Generated: ");
    for (uint32_t i = 0; i < (sizeof(rn_arr) / sizeof(rn_arr[0])); ++i) 
    {
        app_trng_print("%x ", rn_arr[i]);
    }
    app_trng_print("\n");

    // Generate 8-byte RN array
    err = CB_TRNG_GetRng(rn_arr2, 8);

    if (err != EN_CB_TRNG_OK) 
    {
        app_trng_print("[app_trng] Request error: %d\n", err);
    }

    app_trng_print("[app_trng] Generated: ");
    for (uint32_t i = 0; i < 8; ++i) 
    {
        app_trng_print("%x ", rn_arr2[i]);
    }
    app_trng_print("\n\n");

    CB_TRNG_Deinit();
}


/**
 * @brief Generate an RN using nonce-seeded TRNG core.
 * @details config.Alg can be changed to AES128 to change the algorithm chosen. Fixed seed gives fixed rn output stream.
 */
void app_trng_gen_with_nonce(void) 
{
    stTrngConfig config = 
    {
        .Alg = EN_CB_TRNG_AES256,
        .UseAddIn = APP_FALSE,
    };

    // Fixed seed values for nonce-seeded TRNG core initialization.
    const uint32_t nonce[32] = 
    {
        0x168e9d48, 0xbf5d760d, 0xbd117ebb, 0x6691536d, 0x47e200cc, 0xd9092a7f, 0x62725556,
        0xffe6fa28, 0xa93c658e, 0xfe4153fa, 0xcd9b8535, 0xca656358, 0xaa01f585, 0x35fc5ba0,
        0xb390234f, 0x3a16633c, 0xbd2c704c, 0x49f11674, 0xc7b31c4f, 0x69035363, 0xae92044c,
        0x5d151558, 0x3de5c962, 0x752eb086, 0xb50fa4d2, 0xb6eafa88, 0x0beb15e3, 0x3a670561,
        0xf7bf0e08, 0x82f3d39f, 0xa910c801, 0x31e8c886
    };

    /********************************************
     * The expected RN for the above fixed seeds
     ********************************************/
    uint32_t expectedRn128[8] = {0x38cd3199, 0x447a3ced, 0xeae513c7, 0xaac96727,0x50a6e4d7, 0xeca7df48, 0x62a57cac, 0x82e415e8};

    /********************************************
     * The expected RN for the above fixed seeds
     ********************************************/
    uint32_t expectedRn256[8] = {0x660694d6, 0x3c40b09f, 0xb796f1e5, 0x3ea48b41,0x223662d0, 0x0ac2eaf6, 0x6486b353, 0x5e03a3ad};

    uint32_t *expected_rn;
    uint32_t rn[8] = {0};
    uint32_t i = 8;
    uint32_t read_flag = 0;

    CB_TRNG_Init();

    /********************************************
     * Seeding: Expensive, do it once at startup
     ********************************************/
    CB_TRNG_Snonce(&config, nonce);

    if (config.Alg == EN_CB_TRNG_AES128) 
    {
        expected_rn = expectedRn128;
    } 
    else 
    {
        expected_rn = expectedRn256;
    }

    enTrngErrCode err = CB_TRNG_GetRng(rn, 8);
    if (err != EN_CB_TRNG_OK) 
    {
        /********************************************
         * If error == EN_CB_TRNG_RESEED,
         * call Snonce/Snoise again!
         ********************************************/
        app_trng_print("[app_trng] Request error: %d\n", err);
    }

    app_trng_print("nonce-seeded:\n");
    app_trng_print("[app_trng] Generated: ");
    for (int32_t j = 0; j < 8; ++j) 
    {
        app_trng_print("%x ", rn[j]);
    }
    app_trng_print("\n");

    do {
        if (rn[8 - i] != expected_rn[8 - i]) 
        {
            app_trng_print("[ERROR] Failed at %d\n", 8 - i);
            ++read_flag;
            break;
        }
        --i;
    } while (i);

    if (read_flag) 
    {
        app_trng_print("[app_trng] Bad\n\n");
    } 
    else 
    {
        app_trng_print("[app_trng] All good\n\n");
    }

    CB_TRNG_Deinit();
}

/**
 * @brief Generate an RN using nonce-seeded TRNG core, with additional input.
 * @detail The additional input will help add more unpredictability to the TRNG outputs.
 * @details One can also run noise-seeded TRNG core with additional input. Just call Snoise instead.
 */
void app_trng_gen_with_addin(void) 
{
    uint32_t rn[8] = {0};
    uint32_t i = 8;
    uint32_t read_flag = 0;

    enTrngErrCode err;
    stTrngConfig config = 
    {
        .Alg = EN_CB_TRNG_AES128,
        .UseAddIn = APP_TRUE,
    };

    // Fixed seed values for nonce-seeded TRNG core initialization.
    const uint32_t nonce[32] = 
    {
        0x168e9d48, 0xbf5d760d, 0xbd117ebb, 0x6691536d, 0x47e200cc, 0xd9092a7f, 0x62725556,
        0xffe6fa28, 0xa93c658e, 0xfe4153fa, 0xcd9b8535, 0xca656358, 0xaa01f585, 0x35fc5ba0,
        0xb390234f, 0x3a16633c, 0xbd2c704c, 0x49f11674, 0xc7b31c4f, 0x69035363, 0xae92044c,
        0x5d151558, 0x3de5c962, 0x752eb086, 0xb50fa4d2, 0xb6eafa88, 0x0beb15e3, 0x3a670561,
        0xf7bf0e08, 0x82f3d39f, 0xa910c801, 0x31e8c886
    };

    /********************************************
     * The expected RN for the above fixed seeds
     ********************************************/
    uint32_t expected_rn_128_padded[8] = {0x89ac52fa, 0xda557ea6, 0x038c8e65, 0x712b8c4b,0x42c34587, 0x5ea36644, 0xf05f698d, 0x6c7412fc};

    uint32_t expected_rn_128_padd[8] = {0x0b80fe05, 0x688cc082, 0x481b8f80, 0xb241315d,0xe0906e92, 0xc45f1c45, 0xc82b321a, 0xd205e26a};

    /********************************************
     * If Addin is < needed size, has to pad with 0s
     * For AES128, AddIn has to be 256 bits
     * For AES384, AddIn has to be 384 bits
     * Padded simply means the last few elements in
     * the AddIn array (the most significant bytes
     * in the AddIn) are 0s.
     ********************************************/
    const uint8_t use_padded_add_in = APP_FALSE;

    const uint32_t test_add_in[8] = {0x11111111, 0x22222222, 0x33333333, 0x44444444,0x55555555, 0x00000000, 0x00000000, 0x00000000};

    const uint32_t test_add_in_nopad[8] = {0x11111111, 0x22222222, 0x33333333, 0x44444444,0x55555555, 0x66666666, 0x77777777, 0x88888888};

    uint32_t *expected_rn;

    CB_TRNG_Init();
    /********************************************
     * Seeding: Expensive, do it once at startup
     * To use noise-seeded mode, the nonce array won't
     * be needed, and CB_TRNG_Snoise() should be used.
     ********************************************/
    CB_TRNG_Snonce(&config, nonce);

    if (use_padded_add_in == APP_TRUE) 
    {
        expected_rn = expected_rn_128_padded;
    } 
    else 
    {
        expected_rn = expected_rn_128_padd;
    }

    if (use_padded_add_in == APP_TRUE) 
    {
        err = CB_TRNG_GetRngWithAddin(rn, 8, test_add_in);
    } 
    else 
    {
        err = CB_TRNG_GetRngWithAddin(rn, 8, test_add_in_nopad);
    }

    if (err != EN_CB_TRNG_OK) 
    {
        /********************************************
         * If error == EN_CB_TRNG_RESEED,
         * call Snonce/Snoise again!
         ********************************************/
        app_trng_print("[app_trng] Request error: %d\n", err);
    }

    app_trng_print("additional input:\n");
    app_trng_print("[app_trng] Generated: ");
    for (int j = 0; j < 8; ++j) 
    {
        app_trng_print("%x ", rn[j]);
    }
    app_trng_print("\n");

    do {
        if (rn[8 - i] != expected_rn[8 - i]) 
        {
            app_trng_print("[ERROR] Failed at %d\n", 8 - i);
            ++read_flag;
            break;
        }
        --i;
    } while (i);

    if (read_flag) 
    {
        app_trng_print("[app_trng] Bad\n\n");
    } 
    else 
    {
        app_trng_print("[app_trng] All good\n\n");
    }

    CB_TRNG_Deinit();
}

/**
 * @brief Runs HealthCheck on TRNG core.
 */
void app_trng_run_kat(void) 
{
    uint32_t alarms = 0;

    CB_TRNG_Init();

    if (CB_TRNG_RunKat(&alarms) != 0) 
    {
      app_trng_print("[ERROR] Kat on entropy source failed: %x\n\n", alarms);
    } 
    else 
    {
      app_trng_print("Entropy source healthy!\n\n");
    }

    CB_TRNG_Deinit();
}


/**
 * @brief Main function for executing TRNG (True Random Number Generator) peripheral operations.
 */
void app_peripheral_trng_init(void)
{
    // Initialize UART: GPIO00-RX, GPIO01-TX, BPR=115200, Stop bit = 1, BitOrder = LSB, Parity = NONE
    app_uart_init();                            
    
    // Print power-on reset message
    app_trng_print("\n\nPower-on reset\n");    

    // Generate random numbers based on noise
    app_trng_gen_with_noise();                      

    // Generate a one-time random number
    app_trng_gen_with_nonce();                      
    
    // Generate random numbers with additional input
    app_trng_gen_with_addin();                                                   
    
    // Run Known Answer Test (KAT) for TRNG
    app_trng_run_kat();  
}
