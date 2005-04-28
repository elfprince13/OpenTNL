# MAKEFILE for linux GCC
#
# Tom St Denis
# Modified by Clay Culver

# The version
VERSION=1.02

# Compiler and Linker Names
#CC=gcc
#LD=ld

# Archiver [makes .a files]
#AR=ar
#ARFLAGS=r

# Compilation flags. Note the += does not write over the user's CFLAGS!
CFLAGS += -c -I./testprof/ -I./headers/ -Wall -Wsign-compare -W -Wshadow -Wno-unused-parameter

# additional warnings (newer GCC 3.4 and higher)
#CFLAGS += -Wsystem-headers -Wdeclaration-after-statement -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wmissing-prototypes \
#		  -Wmissing-declarations -Wpointer-arith 

# optimize for SPEED
CFLAGS += -O3 -funroll-loops

# add -fomit-frame-pointer.  hinders debugging!
CFLAGS += -fomit-frame-pointer

# optimize for SIZE
#CFLAGS += -Os -DLTC_SMALL_CODE

# older GCCs can't handle the "rotate with immediate" ROLc/RORc/etc macros
# define this to help
#CFLAGS += -DLTC_NO_ROLC

# compile for DEBUGING (required for ccmalloc checking!!!)
#CFLAGS += -g3 -DLTC_NO_ASM

#Output filenames for various targets.
LIBNAME=libtomcrypt.a
LIBTEST=testprof/libtomcrypt_prof.a
HASH=hashsum
CRYPT=encrypt
SMALL=small
PROF=x86_prof
TV=tv_gen
MULTI=multi
TIMING=timing
TEST=test

#LIBPATH-The directory for libtomcrypt to be installed to.
#INCPATH-The directory to install the header files for libtomcrypt.
#DATAPATH-The directory to install the pdf docs.
DESTDIR=
LIBPATH=/usr/lib
INCPATH=/usr/include
DATAPATH=/usr/share/doc/libtomcrypt/pdf

#Who do we install as?
USER=root
GROUP=wheel

#List of objects to compile.

#Leave MPI built-in or force developer to link against libtommath?
MPIOBJECT=misc/mpi/mpi.o

OBJECTS=ciphers/aes/aes_enc.o $(MPIOBJECT) ciphers/aes/aes.o ciphers/anubis.o \
ciphers/blowfish.o ciphers/cast5.o ciphers/des.o ciphers/khazad.o ciphers/noekeon.o \
ciphers/rc2.o ciphers/rc5.o ciphers/rc6.o ciphers/safer/safer.o \
ciphers/safer/safer_tab.o ciphers/safer/saferp.o ciphers/skipjack.o \
ciphers/twofish/twofish.o ciphers/xtea.o encauth/ccm/ccm_memory.o \
encauth/ccm/ccm_test.o encauth/eax/eax_addheader.o encauth/eax/eax_decrypt.o \
encauth/eax/eax_decrypt_verify_memory.o encauth/eax/eax_done.o encauth/eax/eax_encrypt.o \
encauth/eax/eax_encrypt_authenticate_memory.o encauth/eax/eax_init.o \
encauth/eax/eax_test.o encauth/gcm/gcm_add_aad.o encauth/gcm/gcm_add_iv.o \
encauth/gcm/gcm_done.o encauth/gcm/gcm_gf_mult.o encauth/gcm/gcm_init.o \
encauth/gcm/gcm_memory.o encauth/gcm/gcm_process.o encauth/gcm/gcm_reset.o \
encauth/gcm/gcm_test.o encauth/ocb/ocb_decrypt.o encauth/ocb/ocb_decrypt_verify_memory.o \
encauth/ocb/ocb_done_decrypt.o encauth/ocb/ocb_done_encrypt.o encauth/ocb/ocb_encrypt.o \
encauth/ocb/ocb_encrypt_authenticate_memory.o encauth/ocb/ocb_init.o encauth/ocb/ocb_ntz.o \
encauth/ocb/ocb_shift_xor.o encauth/ocb/ocb_test.o encauth/ocb/s_ocb_done.o \
hashes/chc/chc.o hashes/helper/hash_file.o hashes/helper/hash_filehandle.o \
hashes/helper/hash_memory.o hashes/helper/hash_memory_multi.o hashes/md2.o hashes/md4.o \
hashes/md5.o hashes/rmd128.o hashes/rmd160.o hashes/sha1.o hashes/sha2/sha256.o \
hashes/sha2/sha512.o hashes/tiger.o hashes/whirl/whirl.o mac/hmac/hmac_done.o \
mac/hmac/hmac_file.o mac/hmac/hmac_init.o mac/hmac/hmac_memory.o \
mac/hmac/hmac_memory_multi.o mac/hmac/hmac_process.o mac/hmac/hmac_test.o \
mac/omac/omac_done.o mac/omac/omac_file.o mac/omac/omac_init.o mac/omac/omac_memory.o \
mac/omac/omac_memory_multi.o mac/omac/omac_process.o mac/omac/omac_test.o \
mac/pelican/pelican.o mac/pelican/pelican_memory.o mac/pelican/pelican_test.o \
mac/pmac/pmac_done.o mac/pmac/pmac_file.o mac/pmac/pmac_init.o mac/pmac/pmac_memory.o \
mac/pmac/pmac_memory_multi.o mac/pmac/pmac_ntz.o mac/pmac/pmac_process.o \
mac/pmac/pmac_shift_xor.o mac/pmac/pmac_test.o misc/base64/base64_decode.o \
misc/base64/base64_encode.o misc/burn_stack.o misc/crypt/crypt.o \
misc/crypt/crypt_argchk.o misc/crypt/crypt_cipher_descriptor.o \
misc/crypt/crypt_cipher_is_valid.o misc/crypt/crypt_find_cipher.o \
misc/crypt/crypt_find_cipher_any.o misc/crypt/crypt_find_cipher_id.o \
misc/crypt/crypt_find_hash.o misc/crypt/crypt_find_hash_any.o \
misc/crypt/crypt_find_hash_id.o misc/crypt/crypt_find_prng.o \
misc/crypt/crypt_hash_descriptor.o misc/crypt/crypt_hash_is_valid.o \
misc/crypt/crypt_prng_descriptor.o misc/crypt/crypt_prng_is_valid.o \
misc/crypt/crypt_register_cipher.o misc/crypt/crypt_register_hash.o \
misc/crypt/crypt_register_prng.o misc/crypt/crypt_unregister_cipher.o \
misc/crypt/crypt_unregister_hash.o misc/crypt/crypt_unregister_prng.o \
misc/error_to_string.o misc/mpi/is_prime.o misc/mpi/mpi_to_ltc_error.o \
misc/mpi/rand_prime.o misc/pkcs5/pkcs_5_1.o misc/pkcs5/pkcs_5_2.o misc/zeromem.o \
modes/cbc/cbc_decrypt.o modes/cbc/cbc_done.o modes/cbc/cbc_encrypt.o \
modes/cbc/cbc_getiv.o modes/cbc/cbc_setiv.o modes/cbc/cbc_start.o \
modes/cfb/cfb_decrypt.o modes/cfb/cfb_done.o modes/cfb/cfb_encrypt.o \
modes/cfb/cfb_getiv.o modes/cfb/cfb_setiv.o modes/cfb/cfb_start.o \
modes/ctr/ctr_decrypt.o modes/ctr/ctr_done.o modes/ctr/ctr_encrypt.o \
modes/ctr/ctr_getiv.o modes/ctr/ctr_setiv.o modes/ctr/ctr_start.o \
modes/ecb/ecb_decrypt.o modes/ecb/ecb_done.o modes/ecb/ecb_encrypt.o \
modes/ecb/ecb_start.o modes/ofb/ofb_decrypt.o modes/ofb/ofb_done.o \
modes/ofb/ofb_encrypt.o modes/ofb/ofb_getiv.o modes/ofb/ofb_setiv.o \
modes/ofb/ofb_start.o pk/asn1/der/der_decode_integer.o pk/asn1/der/der_encode_integer.o \
pk/asn1/der/der_get_multi_integer.o pk/asn1/der/der_length_integer.o \
pk/asn1/der/der_put_multi_integer.o pk/dh/dh.o pk/dsa/dsa_export.o pk/dsa/dsa_free.o \
pk/dsa/dsa_import.o pk/dsa/dsa_make_key.o pk/dsa/dsa_sign_hash.o \
pk/dsa/dsa_verify_hash.o pk/dsa/dsa_verify_key.o pk/ecc/ecc.o pk/packet_store_header.o \
pk/packet_valid_header.o pk/pkcs1/pkcs_1_i2osp.o pk/pkcs1/pkcs_1_mgf1.o \
pk/pkcs1/pkcs_1_oaep_decode.o pk/pkcs1/pkcs_1_oaep_encode.o pk/pkcs1/pkcs_1_os2ip.o \
pk/pkcs1/pkcs_1_pss_decode.o pk/pkcs1/pkcs_1_pss_encode.o pk/pkcs1/pkcs_1_v15_es_decode.o \
pk/pkcs1/pkcs_1_v15_es_encode.o pk/pkcs1/pkcs_1_v15_sa_decode.o \
pk/pkcs1/pkcs_1_v15_sa_encode.o pk/rsa/rsa_decrypt_key.o pk/rsa/rsa_encrypt_key.o \
pk/rsa/rsa_export.o pk/rsa/rsa_exptmod.o pk/rsa/rsa_free.o pk/rsa/rsa_import.o \
pk/rsa/rsa_make_key.o pk/rsa/rsa_sign_hash.o pk/rsa/rsa_v15_decrypt_key.o \
pk/rsa/rsa_v15_encrypt_key.o pk/rsa/rsa_v15_sign_hash.o pk/rsa/rsa_v15_verify_hash.o \
pk/rsa/rsa_verify_hash.o prngs/fortuna.o prngs/rc4.o prngs/rng_get_bytes.o \
prngs/rng_make_prng.o prngs/sober128.o prngs/sprng.o prngs/yarrow.o 

HEADERS=headers/tommath_superclass.h headers/tomcrypt_cfg.h \
headers/tomcrypt_mac.h headers/tomcrypt_macros.h \
headers/tomcrypt_custom.h headers/tomcrypt_argchk.h \
headers/tomcrypt_cipher.h headers/tomcrypt_pk.h \
headers/tommath_class.h headers/ltc_tommath.h headers/tomcrypt_hash.h \
headers/tomcrypt_misc.h headers/tomcrypt.h headers/tomcrypt_pkcs.h \
headers/tomcrypt_prng.h testprof/tomcrypt_test.h

TESTOBJECTS=demos/test.o
HASHOBJECTS=demos/hashsum.o
CRYPTOBJECTS=demos/encrypt.o
SMALLOBJECTS=demos/small.o
TVS=demos/tv_gen.o
MULTIS=demos/multi.o
TIMINGS=demos/timing.o
TESTS=demos/test.o

#Files left over from making the crypt.pdf.
LEFTOVERS=*.dvi *.log *.aux *.toc *.idx *.ilg *.ind *.out

#Compressed filenames
COMPRESSED=crypt-$(VERSION).tar.bz2 crypt-$(VERSION).zip

#The default rule for make builds the libtomcrypt library.
default:library

#ciphers come in two flavours... enc+dec and enc 
ciphers/aes/aes_enc.o: ciphers/aes/aes.c ciphers/aes/aes_tab.c
	$(CC) $(CFLAGS) -DENCRYPT_ONLY -c ciphers/aes/aes.c -o ciphers/aes/aes_enc.o

#These are the rules to make certain object files.
ciphers/aes/aes.o: ciphers/aes/aes.c ciphers/aes/aes_tab.c
ciphers/twofish/twofish.o: ciphers/twofish/twofish.c ciphers/twofish/twofish_tab.c
hashes/whirl/whirl.o: hashes/whirl/whirl.c hashes/whirl/whirltab.c
pk/ecc/ecc.o: pk/ecc/ecc.c pk/ecc/ecc_sys.c
pk/dh/dh.o: pk/dh/dh.c pk/dh/dh_sys.c
hashes/sha2/sha512.o: hashes/sha2/sha512.c hashes/sha2/sha384.c
hashes/sha2/sha256.o: hashes/sha2/sha256.c hashes/sha2/sha224.c

#This rule makes the libtomcrypt library.
library: $(LIBTEST) $(LIBNAME)

$(LIBTEST): 
	cd testprof ; CFLAGS="$(CFLAGS)" make 

$(LIBNAME): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS) 
	ranlib $(LIBNAME)

#This rule makes the hash program included with libtomcrypt
hashsum: library $(HASHOBJECTS)
	$(CC) $(HASHOBJECTS) $(LIBNAME) -o $(HASH) $(WARN)

#makes the crypt program
crypt: library $(CRYPTOBJECTS)
	$(CC) $(CRYPTOBJECTS) $(LIBNAME) -o $(CRYPT) $(WARN)

#makes the small program
small: library $(SMALLOBJECTS)
	$(CC) $(SMALLOBJECTS) $(LIBNAME) -o $(SMALL) $(WARN)
	
tv_gen: library $(TVS)
	$(CC) $(TVS) $(LIBNAME) $(EXTRALIBS) -o $(TV)

multi: library $(MULTIS)
	$(CC) $(MULTIS) $(LIBNAME) -o $(MULTI)

timing: library $(TIMINGS)
	$(CC) $(TIMINGS) $(LIBTEST) $(LIBNAME) -o $(TIMING)

test: library $(TESTS)
	$(CC) $(TESTS) $(LIBTEST) $(LIBNAME) -o $(TEST)


#This rule installs the library and the header files. This must be run
#as root in order to have a high enough permission to write to the correct
#directories and to set the owner and group to root.
install: library docs
	install -d -g $(GROUP) -o $(USER) $(DESTDIR)$(LIBPATH)
	install -d -g $(GROUP) -o $(USER) $(DESTDIR)$(INCPATH)
	install -d -g $(GROUP) -o $(USER) $(DESTDIR)$(DATAPATH)
	install -g $(GROUP) -o $(USER) $(LIBNAME) $(DESTDIR)$(LIBPATH)
	install -g $(GROUP) -o $(USER) $(HEADERS) $(DESTDIR)$(INCPATH)
	install -g $(GROUP) -o $(USER) doc/crypt.pdf $(DESTDIR)$(DATAPATH)

install_lib: library
	install -d -g $(GROUP) -o $(USER) $(DESTDIR)$(LIBPATH)
	install -d -g $(GROUP) -o $(USER) $(DESTDIR)$(INCPATH)
	install -g $(GROUP) -o $(USER) $(LIBNAME) $(DESTDIR)$(LIBPATH)
	install -g $(GROUP) -o $(USER) $(HEADERS) $(DESTDIR)$(INCPATH)

#This rule cleans the source tree of all compiled code, not including the pdf
#documentation.
clean:
	rm -f `find . -type f | grep "[.]o" | xargs`
	rm -f `find . -type f | grep "[.]lo"  | xargs`
	rm -f `find . -type f | grep "[.]a" | xargs`
	rm -f `find . -type f | grep "[.]la"  | xargs`
	rm -f `find . -type f | grep "[.]obj" | xargs`
	rm -f `find . -type f | grep "[.]lib" | xargs`
	rm -f `find . -type f | grep "[.]exe" | xargs`
	rm -f `find . -type f | grep "[.]gcda" | xargs`
	rm -f `find . -type f | grep "[.]gcno" | xargs`
	rm -f `find . -type f | grep "[.]il" | xargs`
	rm -f `find . -type f | grep "[.]dyn" | xargs`
	rm -f `find . -type f | grep "[.]dpi" | xargs`
	rm -rf `find . -type d | grep "[.]libs" | xargs`
	rm -f crypt.aux  crypt.dvi  crypt.idx  crypt.ilg  crypt.ind  crypt.log crypt.toc
	rm -f $(TV) $(PROF) $(SMALL) $(CRYPT) $(HASHSUM) $(MULTI) $(TIMING) $(TEST)
	rm -rf doc/doxygen
	rm -f doc/*.pdf

#build the doxy files (requires Doxygen, tetex and patience)
doxy:
	doxygen
	cd doc/doxygen/latex ; make ; mv -f refman.pdf ../../.
	echo The huge doxygen PDF should be available as doc/refman.pdf
	
#This builds the crypt.pdf file. Note that the rm -f *.pdf has been removed
#from the clean command! This is because most people would like to keep the
#nice pre-compiled crypt.pdf that comes with libtomcrypt! We only need to
#delete it if we are rebuilding it.
docs: crypt.tex
	rm -f doc/crypt.pdf $(LEFTOVERS)
	echo "hello" > crypt.ind
	latex crypt > /dev/null
	latex crypt > /dev/null
	makeindex crypt.idx > /dev/null
	latex crypt > /dev/null
	dvipdf crypt
	mv -ivf crypt.pdf doc/crypt.pdf
	rm -f $(LEFTOVERS)

docdvi: crypt.tex
	echo hello > crypt.ind
	latex crypt > /dev/null
	latex crypt > /dev/null
	makeindex crypt.idx
	latex crypt > /dev/null

#zipup the project (take that!)
no_oops: clean
	cd .. ; cvs commit 

zipup: no_oops docs
	cd .. ; rm -rf crypt* libtomcrypt-$(VERSION) ; mkdir libtomcrypt-$(VERSION) ; \
	cp -R ./libtomcrypt/* ./libtomcrypt-$(VERSION)/ ; \
	cd libtomcrypt-$(VERSION) ; rm -rf `find . -type d | grep CVS | xargs` ; cd .. ; \
	tar -cjvf crypt-$(VERSION).tar.bz2 libtomcrypt-$(VERSION) ; \
	zip -9r crypt-$(VERSION).zip libtomcrypt-$(VERSION) ; \
	gpg -b -a crypt-$(VERSION).tar.bz2 ; gpg -b -a crypt-$(VERSION).zip ; \
	mv -fv crypt* ~ ; rm -rf libtomcrypt-$(VERSION)
