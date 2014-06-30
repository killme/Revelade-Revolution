#include "game.h"

#ifdef RR_NEED_AUTH
extern "C"
{
    #include <tomcrypt.h>
}
#endif

namespace auth
{

    void makeKey(const char *name)
    {
#ifdef RR_NEED_AUTH
        rsa_key key;
        string fileName;

        if(!name || !name[0])
        {
#ifdef CLIENT
            name = game::player1->name;
#else
            return;
#endif
        }

        int prng_idx = find_prng("sprng");
        int result = rsa_make_key(NULL, prng_idx, 1024/8, 65537, &key);

        if(result != CRYPT_OK)
        {
            conoutf(CON_ERROR, "Could not make key: %s\n", error_to_string(result));
            return;
        }

        formatstring(fileName)("cert/users/%s.private.key", name);
        path(fileName);
        stream *f = openfile(fileName, "w");

        if(!f)
        {
            conoutf(CON_ERROR, "Could not open cert file: %s\n", fileName);
            return;
        }

        unsigned long outlen = 1024*2;
        unsigned char out[outlen];

        result = rsa_export(out, &outlen, PK_PRIVATE, &key);

        if(result != CRYPT_OK)
        {
            conoutf(CON_ERROR, "Could not export key\n");
            return;
        }

        unsigned long outlen2 = outlen*8;
        uchar *encoded = new uchar[outlen2];

        base64_encode(out, outlen, encoded, &outlen2);

        f->write(encoded, outlen2);
        f->close();
        DELETEP(f);
        conoutf(CON_INFO, "Wrote private key to: %s\n", fileName);

        encoded[outlen2+1] = '\0';

        path(fileName);
        formatstring(fileName)("cert/users/%s.public.key", name);
        f = openfile(fileName, "w");

        if(!f)
        {
            conoutf(CON_ERROR, "Could not open cert file: %s\n", fileName);
            return;
        }

        result = rsa_export(out, &outlen, PK_PUBLIC, &key);

        if(result != CRYPT_OK)
        {
            conoutf(CON_ERROR, "Could not export key: %s\n", error_to_string(result));
            return;
        }

        base64_encode(out, outlen, encoded, &outlen2);

        f->write(encoded, outlen2);
        f->close();
        DELETEP(f);
        conoutf(CON_INFO, "Wrote public key to: %s\n", fileName);

        encoded[outlen2+1] = '\0';

        delete[] encoded;
        #endif
    }

#ifdef RR_NEED_AUTH
    COMMAND(makeKey, "s");
#endif

    bool haveCertificateFor(const char *name)
    {
#ifdef RR_NEED_AUTH
        defformatstring(fileName)("cert/users/%s.public.key", name);
        path(fileName);
        stream *f = openfile(fileName, "r");
        if(f)
        {
            delete f;
            return true;
        }
#endif
        return false;
    }

#ifdef CLIENT
    uchar *signMessage(const uchar *msg)
    {
#ifdef RR_NEED_AUTH
        defformatstring(fileName)("cert/users/%s.private.key", game::player1->name);
        path(fileName);
        stream *f = openfile(fileName, "r");

        if(!f)
        {
            conoutf(CON_ERROR, "Failed to sign certificate: could not open file (%s).", fileName);
            return NULL;
        }

        int result;
        rsa_key key;

        int size = f->size();
        uchar *buf = new uchar[size];
        f->read(buf, size);

        long unsigned int outSize = size;

        result = base64_decode(buf, size, buf, &outSize);

        if(result != CRYPT_OK)
        {
            conoutf(CON_ERROR, "Could not base64_decode key: %s\n", error_to_string(result));
            return NULL;
        }

        result = rsa_import(buf, outSize, &key);

        if(result != CRYPT_OK)
        {
            conoutf(CON_ERROR, "Could not import key: %s\n", error_to_string(result));
            return NULL;
        }

        outSize = size;

        int prng_idx = find_prng("sprng");
        int hash_idx = find_hash("sha1");
        result = rsa_sign_hash(msg, strlen((const char *)msg), buf, &outSize, NULL, prng_idx, hash_idx, 0, &key);

        if(result != CRYPT_OK)
        {
            conoutf(CON_ERROR, "Could not sign message: %s\n", error_to_string(result));
            return NULL;
        }

        long unsigned newSize = outSize*3+1;
        uchar *buf2 = new uchar[newSize];

        result = base64_encode(buf, outSize, buf2, &newSize);
        delete[] buf;

        if(result != CRYPT_OK)
        {
            delete[] buf2;
            conoutf(CON_ERROR, "Could not base64_encode signature: %s\n", error_to_string(result));
            return NULL;
        }

        buf2[newSize+1] = '\0';

        return buf2;
#else
        return NULL;
#endif
    }

#ifdef RR_NEED_AUTH
    ICOMMAND(signMessage, "s", (const char *s), {
        uchar *x = signMessage((const uchar *)s);

        if(x)
        {
            result((char *)x);
            delete[] x;
        }
    });
#endif
#endif

    bool checkCertificateFor(const char *name, const char *msg, const char *signature)
    {
#ifdef RR_NEED_AUTH
        int hash_idx = find_hash("sha1");

        int result;
        rsa_key key;

        defformatstring(fileName)("cert/users/%s.public.key", name);
        path(fileName);
        stream *f = openfile(fileName, "r");

        if(!f)
        {
            conoutf(CON_ERROR, "Failed to sign certificate: could not open file (%s).", fileName);
            return false;
        }

        int size = f->size();
        uchar *buf = new uchar[size+1];
        f->read(buf, size);

        DELETEP(f);

        buf[size+1] = 0;

        long unsigned int outSize = size;
        uchar *buf2 = new uchar[outSize];

        result = base64_decode(buf, size, buf2, &outSize);
        DELETEA(buf);

        if(result != CRYPT_OK)
        {
            conoutf(CON_ERROR, "Could not base64_decode key: %s\n", error_to_string(result));
            return false;
        }
   
        result = rsa_import(buf2, outSize, &key);

        DELETEA(buf2);

        if(result != CRYPT_OK)
        {
            rsa_free(&key);
            conoutf(CON_ERROR, "Could not import key: %s\n", error_to_string(result));
            return false;
        }

        long unsigned signatureSize = strlen(signature);
        unsigned char *signatureBuf = new unsigned char [signatureSize];

        result = base64_decode((const uchar *)signature, strlen(signature), signatureBuf, &signatureSize);

        if(result != CRYPT_OK)
        {
            DELETEA(signatureBuf);
            rsa_free(&key);
            conoutf(CON_ERROR, "Could not base64_decode signature: %s\n", error_to_string(result));
            return false;
        }

        int verified = -1;
        result = rsa_verify_hash(signatureBuf, signatureSize, (const uchar *)msg, strlen(msg), hash_idx, 0, &verified, &key);

        DELETEA(signatureBuf);
        rsa_free(&key);

        if(result != CRYPT_OK)
        {
            conoutf(CON_ERROR, "Could not verify hash: %s (%i)\n", error_to_string(result), verified);
            return false;
        }

        return verified == 1;
#else
        return false;
#endif
    }

#ifdef RR_NEED_AUTH
    prng_state randomStatus;

    void getRandom(string *str)
    {
        sprng_read((uchar *)str, sizeof(string)-1, &randomStatus);
    }
#endif
}