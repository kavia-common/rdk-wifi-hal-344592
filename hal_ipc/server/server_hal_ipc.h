#ifndef ONEWIFI_HAL_IPC_H
#define ONEWIFI_HAL_IPC_H

#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
// Initialize HAL IPC mechanism - create server thread
//------------------------------------------------------------------------------
int hal_ipc_init(void);


#ifdef __cplusplus
}
#endif

#endif // ONEWIFI_HAL_IPC_H
