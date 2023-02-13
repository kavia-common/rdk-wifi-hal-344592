#include <stddef.h>
#include "wifi_hal.h"

int platform_preinit()
{
    return 0;
}

int platform_postinit(wifi_vap_info_map_t *vap_map)
{
    return 0;
}

int platform_set_radio(wifi_radio_index_t index, wifi_radio_operationParam_t *operationParam)
{
    return 0;
}

int platform_set_radio_pre_init(wifi_radio_index_t index, wifi_radio_operationParam_t *operationParam)
{
    return 0;
}

int platform_create_vap(wifi_radio_index_t index, wifi_vap_info_map_t *map)
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
    return 0;
}

int platform_get_ssid_default(char *ssid, int vap_index)
{
    return 0;
}

int platform_get_wps_pin_default(char *pin)
{
    return 0;
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
    return 0;
}

int nvram_get_current_ssid(char *l_ssid, int vap_index)
{
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

int platform_update_radio_presence(void)
{
    return 0;
}
