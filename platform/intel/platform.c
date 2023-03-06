#include <stddef.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <math.h>
#include <uci_wrapper.h>
#include "wifi_hal.h"
#include "wifi_hal_priv.h"
#include "arris_rpc.h"

#define COUNTRY_LENGTH 10
#define MAX_KEYPASSPHRASE_LEN 128
#define MAX_SSID_LEN 33

int platform_pre_init()
{

    char region[COUNTRY_LENGTH] = {0};
    char cmd[128] = {0};
    int ret = 0;

    ret = ARM_RPC(region, COUNTRY_LENGTH, "default_region");
    if (ret != 0)
    {
        strcpy(region, "US");
    }
    sprintf(cmd, "iw reg set %s", region);
    system(cmd);

    return 0;
}

int platform_post_init(wifi_vap_info_map_t *vap_map)
{
    wifi_hal_dbg_print("%s: \n", __FUNCTION__);

    return 0;
}

int nvram_get_current_password(char *l_password, int vap_index)
{
    if (l_password == NULL)
    {
        return -1;
    }
    uci_converter_get_optional_str(TYPE_VAP, vap_index, "key", l_password, MAX_KEYPASSPHRASE_LEN, "");
    wifi_hal_dbg_print("nvram_get_current_password vap_index:%d \n",vap_index);
    return 0;
}

int nvram_get_current_ssid(char *l_ssid, int vap_index)
{
    if (l_ssid == NULL)
    {
        return -1;
    }
    wifi_hal_dbg_print("nvram_get_current_password vap_index:%d \n",vap_index);
    return uci_converter_get_str_ext(TYPE_VAP, vap_index, "ssid", l_ssid, MAX_SSID_LEN - 1);
}

/* Stub for wave_api function, should be removed after implementation*/
int wifi_allow2G80211ax(bool enable)
{
    return 0;
}   

int nvram_get_vap_enable_status(bool *vap_enable, int vap_index)
{
    return 0;
}

int nvram_get_current_security_mode(wifi_security_modes_t *security_mode,int vap_index)
{
    return 0;
}

int platform_get_keypassphrase_default(char *password, int vap_index)
{
    int ret = 0;
    char cmMac[17] = {0};
    char buff[128] = {0};
    char key[MAX_KEYPASSPHRASE_LEN] = {0};
    FILE *fp = NULL;

    if (password == NULL)
    {
        return -1;
    }

    if ( is_wifi_hal_vap_private(vap_index) ) {
        /* Return default passphrase for private SSID */
        ret = ARM_RPC(password, MAX_KEYPASSPHRASE_LEN,"nvm_get", "psk");
        if (ret == 0)
        {
           wifi_hal_dbg_print("platform_get_keypassphrase_default pvt - returning success index=%d\n",vap_index);
            return 0;
        }
    }
    else if ( is_wifi_hal_vap_xhs(vap_index)) {
         //Default passphrase for XHS vaps
         wifi_hal_dbg_print("platform_get_keypassphrase_default - XHS %d\n",vap_index);
         fp = popen ("/lib/rdk/xhsScript.sh", "r");
         if(fp != NULL)
         {
           if (fgets (key, sizeof (key), fp) == NULL)
           {
             wifi_hal_dbg_print("platform_get_keypassphrase_default: failed to get default for XHS\n");
             pclose(fp);
             return -1;
           }
           if(key[0] != '\0')
           {
             if( key[strlen(key) - 1] == '\n')
             {
                key[strlen(key) - 1] = '\0';
             }
             strcpy(password,key);
             wifi_hal_dbg_print("platform_get_keypassphrase_default - XHS done.\n");
             pclose(fp);
             memset(key,0,sizeof(key));
             return 0;
           }
           else
           {
             wifi_hal_dbg_print("platform_get_keypassphrase_default - Key NULL\n");
             pclose(fp);
             return -1;
           }
         }
         else
         {
           wifi_hal_dbg_print("platform_get_keypassphrase_default - popen xhsScript.sh failed \n");
           return -1;
         }

    }
    else if (is_wifi_hal_vap_lnf_psk(vap_index)){
        //Default credential for LnF vaps.
        wifi_hal_dbg_print("platform_get_keypassphrase_default - lnf  %d\n",vap_index);
        fp = popen ("/lib/rdk/lnfScript.sh get_default_lnf_auth", "r");
        if(fp != NULL)
        {
            if (fgets (key, sizeof (key), fp) == NULL)
            {
                wifi_hal_dbg_print("platform_get_keypassphrase_default: failed to get default LNF passphrase\n");
                pclose(fp);
                return -1;
            }
            if(key[0] != '\0')
            {
                if( key[strlen(key) - 1] == '\n')
                {
                    key[strlen(key) - 1] = '\0';
                }

                strcpy(password,key);
                wifi_hal_dbg_print("platform_get_keypassphrase_default - LNF done.\n");
                pclose(fp);
                memset(key,0,sizeof(key));
                return 0;
            }
            else
            {
                wifi_hal_dbg_print("platform_get_keypassphrase_default - Key NULL\n");
                pclose(fp);
                return -1;
            }
        }
        else
        {
            wifi_hal_dbg_print("platform_get_keypassphrase_default - popen lnfScript.sh get_default_lnf_auth failed \n");
            return -1;
        }
    }
    else if (is_wifi_hal_vap_lnf_radius(vap_index)){
        //Default passphrase for LnF vaps
        wifi_hal_dbg_print("platform_get_keypassphrase_default - lnf radius %d\n",vap_index);
        fp = popen ("/lib/rdk/lnfScript.sh get_default_lnf_radius_auth", "r");
        if(fp != NULL)
        {
            if (fgets (key, sizeof (key), fp) == NULL)
            {
                wifi_hal_dbg_print("platform_get_keypassphrase_default: failed to get default LNF passphrase\n");
                pclose(fp);
                return -1;
            }
            if(key[0] != '\0')
            {
                if( key[strlen(key) - 1] == '\n')
                {
                    key[strlen(key) - 1] = '\0';
                }

                strcpy(password,key);
                wifi_hal_dbg_print("platform_get_keypassphrase_default - LNF done.\n");
                pclose(fp);
                memset(key,0,sizeof(key));
                return 0;
            }
            else
            {
                wifi_hal_dbg_print("platform_get_keypassphrase_default - Key NULL\n");
                pclose(fp);
                return -1;
            }
        }
        else
        {
            wifi_hal_dbg_print("platform_get_keypassphrase_default - popen lnfScript.sh get_default_lnf_radius_auth failed \n");
            return -1;
        }
    }
    else {
        wifi_hal_dbg_print("platform_get_keypassphrase_default else case - vap %d\n",vap_index);
        return nvram_get_current_password(password,vap_index);
    }
    wifi_hal_dbg_print("platform_get_keypassphrase_default - LnF common Fail\n");
    return -1;
}

int platform_get_ssid_default(char *ssid, int vap_index)
{
    int ret = 0;
    char name[MAX_SSID_LEN] = {0};
    FILE *fp = NULL;
    if (ssid == NULL)
    {
        return -1;
    }

    if ( is_wifi_hal_vap_private(vap_index) ) {
        /* Return default SSID for private SSID */
        ret = ARM_RPC(ssid,MAX_SSID_LEN,"default_ssid");
        if (ret == 0)
        {
            wifi_hal_dbg_print("platform_get_ssid_default  private vap: %d succcess\n",vap_index);
            return 0;
        }
    }
    else if (is_wifi_hal_vap_xhs(vap_index)){
        /* Return default SSID of XHS vap */
        ret = ARM_RPC(ssid,MAX_SSID_LEN,"default_xhs_ssid");
        if(ret==0)
        {
            wifi_hal_dbg_print("platform_get_ssid_default xhs vap: %d, succcess\n",vap_index);
          return 0;
        }
    }
    else if(is_wifi_hal_vap_lnf_psk(vap_index)){
        // Default SSID of PSK LnF vaps
        wifi_hal_dbg_print("platform_get_ssid_default lnf psk vap : %d\n",vap_index);
        fp = popen ("/lib/rdk/lnfScript.sh get_default_lnf_ssid", "r");
        if(fp != NULL)
        {
            if (fgets (name, sizeof (name), fp) == NULL)
            {
                wifi_hal_dbg_print("platform_get_ssid_default: failed to get default LNF ssid\n");
                pclose(fp);
                return -1;
            }
            if(name[0] != '\0')
            {
                if( name[strlen(name) - 1] == '\n')
                {
                    name[strlen(name) - 1] = '\0';
                }
                strcpy(ssid,name);
                wifi_hal_dbg_print("platform_get_ssid_default - LNF done.\n");
                pclose(fp);
                return 0;
            }
            else
            {
                wifi_hal_dbg_print("platform_get_ssid_default - ssid NULL\n");
                pclose(fp);
                return -1;
            }
        }
        else
        {
            wifi_hal_dbg_print("platform_get_ssid_default - popen lnfScript.sh get_default_lnf_ssid failed \n");
            return -1;
        }
    }
    else if(is_wifi_hal_vap_lnf_radius(vap_index)){
        // Default SSID of radius LnF vaps
        wifi_hal_dbg_print("platform_get_ssid_default lnf radius vap : %d\n",vap_index);
                fp = popen ("/lib/rdk/lnfScript.sh get_default_lnf_radius_ssid", "r");
        if(fp != NULL)
        {
            if (fgets (name, sizeof (name), fp) == NULL)
            {
                wifi_hal_dbg_print("platform_get_ssid_default: failed to get default LNF ssid\n");
                pclose(fp);
                return -1;
            }
            if(name[0] != '\0')
            {
                if( name[strlen(name) - 1] == '\n')
                {
                    name[strlen(name) - 1] = '\0';
                }
                strcpy(ssid,name);
                wifi_hal_dbg_print("platform_get_ssid_default - LNF done.\n");
                pclose(fp);
                return 0;
            }
            else
            {
                wifi_hal_dbg_print("platform_get_ssid_default - ssid NULL\n");
                pclose(fp);
                return -1;
            }
        }
        else
        {
            wifi_hal_dbg_print("platform_get_ssid_default - popen lnfScript.sh get_default_lnf_radius_ssid failed \n");
            return -1;
        }
    }
    else{
         wifi_hal_dbg_print("platform_get_ssid_default  vap: %d,succcess\n",vap_index);
         return nvram_get_current_ssid(ssid, vap_index); 
    }
    else if (is_wifi_hal_vap_xhs(vap_index)){
        /* Return default SSID of XHS vap */
        ret = ARM_RPC(ssid,MAX_SSID_LEN,"default_xhs_ssid");
        if(ret==0)
        {
            wifi_hal_dbg_print("platform_get_ssid_default xhs vap: %d, succcess\n",vap_index);
          return 0;
        }
    }
    else if(is_wifi_hal_vap_lnf_psk(vap_index)){
        // Default SSID of PSK LnF vaps
        wifi_hal_dbg_print("platform_get_ssid_default lnf psk vap : %d\n",vap_index);
        fp = popen ("/lib/rdk/lnfScript.sh get_default_lnf_ssid", "r");
        if(fp != NULL)
        {
            if (fgets (name, sizeof (name), fp) == NULL)
            {
                wifi_hal_dbg_print("platform_get_ssid_default: failed to get default LNF ssid\n");
                pclose(fp);
                return -1;
            }
            if(name[0] != '\0')
            {
                if( name[strlen(name) - 1] == '\n')
                {
                    name[strlen(name) - 1] = '\0';
                }
                strcpy(ssid,name);
                wifi_hal_dbg_print("platform_get_ssid_default - LNF done.\n");
                pclose(fp);
                return 0;
            }
            else
            {
                wifi_hal_dbg_print("platform_get_ssid_default - ssid NULL\n");
                pclose(fp);
                return -1;
            }
        }
        else
        {
            wifi_hal_dbg_print("platform_get_ssid_default - popen lnfScript.sh get_default_lnf_ssid failed \n");
            return -1;
        }
    }
    else if(is_wifi_hal_vap_lnf_radius(vap_index)){
        // Default SSID of radius LnF vaps
        wifi_hal_dbg_print("platform_get_ssid_default lnf radius vap : %d\n",vap_index);
                fp = popen ("/lib/rdk/lnfScript.sh get_default_lnf_radius_ssid", "r");
        if(fp != NULL)
        {
            if (fgets (name, sizeof (name), fp) == NULL)
            {
                wifi_hal_dbg_print("platform_get_ssid_default: failed to get default LNF ssid\n");
                pclose(fp);
                return -1;
            }
            if(name[0] != '\0')
            {
                if( name[strlen(name) - 1] == '\n')
                {
                    name[strlen(name) - 1] = '\0';
                }
                strcpy(ssid,name);
                wifi_hal_dbg_print("platform_get_ssid_default - LNF done.\n");
                pclose(fp);
                return 0;
            }
            else
            {
                wifi_hal_dbg_print("platform_get_ssid_default - ssid NULL\n");
                pclose(fp);
                return -1;
            }
        }
        else
        {
            wifi_hal_dbg_print("platform_get_ssid_default - popen lnfScript.sh get_default_lnf_radius_ssid failed \n");
            return -1;
        }
    }
    else{
         wifi_hal_dbg_print("platform_get_ssid_default  vap: %d,succcess\n",vap_index);
         return nvram_get_current_ssid(ssid, vap_index); 
    }
    return -1;
}

int platform_get_channel_bandwidth(wifi_radio_index_t index,  wifi_channelBandwidth_t *channelWidth)
{
  char htmode_str1[MAX_UCI_BUF_LEN];
  wifi_hal_dbg_print("%s:%d: Enter radio index:%d\n", __func__, __LINE__, index);
  if (uci_converter_alloc_local_uci_context()) {
      wifi_hal_dbg_print("%s:%d: alloc local context returned err!\n",__func__, __LINE__);
      return RETURN_ERR;
  }
  if(channelWidth == NULL) {
      wifi_hal_dbg_print("%s:%d: wifi_radio_operationParam_t *operationParam is NULL \n", __func__, __LINE__);
      return RETURN_ERR;
  }
  wifi_hal_dbg_print("%s:%d: Entering uci****************:\n", __func__, __LINE__);
  uci_converter_get_str_ext(TYPE_RADIO, index, "htmode", htmode_str1, sizeof(htmode_str1));
  wifi_hal_dbg_print("%s:%d: Enter radio index:%d htmode_value=%s\n", __func__, __LINE__, index,htmode_str1);
  if (!strncmp(htmode_str1, "HT20", MAX_UCI_BUF_LEN) || !strncmp(htmode_str1, "VHT20", MAX_UCI_BUF_LEN))
      *channelWidth = WIFI_CHANNELBANDWIDTH_20MHZ;
  else if (!strncmp(htmode_str1, "HT40+", MAX_UCI_BUF_LEN) || !strncmp(htmode_str1, "HT40-", MAX_UCI_BUF_LEN) || !strncmp(htmode_str1, "VHT40+", MAX_UCI_BUF_LEN) ||
      !strncmp(htmode_str1, "VHT40-", MAX_UCI_BUF_LEN) || !strncmp(htmode_str1, "VHT40", MAX_UCI_BUF_LEN))
      *channelWidth = WIFI_CHANNELBANDWIDTH_40MHZ;
  else if (!strncmp(htmode_str1, "VHT80", MAX_UCI_BUF_LEN))
      *channelWidth = WIFI_CHANNELBANDWIDTH_80MHZ;
  else if (!strncmp(htmode_str1, "VHT160", MAX_UCI_BUF_LEN))
      *channelWidth = WIFI_CHANNELBANDWIDTH_160MHZ;
  else {
      wifi_hal_dbg_print("%s:%d: htmode_str1 error value:%s \n", __func__, __LINE__,htmode_str1);
      return RETURN_ERR;
  }
  wifi_hal_dbg_print("%s:%d: %u *****successful***********\n", __func__, __LINE__,*channelWidth);
  uci_converter_free_local_uci_context();
  return 0;
}

int platform_get_country_code_default(char *code)
{
    if (code == NULL)
    {
        return -1;
    }
    if( ARM_RPC(code, COUNTRY_LENGTH,"default_region") == -1) {

        wifi_hal_dbg_print("%s:%d:Error value of default_code= %s\n", __func__, __LINE__,code);

        return -1;
    }
    wifi_hal_info_print("%s:%d:Actual value of default_code= %s\n", __func__, __LINE__,code);
    return 0;
}

int platform_set_radio_pre_init(wifi_radio_index_t index, wifi_radio_operationParam_t *operationParam)
{
    return 0;
}


int platform_get_wps_pin_default(char *pin)
{
    return -1;
}

int platform_set_radio(wifi_radio_index_t index, wifi_radio_operationParam_t *operationParam)
{
    char temp_buff[MAX_UCI_BUF_LEN];
    memset(temp_buff, 0 ,sizeof(temp_buff));
    wifi_hal_dbg_print("%s:%d: Enter radio index:%d\n", __func__, __LINE__, index);

    if (uci_converter_alloc_local_uci_context()) {
        wifi_hal_dbg_print("%s:%d: alloc local context returned err!\n",
            __func__, __LINE__);
        return RETURN_ERR;
    }

    get_coutry_str_from_code(operationParam->countryCode, temp_buff);
    // Canada 'CA' uses high power mode set as "CB" in the driver
    if( temp_buff[0] == 'C' && temp_buff[1] == 'A') {
        temp_buff[1] = 'B';
        wifi_hal_dbg_print("%s:%d: Forcing to CA High Power\n", __func__, __LINE__);
    }

    wifi_hal_dbg_print("%s:%d:setting UCI country_str %s\n", __func__, __LINE__, temp_buff);

    uci_converter_set_str(TYPE_RADIO, index, "country", temp_buff);

    memset(temp_buff, 0 ,sizeof(temp_buff));

    switch (operationParam->band)
    {
        case WIFI_FREQUENCY_2_4_BAND:
            strcpy(temp_buff, "2.4GHz");
            break;
        case WIFI_FREQUENCY_5_BAND:
            strcpy(temp_buff, "5GHz");
            break;
        case WIFI_FREQUENCY_5L_BAND:
            strcpy(temp_buff, "Low 5GHz");
            break;
        case WIFI_FREQUENCY_5H_BAND:
            strcpy(temp_buff, "High 5Ghz");
            break;
        case WIFI_FREQUENCY_6_BAND:
            strcpy(temp_buff, "6GHz");
            break;
        case WIFI_FREQUENCY_60_BAND:
            strcpy(temp_buff, "60GHz");
            break;
        default:
            strcpy(temp_buff, "");
            break;
    }

    uci_converter_set_str(TYPE_RADIO, index, "band", temp_buff);
    uci_converter_set_uint(TYPE_RADIO, index, "beacon_int",
        operationParam->beaconInterval);
    memset(temp_buff, 0 ,sizeof(temp_buff));
    get_radio_variant_str_from_int(operationParam->variant, temp_buff);
    uci_converter_set_str(TYPE_RADIO, index, "hwmode", temp_buff);

    if (operationParam->autoChannelEnabled) {
        uci_converter_set_str(TYPE_RADIO, index, "channel", "auto");
    } else {
        uci_converter_set_ulong(TYPE_RADIO, index, "channel",
            operationParam->channel);
    }

    uci_converter_commit_wireless();
    uci_converter_free_local_uci_context();

    return 0;
}

int platform_pre_create_vap(wifi_radio_index_t index, wifi_vap_info_map_t *map)
{
    wifi_vap_info_t *vap;
    unsigned int i;

    wifi_hal_dbg_print("%s:%d: \n", __func__, __LINE__);

    if (map == NULL)
    {
        wifi_hal_dbg_print("%s:%d: wifi_vap_info_map_t *map is NULL \n", __func__, __LINE__);
    }

    for (i = 0; i < map->num_vaps; i++)
    {
        mac_address_t dummy_mac;
        char interface_name[8];
        char bssid[18] = { 0 };
        vap = &map->vap_array[i];
        get_interface_name_from_vap_index(map->vap_array[i].vap_index,
            interface_name);
        char cmd[128] = {};
        snprintf(cmd, sizeof(cmd), "atom_util macdb vap %s", interface_name);
        FILE *fp = popen(cmd, "r");

        fscanf(fp, "%s", bssid);
        pclose(fp);

        to_mac_bytes(bssid, dummy_mac);

        memcpy(vap->u.bss_info.bssid, dummy_mac, sizeof(dummy_mac));
    }

    return 0;
}

int platform_wps_event(wifi_wps_event_t data)
{
    return 0;
}

/* XXX: should be refactored, using uci set */
int platform_create_vap(wifi_radio_index_t r_index, wifi_vap_info_map_t *map)
{
    char temp_buff[MAX_UCI_BUF_LEN];
    int index =0;
    wifi_hal_dbg_print("%s:%d: Enter radio index:%d\n", __func__, __LINE__, r_index);

    if (uci_converter_alloc_local_uci_context())
    {
        wifi_hal_dbg_print("%s:%d: alloc local context returned err!\n",
            __func__, __LINE__);
        return RETURN_ERR;
    }
    if (map == NULL)
    {
        wifi_hal_dbg_print("%s:%d: wifi_vap_info_map_t *map is NULL \n", __func__, __LINE__);
    }
    for (index = 0; index < map->num_vaps; index++)
    {
      if (map->vap_array[index].vap_mode == wifi_vap_mode_ap)
      {
        memset(temp_buff, 0 ,sizeof(temp_buff));
        if (get_security_mode_str_from_int(map->vap_array[index].u.bss_info.security.mode, temp_buff) == RETURN_OK)
        {
          if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "encryption", temp_buff))
            wifi_hal_dbg_print("%s:%d: Failed to set the encryption type:%s for apIndex:%d\n", __func__, __LINE__,temp_buff,map->vap_array[index].vap_index);
        }
        if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "ssid", map->vap_array[index].u.bss_info.ssid))
          wifi_hal_dbg_print("%s:%d:Failed to set the SSID:%s for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.ssid,map->vap_array[index].vap_index);
        if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index,"wps_pin",map->vap_array[index].u.bss_info.wps.pin))
          wifi_hal_dbg_print("%s:%d: Failed to set the wps:%s for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.wps.pin,map->vap_array[index].vap_index);
        if ((get_security_mode_support_radius(map->vap_array[index].u.bss_info.security.mode))|| is_wifi_hal_vap_hotspot_open(map->vap_array[index].vap_index))
        {
          if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "auth_server", map->vap_array[index].u.bss_info.security.u.radius.ip))
            wifi_hal_dbg_print("%s:%d:  Failed to set the auth server:%s for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.security.u.radius.ip,map->vap_array[index].vap_index);
          if(uci_converter_set_uint(TYPE_VAP, map->vap_array[index].vap_index, "auth_port", map->vap_array[index].u.bss_info.security.u.radius.port))
            wifi_hal_dbg_print("%s:%d: Failed to set the auth port:%d for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.security.u.radius.port,map->vap_array[index].vap_index);
          if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "auth_secret", map->vap_array[index].u.bss_info.security.u.radius.key))
            wifi_hal_dbg_print("%s:%d: Failed to set the auth secret:%s for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.security.u.radius.key,map->vap_array[index].vap_index);
          if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "sec_auth_server", map->vap_array[index].u.bss_info.security.u.radius.ip))
            wifi_hal_dbg_print("%s:%d: Failed to set the auth server:%s for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.security.u.radius.ip,map->vap_array[index].vap_index);
          if(uci_converter_set_uint(TYPE_VAP, map->vap_array[index].vap_index, "sec_auth_port", map->vap_array[index].u.bss_info.security.u.radius.port))
            wifi_hal_dbg_print("%s:%d: Failed to set the auth port:%d for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.security.u.radius.port,map->vap_array[index].vap_index);
          if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "sec_auth_secret", map->vap_array[index].u.bss_info.security.u.radius.key))
            wifi_hal_dbg_print("%s:%d: Failed to set the auth secret:%s for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.security.u.radius.key,map->vap_array[index].vap_index);
        }
        else
        {
          if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "key", map->vap_array[index].u.bss_info.security.u.key.key))
            wifi_hal_dbg_print("%s:%d: Failed to set the KeyPassPhrase:%s for index:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.security.u.key.key,map->vap_array[index].vap_index);
        }
        if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "hessid" ,map->vap_array[index].u.bss_info.interworking.interworking.hessid))
          wifi_hal_dbg_print("%s:%d: Failed to set the hessid:%s for index:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.interworking.interworking.hessid,map->vap_array[index].vap_index);
        if(uci_converter_set_uint(TYPE_VAP, map->vap_array[index].vap_index, "venue_group" , map->vap_array[index].u.bss_info.interworking.interworking.venueGroup))
          wifi_hal_dbg_print("%s:%d: Failed to set the venuegroup:%d for index:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.interworking.interworking.venueGroup,map->vap_array[index].vap_index);
        if(uci_converter_set_uint(TYPE_VAP, map->vap_array[index].vap_index, "venue_type" , map->vap_array[index].u.bss_info.interworking.interworking.venueType))
          wifi_hal_dbg_print("%s:%d: Failed to set the venuetype:%d for index:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.interworking.interworking.venueType,map->vap_array[index].vap_index);
      }
      else if (map->vap_array[index].vap_mode == wifi_vap_mode_sta)
      {
        memset(temp_buff, 0 ,sizeof(temp_buff));
        if (get_security_mode_str_from_int(map->vap_array[index].u.bss_info.security.mode, temp_buff) == RETURN_OK)
        {
          if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "encryption", temp_buff))
            wifi_hal_dbg_print("%s:%d: Failed to set the encryption type:%s for apIndex:%d\n", __func__, __LINE__,temp_buff,map->vap_array[index].vap_index);
        }
        if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "ssid", map->vap_array[index].u.bss_info.ssid))
          wifi_hal_dbg_print("%s:%d:Failed to set the SSID:%s for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.ssid,map->vap_array[index].vap_index);
        if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index,"wps_pin",map->vap_array[index].u.bss_info.wps.pin))
          wifi_hal_dbg_print("%s:%d: Failed to set the wps:%s for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.wps.pin,map->vap_array[index].vap_index);
        if ((get_security_mode_support_radius(map->vap_array[index].u.bss_info.security.mode))|| is_wifi_hal_vap_hotspot_open(map->vap_array[index].vap_index))
        {
          if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "auth_server", map->vap_array[index].u.bss_info.security.u.radius.ip))
            wifi_hal_dbg_print("%s:%d:  Failed to set the auth server:%s for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.security.u.radius.ip,map->vap_array[index].vap_index);
          if(uci_converter_set_uint(TYPE_VAP, map->vap_array[index].vap_index, "auth_port", map->vap_array[index].u.bss_info.security.u.radius.port))
            wifi_hal_dbg_print("%s:%d: Failed to set the auth port:%d for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.security.u.radius.port,map->vap_array[index].vap_index);
          if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "auth_secret", map->vap_array[index].u.bss_info.security.u.radius.key))
            wifi_hal_dbg_print("%s:%d: Failed to set the auth secret:%s for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.security.u.radius.key,map->vap_array[index].vap_index);
          if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "sec_auth_server", map->vap_array[index].u.bss_info.security.u.radius.ip))
            wifi_hal_dbg_print("%s:%d: Failed to set the auth server:%s for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.security.u.radius.ip,map->vap_array[index].vap_index);
          if(uci_converter_set_uint(TYPE_VAP, map->vap_array[index].vap_index, "sec_auth_port", map->vap_array[index].u.bss_info.security.u.radius.port))
            wifi_hal_dbg_print("%s:%d: Failed to set the auth port:%d for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.security.u.radius.port,map->vap_array[index].vap_index);
          if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "sec_auth_secret", map->vap_array[index].u.bss_info.security.u.radius.key))
            wifi_hal_dbg_print("%s:%d: Failed to set the auth secret:%s for apIndex:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.security.u.radius.key,map->vap_array[index].vap_index);
        }
        else
        {
          if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "key", map->vap_array[index].u.bss_info.security.u.key.key))
            wifi_hal_dbg_print("%s:%d: Failed to set the KeyPassPhrase:%s for index:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.security.u.key.key,map->vap_array[index].vap_index);
        }
        if(uci_converter_set_str(TYPE_VAP, map->vap_array[index].vap_index, "hessid" ,map->vap_array[index].u.bss_info.interworking.interworking.hessid))
          wifi_hal_dbg_print("%s:%d: Failed to set the hessid:%s for index:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.interworking.interworking.hessid,map->vap_array[index].vap_index);
        if(uci_converter_set_uint(TYPE_VAP, map->vap_array[index].vap_index, "venue_group" , map->vap_array[index].u.bss_info.interworking.interworking.venueGroup))
          wifi_hal_dbg_print("%s:%d: Failed to set the venuegroup:%d for index:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.interworking.interworking.venueGroup,map->vap_array[index].vap_index);
        if(uci_converter_set_uint(TYPE_VAP, map->vap_array[index].vap_index, "venue_type" , map->vap_array[index].u.bss_info.interworking.interworking.venueType))
          wifi_hal_dbg_print("%s:%d: Failed to set the venuetype:%d for index:%d\n", __func__, __LINE__,map->vap_array[index].u.bss_info.interworking.interworking.venueType,map->vap_array[index].vap_index);
      }
    }
    uci_converter_commit_wireless();
    uci_converter_free_local_uci_context();
    return 0;
}

int platform_flags_init(int *flags)
{
    *flags = PLATFORM_FLAGS_SET_BSS | PLATFORM_FLAGS_CONTROL_PORT_FRAME |
             PLATFORM_FLAGS_PROBE_RESP_OFFLOAD |
             PLATFORM_FLAGS_UPDATE_WIPHY_ON_PRIMARY;

    return 0;
}

int platform_get_aid(void* priv, u16* aid, const u8* addr)
{
    int res = -1;
    struct wpabuf *rsp_aid;
    int aid_size = sizeof(u16);

    if (!addr){
        return res;
    }

    if (*aid) {
        wifi_hal_dbg_print("Reusing old AID %hu\n", *aid);
        return 0;
    }

    rsp_aid = wpabuf_alloc(aid_size);
    if (!rsp_aid) {
        return -ENOBUFS;
    }

#if HOSTAPD_VERSION >= 210 //2.10
    res = wifi_drv_vendor_cmd(priv, OUI_LTQ, LTQ_NL80211_VENDOR_SUBCMD_GET_AID,
                                addr, ETH_ALEN, NESTED_ATTR_NOT_USED, rsp_aid);
#else
    res = wifi_drv_vendor_cmd(priv, OUI_LTQ, LTQ_NL80211_VENDOR_SUBCMD_GET_AID,
                                addr, ETH_ALEN, rsp_aid);
#endif

    if (res) {
        wifi_hal_dbg_print("nl80211: sending/receiving GET_AID failed: %i "
            "(%s)\n", res, strerror(res));
        *aid = 0;
    } else {
        memcpy(aid, rsp_aid->buf, aid_size);
        wifi_hal_dbg_print("Received a new AID %hu\n", *aid);
    }

    wpabuf_free(rsp_aid);

    return res;
}

int platform_free_aid(void* priv, u16* aid)
{
    int res = -1;

    if (!aid){
        return res;
    }

    if (0 == *aid) {
        return 0;
    }

#if HOSTAPD_VERSION >= 210 //2.10
    res = wifi_drv_vendor_cmd(priv, OUI_LTQ, LTQ_NL80211_VENDOR_SUBCMD_FREE_AID,
                                (u8*) aid, sizeof(*aid), NESTED_ATTR_NOT_USED, NULL);
#else
    res = wifi_drv_vendor_cmd(priv, OUI_LTQ, LTQ_NL80211_VENDOR_SUBCMD_FREE_AID,
                                (u8*) aid, sizeof(*aid), NULL);
#endif

    if (res) {
        wifi_hal_dbg_print("nl80211: sending FREE_AID failed: %i "
            "(%s)\n", res, strerror(res));
    } else {
        wifi_hal_dbg_print("AID %hu released\n", *aid);
        *aid = 0;
    }

    return res;
}

int platform_sync_done(void* priv)
{
    int res = -1;

    if (!priv){
        return res;
    }

#if HOSTAPD_VERSION >= 210 //2.10
    res = wifi_drv_vendor_cmd(priv, OUI_LTQ, LTQ_NL80211_VENDOR_SUBCMD_SYNC_DONE,
                                NULL, 0, NESTED_ATTR_NOT_USED, NULL);
#else
    res = wifi_drv_vendor_cmd(priv, OUI_LTQ, LTQ_NL80211_VENDOR_SUBCMD_SYNC_DONE,
                                NULL, 0, NULL);
#endif

    if (res) {
        wifi_hal_dbg_print("nl80211: sending SYNC_DONE failed: %i "
            "(%s)\n", res, strerror(res));
    }

    return res;
}

int platform_get_radius_key_default(char *radius_key)
{
    return -1;
}

int platform_update_radio_presence(void)
{
    return 0;
}
