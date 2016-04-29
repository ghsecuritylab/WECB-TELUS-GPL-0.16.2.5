/* apps/verify.c */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apps.h"
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

//Patch for wapi
#include <sys/stat.h>
//End patch

#undef PROG
#define PROG	verify_main

//Patch for wapi
static int isFileExist(const char * filename);
//End patch

static int MS_CALLBACK cb(int ok, X509_STORE_CTX *ctx);
static int check(X509_STORE *ctx, char *file, STACK_OF(X509) *uchain, STACK_OF(X509) *tchain, int purpose, ENGINE *e);
static STACK_OF(X509) *load_untrusted(char *file);
static int v_verbose=0, vflags = 0;

int MAIN(int, char **);

int MAIN(int argc, char **argv)
	{
	ENGINE *e = NULL;
	int i,ret=1, badarg = 0;
	int purpose = -1;
	char *CApath=NULL,*CAfile=NULL;
	char *untfile = NULL, *trustfile = NULL;
	STACK_OF(X509) *untrusted = NULL, *trusted = NULL;
	X509_STORE *cert_ctx=NULL;
	X509_LOOKUP *lookup=NULL;
	X509_VERIFY_PARAM *vpm = NULL;
#ifndef OPENSSL_NO_ENGINE
	char *engine=NULL;
#endif

//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	system("rm -f /var/myca/certVerifyResult 2>/dev/null");//Patch for wapi

	cert_ctx=X509_STORE_new();
	if (cert_ctx == NULL) goto end;
	X509_STORE_set_verify_cb_func(cert_ctx,cb);

//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	ERR_load_crypto_strings();

//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	apps_startup();

	if (bio_err == NULL)
		if ((bio_err=BIO_new(BIO_s_file())) != NULL)
			BIO_set_fp(bio_err,stderr,BIO_NOCLOSE|BIO_FP_TEXT);

	if (!load_config(bio_err, NULL))
		goto end;

	argc--;
	argv++;
	for (;;)
		{
		if (argc >= 1)
			{
			if (strcmp(*argv,"-CApath") == 0)
				{
				if (argc-- < 1) goto end;
				CApath= *(++argv);
				}
			else if (strcmp(*argv,"-CAfile") == 0)
				{
				if (argc-- < 1) goto end;
				CAfile= *(++argv);
				}
			else if (args_verify(&argv, &argc, &badarg, bio_err,
									&vpm))
				{
				if (badarg)
					goto end;
				continue;
				}
			else if (strcmp(*argv,"-untrusted") == 0)
				{
				if (argc-- < 1) goto end;
				untfile= *(++argv);
				}
			else if (strcmp(*argv,"-trusted") == 0)
				{
				if (argc-- < 1) goto end;
				trustfile= *(++argv);
				}
#ifndef OPENSSL_NO_ENGINE
			else if (strcmp(*argv,"-engine") == 0)
				{
				if (--argc < 1) goto end;
				engine= *(++argv);
				}
#endif
			else if (strcmp(*argv,"-help") == 0)
				goto end;
			else if (strcmp(*argv,"-verbose") == 0)
				v_verbose=1;
			else if (argv[0][0] == '-')
				goto end;
			else
				break;
			argc--;
			argv++;
			}
		else
			break;
		}

#ifndef OPENSSL_NO_ENGINE
        e = setup_engine(bio_err, engine, 0);
#endif

//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	if (vpm)
		X509_STORE_set1_param(cert_ctx, vpm);

	lookup=X509_STORE_add_lookup(cert_ctx,X509_LOOKUP_file());
	if (lookup == NULL) abort();
	if (CAfile) {
		i=X509_LOOKUP_load_file(lookup,CAfile,X509_FILETYPE_PEM);
		if(!i) {
			BIO_printf(bio_err, "Error loading file %s\n", CAfile);
			ERR_print_errors(bio_err);
			goto end;
		}
	} else X509_LOOKUP_load_file(lookup,NULL,X509_FILETYPE_DEFAULT);

//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	lookup=X509_STORE_add_lookup(cert_ctx,X509_LOOKUP_hash_dir());
	if (lookup == NULL) abort();
	if (CApath) {
		i=X509_LOOKUP_add_dir(lookup,CApath,X509_FILETYPE_PEM);
		if(!i) {
			BIO_printf(bio_err, "Error loading directory %s\n", CApath);
			ERR_print_errors(bio_err);
			goto end;
		}
	} else X509_LOOKUP_add_dir(lookup,NULL,X509_FILETYPE_DEFAULT);

//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	ERR_clear_error();

//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	if(untfile) {
		if(!(untrusted = load_untrusted(untfile))) {
			BIO_printf(bio_err, "Error loading untrusted file %s\n", untfile);
			ERR_print_errors(bio_err);
			goto end;
		}
	}

//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	if(trustfile) {
		if(!(trusted = load_untrusted(trustfile))) {
			BIO_printf(bio_err, "Error loading untrusted file %s\n", trustfile);
			ERR_print_errors(bio_err);
			goto end;
		}
	}

//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	if (argc < 1) 
	{
//		printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
		check(cert_ctx, NULL, untrusted, trusted, purpose, e);
	}
	else
	{
//		printf("%s(%d), argc=%d\n",__FUNCTION__,__LINE__, argc);//Added for test
		for (i=0; i<argc; i++)
			check(cert_ctx,argv[i], untrusted, trusted, purpose, e);
	}
	ret=0;
end:
	if (ret == 1) {
		BIO_printf(bio_err,"usage: verify [-verbose] [-CApath path] [-CAfile file] [-purpose purpose] [-crl_check]");
#ifndef OPENSSL_NO_ENGINE
		BIO_printf(bio_err," [-engine e]");
#endif
		BIO_printf(bio_err," cert1 cert2 ...\n");
		BIO_printf(bio_err,"recognized usages:\n");
		for(i = 0; i < X509_PURPOSE_get_count(); i++) {
			X509_PURPOSE *ptmp;
			ptmp = X509_PURPOSE_get0(i);
			BIO_printf(bio_err, "\t%-10s\t%s\n", X509_PURPOSE_get0_sname(ptmp),
								X509_PURPOSE_get0_name(ptmp));
		}
	}
	if (vpm) X509_VERIFY_PARAM_free(vpm);
	if (cert_ctx != NULL) X509_STORE_free(cert_ctx);
	sk_X509_pop_free(untrusted, X509_free);
	sk_X509_pop_free(trusted, X509_free);
	apps_shutdown();
//	printf("%s(%d),ret=%d\n",__FUNCTION__,__LINE__,ret);//Added for test
	OPENSSL_EXIT(ret);
	}

static int check(X509_STORE *ctx, char *file, STACK_OF(X509) *uchain, STACK_OF(X509) *tchain, int purpose, ENGINE *e)
	{
	X509 *x=NULL;
	int i=0,ret=0;
	X509_STORE_CTX *csc;

	//Patch for wapi
	//BIO *STDout=NULL;
	//FILE *fp=NULL;
	char strSerail[17];
	char tmpBuf[50];
	int len;
	//End patch

//	printf("%s(%d): file=%s\n",__FUNCTION__,__LINE__,file);//Added for test
	x = load_cert(bio_err, file, FORMAT_PEM, NULL, e, "certificate file");
	if (x == NULL)
		goto end;
#ifndef CLOSE_WAPI_DEBUG
	fprintf(stdout,"%s: ",(file == NULL)?"stdin":file);
#endif

	csc = X509_STORE_CTX_new();
	if (csc == NULL)
		{
		ERR_print_errors(bio_err);
		goto end;
		}

//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	X509_STORE_set_flags(ctx, vflags);

//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	if(!X509_STORE_CTX_init(csc,ctx,x,uchain))
		{
		ERR_print_errors(bio_err);
		goto end;
		}

//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	if(tchain) X509_STORE_CTX_trusted_stack(csc, tchain);

//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	if(purpose >= 0) X509_STORE_CTX_set_purpose(csc, purpose);

//	printf("%s(%d), file=%s\n",__FUNCTION__,__LINE__, file);//Added for test
	i=X509_verify_cert(csc);//?????

//	printf("%s(%d), i=%d\n",__FUNCTION__,__LINE__, i);//Added for test

	X509_STORE_CTX_free(csc);

	//Patch for wapi
//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	memset(strSerail, 0, sizeof(strSerail));
	len=i2a_ASN1_INTEGER2(strSerail,X509_get_serialNumber(x));
//	printf("%s(%d), len=%d, strSerail=%s\n",__FUNCTION__,__LINE__,len,strSerail);//Added for test
	
	memset(tmpBuf, 0, sizeof(tmpBuf));
	sprintf(tmpBuf, "openssl ca -status %s", strSerail);	//To verify cert status according to certs database: index.txt
	system(tmpBuf);
	
	/*
	fp = fopen("/var/tmp/tmp_serial", "w+");
	if (fp!=NULL)
	{
        	printf("Open /var/tmp/tmp_serial file ok!\n");
		STDout=BIO_new_fp(fp, BIO_NOCLOSE);
		if(STDout!=NULL)
		{
			BIO_printf(STDout,"serial=");
			i2a_ASN1_INTEGER(STDout,X509_get_serialNumber(x));
			BIO_printf(STDout,"\n");
			
			BIO_free_all(STDout);
		}

		fclose(fp);
	}*/
//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test

	ret=0;
end:
	if (i)
	{
#ifndef CLOSE_WAPI_DEBUG
		fprintf(stdout,"OK\n");
#endif
		ret=1;
//		printf("%s(%d),ret=%d\n",__FUNCTION__,__LINE__,ret);//Added for test

		//Patch for wapi
		if(isFileExist("/var/myca/certVerifyResult")!=1)///var/myca/certVerifyResult not exist
			system("echo \"0\" > /var/myca/certVerifyResult");// cert valid
		//End patch
	}
	else
		ERR_print_errors(bio_err);
	if (x != NULL) X509_free(x);

//	printf("%s(%d),ret=%d\n",__FUNCTION__,__LINE__,ret);//Added for test
	return(ret);
	}

static STACK_OF(X509) *load_untrusted(char *certfile)
{
	STACK_OF(X509_INFO) *sk=NULL;
	STACK_OF(X509) *stack=NULL, *ret=NULL;
	BIO *in=NULL;
	X509_INFO *xi;

	if(!(stack = sk_X509_new_null())) {
		BIO_printf(bio_err,"memory allocation failure\n");
		goto end;
	}

	if(!(in=BIO_new_file(certfile, "r"))) {
		BIO_printf(bio_err,"error opening the file, %s\n",certfile);
		goto end;
	}

	/* This loads from a file, a stack of x509/crl/pkey sets */
	if(!(sk=PEM_X509_INFO_read_bio(in,NULL,NULL,NULL))) {
		BIO_printf(bio_err,"error reading the file, %s\n",certfile);
		goto end;
	}

	/* scan over it and pull out the certs */
	while (sk_X509_INFO_num(sk))
		{
		xi=sk_X509_INFO_shift(sk);
		if (xi->x509 != NULL)
			{
			sk_X509_push(stack,xi->x509);
			xi->x509=NULL;
			}
		X509_INFO_free(xi);
		}
	if(!sk_X509_num(stack)) {
		BIO_printf(bio_err,"no certificates in file, %s\n",certfile);
		sk_X509_free(stack);
		goto end;
	}
	ret=stack;
end:
	BIO_free(in);
	sk_X509_INFO_free(sk);
	return(ret);
	}

static int MS_CALLBACK cb(int ok, X509_STORE_CTX *ctx)
	{
	char buf[256];

	if (!ok)
		{
		if (ctx->current_cert)
			{
			X509_NAME_oneline(
				X509_get_subject_name(ctx->current_cert),buf,
				sizeof buf);
#ifndef CLOSE_WAPI_DEBUG
			printf("%s(%d): %s\n",__FUNCTION__,__LINE__,buf);
#endif
			}
#ifndef CLOSE_WAPI_DEBUG
		printf("error %d at %d depth lookup:%s\n",ctx->error,
			ctx->error_depth,
			X509_verify_cert_error_string(ctx->error));
#endif

//		printf("%s(%d),ctx->error=%d\n",__FUNCTION__,__LINE__,ctx->error);//Added for test

		//Patch for wapi
		if (ctx->error == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY)
		{
			//unable to get local issuer certificate
			system("echo \"1\" > /var/myca/certVerifyResult");// cert issuer unknown
		}
		else if(ctx->error == X509_V_ERR_CERT_HAS_EXPIRED)
		{
			//cert expired
			system("echo \"3\" > /var/myca/certVerifyResult");
		}
		
		if (ctx->error == X509_V_ERR_CERT_HAS_EXPIRED) ok=1;
		/* since we are just checking the certificates, it is
		 * ok if they are self signed. But we should still warn
		 * the user.
 		 */
		if (ctx->error == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT) ok=1;
		/* Continue after extension errors too */
		if (ctx->error == X509_V_ERR_INVALID_CA) ok=1;
		if (ctx->error == X509_V_ERR_INVALID_NON_CA) ok=1;
		if (ctx->error == X509_V_ERR_PATH_LENGTH_EXCEEDED) ok=1;
		if (ctx->error == X509_V_ERR_INVALID_PURPOSE) ok=1;
		if (ctx->error == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT) ok=1;
		if (ctx->error == X509_V_ERR_CRL_HAS_EXPIRED) ok=1;
		if (ctx->error == X509_V_ERR_CRL_NOT_YET_VALID) ok=1;
		if (ctx->error == X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION) ok=1;

		if (ctx->error == X509_V_ERR_NO_EXPLICIT_POLICY)
			policies_print(NULL, ctx);
		return ok;

		}
	if ((ctx->error == X509_V_OK) && (ok == 2))
		policies_print(NULL, ctx);
	if (!v_verbose)
		ERR_clear_error();
	return(ok);
	}

//Patch for wapi
/*
*  function description: To check file wether exist
*  parameters:
*  filename (input): file name
*  return 1: filename exist; return 0: filename doesn't exist
*/
static int isFileExist(const char * filename)
{
	struct stat status;

	if(filename==NULL)
		return 0;

	if ( stat(filename, &status) < 0)
		return 0;

	return 1;
}
//End patch
