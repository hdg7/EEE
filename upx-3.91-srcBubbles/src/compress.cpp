/* compress.cpp --

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
 */


#include "conf.h"
#include "compress.h"
#include "mem.h"
#include "readConfig.h"
#include "BubbleType.h"
#include <zlib.h>
#include <math.h>

/***********************************************************/
/* Attributes for the environment */
int * positions;
int * sizes;
double *densities;
int total_bubbles;
int total_types;
char *pattern;
char *delimiter;
unsigned ori_dst_len;
upx_bytep ori_dst = NULL;

/**********************************************************/
/* Uploading configuration */

#include <math.h>
#include <stdlib.h>

double randn (double mu, double sigma)
{
  double U1, U2, W, mult;
  static double X1, X2;
  static int call = 0;

  if (call == 1)
    {
      call = !call;
      return (mu + sigma * (double) X2);
    }

  do
    {
      U1 = -1 + ((double) rand () / RAND_MAX) * 2;
      U2 = -1 + ((double) rand () / RAND_MAX) * 2;
      W = pow (U1, 2) + pow (U2, 2);
    }
  while (W >= 1 || W == 0);

  mult = sqrt ((-2 * log (W)) / W);
  X1 = U1 * mult;
  X2 = U2 * mult;

  call = !call;

  return (mu + sigma * (double) X1);
}

void upload_configuration(unsigned compressArea,BubbleType ***bubbles)
{
	char line[LINE_L];
	char folder_name[LINE_L] = "Bubbles";
	char totalTypes_name[LINE_L] = "total";
	int i,j;
//	char pattern_name[LINE_L] = "pattern";
//       	char total_name[LINE_L] = "total";
      	char delimiter_name[LINE_L] = "delimiter";

//       	readConfigurationS(folder_name,pattern_name,line,LINE_L);
//	pattern = (char *) malloc((strlen(line)+1)*sizeof(char));
//	strcpy(pattern,line);
	total_bubbles = 0;
	total_types = 0;
        readConfigurationI(folder_name,totalTypes_name,&total_types);
	*bubbles = (BubbleType**) malloc(sizeof(BubbleType*)*(total_types));
	for(i=0; i< total_types;i++)
	{
		(*bubbles)[i] = new BubbleType(i);
		(*bubbles)[i]->InitializeBubbles(compressArea);
		total_bubbles += (*bubbles)[i]->getBubbleNumber();
	}

	positions = (int *) malloc(sizeof(int)*total_bubbles);
	sizes = (int * ) malloc(sizeof(int)*total_bubbles);
	densities = (double *) malloc(sizeof(double)*total_bubbles);
	total_bubbles = 0;
	for(i=0; i< total_types;i++)
        {
                for(j=0;j < (*bubbles)[i]->getBubbleNumber();j++)
                {

                        sizes[total_bubbles+j] = (*bubbles)[i]->getBubbleSize();
                        positions[total_bubbles+j] = (*bubbles)[i]->getBubblePosition(j);
			densities[total_bubbles+j] =(*bubbles)[i]->getBubbleDensity();
                }
               	total_bubbles += (*bubbles)[i]->getBubbleNumber();
        }


        readConfigurationS(folder_name,delimiter_name,line,LINE_L);
	delimiter = (char *) malloc((strlen(line)+1)*sizeof(char));
	strcpy(delimiter,line);
}

void swap(int *i, int *j)
{
        int middle;
        middle = *i;
        *i =*j;
        *j=middle;
}

int *mybubble(int * list, int size)
{
	int *aux_list;

	aux_list = (int *) malloc(sizeof(int)*size);
	for(int i = 0 ; i < size; i++)
		aux_list[i] = i;

	for(int i =0; i < size -1; i++)
	{
		for(int j = 0; j< size-i-1; j++)
		{
			if(list[j] > list[j+1])
			{
				swap(&list[j], &list[j+1]);
                                swap(&aux_list[j], &aux_list[j+1]);
			}
		}

	}
	return aux_list;
}

int normalizeSize(int availOffset)
{
       	int totalSize=0;
       	int i;

       	for(i=0; i < total_bubbles; i++)
        {
		if(positions[i]>0)
                	totalSize += sizes[i];
		else
			sizes[i] = 0;
	}
	for(i=0; i < total_bubbles; i++)
        {
                sizes[i] = (int) ((unsigned int) (sizes[i]*availOffset)/totalSize);
        }
	return 0;
}

int setDensity(int size, upx_bytep buffer,double density)
{
	int i=0;
	unsigned char byte_sequence[256];
	int byte_sequence_length = ((int) 256.0*density);
	for (i = 0; i < 256; i++)
	{
		byte_sequence[i]=rand()%256;
	}
	memcpy(buffer, delimiter,
                       	sizeof(unsigned char)*strlen(delimiter));
	size -=sizeof(unsigned char)*strlen(delimiter);
	buffer +=sizeof(unsigned char)*strlen(delimiter);

        for (i=0; i < (int) (size-( (int) sizeof(unsigned char))*strlen(delimiter))/byte_sequence_length; ++i)
                memcpy(buffer + i * byte_sequence_length, byte_sequence,
			sizeof(unsigned char)*byte_sequence_length);
	memcpy(buffer + i * byte_sequence_length, byte_sequence,
		(size-sizeof(unsigned char)*strlen(delimiter))%byte_sequence_length);
	memcpy(buffer + size - sizeof(unsigned char)*strlen(delimiter),delimiter,
		sizeof(unsigned char)*strlen(delimiter));
	return 0;
}
/*
int setPosition(int * positions, int totalTypes, int compressArea,
	BubbleTypes* bubbles)
{
	int position;
	int i,j,counter=0;

	for (i = 0; i < totalTypes; i++)
	{
		for(j=0; j < bubbles[i].getTotalBubbles(); j++)
		{
			position = bubble[i].getBubblePosition(j);
			counter++;
		}
	}
	return 0;

}
*/
/*************************************************************************
//
**************************************************************************/

unsigned upx_adler32(const void *buf, unsigned len, unsigned adler)
{
    if (len == 0)
        return adler;
    assert(buf != NULL);
#if 0
    return adler32(adler, (const Bytef *) buf, len); // zlib
#elif (WITH_UCL)
    return ucl_adler32(adler, (const ucl_bytep) buf, len);
#else
#  error
#endif
}


#if 0 /* UNUSED */
unsigned upx_crc32(const void *buf, unsigned len, unsigned crc)
{
    if (len == 0)
        return crc;
    assert(buf != NULL);
#if 0
    return crc32(crc, (const Bytef *) buf, len); // zlib
#elif (WITH_UCL)
    return ucl_crc32(crc, (const ucl_bytep) buf, len);
#else
#  error
#endif
}
#endif /* UNUSED */


/*************************************************************************
//
**************************************************************************/

int upx_compress           ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   upx_callback_p cb,
                                   int method, int level,
                             const upx_compress_config_t *cconf,
                                   upx_compress_result_t *cresult )
{
    int r = UPX_E_ERROR;
    upx_compress_result_t cresult_buffer;

    assert(method > 0); assert(level > 0);

#if 1
    // set available bytes in dst
    if (*dst_len == 0)
        *dst_len = MemBuffer::getSizeForCompression(src_len);
#else
    // force users to provide *dst_len
    assert(*dst_len != 0);
#endif
    // for UPX, we always require a reasonably sized outbut buffer
    assert(*dst_len >= MemBuffer::getSizeForCompression(src_len));

    if (!cresult)
        cresult = &cresult_buffer;
    memset(cresult, 0, sizeof(*cresult));
#if 1
    // debug
    cresult->method = method;
    cresult->level = level;
    cresult->u_len = src_len;
    cresult->c_len = 0;
#endif

    if (0) {
    }
#if (WITH_LZMA)
    else if (M_IS_LZMA(method))
        r = upx_lzma_compress(src, src_len, dst, dst_len,
                              cb, method, level, cconf, cresult);
#endif
#if (WITH_NRV)
    else if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        r = upx_nrv_compress(src, src_len, dst, dst_len,
                             cb, method, level, cconf, cresult);
#endif
#if (WITH_UCL)
    else if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        r = upx_ucl_compress(src, src_len, dst, dst_len,
                             cb, method, level, cconf, cresult);
#endif
    else {
        throwInternalError("unknown compression method");
    }
/*********************************************
Modifying compressed information
Adding 10 bytes
*/
//upx_bytep new_dst = NULL;
//new_dst = (upx_bytep) calloc(*dst_len + 10,sizeof(upx_byte));
//memcpy(new_dst+10,dst,(*dst_len)*sizeof(upx_byte));
//*dst_len+=10;
//memcpy(dst,new_dst,(*dst_len)*sizeof(upx_byte));
//free(new_dst);
/*********************************************/

/********************************************/
/*
Adding random bubbles
*/

BubbleType **bubbles = NULL;



ori_dst_len = *dst_len;
ori_dst = (upx_bytep) calloc(*dst_len,sizeof(upx_byte));
memcpy(ori_dst,dst,*dst_len);

upload_configuration(*dst_len,&bubbles);

/*
1-Consigo el tamaño posible.
2-Itero las burbujas y extraigo el número de burbujas
3-Tengo tres vectores: position, size, density
4-En cada iteración uso los números aleatorios para preguntar dónde
genero cada burbuja, y con eso copio tamaño y desviación.
5-Actualizo los tamaños respecto a lo que tengo.
6-Creo un string con la desidad deseada.
7-Lo añado como antes.

La densidad se usa para generar la sequencia de bytes que será
un número aleatorio módulo 256*densidad.

- Dado que la densidad es constante para una serie de bubbles,
se puede generar una secuencia con una densidad específica para
esas bubbles. Recomendado que tanga la longitud de la bubble más
grande.

-Hay que normalizar el tamaño al principio

-Ya hay una función que calcula los números aleatorios con
la distribución normal.

-Cuando tengas realmente un delimitador, tendrás que descontar su
tamaño en el cómputo.

*/


int maxsize = (src_len-*dst_len-1000)*sizeof(upx_byte);
int totalsize=0;
int *order_list =NULL;
int current_position=0;
int current_dst=0;
upx_bytep new_dst = NULL;
int buffer_size;
normalizeSize(maxsize);
for(int i = 0 ; i < total_bubbles ; i++)
{
	totalsize += sizes[i];
}

new_dst = (upx_bytep) calloc(*dst_len + totalsize,sizeof(upx_byte));
memset(new_dst,1,(*dst_len + totalsize)*sizeof(upx_byte));
order_list = mybubble(positions,total_bubbles);

/*
We have calculate the points where the entropy is going to be
modified, and we set the new buffer to the pattern value
(currently 1), now we overwrite specific sections of this buffer
with the previous one according to the positions and the size
of the crash points.
*/
for(int i = 0; i < total_bubbles; i++)
{
	buffer_size=positions[i] - current_dst;
	if(buffer_size > 0)
	{
		memcpy(new_dst+current_position,dst+current_dst,buffer_size);
		current_dst+=buffer_size;
		current_position+=buffer_size;
	}
	else
	{
               	sizes[order_list[i]]=0;
	}
	if(sizes[order_list[i]] > (int) strlen(delimiter)*2+4)
	{
		setDensity(sizes[order_list[i]],new_dst+current_position,densities[order_list[i]]);
	}
	else
	{
		sizes[order_list[i]]=0;
	}
	current_position += sizes[order_list[i]];

}
memcpy(new_dst+current_position,dst+current_dst,*dst_len-current_dst);
*dst_len += totalsize;
memcpy(dst,new_dst,(*dst_len)*sizeof(upx_byte));
free(new_dst);
/*******************************************/

#if 1
    // debug
    cresult->c_len = *dst_len;
#endif
    return r;
}


/*************************************************************************
//
**************************************************************************/

int upx_decompress         ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult )
{
    int r = UPX_E_ERROR;

    assert(*dst_len > 0);
//    assert(src_len < *dst_len); // must be compressed
/*********************************************
Modifying compressed information
Eliminating 10 bytes
*/
//upx_bytep new_src = NULL;
//new_src = (upx_bytep) calloc(src_len - 10,sizeof(upx_byte));
//memcpy(new_src,src+10,(src_len-10)*sizeof(upx_byte));
//src_len-=10;
//src=new_src;
/*********************************************/

/********************************************/
src = ori_dst;
src_len	= ori_dst_len;
/********************************************/

    if (cresult && cresult->method == 0)
        cresult = NULL;

    if (0) {
    }
#if (WITH_LZMA)
    else if (M_IS_LZMA(method))
        r = upx_lzma_decompress(src, src_len, dst, dst_len, method, cresult);
#endif
#if (WITH_NRV)
    else if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        r = upx_nrv_decompress(src, src_len, dst, dst_len, method, cresult);
#endif
#if (WITH_UCL)
    else if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        r = upx_ucl_decompress(src, src_len, dst, dst_len, method, cresult);
#endif
#if (WITH_ZLIB)
    else if (M_IS_DEFLATE(method))
        r = upx_zlib_decompress(src, src_len, dst, dst_len, method, cresult);
#endif
    else {
        throwInternalError("unknown decompression method");
    }

    return r;
}


/*************************************************************************
//
**************************************************************************/

int upx_test_overlap       ( const upx_bytep buf,
                             const upx_bytep tbuf,
                                   unsigned  src_off, unsigned src_len,
                                   unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult )
{
    int r = UPX_E_ERROR;

    if (cresult && cresult->method == 0)
        cresult = NULL;

    assert(*dst_len > 0);
    assert(src_len < *dst_len); // must be compressed


/*********************************************
Modifying compressed information
Eliminating 10 bytes
*/
//upx_bytep new_src = NULL;
//new_src = (upx_bytep) calloc(src_len - 10,sizeof(upx_byte));
//memcpy(new_src,buf+src_off+10,(src_len-10)*sizeof(upx_byte));
//src_len-=10;
//buf=new_src-src_off;
/*********************************************/

/********************************************/
buf = ori_dst-src_off;
src_len = ori_dst_len;
/*******************************************/

//    unsigned overlap_overhead = src_off + src_len - *dst_len;
//    assert((int)overlap_overhead > 0);

    if (0) {
    }
#if (WITH_LZMA)
    else if (M_IS_LZMA(method))
        r = upx_lzma_test_overlap(buf, tbuf, src_off, src_len, dst_len, method, cresult);
#endif
#if (WITH_NRV)
    else if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        r = upx_nrv_test_overlap(buf, tbuf, src_off, src_len, dst_len, method, cresult);
#endif
#if (WITH_UCL)
    else if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        r = upx_ucl_test_overlap(buf, tbuf, src_off, src_len, dst_len, method, cresult);
#endif
    else {
        throwInternalError("unknown decompression method");
    }

    return r;
}


/*
vi:ts=4:et:nowrap
*/

