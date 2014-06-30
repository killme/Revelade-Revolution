namespace auth
{
  
    void makeKey(const char *name);
    bool haveCertificateFor(const char *name);

    #ifdef CLIENT
    uchar *signMessage(const uchar *msg);
    #endif
  
    bool checkCertificateFor(const char *name, const char *random, const char *signature);
#ifdef RR_NEED_AUTH
    void getRandom(string *str);
#endif
}
