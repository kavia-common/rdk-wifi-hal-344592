#include <stddef.h>
#include "wifi_hal.h"
#include "wifi_hal_priv.h"
#include "secure_wrapper.h"

#define BUFFER_LENGTH_WIFIDB 256

/* API to get encrypted default psk and ssid. */
static int get_default_encrypted_password (char* password);
static int get_default_encrypted_ssid (char* ssid);

/* Get default encrypted PSK key. */
static int get_default_encrypted_password (char* password) {

    if (NULL == password) {
        wifi_hal_error_print("%s:%d Invalid parameter \r\n", __func__, __LINE__);
        return -1;
    }

    const char* default_pwd_encrypted_key = "onewifidefaultcred";
    FILE* fp = NULL;
    char default_decrypted_pwd[128] = {0};
    char *result = NULL;

    fp = v_secure_popen("r", "/usr/bin/GetConfigFile %s stdout", default_pwd_encrypted_key);
    if (NULL == fp) {
         wifi_hal_error_print("%s:%d Failed to read GetConfigFile \n", __func__, __LINE__);
         return -1;
    }

    if ((result = fgets(default_decrypted_pwd, sizeof(default_decrypted_pwd), fp)) == NULL) {
        wifi_hal_error_print("%s:%d Failed to read encrypted password \n",__func__, __LINE__);
        v_secure_pclose(fp);
        return -1;
    }

    v_secure_pclose(fp);
    //copy password.
    strncpy(password, default_decrypted_pwd, strlen(default_decrypted_pwd) + 1);
    return 0;
}

/* Get default encrypted SSID. */
static int get_default_encrypted_ssid (char* ssid) {

    if (NULL == ssid) {
        wifi_hal_error_print("%s:%d Invalid parameter \r\n", __func__, __LINE__);
        return -1;
    }

    const char* default_ssid_encrypted_key = "onewifidefaultssid";
    FILE* fp = NULL;
    char default_decrypted_ssid[128] = {0};
    char *result = NULL;

    fp = v_secure_popen("r", "/usr/bin/GetConfigFile %s stdout", default_ssid_encrypted_key);
    if (NULL == fp) {
         wifi_hal_error_print("%s:%d Failed to read GetConfigFile \n", __func__, __LINE__);
         return -1;
    }

    if ((result = fgets(default_decrypted_ssid, sizeof(default_decrypted_ssid), fp)) == NULL) {
        wifi_hal_error_print("%s:%d Failed to read encrypted ssid \n",__func__, __LINE__);
        v_secure_pclose(fp);
        return -1;
    }

    v_secure_pclose(fp);
    //copy ssid.
    strncpy(ssid, default_decrypted_ssid, strlen(default_decrypted_ssid) + 1);
    return 0;
}

extern char *wlcsm_nvram_get(char *name);

int platform_pre_init()
{
    wifi_hal_dbg_print("%s \n", __func__);
    return 0;
}

int platform_post_init(wifi_vap_info_map_t *vap_map)
{
    wifi_hal_dbg_print("%s \n", __func__);
    return 0;
}

int platform_set_radio(wifi_radio_index_t index, wifi_radio_operationParam_t *operationParam)
{
    wifi_hal_dbg_print("%s \n", __func__);
    return 0;
}

int platform_create_vap(wifi_radio_index_t index, wifi_vap_info_map_t *map)
{
    wifi_hal_dbg_print("%s \n", __func__);
    return 0;
}

int platform_set_radio_pre_init(wifi_radio_index_t index, wifi_radio_operationParam_t *operationParam)
{
    wifi_hal_dbg_print("%s \n", __func__);
    return 0;
}

int nvram_get_vap_enable_status(bool *vap_enable, int vap_index)
{
    wifi_hal_dbg_print("%s \n", __func__);
    return 0;
}

int nvram_get_current_security_mode(wifi_security_modes_t *security_mode,int vap_index)
{
    wifi_hal_dbg_print("%s \n", __func__);
    return 0;
}

int nvram_get_default_password(char *l_password, int vap_index)
{
    char nvram_name[NVRAM_NAME_SIZE];
    char interface_name[8];
    int len;
    char *key_passphrase;

    memset(interface_name, 0, sizeof(interface_name));
    get_interface_name_from_vap_index(vap_index, interface_name);
    snprintf(nvram_name, sizeof(nvram_name), "%s_wpa_psk", interface_name);
    key_passphrase = wlcsm_nvram_get(nvram_name);
    if (key_passphrase == NULL) {
        wifi_hal_error_print("%s:%d nvram key_passphrase value is NULL\r\n", __func__, __LINE__);
        return -1;
    }
    len = strlen(key_passphrase);
    if (len < 8 || len > 63) {
        wifi_hal_error_print("%s:%d invalid wpa passphrase length [%d], expected length is [8..63]\r\n", __func__, __LINE__, len);
        return -1;
    }
    strcpy(l_password, key_passphrase);
    wifi_hal_dbg_print("%s:%d vap[%d] security password:%s nvram name:%s\r\n", __func__, __LINE__, vap_index, l_password, nvram_name);
    return 0;
}

int platform_get_keypassphrase_default(char *password, int vap_index)
{
    if(is_wifi_hal_vap_mesh_sta(vap_index)) {
        return get_default_encrypted_password(password);
    }else {
        strncpy(password,"123456789",strlen("123456789")+1);
        return 0;
    }
    return -1;
}
int platform_get_radius_key_default(char *radius_key)
{
    char nvram_name[NVRAM_NAME_SIZE];
    char *key;

    snprintf(nvram_name, sizeof(nvram_name), "default_radius_key");
    key = wlcsm_nvram_get(nvram_name);
    if (key == NULL) {
        wifi_hal_error_print("%s:%d nvram  radius_keydefault value is NULL\r\n", __func__, __LINE__);
        return -1;
    }
    else {
        strcpy(radius_key,key);
        wifi_hal_dbg_print("%s:%d::nvram name %s and radius_key %s\n",__func__, __LINE__, nvram_name,radius_key);
    }
        return 0;
}

int platform_get_ssid_default(char *ssid, int vap_index){
    char *str = NULL;
    if(is_wifi_hal_vap_mesh_sta(vap_index)) {
        char default_ssid[128] = {0};
        if (get_default_encrypted_ssid(default_ssid) == -1) {
            //Failed to get encrypted ssid.
            str = "OutOfService";
            strncpy(ssid,str,strlen(str)+1);
        }else {
            strncpy(ssid,default_ssid,strlen(default_ssid)+1);
        }
    }else {
        str = "OutOfService";
        strncpy(ssid,str,strlen(str)+1);
    }
    return 0;
}

int platform_get_wps_pin_default(char *pin)
{
    strcpy(pin, "88626277"); /* remove this and read the factory defaults below */
    wifi_hal_dbg_print("%s default wps pin:%s\n", __func__, pin);
    return 0;
#if 0
    char value[BUFFER_LENGTH_WIFIDB] = {0};
    FILE *fp = NULL;
    fp = popen("grep \"Default WPS Pin:\" /tmp/factory_nvram.data | cut -d ':' -f2 | cut -d ' ' -f2","r");
    if(fp != NULL) {
        while (fgets(value, sizeof(value), fp) != NULL) {
            strncpy(pin, value, strlen(value) - 1);
        }
        pclose(fp);
        return 0;
    }
    return -1;
#endif
}

int platform_wps_event(wifi_wps_event_t data)
{
    return 0;
}

int platform_get_country_code_default(char *code)
{
    return 0;
}

int nvram_get_current_password(char *l_password, int vap_index)
{
    return nvram_get_default_password(l_password, vap_index);
}

int nvram_get_current_ssid(char *l_ssid, int vap_index)
{
    char nvram_name[NVRAM_NAME_SIZE];
    char interface_name[8];
    int len;
    char *ssid;

    memset(interface_name, 0, sizeof(interface_name));
    get_interface_name_from_vap_index(vap_index, interface_name);
    snprintf(nvram_name, sizeof(nvram_name), "%s_ssid", interface_name);
    ssid = wlcsm_nvram_get(nvram_name);
    if (ssid == NULL) {
        wifi_hal_error_print("%s:%d nvram ssid value is NULL\r\n", __func__, __LINE__);
        return -1;
    }
    len = strlen(ssid);
    if (len < 0 || len > 63) {
        wifi_hal_error_print("%s:%d invalid ssid length [%d], expected length is [0..63]\r\n", __func__, __LINE__, len);
        return -1;
    }
    strcpy(l_ssid, ssid);
    wifi_hal_dbg_print("%s:%d vap[%d] ssid:%s nvram name:%s\r\n", __func__, __LINE__, vap_index, l_ssid, nvram_name);
    return 0;
}

int platform_pre_create_vap(wifi_radio_index_t index, wifi_vap_info_map_t *map)
{
    return 0;
}

int platform_flags_init(int *flags)
{
    return 0;
}

int platform_get_aid(void* priv, u16* aid, const u8* addr)
{
    return 0;
}

int platform_free_aid(void* priv, u16* aid)
{
    return 0;
}

int platform_sync_done(void* priv)
{
    return 0;
}

int platform_get_channel_bandwidth(wifi_radio_index_t index,  wifi_channelBandwidth_t *channelWidth)
{
    return 0;
}
