/* p_w64pep.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2013 Laszlo Molnar
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>

   -------------------------------------------------------------------

   PE+ format extension changes         (C) 2010 Stefan Widmann
   changes in:

 */


#include "conf.h"
#include "file.h"
#include "filter.h"
#include "packer.h"
#include "pepfile.h"
#include "p_w64pep.h"
#include "linker.h"

static const
#include "stub/amd64-win64.pep.h"

#define IDSIZE(x)       ih.ddirs[x].size
#define IDADDR(x)       ih.ddirs[x].vaddr
#define ODSIZE(x)       oh.ddirs[x].size
#define ODADDR(x)       oh.ddirs[x].vaddr

#define isdll           ((ih.flags & DLL_FLAG) != 0)

#define FILLVAL         0


/*************************************************************************
//
**************************************************************************/

#if defined(__BORLANDC__)
#  undef strcpy
#  define strcpy(a,b)   strcpy((char *)(a),(const char *)(b))
#endif

#if 0  //NEW: Stefan Widmann
//static
unsigned my_strlen(const char *s)
{
    size_t l = strlen((const char*)s); assert((unsigned) l == l); return (unsigned) l;
}
static unsigned my_strlen(const unsigned char *s)
{
    size_t l = strlen((const char*)s); assert((unsigned) l == l); return (unsigned) l;
}
#undef strlen
#define strlen my_strlen
#endif


#if (__ACC_CXX_HAVE_PLACEMENT_DELETE) || defined(__DJGPP__)
#include "bptr.h"
#define IPTR(type, var)         BoundedPtr<type> var(ibuf, ibuf.getSize())
#define OPTR(type, var)         BoundedPtr<type> var(obuf, obuf.getSize())
#define IPTR_I(type, var, v)    BoundedPtr<type> var(ibuf, ibuf.getSize(), v)
#define OPTR_I(type, var, v)    BoundedPtr<type> var(obuf, obuf.getSize(), v)
#define IPTR_C(type, var, v)    const BoundedPtr<type> var(ibuf, ibuf.getSize(), v)
#define OPTR_C(type, var, v)    const BoundedPtr<type> var(obuf, obuf.getSize(), v)
#else
#define IPTR(type, var)         type* var = 0
#define OPTR(type, var)         type* var = 0
#define IPTR_I(type, var, v)    type* var = (v)
#define OPTR_I(type, var, v)    type* var = (v)
#define IPTR_C(type, var, v)    type* const var = (v)
#define OPTR_C(type, var, v)    type* const var = (v)
#endif

static void xcheck(const void *p, size_t plen, const void *b, size_t blen)
{
    const char *pp = (const char *) p;
    const char *bb = (const char *) b;
    if (pp < bb || pp > bb + blen || pp + plen > bb + blen)
        throwCantUnpack("pointer out of range; take care!");
}
#if 0
static void xcheck(size_t poff, size_t plen, const void *b, size_t blen)
{
    ACC_UNUSED(b);
    if (poff > blen || poff + plen > blen)
        throwCantUnpack("pointer out of range; take care!");
}
#endif
#define ICHECK(x, size)     xcheck(x, size, ibuf, ibuf.getSize())
#define OCHECK(x, size)     xcheck(x, size, obuf, obuf.getSize())

#define imemset(a,b,c)      ICHECK(a,c), memset(a,b,c)
#define omemset(a,b,c)      OCHECK(a,c), memset(a,b,c)
#define imemcpy(a,b,c)      ICHECK(a,c), memcpy(a,b,c)
#define omemcpy(a,b,c)      OCHECK(a,c), memcpy(a,b,c)


/*************************************************************************
//
**************************************************************************/

PackW64Pep::PackW64Pep(InputFile *f) : super(f)
{
    oloadconf = NULL;
    soloadconf = 0;
    isrtm = false;
    use_dep_hack = true;
    use_clear_dirty_stack = true;
    use_tls_callbacks = false;
}


PackW64Pep::~PackW64Pep()
{
    delete [] oloadconf;
}


const int *PackW64Pep::getCompressionMethods(int method, int level) const
{
    bool small = ih.codesize + ih.datasize <= 256*1024;
    return Packer::getDefaultCompressionMethods_le32(method, level, small);
}


const int *PackW64Pep::getFilters() const
{
    static const int filters[] = { 0x49, FT_END };
    return filters;
}


Linker* PackW64Pep::newLinker() const
{
    return new ElfLinkerAMD64;
}


/*************************************************************************
// util
**************************************************************************/

int PackW64Pep::readFileHeader()
{
    char buf[6];
    fi->seek(0x200, SEEK_SET);
    fi->readx(buf, 6);
    isrtm = 0;
    return super::readFileHeader();
}

/*************************************************************************
// import handling
**************************************************************************/

__packed_struct(import_desc)
    LE32  oft;      // orig first thunk
    char  _[8];
    LE32  dllname;
    LE32  iat;      // import address table
__packed_struct_end()

void PackW64Pep::processImports(unsigned myimport, unsigned) // pass 2
{
    COMPILE_TIME_ASSERT(sizeof(import_desc) == 20);

    // adjust import data
    for (import_desc *im = (import_desc*) oimpdlls; im->dllname; im++)
    {
        if (im->dllname < myimport)
            im->dllname += myimport;
        LE64 *p = (LE64*) (oimpdlls + im->iat);  //changed to LE64 - Stefan Widmann
        im->iat += myimport;

        while (*p)
            if ((*p++ & 0x8000000000000000ULL) == 0)  // changed to 64 bit - Stefan Widmann
                p[-1] += myimport;
    }
}

unsigned PackW64Pep::processImports() // pass 1
{
    static const unsigned char kernel32dll[] = "KERNEL32.DLL";
    static const char llgpa[] = "\x0\x0""LoadLibraryA\x0\x0"
                                "GetProcAddress\x0\x0"
                                "VirtualProtect\x0\x0"
                                "VirtualAlloc\x0\x0"
                                "VirtualFree\x0\x0\x0";
    static const char exitp[] = "ExitProcess\x0\x0\x0";

    unsigned dllnum = 0;
    import_desc *im = (import_desc*) (ibuf + IDADDR(PEDIR_IMPORT));
    import_desc * const im_save = im;
    if (IDADDR(PEDIR_IMPORT))
    {
        while (im->dllname)
            dllnum++, im++;
        im = im_save;
    }

    struct udll
    {
        const upx_byte *name;
        const upx_byte *shname;
        unsigned   ordinal;
        unsigned   iat;
        LE64       *lookupt; //changed to LE64* - Stefan Widmann
        unsigned   npos;
        unsigned   original_position;
        bool       isk32;

        static int __acc_cdecl_qsort compare(const void *p1, const void *p2)
        {
            const udll *u1 = * (const udll * const *) p1;
            const udll *u2 = * (const udll * const *) p2;
            if (u1->isk32) return -1;
            if (u2->isk32) return 1;
            if (!*u1->lookupt) return 1;
            if (!*u2->lookupt) return -1;
            int rc = strcasecmp(u1->name,u2->name);
            if (rc) return rc;
            if (u1->ordinal) return -1;
            if (u2->ordinal) return 1;
            if (!u1->shname) return 1;
            if (!u2->shname) return -1;
            return strlen(u1->shname) - strlen(u2->shname);
        }
    };

    // +1 for dllnum=0
    Array(struct udll, dlls, dllnum+1);
    Array(struct udll *, idlls, dllnum+1);

    soimport = 1024; // safety

    unsigned ic,k32o;
    for (ic = k32o = 0; dllnum && im->dllname; ic++, im++)
    {
        idlls[ic] = dlls + ic;
        dlls[ic].name = ibuf + im->dllname;
        dlls[ic].shname = NULL;
        dlls[ic].ordinal = 0;
        dlls[ic].iat = im->iat;
        dlls[ic].lookupt = (LE64*) (ibuf + (im->oft ? im->oft : im->iat)); //changed to LE64* - Stefan Widmann
        dlls[ic].npos = 0;
        dlls[ic].original_position = ic;
        dlls[ic].isk32 = strcasecmp(kernel32dll,dlls[ic].name) == 0;

        soimport += strlen(dlls[ic].name) + 1 + 4;

        // FIXME use IPTR_I as in p32pe.cpp
        for (LE64 *tarr = dlls[ic].lookupt; *tarr; tarr++) //changed to LE64* - Stefan Widmann
        {
            if (*tarr & 0x8000000000000000ULL) //in PE32+, bit 63 indicates an ordinal
            {
                importbyordinal = true;
                soimport += 2; // ordinal num: 2 bytes
                dlls[ic].ordinal = *tarr & 0xffff;
                if (dlls[ic].isk32)
                    kernel32ordinal = true,k32o++;
            }
            else //it's an import by name
            {
                unsigned len = strlen(ibuf + *tarr + 2);
                soimport += len + 1;
                if (dlls[ic].shname == NULL || len < strlen (dlls[ic].shname))
                    dlls[ic].shname = ibuf + *tarr + 2;
            }
            soimport++; // separator
        }
    }
    oimport = new upx_byte[soimport];
    memset(oimport,0,soimport);
    oimpdlls = new upx_byte[soimport];
    memset(oimpdlls,0,soimport);

    qsort(idlls,dllnum,sizeof (udll*),udll::compare);

    unsigned dllnamelen = sizeof (kernel32dll);
    unsigned dllnum2 = 1;
    for (ic = 0; ic < dllnum; ic++)
        if (!idlls[ic]->isk32 && (ic == 0 || strcasecmp(idlls[ic - 1]->name,idlls[ic]->name)))
        {
            dllnum2++;
            dllnamelen += strlen(idlls[ic]->name) + 1;
        }
    //fprintf(stderr,"dllnum=%d dllnum2=%d soimport=%d\n",dllnum,dllnum2,soimport); //

    info("Processing imports: %d DLLs", dllnum);

    // create the new import table
    im = (import_desc*) oimpdlls;

    LE64 *ordinals = (LE64*) (oimpdlls + (dllnum2 + 1) * sizeof(import_desc)); //changed to LE64* - Stefan Widmann
    LE64 *lookuptable = ordinals + 6 + k32o + (isdll ? 0 : 1); //changed to LE64* - Stefan Widmann
    upx_byte *dllnames = ((upx_byte*) lookuptable) + (dllnum2 - 1) * 16; //double the size per hint - is that correct? - Stefan Widmann
    upx_byte *importednames = dllnames + (dllnamelen &~ 1);

    unsigned k32namepos = ptr_diff(dllnames,oimpdlls);

    memcpy(importednames, llgpa, sizeof(llgpa));
    if (!isdll)
        memcpy(importednames + sizeof(llgpa) - 1, exitp, sizeof(exitp));
    strcpy(dllnames,kernel32dll);
    im->dllname = k32namepos;
    im->iat = ptr_diff(ordinals,oimpdlls);
    *ordinals++ = ptr_diff(importednames,oimpdlls);             // LoadLibraryA
    *ordinals++ = ptr_diff(importednames,oimpdlls) + 14;        // GetProcAddress
    *ordinals++ = ptr_diff(importednames,oimpdlls) + 14 + 16;   // VirtualProtect
    *ordinals++ = ptr_diff(importednames,oimpdlls) + 14 + 16 + 16;   // VirtualAlloc
    *ordinals++ = ptr_diff(importednames,oimpdlls) + 14 + 16 + 16 + 14;   // VirtualFree
    if (!isdll)
        *ordinals++ = ptr_diff(importednames,oimpdlls) + sizeof(llgpa) - 3; // ExitProcess
    dllnames += sizeof(kernel32dll);
    importednames += sizeof(llgpa) - 2 + (isdll ? 0 : sizeof(exitp) - 1);

    im++;
    for (ic = 0; ic < dllnum; ic++)
        if (idlls[ic]->isk32)
        {
            idlls[ic]->npos = k32namepos;
            if (idlls[ic]->ordinal)
                for (LE64 *tarr = idlls[ic]->lookupt; *tarr; tarr++) //changed to LE64* - Stefan Widmann
                    if (*tarr & 0x8000000000000000ULL) //in PE32+, bit 63 indicates an ordinal - Stefan Widmann
                        *ordinals++ = *tarr;
        }
        else if (ic && strcasecmp(idlls[ic-1]->name,idlls[ic]->name) == 0)
            idlls[ic]->npos = idlls[ic-1]->npos;
        else
        {
            im->dllname = idlls[ic]->npos = ptr_diff(dllnames,oimpdlls);
            im->iat = ptr_diff(lookuptable,oimpdlls);

            strcpy(dllnames,idlls[ic]->name);
            dllnames += strlen(idlls[ic]->name)+1;
            if (idlls[ic]->ordinal)
                *lookuptable = idlls[ic]->ordinal + 0x8000000000000000ULL; //bit 63 indicates an ordinal - Stefan Widmann
            else if (idlls[ic]->shname)
            {
                if (ptr_diff(importednames,oimpdlls) & 1)
                    importednames--;
                *lookuptable = ptr_diff(importednames,oimpdlls);
                importednames += 2;
                strcpy(importednames,idlls[ic]->shname);
                importednames += strlen(idlls[ic]->shname) + 1;
            }
            lookuptable += 2;
            im++;
        }
    soimpdlls = ALIGN_UP(ptr_diff(importednames,oimpdlls),4);

    Interval names(ibuf),iats(ibuf),lookups(ibuf);
    // create the preprocessed data
    ordinals -= k32o;
    upx_byte *ppi = oimport;  // preprocessed imports
    for (ic = 0; ic < dllnum; ic++)
    {
        //LE32 *tarr = idlls[ic]->lookupt;
        LE64 *tarr = idlls[ic]->lookupt; //changed to LE64* - Stefan Widmann
#if 0 && ENABLE_THIS_AND_UNCOMPRESSION_WILL_BREAK // FIXME
        if (!*tarr)  // no imports from this dll
            continue;
#endif
        set_le32(ppi,idlls[ic]->npos);
        set_le32(ppi+4,idlls[ic]->iat - rvamin);
        ppi += 8;
        for (; *tarr; tarr++)
            if (*tarr & 0x8000000000000000ULL) //bit 63 indicates ordinal - Stefan Widmann
            {
                if (idlls[ic]->isk32)
                {
                    *ppi++ = 0xfe; // signed + odd parity
                    set_le32(ppi,ptr_diff(ordinals,oimpdlls));
                    ordinals++;
                    ppi += 4;
                }
                else
                {
                    *ppi++ = 0xff;
                    set_le16(ppi,*tarr & 0xffff);
                    ppi += 2;
                }
            }
            else
            {
                *ppi++ = 1;
                unsigned len = strlen(ibuf + *tarr + 2) + 1;
                memcpy(ppi,ibuf + *tarr + 2,len);
                ppi += len;
                names.add(*tarr,len + 2 + 1);
            }
        ppi++;

        unsigned esize = ptr_diff((char *)tarr, (char *)idlls[ic]->lookupt);
        lookups.add(idlls[ic]->lookupt,esize);
        if (ptr_diff(ibuf + idlls[ic]->iat, (char *)idlls[ic]->lookupt))
        {
            memcpy(ibuf + idlls[ic]->iat, idlls[ic]->lookupt, esize);
            iats.add(idlls[ic]->iat,esize);
        }
        names.add(idlls[ic]->name,strlen(idlls[ic]->name) + 1 + 1);
    }
    ppi += 4;
    assert(ppi < oimport+soimport);
    soimport = ptr_diff(ppi,oimport);

    if (soimport == 4)
        soimport = 0;

    //OutputFile::dump("x0.imp", oimport, soimport);
    //OutputFile::dump("x1.imp", oimpdlls, soimpdlls);

    unsigned ilen = 0;
    names.flatten();
    if (names.ivnum > 1)
    {
        // The area occupied by the dll and imported names is not continuous
        // so to still support uncompression, I can't zero the iat area.
        // This decreases compression ratio, so FIXME somehow.
        infoWarning("can't remove unneeded imports");
        ilen += sizeof(import_desc) * dllnum;
#if defined(DEBUG)
        if (opt->verbose > 3)
            names.dump();
#endif
        // do some work for the unpacker
        im = im_save;
        for (ic = 0; ic < dllnum; ic++, im++)
        {
            memset(im,FILLVAL,sizeof(*im));
            im->dllname = ptr_diff(dlls[idlls[ic]->original_position].name,ibuf);
        }
    }
    else
    {
        iats.add(im_save,sizeof(import_desc) * dllnum);
        // zero unneeded data
        iats.clear();
        lookups.clear();
    }
    names.clear();

    iats.add(&names);
    iats.add(&lookups);
    iats.flatten();
    for (ic = 0; ic < iats.ivnum; ic++)
        ilen += iats.ivarr[ic].len;

    info("Imports: original size: %u bytes, preprocessed size: %u bytes",ilen,soimport);
    return names.ivnum == 1 ? names.ivarr[0].start : 0;
}

/*************************************************************************
// TLS handling
**************************************************************************/

// thanks for theowl for providing me some docs, so that now I understand
// what I'm doing here :)

// 1999-10-17: this was tricky to find:
// when the fixup records and the tls area are on the same page, then
// the tls area is not relocated, because the relocation is done by
// the virtual memory manager only for pages which are not yet loaded.
// of course it was impossible to debug this ;-)

#define TLS_CB_ALIGNMENT 8u   // alignment of tls callbacks

__packed_struct(tls)
    LE64 datastart; // VA tls init data start
    LE64 dataend;   // VA tls init data end
    LE64 tlsindex;  // VA tls index
    LE64 callbacks; // VA tls callbacks
    char _[8];      // zero init, characteristics
__packed_struct_end()

void PackW64Pep::processTls(Interval *iv) // pass 1
{
    COMPILE_TIME_ASSERT(sizeof(tls) == 40) //size of TLS structure is 40 byte now
    COMPILE_TIME_ASSERT_ALIGNED1(tls)

    if ((sotls = ALIGN_UP(IDSIZE(PEDIR_TLS),4)) == 0)
        return;

    const tls * const tlsp = (const tls*) (ibuf + IDADDR(PEDIR_TLS));
    // note: TLS callbacks are not implemented in Windows 95/98/ME
    if (tlsp->callbacks)
    {
        if (tlsp->callbacks < ih.imagebase)
            throwCantPack("invalid TLS callback");
        else if (tlsp->callbacks - ih.imagebase + 4 >= ih.imagesize)
            throwCantPack("invalid TLS callback");
        upx_uint64_t v = get_le64(ibuf + (tlsp->callbacks - ih.imagebase));
        if (v != 0)
        {
            //count number of callbacks, just for information string - Stefan Widmann
            unsigned num_callbacks = 0;
            unsigned callback_offset = 0;
            while(get_le64(ibuf + (tlsp->callbacks - ih.imagebase) + callback_offset))
            {
                //increment number of callbacks
                num_callbacks++;
                //increment pointer by 8
                callback_offset += 8;
            }
            info("TLS: %u callback(s) found, adding TLS callback handler", num_callbacks);
            //set flag to include necessary sections in loader
            use_tls_callbacks = true;
            //define linker symbols
            tlscb_ptr = tlsp->callbacks;
        }
    }

    const unsigned tlsdatastart = tlsp->datastart - ih.imagebase;
    const unsigned tlsdataend = tlsp->dataend - ih.imagebase;

    // now some ugly stuff: find the relocation entries in the tls data area
    unsigned pos,type;
    Reloc rel(ibuf + IDADDR(PEDIR_RELOC),IDSIZE(PEDIR_RELOC));
    while (rel.next(pos,type))
        if (pos >= tlsdatastart && pos < tlsdataend)
            iv->add(pos,type);

    sotls = sizeof(tls) + tlsdataend - tlsdatastart;
    // if TLS callbacks are used, we need two more QWORDS at the end of the TLS
    // ... and those qwords should be correctly aligned
    if (use_tls_callbacks)
        sotls = ALIGN_UP(sotls, TLS_CB_ALIGNMENT) + 16;

    // the PE loader wants this stuff uncompressed
    otls = new upx_byte[sotls];
    memset(otls,0,sotls);
    memcpy(otls,ibuf + IDADDR(PEDIR_TLS), sizeof(tls));
    // WARNING: this can acces data in BSS
    memcpy(otls + sizeof(tls),ibuf + tlsdatastart,sotls - sizeof(tls));
    tlsindex = tlsp->tlsindex - ih.imagebase;
    info("TLS: %u bytes tls data and %u relocations added",sotls - (unsigned) sizeof(tls) - (use_tls_callbacks ? 16 : 0),iv->ivnum);

    // makes sure tls index is zero after decompression
    if (tlsindex && tlsindex < ih.imagesize)
        set_le32(ibuf + tlsindex, 0);
}

void PackW64Pep::processTls(Reloc *rel,const Interval *iv,unsigned newaddr) // pass 2
{
    if (sotls == 0)
        return;
    // add new relocation entries
    unsigned ic;
    for (ic = 0; ic < (use_tls_callbacks ? 32u : 24u); ic += 8)
        rel->add(newaddr + ic,10);

    tls * const tlsp = (tls*) otls;
    // now the relocation entries in the tls data area


    // FIXME check this code below!!!
    for (ic = 0; ic < iv->ivnum; ic += 4)
    {
        void *p = otls + iv->ivarr[ic].start - (tlsp->datastart - ih.imagebase) + sizeof(tls);
        upx_uint64_t kc = get_le64(p); //changed to LE64 - Stefan Widmann
        if (kc < tlsp->dataend && kc >= tlsp->datastart)
        {
            kc +=  newaddr + sizeof(tls) - tlsp->datastart;
            set_le64(p,kc + ih.imagebase); //changed to LE64 - Stefan Widmann
            rel->add(kc,iv->ivarr[ic].len);
        }
        else
            rel->add(kc - ih.imagebase,iv->ivarr[ic].len);
    }

    const unsigned tls_data_size = tlsp->dataend - tlsp->datastart;
    tlsp->datastart = newaddr + sizeof(tls) + ih.imagebase;
    tlsp->dataend = tlsp->datastart + tls_data_size;

    tlsp->callbacks = (use_tls_callbacks ? newaddr + sotls + ih.imagebase - 16 : 0);

    if (use_tls_callbacks)
    {
        //set handler offset
        set_le64(otls + sotls - 16, tls_handler_offset + ih.imagebase);
        //add relocation for TLS handler offset
        rel->add(newaddr + sotls - 16, 10);
    }
}


/*************************************************************************
// Load Configuration handling
**************************************************************************/

void PackW64Pep::processLoadConf(Interval *iv) // pass 1
{
    if (IDSIZE(PEDIR_LOADCONF) == 0)
        return;

    const unsigned lcaddr = IDADDR(PEDIR_LOADCONF);
    const upx_byte * const loadconf = ibuf + lcaddr;
    soloadconf = get_le32(loadconf);
    if (soloadconf == 0)
        return;
    if (soloadconf > 256)
        throwCantPack("size of Load Configuration directory unexpected");

    // if there were relocation entries referring to the load config table
    // then we need them for the copy of the table too
    unsigned pos,type;
    Reloc rel(ibuf + IDADDR(PEDIR_RELOC), IDSIZE(PEDIR_RELOC));
    while (rel.next(pos, type))
        if (pos >= lcaddr && pos < lcaddr + soloadconf)
        {
            iv->add(pos - lcaddr, type);
            // printf("loadconf reloc detected: %x\n", pos);
        }

    oloadconf = new upx_byte[soloadconf];
    memcpy(oloadconf, loadconf, soloadconf);
}

void PackW64Pep::processLoadConf(Reloc *rel, const Interval *iv,
                                unsigned newaddr) // pass2
{
    // now we have the address of the new load config table
    // so we can create the new relocation entries
    for (unsigned ic = 0; ic < iv->ivnum; ic++)
    {
        rel->add(iv->ivarr[ic].start + newaddr, iv->ivarr[ic].len);
        //printf("loadconf reloc added: %x %d\n",
        //       iv->ivarr[ic].start + newaddr, iv->ivarr[ic].len);
    }
}


/*************************************************************************
// pack
**************************************************************************/

bool PackW64Pep::canPack()
{
    //just check if machine type is 0x8664
    if (!readFileHeader() || ih.cpu != 0x8664)   // CPU magic of AMD64 is 0x8664
        return false;
    return true;
}


void PackW64Pep::buildLoader(const Filter *ft)
{
    // recompute tlsindex (see pack() below)
    unsigned tmp_tlsindex = tlsindex;
    const unsigned oam1 = ih.objectalign - 1;
    const unsigned newvsize = (ph.u_len + rvamin + ph.overlap_overhead + oam1) &~ oam1;
    if (tlsindex && ((newvsize - ph.c_len - 1024 + oam1) &~ oam1) > tlsindex + 4)
        tmp_tlsindex = 0;

    // prepare loader
    initLoader(stub_amd64_win64_pep, sizeof(stub_amd64_win64_pep), 2);
    addLoader("START");
    if (ih.entry && isdll)
        addLoader("PEISDLL0");
    addLoader(isdll ? "PEISDLL1" : "",
              "PEMAIN01",
              icondir_count > 1 ? (icondir_count == 2 ? "PEICONS1" : "PEICONS2") : "",
              tmp_tlsindex ? "PETLSHAK" : "",
              "PEMAIN02",
              //ph.first_offset_found == 1 ? "PEMAIN03" : "",
              M_IS_LZMA(ph.method) ? "LZMA_HEAD,LZMA_ELF00,LZMA_DEC20,LZMA_TAIL" :
              M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B" :
              M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D" : "NRV_HEAD,NRV2E",
              //getDecompressorSections(),
              /*multipass ? "PEMULTIP" :  */  "",
              "PEMAIN10",
              NULL
             );
    addLoader(tmp_tlsindex ? "PETLSHAK2" : "");
    if (ft->id)
    {
        const unsigned texv = ih.codebase - rvamin;
        assert(ft->calls > 0);
        addLoader(texv ? "PECTTPOS" : "PECTTNUL",NULL);
        addLoader("PEFILTER49");
    }
    if (soimport)
        addLoader("PEIMPORT",
                  importbyordinal ? "PEIBYORD" : "",
                  kernel32ordinal ? "PEK32ORD" : "",
                  importbyordinal ? "PEIMORD1" : "",
                  "PEIMPOR2",
                  isdll ? "PEIERDLL" : "PEIEREXE",
                  "PEIMDONE",
                  NULL
                 );
    if (sorelocs)
    {
        addLoader(soimport == 0 || soimport + cimports != crelocs ? "PERELOC1" : "PERELOC2",
                  "PERELOC3",
                  big_relocs ? "REL64BIG" : "",
                  "RELOC64J",
                  NULL
                 );
        if (0)
        {
            addLoader(big_relocs&6 ? "PERLOHI0" : "",
                      big_relocs&4 ? "PERELLO0" : "",
                      big_relocs&2 ? "PERELHI0" : "",
                      NULL
                     );
        }
    }
    if (use_dep_hack)
        addLoader("PEDEPHAK", NULL);

    //NEW: TLS callback support PART 1, the callback handler installation - Stefan Widmann
    if(use_tls_callbacks)
        addLoader("PETLSC", NULL);

    addLoader("PEMAIN20", NULL);
    if (use_clear_dirty_stack)
        addLoader("CLEARSTACK", NULL);
    addLoader("PEMAIN21", NULL);

    if (ih.entry && isdll)
        addLoader("PEISDLL9");
    addLoader(ih.entry ? "PEDOJUMP" : "PERETURN", NULL);

    //NEW: TLS callback support PART 2, the callback handler - Stefan Widmann
    if(use_tls_callbacks)
        addLoader("PETLSC2", NULL);

    addLoader("IDENTSTR,UPX1HEAD", NULL);
}


void PackW64Pep::pack(OutputFile *fo)
{
    // FIXME: we need to think about better support for --exact
    if (opt->exact)
        throwCantPackExact();

    const unsigned objs = ih.objects;
    isection = new pe_section_t[objs];
    fi->seek(pe_offset+sizeof(ih),SEEK_SET);
    fi->readx(isection,sizeof(pe_section_t)*objs);

    rvamin = isection[0].vaddr;

    infoHeader("[Processing %s, format %s, %d sections]", fn_basename(fi->getName()), getName(), objs);

    // check the PE header
    // FIXME: add more checks
    // subsystem check moved to switch ... case below
    if (!opt->force && (
                        (ih.cpu != 0x8664)  //CPU magic of AMD64 is 0x8664
                        || (ih.opthdrsize != 0xF0) //optional header size is 0xF0 in PE32+ files - Stefan Widmann
                        || (ih.coffmagic != 0x20B) //COFF magic is 0x20B in PE+ files, 0x10B in "normal" 32 bit PE files - Stefan Widmann
                        || ((ih.flags & EXECUTABLE) == 0)
                        || ((ih.flags & BITS_32_MACHINE) == 1) //NEW: 32 bit machine flag may not be set - Stefan Widmann
                        || (ih.entry == 0 && !isdll)
                        || (ih.ddirsentries != 16)
                       ))
        throwCantPack("unexpected value in PE header (try --force)");

    switch(ih.subsystem)  //let's take a closer look at the subsystem
    {
    case 1: //NATIVE
        {
            throwCantPack("win64/native applications are not yet supported");
            break;
        }
    case 2: //GUI
        {
            //fine, continue
            break;
        }
    case 3: //CONSOLE
        {
            //fine, continue
            break;
        }
    case 5: //OS2 - are there any x64/os2 files?
        {
            throwCantPack("win64/os2 files are not yet supported");
            break;
        }
    case 7: //POSIX - do win64/posix files exist?
        {
            throwCantPack("win64/posix files are not yet supported");
            break;
        }
    case 9: //WINCE - same question: do they exist?
        {
            throwCantPack("PE32+/wince files are not yet supported");
            break;
        }
    case 10: //EFI APPLICATION
        {
            throwCantPack("PE32+/EFIapplication files are not yet supported");
            break;
        }
    case 11: //EFI BOOT SERVICE DRIVER
        {
            throwCantPack("PE32+/EFIbootservicedriver files are not yet supported");
            break;
        }
    case 12: //EFI RUNTIME DRIVER
        {
            throwCantPack("PE32+/EFIruntimedriver files are not yet supported");
            break;
        }
    case 13: //EFI ROM
        {
            throwCantPack("PE32+/EFIROM files are not yet supported");
            break;
        }
    case 14: //XBOX - will there ever be PE32+ xbox files?
        {
            throwCantPack("PE32+/xbox files are not yet supported");
            break;
        }
    case 16: //WINDOWS BOOT APPLICATION
        {
            throwCantPack("win64/windowsbootapplication files are not yet supported");
            break;
        }
    default: //UNKNOWN SUBSYSTEM
        {
            throwCantPack("PE32+/? unknown subsystem");
            break;
        }
    }

    //remove certificate pointers from directory table
    if (IDSIZE(PEDIR_SEC))
        IDSIZE(PEDIR_SEC) = IDADDR(PEDIR_SEC) = 0;

    //check if we have a CLR Runtime Header pointer
    if (IDSIZE(PEDIR_COMRT))
        throwCantPack(".NET files (win64/.net) are not yet supported");

    //FIXME: Relocation stripping disabled yet - Stefan Widmann
    opt->win32_pe.strip_relocs = false;
#if 0 //removed - Stefan Widmann
    if (isdll)
        opt->win32_pe.strip_relocs = false;
    else if (opt->win32_pe.strip_relocs < 0)
        opt->win32_pe.strip_relocs = (ih.imagebase >= 0x0000000140000000ULL);
    if (opt->win32_pe.strip_relocs)
    {
        if (ih.imagebase < 0x0000000140000000ULL)
            throwCantPack("--strip-relocs is not allowed when imagebase < 0x0000000140000000");
        else
            ih.flags |= RELOCS_STRIPPED;
    }
#endif

    //check if first section's name is "UPX"
    if (memcmp(isection[0].name,"UPX",3) == 0)
        throwAlreadyPackedByUPX();
    if (!opt->force && IDSIZE(15))
        throwCantPack("file is possibly packed/protected (try --force)");
    if (ih.entry && ih.entry < rvamin)
        throwCantPack("run a virus scanner on this file!");
#if 0 //subsystem check moved to switch ... case above - Stefan Widmann
    if (!opt->force && ih.subsystem == 1)
        throwCantPack("subsystem 'native' is not supported (try --force)");
#endif
    if (ih.filealign < 0x200)
        throwCantPack("filealign < 0x200 is not yet supported");

    handleStub(fi,fo,pe_offset);
    const unsigned usize = ih.imagesize;
    const unsigned xtrasize = UPX_MAX(ih.datasize, 65536u) + IDSIZE(PEDIR_IMPORT) + IDSIZE(PEDIR_BOUNDIM) + IDSIZE(PEDIR_IAT) + IDSIZE(PEDIR_DELAYIMP) + IDSIZE(PEDIR_RELOC);
    ibuf.alloc(usize + xtrasize);

    // BOUND IMPORT support. FIXME: is this ok?
    fi->seek(0,SEEK_SET);
    fi->readx(ibuf,isection[0].rawdataptr);

    Interval holes(ibuf);

    unsigned ic,jc,overlaystart = 0;
    ibuf.clear(0, usize);
    for (ic = jc = 0; ic < objs; ic++)
    {
        if (isection[ic].rawdataptr && overlaystart < isection[ic].rawdataptr + isection[ic].size)
            overlaystart = ALIGN_UP(isection[ic].rawdataptr + isection[ic].size,ih.filealign);
        if (isection[ic].vsize == 0)
            isection[ic].vsize = isection[ic].size;
        if ((isection[ic].flags & PEFL_BSS) || isection[ic].rawdataptr == 0
            || (isection[ic].flags & PEFL_INFO))
        {
            holes.add(isection[ic].vaddr,isection[ic].vsize);
            continue;
        }
        if (isection[ic].vaddr + isection[ic].size > usize)
            throwCantPack("section size problem");
        if (!isrtm && ((isection[ic].flags & (PEFL_WRITE|PEFL_SHARED))
            == (PEFL_WRITE|PEFL_SHARED)))
            if (!opt->force)
                throwCantPack("writable shared sections not supported (try --force)");
        if (jc && isection[ic].rawdataptr - jc > ih.filealign)
            throwCantPack("superfluous data between sections");
        fi->seek(isection[ic].rawdataptr,SEEK_SET);
        jc = isection[ic].size;
        if (jc > isection[ic].vsize)
            jc = isection[ic].vsize;
        if (isection[ic].vsize == 0) // hack for some tricky programs - may this break other progs?
            jc = isection[ic].vsize = isection[ic].size;
        if (isection[ic].vaddr + jc > ibuf.getSize())
            throwInternalError("buffer too small 1");
        fi->readx(ibuf + isection[ic].vaddr,jc);
        jc += isection[ic].rawdataptr;
    }

#if 0 //Stefan Widmann: still necessary?
    // check for NeoLite
    if (find(ibuf + ih.entry, 64+7, "NeoLite", 7) >= 0)
        throwCantPack("file is already compressed with another packer");
#endif

    unsigned overlay = file_size - stripDebug(overlaystart);
    if (overlay >= (unsigned) file_size)
    {
#if 0
        if (overlay < file_size + ih.filealign)
            overlay = 0;
        else if (!opt->force)
            throwNotCompressible("overlay problem (try --force)");
#endif
        overlay = 0;
    }
    checkOverlay(overlay);

    Resource res;
    Interval tlsiv(ibuf);
    Interval loadconfiv(ibuf);
    Export xport((char*)(unsigned char*)ibuf);

    const unsigned dllstrings = processImports();
    processTls(&tlsiv); // call before processRelocs!!
    processLoadConf(&loadconfiv);
    processResources(&res);
    processExports(&xport);
    processRelocs();

    //OutputFile::dump("x1", ibuf, usize);

    // some checks for broken linkers - disable filter if necessary
    bool allow_filter = true;
    if (ih.codebase + ih.codesize > ih.imagesize
        || (isection[virta2objnum(ih.codebase,isection,objs)].flags & PEFL_CODE) == 0)
        allow_filter = false;

    const unsigned oam1 = ih.objectalign - 1;

    // FIXME: disabled: the uncompressor would not allocate enough memory
    //objs = tryremove(IDADDR(PEDIR_RELOC),objs);

    // FIXME: if the last object has a bss then this won't work
    // newvsize = (isection[objs-1].vaddr + isection[objs-1].size + oam1) &~ oam1;
    // temporary solution:
    unsigned newvsize = (isection[objs-1].vaddr + isection[objs-1].vsize + oam1) &~ oam1;

    //fprintf(stderr,"newvsize=%x objs=%d\n",newvsize,objs);
    if (newvsize + soimport + sorelocs > ibuf.getSize())
         throwInternalError("buffer too small 2");
    memcpy(ibuf+newvsize,oimport,soimport);
    memcpy(ibuf+newvsize+soimport,orelocs,sorelocs);

    cimports = newvsize - rvamin;   // rva of preprocessed imports
    crelocs = cimports + soimport;  // rva of preprocessed fixups

    ph.u_len = newvsize + soimport + sorelocs;

    // some extra data for uncompression support
    unsigned s = 0;
    upx_byte * const p1 = ibuf + ph.u_len;
    memcpy(p1 + s,&ih,sizeof (ih));
    s += sizeof (ih);
    memcpy(p1 + s,isection,ih.objects * sizeof(*isection));
    s += ih.objects * sizeof(*isection);
    if (soimport)
    {
        set_le32(p1 + s,cimports);
        set_le32(p1 + s + 4,dllstrings);
        s += 8;
    }
    if (sorelocs)
    {
        set_le32(p1 + s,crelocs);
        p1[s + 4] = (unsigned char) (big_relocs & 6);
        s += 5;
    }
    if (soresources)
    {
        set_le16(p1 + s,icondir_count);
        s += 2;
    }
    // end of extra data
    set_le32(p1 + s,ptr_diff(p1,ibuf) - rvamin);
    s += 4;
    ph.u_len += s;
    obuf.allocForCompression(ph.u_len);

    // prepare packheader
    ph.u_len -= rvamin;
    // prepare filter
    Filter ft(ph.level);
    ft.buf_len = ih.codesize;
    ft.addvalue = ih.codebase - rvamin;
    // compress
    int filter_strategy = allow_filter ? 0 : -3;

    // disable filters for files with broken headers
    if (ih.codebase + ih.codesize > ph.u_len)
    {
        ft.buf_len = 1;
        filter_strategy = -3;
    }

    compressWithFilters(&ft, 2048, NULL_cconf, filter_strategy,
                        ih.codebase, rvamin, 0, NULL, 0);
// info: see buildLoader()
    newvsize = (ph.u_len + rvamin + ph.overlap_overhead + oam1) &~ oam1;
    if (tlsindex && ((newvsize - ph.c_len - 1024 + oam1) &~ oam1) > tlsindex + 4)
        tlsindex = 0;

    int identsize = 0;
    const unsigned codesize = getLoaderSection("IDENTSTR",&identsize);
    assert(identsize > 0);
    getLoaderSection("UPX1HEAD",(int*)&ic);
    identsize += ic;

    pe_section_t osection[3];
    // section 0 : bss
    //         1 : [ident + header] + packed_data + unpacker + tls + loadconf
    //         2 : not compressed data

    // section 2 should start with the resource data, because lots of lame
    // windoze codes assume that resources starts on the beginning of a section

    // note: there should be no data in section 2 which needs fixup

    // identsplit - number of ident + (upx header) bytes to put into the PE header
    int identsplit = pe_offset + sizeof(osection) + sizeof(oh);
    if ((identsplit & 0x1ff) == 0)
        identsplit = 0;
    else if (((identsplit + identsize) ^ identsplit) < 0x200)
        identsplit = identsize;
    else
        identsplit = ALIGN_GAP(identsplit, 0x200);
    ic = identsize - identsplit;

    const unsigned c_len = ((ph.c_len + ic) & 15) == 0 ? ph.c_len : ph.c_len + 16 - ((ph.c_len + ic) & 15);
    obuf.clear(ph.c_len, c_len - ph.c_len);

    const unsigned s1size = ALIGN_UP(ic + c_len + codesize,8u) + sotls + soloadconf;
    const unsigned s1addr = (newvsize - (ic + c_len) + oam1) &~ oam1;

    const unsigned ncsection = (s1addr + s1size + oam1) &~ oam1;
    const unsigned upxsection = s1addr + ic + c_len;
    const unsigned myimport = ncsection + soresources - rvamin;

    // patch loader
    linker->defineSymbol("original_entry", ih.entry);
    if (use_dep_hack)
    {
        // This works around a "protection" introduced in MSVCRT80, which
        // works like this:
        // When the compiler detects that it would link in some code from its
        // C runtime library which references some data in a read only
        // section then it compiles in a runtime check whether that data is
        // still in a read only section by looking at the pe header of the
        // file. If this check fails the runtime does "interesting" things
        // like not running the floating point initialization code - the result
        // is a R6002 runtime error.
        // These supposed to be read only addresses are covered by the sections
        // UPX0 & UPX1 in the compressed files, so we have to patch the PE header
        // in the memory. And the page on which the PE header is stored is read
        // only so we must make it rw, fix the flags (i.e. clear
        // PEFL_WRITE of osection[x].flags), and make it ro again.

        // rva of the most significant byte of member "flags" in section "UPX0"
        const unsigned swri = pe_offset + sizeof(oh) + sizeof(pe_section_t) - 1;
        // make sure we only touch the minimum number of pages
        const unsigned addr = 0u - rvamin + swri;
        linker->defineSymbol("swri", addr &  0xfff);    // page offset
        // check whether osection[0].flags and osection[1].flags
        // are on the same page
        linker->defineSymbol("vp_size", ((addr & 0xfff) + 0x28 >= 0x1000) ?
                             0x2000 : 0x1000);          // 2 pages or 1 page
        linker->defineSymbol("vp_base", addr &~ 0xfff); // page mask
        //NEW: offset of import adjusted - Stefan Widmann
        linker->defineSymbol("VirtualProtect", myimport + get_le32(oimpdlls + 16) + 16);
    }
// FIXME    linker->defineSymbol("reloc_delt", 0u - (unsigned) (ih.imagebase - rvamin));
    linker->defineSymbol("start_of_relocs", crelocs);
    //NEW: offset of import adjusted - Stefan Widmann
    linker->defineSymbol("ExitProcess", myimport + get_le32(oimpdlls + 16) + 40);
    //NEW: offset of import adjusted - Stefan Widmann
    linker->defineSymbol("GetProcAddress", myimport + get_le32(oimpdlls + 16) + 8);
    linker->defineSymbol("kernel32_ordinals", myimport);
    //NEW: offset of import adjusted - Stefan Widmann
    linker->defineSymbol("LoadLibraryA", myimport + get_le32(oimpdlls + 16));
    linker->defineSymbol("start_of_imports", myimport);
    linker->defineSymbol("compressed_imports", cimports);
#if 0
    //NEW: offset of import adjusted - Stefan Widmann
    linker->defineSymbol("VirtualAlloc", myimport + get_le32(oimpdlls + 16) + 24);
    //NEW: offset of import adjusted - Stefan Widmann
    linker->defineSymbol("VirtualFree", myimport + get_le32(oimpdlls + 16) + 32);
#endif

    if (M_IS_LZMA(ph.method))
    {
        linker->defineSymbol("lzma_c_len", ph.c_len - 2);
        linker->defineSymbol("lzma_u_len", ph.u_len);
    }
    defineFilterSymbols(&ft);
    linker->defineSymbol("filter_buffer_start", ih.codebase - rvamin);

    // in case of overlapping decompression, this hack is needed,
    // because windoze zeroes the word pointed by tlsindex before
    // it starts programs
    linker->defineSymbol("tls_value", (tlsindex + 4 > s1addr) ?
                         get_le32(obuf + tlsindex - s1addr - ic) : 0);
    linker->defineSymbol("tls_address", tlsindex - rvamin);

    linker->defineSymbol("icon_delta", icondir_count - 1);
    linker->defineSymbol("icon_offset", ncsection + icondir_offset - rvamin);

    const unsigned esi0 = s1addr + ic;
    linker->defineSymbol("start_of_uncompressed", 0u - esi0 + rvamin);
    linker->defineSymbol("start_of_compressed", esi0);
    //NEW: TLS callback support - Stefan Widmann
    ic = s1addr + s1size - sotls - soloadconf; //moved here, we need the address of the new TLS!
    if (use_tls_callbacks)
    {
        linker->defineSymbol("tls_callbacks_ptr", tlscb_ptr - ih.imagebase);
        linker->defineSymbol("tls_module_base", 0u - rvamin);
    }

    linker->defineSymbol("START", upxsection);
    //linker->dumpSymbols();
    relocateLoader();

    const unsigned lsize = getLoaderSize();
    MemBuffer loader(lsize);
    memcpy(loader,getLoader(),lsize);
    patchPackHeader(loader, lsize);

    Reloc rel(1024); // new relocations are put here

    // new PE header
    memcpy(&oh,&ih,sizeof(oh));
    oh.filealign = 0x200; // identsplit depends on this
    memset(osection,0,sizeof(osection));

    oh.entry = upxsection;
    oh.objects = 3;
    oh.chksum = 0;

    // fill the data directory
    ODADDR(PEDIR_DEBUG) = 0;  //remove debug data
    ODSIZE(PEDIR_DEBUG) = 0;
    ODADDR(PEDIR_IAT) = 0;  //remove IAT pointer
    ODSIZE(PEDIR_IAT) = 0;
    ODADDR(PEDIR_BOUNDIM) = 0;  //remove bound import table
    ODSIZE(PEDIR_BOUNDIM) = 0;

    // tls & loadconf are put into section 1

    //ic = s1addr + s1size - sotls - soloadconf;  //ATTENTION: moved upwards to TLS callback handling - Stefan Widmann
    if (use_tls_callbacks)
        tls_handler_offset = linker->getSymbolOffset("PETLSC2");

    processTls(&rel,&tlsiv,ic);
    ODADDR(PEDIR_TLS) = sotls ? ic : 0;
    ODSIZE(PEDIR_TLS) = sotls ? 0x28 : 0;  // size of TLS is 0x28 in PE32+ - Stefan Widmann
    ic += sotls;

    processLoadConf(&rel, &loadconfiv, ic);
    ODADDR(PEDIR_LOADCONF) = soloadconf ? ic : 0;
    ODSIZE(PEDIR_LOADCONF) = soloadconf;
    ic += soloadconf;

    // these are put into section 2

    ic = ncsection;
    if (soresources)
        processResources(&res,ic);
    ODADDR(PEDIR_RESOURCE) = soresources ? ic : 0;
    ODSIZE(PEDIR_RESOURCE) = soresources;
    ic += soresources;

    processImports(ic, 0);
    ODADDR(PEDIR_IMPORT) = ic;
    ODSIZE(PEDIR_IMPORT) = soimpdlls;
    ic += soimpdlls;

    processExports(&xport,ic);
    ODADDR(PEDIR_EXPORT) = soexport ? ic : 0;
    ODSIZE(PEDIR_EXPORT) = soexport;
    if (!isdll && opt->win32_pe.compress_exports)
    {
        ODADDR(PEDIR_EXPORT) = IDADDR(PEDIR_EXPORT);
        ODSIZE(PEDIR_EXPORT) = IDSIZE(PEDIR_EXPORT);
    }
    ic += soexport;

    processRelocs(&rel);
    ODADDR(PEDIR_RELOC) = soxrelocs ? ic : 0;
    ODSIZE(PEDIR_RELOC) = soxrelocs;
    ic += soxrelocs;

    // this is computed here, because soxrelocs changes some lines above
    const unsigned ncsize = soresources + soimpdlls + soexport + soxrelocs;
    ic = oh.filealign - 1;

    // this one is tricky: it seems windoze touches 4 bytes after
    // the end of the relocation data - so we have to increase
    // the virtual size of this section
    const unsigned ncsize_virt_increase = (ncsize & oam1) == 0 ? 8 : 0;

    // fill the sections
    strcpy(osection[0].name,"UPX0");
    strcpy(osection[1].name,"UPX1");
    // after some windoze debugging I found that the name of the sections
    // DOES matter :( .rsrc is used by oleaut32.dll (TYPELIBS)
    // and because of this lame dll, the resource stuff must be the
    // first in the 3rd section - the author of this dll seems to be
    // too idiot to use the data directories... M$ suxx 4 ever!
    // ... even worse: exploder.exe in NiceTry also depends on this to
    // locate version info

    strcpy(osection[2].name,soresources ? ".rsrc" : "UPX2");

    osection[0].vaddr = rvamin;
    osection[1].vaddr = s1addr;
    osection[2].vaddr = ncsection;

    osection[0].size = 0;
    osection[1].size = (s1size + ic) &~ ic;
    osection[2].size = (ncsize + ic) &~ ic;

    osection[0].vsize = osection[1].vaddr - osection[0].vaddr;
    osection[1].vsize = (osection[1].size + oam1) &~ oam1;
    osection[2].vsize = (osection[2].size + ncsize_virt_increase + oam1) &~ oam1;

    osection[0].rawdataptr = (pe_offset + sizeof(oh) + sizeof(osection) + ic) &~ ic;
    osection[1].rawdataptr = osection[0].rawdataptr;
    osection[2].rawdataptr = osection[1].rawdataptr + osection[1].size;

    osection[0].flags = (unsigned) (PEFL_BSS|PEFL_EXEC|PEFL_WRITE|PEFL_READ);
    osection[1].flags = (unsigned) (PEFL_DATA|PEFL_EXEC|PEFL_WRITE|PEFL_READ);
    osection[2].flags = (unsigned) (PEFL_DATA|PEFL_WRITE|PEFL_READ);

    oh.imagesize = osection[2].vaddr + osection[2].vsize;
    oh.bsssize  = osection[0].vsize;
    oh.datasize = osection[2].vsize;
    //oh.database = osection[2].vaddr;  //no base of data in PE32+ files - Stefan Widmann
    oh.codesize = osection[1].vsize;
    oh.codebase = osection[1].vaddr;
    // oh.headersize = osection[0].rawdataptr;
    oh.headersize = rvamin;
    if (rvamin < osection[0].rawdataptr)
        throwCantPack("object alignment too small");

    if (opt->win32_pe.strip_relocs && !isdll)
        oh.flags |= RELOCS_STRIPPED;

    //for (ic = 0; ic < oh.filealign; ic += 4)
    //    set_le32(ibuf + ic,get_le32("UPX "));
    ibuf.clear(0, oh.filealign);

    info("Image size change: %u -> %u KiB",
         ih.imagesize / 1024, oh.imagesize / 1024);

    infoHeader("[Writing compressed file]");

    // write loader + compressed file
    fo->write(&oh,sizeof(oh));
    fo->write(osection,sizeof(osection));
    // some alignment
    if (identsplit == identsize)
    {
        unsigned n = osection[0].rawdataptr - fo->getBytesWritten() - identsize;
        assert(n <= oh.filealign);
        fo->write(ibuf, n);
    }
    fo->write(loader + codesize,identsize);
    infoWriting("loader", fo->getBytesWritten());
    fo->write(obuf,c_len);
    infoWriting("compressed data", c_len);
    fo->write(loader,codesize);
    if (opt->debug.dump_stub_loader)
        OutputFile::dump(opt->debug.dump_stub_loader, loader, codesize);
    if ((ic = fo->getBytesWritten() & 7) != 0)
        fo->write(ibuf,8 - ic);
    fo->write(otls,sotls);
    fo->write(oloadconf, soloadconf);
    if ((ic = fo->getBytesWritten() & (oh.filealign-1)) != 0)
        fo->write(ibuf,oh.filealign - ic);
    fo->write(oresources,soresources);
    fo->write(oimpdlls,soimpdlls);
    fo->write(oexport,soexport);
    fo->write(oxrelocs,soxrelocs);

    if ((ic = fo->getBytesWritten() & (oh.filealign-1)) != 0)
        fo->write(ibuf,oh.filealign - ic);

#if 0
    printf("%-13s: program hdr  : %8ld bytes\n", getName(), (long) sizeof(oh));
    printf("%-13s: sections     : %8ld bytes\n", getName(), (long) sizeof(osection));
    printf("%-13s: ident        : %8ld bytes\n", getName(), (long) identsize);
    printf("%-13s: compressed   : %8ld bytes\n", getName(), (long) c_len);
    printf("%-13s: decompressor : %8ld bytes\n", getName(), (long) codesize);
    printf("%-13s: tls          : %8ld bytes\n", getName(), (long) sotls);
    printf("%-13s: resources    : %8ld bytes\n", getName(), (long) soresources);
    printf("%-13s: imports      : %8ld bytes\n", getName(), (long) soimpdlls);
    printf("%-13s: exports      : %8ld bytes\n", getName(), (long) soexport);
    printf("%-13s: relocs       : %8ld bytes\n", getName(), (long) soxrelocs);
    printf("%-13s: loadconf     : %8ld bytes\n", getName(), (long) soloadconf);
#endif

    // verify
    verifyOverlappingDecompression();

    // copy the overlay
    copyOverlay(fo, overlay, &obuf);

#if 0 //REMOVED FOR TESTING! Stefan Widmann
    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
#endif
}

/*************************************************************************
// unpack
**************************************************************************/

int PackW64Pep::canUnpack()
{
    if (!readFileHeader() || ih.cpu != 0x8664)
        return false;

    unsigned objs = ih.objects;
    isection = new pe_section_t[objs];
    fi->seek(pe_offset+sizeof(ih),SEEK_SET);
    fi->readx(isection,sizeof(pe_section_t)*objs);
    if (ih.objects < 3)
        return -1;
    bool is_packed = (ih.objects == 3 &&
                      (IDSIZE(15) || ih.entry > isection[1].vaddr));
    bool found_ph = false;
    if (memcmp(isection[0].name,"UPX",3) == 0)
    {
        // current version
        fi->seek(isection[1].rawdataptr - 64, SEEK_SET);
        found_ph = readPackHeader(1024);
        if (!found_ph)
        {
            // old versions
            fi->seek(isection[2].rawdataptr, SEEK_SET);
            found_ph = readPackHeader(1024);
        }
    }
    if (is_packed && found_ph)
        return true;
    if (!is_packed && !found_ph)
        return -1;
    if (is_packed && ih.entry < isection[2].vaddr)
    {
        unsigned char buf[256];
        bool x = false;

        memset(buf, 0, sizeof(buf));
        try {
            fi->seek(ih.entry - isection[1].vaddr + isection[1].rawdataptr, SEEK_SET);
            fi->read(buf, sizeof(buf));

            static const unsigned char magic[] = "\x8b\x1e\x83\xee\xfc\x11\xdb";
            // mov ebx, [esi];    sub esi, -4;    adc ebx,ebx

            int offset = find(buf, sizeof(buf), magic, 7);
            if (offset >= 0 && find(buf + offset + 1, sizeof(buf) - offset - 1, magic, 7) >= 0)
                x = true;
        } catch (...) {
            //x = true;
        }
        if (x)
            throwCantUnpack("file is modified/hacked/protected; take care!!!");
        else
            throwCantUnpack("file is possibly modified/hacked/protected; take care!");
        return false;   // not reached
    }

    // FIXME: what should we say here ?
    //throwCantUnpack("file is possibly modified/hacked/protected; take care!");
    return false;
}


void PackW64Pep::rebuildImports(upx_byte *& extrainfo)
{
    if (ODADDR(PEDIR_IMPORT) == 0
        || ODSIZE(PEDIR_IMPORT) <= sizeof(import_desc))
        return;

//    const upx_byte * const idata = obuf + get_le32(extrainfo);
    OPTR_C(const upx_byte, idata, obuf + get_le32(extrainfo));
    const unsigned inamespos = get_le32(extrainfo + 4);
    extrainfo += 8;

    unsigned sdllnames = 0;

//    const upx_byte *import = ibuf + IDADDR(PEDIR_IMPORT) - isection[2].vaddr;
//    const upx_byte *p;
    IPTR_I(const upx_byte, import, ibuf + IDADDR(PEDIR_IMPORT) - isection[2].vaddr);
    OPTR(const upx_byte, p);

    for (p = idata; get_le32(p) != 0; ++p)
    {
        const upx_byte *dname = get_le32(p) + import;
        ICHECK(dname, 1);
        const unsigned dlen = strlen(dname);
        ICHECK(dname, dlen + 1);

        sdllnames += dlen + 1;
        for (p += 8; *p;)
            if (*p == 1)
                p += strlen(++p) + 1;
            else if (*p == 0xff)
                p += 3; // ordinal
            else
                p += 5;
    }
    sdllnames = ALIGN_UP(sdllnames, 2u);

    upx_byte * const Obuf = obuf - rvamin;
    import_desc * const im0 = (import_desc*) (Obuf + ODADDR(PEDIR_IMPORT));
    import_desc *im = im0;
    upx_byte *dllnames = Obuf + inamespos;
    upx_byte *importednames = dllnames + sdllnames;
    upx_byte * const importednames_start = importednames;

    for (p = idata; get_le32(p) != 0; ++p)
    {
        // restore the name of the dll
        const upx_byte *dname = get_le32(p) + import;
        ICHECK(dname, 1);
        const unsigned dlen = strlen(dname);
        ICHECK(dname, dlen + 1);

        const unsigned iatoffs = get_le32(p + 4) + rvamin;
        if (inamespos)
        {
            // now I rebuild the dll names
            OCHECK(dllnames, dlen + 1);
            strcpy(dllnames, dname);
            im->dllname = ptr_diff(dllnames,Obuf);
            //;;;printf("\ndll: %s:",dllnames);
            dllnames += dlen + 1;
        }
        else
        {
            OCHECK(Obuf + im->dllname, dlen + 1);
            strcpy(Obuf + im->dllname, dname);
        }
        im->iat = iatoffs;

        OPTR_I(LE64, newiat, (LE64 *) (Obuf + iatoffs));

        // restore the imported names+ordinals
        for (p += 8; *p; ++newiat)
            if (*p == 1)
            {
                const unsigned ilen = strlen(++p) + 1;
                if (inamespos)
                {
                    if (ptr_diff(importednames, importednames_start) & 1)
                        importednames -= 1;
                    omemcpy(importednames + 2, p, ilen);
                    //;;;printf(" %s",importednames+2);
                    *newiat = ptr_diff(importednames, Obuf);
                    importednames += 2 + ilen;
                }
                else
                {
                    OCHECK(Obuf + (*newiat + 2), ilen + 1);
                    strcpy(Obuf + (*newiat + 2), p);
                }
                p += ilen;
            }
            else if (*p == 0xff)
            {
                *newiat = get_le16(p + 1) + (1ULL << 63);
                //;;;printf(" %llx",(unsigned long long)*newiat);
                p += 3;
            }
            else
            {
                *newiat = get_le64(get_le32(p + 1) + import);
                assert(*newiat & (1ULL << 63));
                p += 5;
            }
        *newiat = 0;
        im++;
    }
    //memset(idata,0,p - idata);
}

/*
 extra info added to help uncompression:

 <ih sizeof(pe_head)>
 <pe_section_t objs*sizeof(pe_section_t)>
 <start of compressed imports 4> - optional           \
 <start of the names from uncompressed imports> - opt /
 <start of compressed relocs 4> - optional   \
 <relocation type indicator 1> - optional    /
 <icondir_count 2> - optional
 <offset of extra info 4>
*/


/*
vi:ts=4:et
*/

