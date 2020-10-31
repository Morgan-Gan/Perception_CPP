#ifdef  __cplusplus
extern "C" {
#endif

int LKSC_Open(char * SC_Path , int *SC_Handle);
int LKSC_Close(int SC_Handle);
int LKSC_Reset( int SC_Handle, unsigned char *ATR, int * LenOfAtr);
int LKSC_SendAPDU( int SC_Handle, unsigned char *Command, int LenOfCmd, unsigned char *ResBuf, int * LenOfRes);

#ifdef  __cplusplus
}
#endif