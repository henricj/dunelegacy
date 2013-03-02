/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 This code is taken from the streflop library which itself is based on fdlibm.
 Please take note of the following copyright notice:
*/

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#include <misc/strictmath.h>

#include <stdint.h>

namespace strictmath {

union flt_uint32_conversion {
	float flt;
	uint32_t uint32;
};

uint32_t getFloatWord(const float x) {
	flt_uint32_conversion conv;
	conv.flt = x;
	return conv.uint32;
}

float setFloatWord(const uint32_t x) {
	flt_uint32_conversion conv2;
	conv2.uint32 = x;
	return conv2.flt;
}


/// sinus and cosinus

float half =  5.0000000000e-01f;/* 0x3f000000 */
float S1  = -1.6666667163e-01f; /* 0xbe2aaaab */
float S2  =  8.3333337680e-03f; /* 0x3c088889 */
float S3  = -1.9841270114e-04f; /* 0xb9500d01 */
float S4  =  2.7557314297e-06f; /* 0x3638ef1b */
float S5  = -2.5050759689e-08f; /* 0xb2d72f34 */
float S6  =  1.5896910177e-10f; /* 0x2f2ec9d3 */

float __kernel_sinf(float x, float y, int iy) {
	float z,r,v;

	int32_t ix = getFloatWord(x);

	ix &= 0x7fffffff;			/* high word of x */
	if(ix<0x32000000)			/* |x| < 2**-27 */
	   {if((int)x==0) return x;}		/* generate inexact */
	z	=  x*x;
	v	=  z*x;
	r	=  S2+z*(S3+z*(S4+z*(S5+z*S6)));
	if(iy==0) return x+v*(S1+z*r);
	else      return x-((z*(half*y-v*r)-y)-v*S1);
}

float one =  1.0000000000e+00f; /* 0x3f800000 */
float C1  =  4.1666667908e-02f; /* 0x3d2aaaab */
float C2  = -1.3888889225e-03f; /* 0xbab60b61 */
float C3  =  2.4801587642e-05f; /* 0x37d00d01 */
float C4  = -2.7557314297e-07f; /* 0xb493f27c */
float C5  =  2.0875723372e-09f; /* 0x310f74f6 */
float C6  = -1.1359647598e-11f; /* 0xad47d74e */

float __kernel_cosf(float x, float y) {
	float a,hz,z,r,qx;

	int32_t ix = getFloatWord(x);

	ix &= 0x7fffffff;			/* ix = |x|'s high word*/
	if(ix<0x32000000) {			/* if x < 2**27 */
	    if(((int)x)==0) return one;		/* generate inexact */
	}
	z  = x*x;
	r  = z*(C1+z*(C2+z*(C3+z*(C4+z*(C5+z*C6)))));
	if(ix < 0x3e99999a) 			/* if |x| < 0.3f */
	    return one - ((float)0.5f*z - (z*r - x*y));
	else {
	    if(ix > 0x3f480000) {		/* x > 0.78125f */
			qx = (float)0.28125f;
	    } else {
			qx = setFloatWord(ix-0x01000000);	/* x/4 */
	    }
	    hz = (float)0.5f*z-qx;
	    a  = one-qx;
	    return a - (hz - (z*r-x*y));
	}
}


//static const float one   =  1.0000000000e+00f; /* 0x3f800000 */
static const float pio4  =  7.8539812565e-01f; /* 0x3f490fda */
static const float pio4lo=  3.7748947079e-08f; /* 0x33222168 */
static const float T[] =  {
  3.3333334327e-01f, /* 0x3eaaaaab */
  1.3333334029e-01f, /* 0x3e088889 */
  5.3968254477e-02f, /* 0x3d5d0dd1 */
  2.1869488060e-02f, /* 0x3cb327a4 */
  8.8632395491e-03f, /* 0x3c11371f */
  3.5920790397e-03f, /* 0x3b6b6916 */
  1.4562094584e-03f, /* 0x3abede48 */
  5.8804126456e-04f, /* 0x3a1a26c8 */
  2.4646313977e-04f, /* 0x398137b9 */
  7.8179444245e-05f, /* 0x38a3f445 */
  7.1407252108e-05f, /* 0x3895c07a */
 -1.8558637748e-05f, /* 0xb79bae5f */
  2.5907305826e-05f, /* 0x37d95384 */
};

float __kernel_tanf(float x, float y, int iy)
{
	float z,r,v,w,s;
	int32_t ix,hx;
	hx = getFloatWord(x);
	ix = hx&0x7fffffff;	/* high word of |x| */
	if(ix<0x31800000)			/* x < 2**-28 */
	    {if((int)x==0) {			/* generate inexact */
		if((ix|(iy+1))==0) return one/abs(x);
		else return (iy==1)? x: -one/x;
	    }
	    }
	if(ix>=0x3f2ca140) { 			/* |x|>=0.6744f */
	    if(hx<0) {x = -x; y = -y;}
	    z = pio4-x;
	    w = pio4lo-y;
	    x = z+w; y = 0.0f;
	}
	z	=  x*x;
	w 	=  z*z;
    /* Break x^5*(T[1]+x^2*T[2]+...) into
     *	  x^5(T[1]+x^4*T[3]+...+x^20*T[11]) +
     *	  x^5(x^2*(T[2]+x^4*T[4]+...+x^22*[T12]))
     */
	r = T[1]+w*(T[3]+w*(T[5]+w*(T[7]+w*(T[9]+w*T[11]))));
	v = z*(T[2]+w*(T[4]+w*(T[6]+w*(T[8]+w*(T[10]+w*T[12])))));
	s = z*x;
	r = y + z*(s*(r+v)+y);
	r += T[0]*s;
	w = x+r;
	if(ix>=0x3f2ca140) {
	    v = (float)iy;
	    return (float)(1-((hx>>30)&2))*(v-(float)2.0f*(x-(w*w/(w+v)-r)));
	}
	if(iy==1) return w;
	else {		/* if allow error up to 2 ulp,
			   simply return -1.0f/(x+r) here */
     /*  compute -1.0f/(x+r) accurately */
	    float a,t;
	    int32_t i;
	    z  = w;
	    i = getFloatWord(z);
	    z = setFloatWord(i&0xfffff000);
	    v  = r-(z - x); 	/* z+v = r+x */
	    t = a  = -(float)1.0f/w;	/* a = -1.0f/w */
	    i = getFloatWord(t);
	    t = setFloatWord(i&0xfffff000);
	    s  = (float)1.0f+t*z;
	    return t+a*(s+t*v);
	}
}

float __copysignf(float x, float y)
{
	uint32_t ix,iy;
	ix = getFloatWord(x);
	iy = getFloatWord(y);
	x = setFloatWord((ix&0x7fffffff)|(iy&0x80000000));
    return x;
}

static const float two25   =  3.355443200e+07f;	/* 0x4c000000 */
static const float twom25  =  2.9802322388e-08f;	/* 0x33000000 */
static const float huge   = 1.0e+30f;
static const float tiny   = 1.0e-30f;

float __scalbnf (float x, int n)
{
	int32_t k,ix;
	ix = getFloatWord(x);
        k = (ix&0x7f800000)>>23;		/* extract exponent */
        if (k==0) {				/* 0 or subnormal x */
            if ((ix&0x7fffffff)==0) return x; /* +-0 */
	    x *= two25;
	    ix = getFloatWord(x);
	    k = ((ix&0x7f800000)>>23) - 25;
	    }
        if (k==0xff) return x+x;		/* NaN or Inf */
        k = k+n;
        if (n> 50000 || k >  0xfe)
	  return huge*__copysignf(huge,x); /* overflow  */
	if (n< -50000)
	  return tiny*__copysignf(tiny,x);	/*underflow*/
        if (k > 0) 				/* normal result */
	    {x = setFloatWord((ix&0x807fffff)|(k<<23)); return x;}
        if (k <= -25)
	    return tiny*__copysignf(tiny,x);	/*underflow*/
        k += 25;				/* subnormal result */
		x = setFloatWord((ix&0x807fffff)|(k<<23));
        return x*twom25;
}


//static const float huge = 1.0e30f;

float __floorf(float x)
{
	int32_t i0,j0;
	uint32_t i;
	i0 = getFloatWord(x);
	j0 = ((i0>>23)&0xff)-0x7f;
	if(j0<23) {
	    if(j0<0) { 	/* raise inexact if x != 0 */
		if(huge+x>(float)0.0f) {/* return 0*sign(x) if |x|<1 */
		    if(i0>=0) {i0=0;}
		    else if((i0&0x7fffffff)!=0)
			{ i0=0xbf800000;}
		}
	    } else {
		i = (0x007fffff)>>j0;
		if((i0&i)==0) return x; /* x is integral */
		if(huge+x>(float)0.0f) {	/* raise inexact flag */
		    if(i0<0) i0 += (0x00800000)>>j0;
		    i0 &= (~i);
		}
	    }
	} else {
	    if(j0==0x80) return x+x;	/* inf or NaN */
	    else return x;		/* x is integral */
	}
	x = setFloatWord(i0);
	return x;
}


static const int init_jk[] = {4,7,9}; /* initial value for jk */

static const float PIo2[] = {
  1.5703125000e+00f, /* 0x3fc90000 */
  4.5776367188e-04f, /* 0x39f00000 */
  2.5987625122e-05f, /* 0x37da0000 */
  7.5437128544e-08f, /* 0x33a20000 */
  6.0026650317e-11f, /* 0x2e840000 */
  7.3896444519e-13f, /* 0x2b500000 */
  5.3845816694e-15f, /* 0x27c20000 */
  5.6378512969e-18f, /* 0x22d00000 */
  8.3009228831e-20f, /* 0x1fc40000 */
  3.2756352257e-22f, /* 0x1bc60000 */
  6.3331015649e-25f, /* 0x17440000 */
};

static const float zero   = 0.0f;
//float one    = 1.0f;
static const float two8   =  2.5600000000e+02f; /* 0x43800000 */
static const float twon8  =  3.9062500000e-03f; /* 0x3b800000 */


int __kernel_rem_pio2f(float *x, float *y, int e0, int nx, int prec, const int32_t *ipio2)
{
	int32_t jz,jx,jv,jp,jk,carry,n,iq[20],i,j,k,m,q0,ih;
	float z,fw,f[20],fq[20],q[20];

    /* initialize jk*/
	jk = init_jk[prec];
	jp = jk;

    /* determine jx,jv,q0, note that 3>q0 */
	jx =  nx-1;
	jv = (e0-3)/8; if(jv<0) jv=0;
	q0 =  e0-8*(jv+1);

    /* set up f[0] to f[jx+jk] where f[jx+jk] = ipio2[jv+jk] */
	j = jv-jx; m = jx+jk;
	for(i=0;i<=m;i++,j++) f[i] = (j<0)? zero : (float) ipio2[j];

    /* compute q[0],q[1],...q[jk] */
	for (i=0;i<=jk;i++) {
	    for(j=0,fw=0.0f;j<=jx;j++) fw += x[j]*f[jx+i-j]; q[i] = fw;
	}

	jz = jk;
recompute:
    /* distill q[] into iq[] reversingly */
	for(i=0,j=jz,z=q[jz];j>0;i++,j--) {
	    fw    =  (float)((int32_t)(twon8* z));
	    iq[i] =  (int32_t)(z-two8*fw);
	    z     =  q[j-1]+fw;
	}

    /* compute n */
	z  = __scalbnf(z,q0);		/* actual value of z */
	z -= (float)8.0f*__floorf(z*(float)0.125f);	/* trim off integer >= 8 */
	n  = (int32_t) z;
	z -= (float)n;
	ih = 0;
	if(q0>0) {	/* need iq[jz-1] to determine n */
	    i  = (iq[jz-1]>>(8-q0)); n += i;
	    iq[jz-1] -= i<<(8-q0);
	    ih = iq[jz-1]>>(7-q0);
	}
	else if(q0==0) ih = iq[jz-1]>>8;
	else if(z>=(float)0.5f) ih=2;

	if(ih>0) {	/* q > 0.5f */
	    n += 1; carry = 0;
	    for(i=0;i<jz ;i++) {	/* compute 1-q */
		j = iq[i];
		if(carry==0) {
		    if(j!=0) {
			carry = 1; iq[i] = 0x100- j;
		    }
		} else  iq[i] = 0xff - j;
	    }
	    if(q0>0) {		/* rare case: chance is 1 in 12 */
	        switch(q0) {
	        case 1:
	    	   iq[jz-1] &= 0x7f; break;
	    	case 2:
	    	   iq[jz-1] &= 0x3f; break;
	        }
	    }
	    if(ih==2) {
		z = one - z;
		if(carry!=0) z -= __scalbnf(one,q0);
	    }
	}

    /* check if recomputation is needed */
	if(z==zero) {
	    j = 0;
	    for (i=jz-1;i>=jk;i--) j |= iq[i];
	    if(j==0) { /* need recomputation */
		for(k=1;iq[jk-k]==0;k++);   /* k = no. of terms needed */

		for(i=jz+1;i<=jz+k;i++) {   /* add q[jz+1] to q[jz+k] */
		    f[jx+i] = (float) ipio2[jv+i];
		    for(j=0,fw=0.0f;j<=jx;j++) fw += x[j]*f[jx+i-j];
		    q[i] = fw;
		}
		jz += k;
		goto recompute;
	    }
	}

    /* chop off zero terms */
	if(z==(float)0.0f) {
	    jz -= 1; q0 -= 8;
	    while(iq[jz]==0) { jz--; q0-=8;}
	} else { /* break z into 8-bit if necessary */
	    z = __scalbnf(z,-q0);
	    if(z>=two8) {
		fw = (float)((int32_t)(twon8*z));
		iq[jz] = (int32_t)(z-two8*fw);
		jz += 1; q0 += 8;
		iq[jz] = (int32_t) fw;
	    } else iq[jz] = (int32_t) z ;
	}

    /* convert integer "bit" chunk to floating-point value */
	fw = __scalbnf(one,q0);
	for(i=jz;i>=0;i--) {
	    q[i] = fw*(float)iq[i]; fw*=twon8;
	}

    /* compute PIo2[0,...,jp]*q[jz,...,0] */
	for(i=jz;i>=0;i--) {
	    for(fw=0.0f,k=0;k<=jp&&k<=jz-i;k++) fw += PIo2[k]*q[i+k];
	    fq[jz-i] = fw;
	}

    /* compress fq[] into y[] */
	switch(prec) {
	    case 0:
		fw = 0.0f;
		for (i=jz;i>=0;i--) fw += fq[i];
		y[0] = (ih==0)? fw: -fw;
		break;
	    case 1:
	    case 2:
		fw = 0.0f;
		for (i=jz;i>=0;i--) fw += fq[i];
		y[0] = (ih==0)? fw: -fw;
		fw = fq[0]-fw;
		for (i=1;i<=jz;i++) fw += fq[i];
		y[1] = (ih==0)? fw: -fw;
		break;
	    case 3:	/* painful */
		for (i=jz;i>0;i--) {
		    fw      = fq[i-1]+fq[i];
		    fq[i]  += fq[i-1]-fw;
		    fq[i-1] = fw;
		}
		for (i=jz;i>1;i--) {
		    fw      = fq[i-1]+fq[i];
		    fq[i]  += fq[i-1]-fw;
		    fq[i-1] = fw;
		}
		for (fw=0.0f,i=jz;i>=2;i--) fw += fq[i];
		if(ih==0) {
		    y[0] =  fq[0]; y[1] =  fq[1]; y[2] =  fw;
		} else {
		    y[0] = -fq[0]; y[1] = -fq[1]; y[2] = -fw;
		}
	}
	return n&7;
}

static const int32_t two_over_pi[] = {
0xA2, 0xF9, 0x83, 0x6E, 0x4E, 0x44, 0x15, 0x29, 0xFC,
0x27, 0x57, 0xD1, 0xF5, 0x34, 0xDD, 0xC0, 0xDB, 0x62,
0x95, 0x99, 0x3C, 0x43, 0x90, 0x41, 0xFE, 0x51, 0x63,
0xAB, 0xDE, 0xBB, 0xC5, 0x61, 0xB7, 0x24, 0x6E, 0x3A,
0x42, 0x4D, 0xD2, 0xE0, 0x06, 0x49, 0x2E, 0xEA, 0x09,
0xD1, 0x92, 0x1C, 0xFE, 0x1D, 0xEB, 0x1C, 0xB1, 0x29,
0xA7, 0x3E, 0xE8, 0x82, 0x35, 0xF5, 0x2E, 0xBB, 0x44,
0x84, 0xE9, 0x9C, 0x70, 0x26, 0xB4, 0x5F, 0x7E, 0x41,
0x39, 0x91, 0xD6, 0x39, 0x83, 0x53, 0x39, 0xF4, 0x9C,
0x84, 0x5F, 0x8B, 0xBD, 0xF9, 0x28, 0x3B, 0x1F, 0xF8,
0x97, 0xFF, 0xDE, 0x05, 0x98, 0x0F, 0xEF, 0x2F, 0x11,
0x8B, 0x5A, 0x0A, 0x6D, 0x1F, 0x6D, 0x36, 0x7E, 0xCF,
0x27, 0xCB, 0x09, 0xB7, 0x4F, 0x46, 0x3F, 0x66, 0x9E,
0x5F, 0xEA, 0x2D, 0x75, 0x27, 0xBA, 0xC7, 0xEB, 0xE5,
0xF1, 0x7B, 0x3D, 0x07, 0x39, 0xF7, 0x8A, 0x52, 0x92,
0xEA, 0x6B, 0xFB, 0x5F, 0xB1, 0x1F, 0x8D, 0x5D, 0x08,
0x56, 0x03, 0x30, 0x46, 0xFC, 0x7B, 0x6B, 0xAB, 0xF0,
0xCF, 0xBC, 0x20, 0x9A, 0xF4, 0x36, 0x1D, 0xA9, 0xE3,
0x91, 0x61, 0x5E, 0xE6, 0x1B, 0x08, 0x65, 0x99, 0x85,
0x5F, 0x14, 0xA0, 0x68, 0x40, 0x8D, 0xFF, 0xD8, 0x80,
0x4D, 0x73, 0x27, 0x31, 0x06, 0x06, 0x15, 0x56, 0xCA,
0x73, 0xA8, 0xC9, 0x60, 0xE2, 0x7B, 0xC0, 0x8C, 0x6B,
};

/* This array is like the one in e_rem_pio2.c, but the numbers are
   single precision and the last 8 bits are forced to 0.  */
static const int32_t npio2_hw[] = {
0x3fc90f00, 0x40490f00, 0x4096cb00, 0x40c90f00, 0x40fb5300, 0x4116cb00,
0x412fed00, 0x41490f00, 0x41623100, 0x417b5300, 0x418a3a00, 0x4196cb00,
0x41a35c00, 0x41afed00, 0x41bc7e00, 0x41c90f00, 0x41d5a000, 0x41e23100,
0x41eec200, 0x41fb5300, 0x4203f200, 0x420a3a00, 0x42108300, 0x4216cb00,
0x421d1400, 0x42235c00, 0x4229a500, 0x422fed00, 0x42363600, 0x423c7e00,
0x4242c700, 0x42490f00
};

/*
 * invpio2:  24 bits of 2/pi
 * pio2_1:   first  17 bit of pi/2
 * pio2_1t:  pi/2 - pio2_1
 * pio2_2:   second 17 bit of pi/2
 * pio2_2t:  pi/2 - (pio2_1+pio2_2)
 * pio2_3:   third  17 bit of pi/2
 * pio2_3t:  pi/2 - (pio2_1+pio2_2+pio2_3)
 */

//static const float zero =  0.0000000000e+00f; /* 0x00000000 */
//static const float half =  5.0000000000e-01f; /* 0x3f000000 */
//static const float two8 =  2.5600000000e+02f; /* 0x43800000 */
static const float invpio2 =  6.3661980629e-01f; /* 0x3f22f984 */
static const float pio2_1  =  1.5707855225e+00f; /* 0x3fc90f80 */
static const float pio2_1t =  1.0804334124e-05f; /* 0x37354443 */
static const float pio2_2  =  1.0804273188e-05f; /* 0x37354400 */
static const float pio2_2t =  6.0770999344e-11f; /* 0x2e85a308 */
static const float pio2_3  =  6.0770943833e-11f; /* 0x2e85a300 */
static const float pio2_3t =  6.1232342629e-17f; /* 0x248d3132 */

int32_t __ieee754_rem_pio2f(float x, float *y)
{
	float z,w,t,r,fn;
	float tx[3];
	int32_t e0,i,j,nx,n,ix,hx;

	hx = getFloatWord(x);
	ix = hx&0x7fffffff;
	if(ix<=0x3f490fd8)   /* |x| ~<= pi/4 , no need for reduction */
	    {y[0] = x; y[1] = 0; return 0;}
	if(ix<0x4016cbe4) {  /* |x| < 3pi/4, special case with n=+-1 */
	    if(hx>0) {
		z = x - pio2_1;
		if((ix&0xfffffff0)!=0x3fc90fd0) { /* 24+24 bit pi OK */
		    y[0] = z - pio2_1t;
		    y[1] = (z-y[0])-pio2_1t;
		} else {		/* near pi/2, use 24+24+24 bit pi */
		    z -= pio2_2;
		    y[0] = z - pio2_2t;
		    y[1] = (z-y[0])-pio2_2t;
		}
		return 1;
	    } else {	/* negative x */
		z = x + pio2_1;
		if((ix&0xfffffff0)!=0x3fc90fd0) { /* 24+24 bit pi OK */
		    y[0] = z + pio2_1t;
		    y[1] = (z-y[0])+pio2_1t;
		} else {		/* near pi/2, use 24+24+24 bit pi */
		    z += pio2_2;
		    y[0] = z + pio2_2t;
		    y[1] = (z-y[0])+pio2_2t;
		}
		return -1;
	    }
	}
	if(ix<=0x43490f80) { /* |x| ~<= 2^7*(pi/2), medium size */
	    t  = abs(x);
	    n  = (int32_t) (t*invpio2+half);
	    fn = (float)n;
	    r  = t-fn*pio2_1;
	    w  = fn*pio2_1t;	/* 1st round good to 40 bit */
	    if(n<32&&(int32_t)(ix&0xffffff00)!=npio2_hw[n-1]) {
		y[0] = r-w;	/* quick check no cancellation */
	    } else {
	        uint32_t high;
	        j  = ix>>23;
	        y[0] = r-w;
			high = getFloatWord(y[0]);
	        i = j-((high>>23)&0xff);
	        if(i>8) {  /* 2nd iteration needed, good to 57 */
		    t  = r;
		    w  = fn*pio2_2;
		    r  = t-w;
		    w  = fn*pio2_2t-((t-r)-w);
		    y[0] = r-w;
		    high = getFloatWord(y[0]);
		    i = j-((high>>23)&0xff);
		    if(i>25)  {	/* 3rd iteration need, 74 bits acc */
		    	t  = r;	/* will cover all possible cases */
		    	w  = fn*pio2_3;
		    	r  = t-w;
		    	w  = fn*pio2_3t-((t-r)-w);
		    	y[0] = r-w;
		    }
		}
	    }
	    y[1] = (r-y[0])-w;
	    if(hx<0) 	{y[0] = -y[0]; y[1] = -y[1]; return -n;}
	    else	 return n;
	}
    /*
     * all other (large) arguments
     */
	if(ix>=0x7f800000) {		/* x is inf or NaN */
	    y[0]=y[1]=x-x; return 0;
	}
    /* set z = scalbn(|x|,ilogb(x)-7) */
	e0 	= (ix>>23)-134;		/* e0 = ilogb(z)-7; */
	z = setFloatWord(ix - ((int32_t)(e0<<23)));
	for(i=0;i<2;i++) {
		tx[i] = (float)((int32_t)(z));
		z     = (z-tx[i])*two8;
	}
	tx[2] = z;
	nx = 3;
	while(tx[nx-1]==zero) nx--;	/* skip zero term */
	n  =  __kernel_rem_pio2f(tx,y,e0,nx,2,two_over_pi);
	if(hx<0) {y[0] = -y[0]; y[1] = -y[1]; return -n;}
	return n;
}

float sin(float x)
{
	float y[2],z=0.0f;
	int32_t n, ix;

	ix = getFloatWord(x);

    /* |x| ~< pi/4 */
	ix &= 0x7fffffff;
	if(ix <= 0x3f490fd8) return __kernel_sinf(x,z,0);

    /* sin(Inf or NaN) is NaN */
	else if (ix>=0x7f800000) return x-x;

    /* argument reduction needed */
	else {
	    n = __ieee754_rem_pio2f(x,y);
	    switch(n&3) {
		case 0: return  __kernel_sinf(y[0],y[1],1);
		case 1: return  __kernel_cosf(y[0],y[1]);
		case 2: return -__kernel_sinf(y[0],y[1],1);
		default:
			return -__kernel_cosf(y[0],y[1]);
	    }
	}
}

float cos(float x)
{
	float y[2],z=0.0f;
	int32_t n,ix;

	ix = getFloatWord(x);

    /* |x| ~< pi/4 */
	ix &= 0x7fffffff;
	if(ix <= 0x3f490fd8) return __kernel_cosf(x,z);

    /* cos(Inf or NaN) is NaN */
	else if (ix>=0x7f800000) return x-x;

    /* argument reduction needed */
	else {
	    n = __ieee754_rem_pio2f(x,y);
	    switch(n&3) {
		case 0: return  __kernel_cosf(y[0],y[1]);
		case 1: return -__kernel_sinf(y[0],y[1],1);
		case 2: return -__kernel_cosf(y[0],y[1]);
		default:
		        return  __kernel_sinf(y[0],y[1],1);
	    }
	}
}

float tan(float x)
{
	float y[2],z=0.0f;
	int32_t n, ix;

	ix = getFloatWord(x);

    /* |x| ~< pi/4 */
	ix &= 0x7fffffff;
	if(ix <= 0x3f490fda) return __kernel_tanf(x,z,1);

    /* tan(Inf or NaN) is NaN */
	else if (ix>=0x7f800000) return x-x;		/* NaN */

    /* argument reduction needed */
	else {
	    n = __ieee754_rem_pio2f(x,y);
	    return __kernel_tanf(y[0],y[1],1-((n&1)<<1)); /*   1 -- n even
							      -1 -- n odd */
	}
}

//static const float one =  1.0000000000e+00f; /* 0x3F800000 */
//static const float huge =  1.000e+30f;

static const float pio2_hi = 1.57079637050628662109375f;
static const float pio2_lo = -4.37113900018624283e-8f;
static const float pio4_hi = 0.785398185253143310546875f;

/* asin x = x + x^3 p(x^2)
   -0.5f <= x <= 0.5f;
   Peak relative error 4.8e-9f */
static const float p0 = 1.666675248e-1f;
static const float p1 = 7.495297643e-2f;
static const float p2 = 4.547037598e-2f;
static const float p3 = 2.417951451e-2f;
static const float p4 = 4.216630880e-2f;

float asin(float x)
{
	float t,w,p,q,c,r,s;
	int32_t hx,ix;
	hx = getFloatWord(x);
	ix = hx&0x7fffffff;
	if(ix==0x3f800000) {
		/* asin(1)=+-pi/2 with inexact */
	    return x*pio2_hi+x*pio2_lo;
	} else if(ix> 0x3f800000) {	/* |x|>= 1 */
	    return (x-x)/(x-x);		/* asin(|x|>1) is NaN */
	} else if (ix<0x3f000000) {	/* |x|<0.5f */
	    if(ix<0x32000000) {		/* if |x| < 2**-27 */
		if(huge+x>one) return x;/* return x with inexact if x!=0*/
	    } else {
		t = x*x;
		w = t * (p0 + t * (p1 + t * (p2 + t * (p3 + t * p4))));
		return x+x*w;
	    }
	}
	/* 1> |x|>= 0.5f */
	w = one-abs(x);
	t = w*0.5f;
	p = t * (p0 + t * (p1 + t * (p2 + t * (p3 + t * p4))));
	s = sqrt(t);
	if(ix>=0x3F79999A) { 	/* if |x| > 0.975f */
	    t = pio2_hi-(2.0f*(s+s*p)-pio2_lo);
	} else {
	    int32_t iw;
	    w  = s;
	    iw = getFloatWord(w);
	    w = setFloatWord(iw&0xfffff000);
	    c  = (t-w*w)/(s+w);
	    r  = p;
	    p  = 2.0f*s*r-(pio2_lo-2.0f*c);
	    q  = pio4_hi-2.0f*w;
	    t  = pio4_hi-(p-q);
	}
	if(hx>0) return t; else return -t;
}

//static const float one =  1.0000000000e+00f; /* 0x3F800000 */
//static const float pi =  3.1415925026e+00f; /* 0x40490fda */
//static const float pio2_hi =  1.5707962513e+00f; /* 0x3fc90fda */
//static const float pio2_lo =  7.5497894159e-08f; /* 0x33a22168 */
static const float pS0 =  1.6666667163e-01f; /* 0x3e2aaaab */
static const float pS1 = -3.2556581497e-01f; /* 0xbea6b090 */
static const float pS2 =  2.0121252537e-01f; /* 0x3e4e0aa8 */
static const float pS3 = -4.0055535734e-02f; /* 0xbd241146 */
static const float pS4 =  7.9153501429e-04f; /* 0x3a4f7f04 */
static const float pS5 =  3.4793309169e-05f; /* 0x3811ef08 */
static const float qS1 = -2.4033949375e+00f; /* 0xc019d139 */
static const float qS2 =  2.0209457874e+00f; /* 0x4001572d */
static const float qS3 = -6.8828397989e-01f; /* 0xbf303361 */
static const float qS4 =  7.7038154006e-02f; /* 0x3d9dc62e */

float acos(float x)
{
	float z,p,q,r,w,s,c,df;
	int32_t hx,ix;
	hx = getFloatWord(x);
	ix = hx&0x7fffffff;
	if(ix==0x3f800000) {		/* |x|==1 */
	    if(hx>0) return 0.0f;	/* acos(1) = 0  */
	    else return pi+(float)2.0f*pio2_lo;	/* acos(-1)= pi */
	} else if(ix>0x3f800000) {	/* |x| >= 1 */
	    return (x-x)/(x-x);		/* acos(|x|>1) is NaN */
	}
	if(ix<0x3f000000) {	/* |x| < 0.5f */
	    if(ix<=0x23000000) return pio2_hi+pio2_lo;/*if|x|<2**-57*/
	    z = x*x;
	    p = z*(pS0+z*(pS1+z*(pS2+z*(pS3+z*(pS4+z*pS5)))));
	    q = one+z*(qS1+z*(qS2+z*(qS3+z*qS4)));
	    r = p/q;
	    return pio2_hi - (x - (pio2_lo-x*r));
	} else  if (hx<0) {		/* x < -0.5f */
	    z = (one+x)*(float)0.5f;
	    p = z*(pS0+z*(pS1+z*(pS2+z*(pS3+z*(pS4+z*pS5)))));
	    q = one+z*(qS1+z*(qS2+z*(qS3+z*qS4)));
	    s = sqrt(z);
	    r = p/q;
	    w = r*s-pio2_lo;
	    return pi - (float)2.0f*(s+w);
	} else {			/* x > 0.5f */
	    int32_t idf;
	    z = (one-x)*(float)0.5f;
	    s = sqrt(z);
	    df = s;
	    idf = getFloatWord(df);
	    df = setFloatWord(idf&0xfffff000);
	    c  = (z-df*df)/(s+df);
	    p = z*(pS0+z*(pS1+z*(pS2+z*(pS3+z*(pS4+z*pS5)))));
	    q = one+z*(qS1+z*(qS2+z*(qS3+z*qS4)));
	    r = p/q;
	    w = r*s+c;
	    return (float)2.0f*(df+w);
	}
}


static const float atanhi[] = {
  4.6364760399e-01f, /* atan(0.5f)hi 0x3eed6338 */
  7.8539812565e-01f, /* atan(1.0f)hi 0x3f490fda */
  9.8279368877e-01f, /* atan(1.5f)hi 0x3f7b985e */
  1.5707962513e+00f, /* atan(inf)hi 0x3fc90fda */
};

static const float atanlo[] = {
  5.0121582440e-09f, /* atan(0.5f)lo 0x31ac3769 */
  3.7748947079e-08f, /* atan(1.0f)lo 0x33222168 */
  3.4473217170e-08f, /* atan(1.5f)lo 0x33140fb4 */
  7.5497894159e-08f, /* atan(inf)lo 0x33a22168 */
};

static const float aT[] = {
  3.3333334327e-01f, /* 0x3eaaaaaa */
 -2.0000000298e-01f, /* 0xbe4ccccd */
  1.4285714924e-01f, /* 0x3e124925 */
 -1.1111110449e-01f, /* 0xbde38e38 */
  9.0908870101e-02f, /* 0x3dba2e6e */
 -7.6918758452e-02f, /* 0xbd9d8795 */
  6.6610731184e-02f, /* 0x3d886b35 */
 -5.8335702866e-02f, /* 0xbd6ef16b */
  4.9768779427e-02f, /* 0x3d4bda59 */
 -3.6531571299e-02f, /* 0xbd15a221 */
  1.6285819933e-02f, /* 0x3c8569d7 */
};

//static const float one   = 1.0f;
//static const float huge   = 1.0e30f;

float atan(float x)
{
	float w,s1,s2,z;
	int32_t ix,hx,id;

	hx = getFloatWord(x);
	ix = hx&0x7fffffff;
	if(ix>=0x50800000) {	/* if |x| >= 2^34 */
	    if(ix>0x7f800000)
		return x+x;		/* NaN */
	    if(hx>0) return  atanhi[3]+atanlo[3];
	    else     return -atanhi[3]-atanlo[3];
	} if (ix < 0x3ee00000) {	/* |x| < 0.4375f */
	    if (ix < 0x31000000) {	/* |x| < 2^-29 */
		if(huge+x>one) return x;	/* raise inexact */
	    }
	    id = -1;
	} else {
	x = abs(x);
	if (ix < 0x3f980000) {		/* |x| < 1.1875f */
	    if (ix < 0x3f300000) {	/* 7/16 <=|x|<11/16 */
		id = 0; x = ((float)2.0f*x-one)/((float)2.0f+x);
	    } else {			/* 11/16<=|x|< 19/16 */
		id = 1; x  = (x-one)/(x+one);
	    }
	} else {
	    if (ix < 0x401c0000) {	/* |x| < 2.4375f */
		id = 2; x  = (x-(float)1.5f)/(one+(float)1.5f*x);
	    } else {			/* 2.4375f <= |x| < 2^66 */
		id = 3; x  = -(float)1.0f/x;
	    }
	}}
    /* end of argument reduction */
	z = x*x;
	w = z*z;
    /* break sum from i=0 to 10 aT[i]z**(i+1) into odd and even poly */
	s1 = z*(aT[0]+w*(aT[2]+w*(aT[4]+w*(aT[6]+w*(aT[8]+w*aT[10])))));
	s2 = w*(aT[1]+w*(aT[3]+w*(aT[5]+w*(aT[7]+w*aT[9]))));
	if (id<0) return x - x*(s1+s2);
	else {
	    z = atanhi[id] - ((x*(s1+s2) - atanlo[id]) - x);
	    return (hx<0)? -z:z;
	}
}

//static const float tiny  = 1.0e-30f;
//static const float zero  = 0.0f;
static const float pi_o_4  = 7.8539818525e-01f;  /* 0x3f490fdb */
static const float pi_o_2  = 1.5707963705e+00f;  /* 0x3fc90fdb */
//static const float pi      = 3.1415927410e+00f;  /* 0x40490fdb */
static const float pi_lo   = -8.7422776573e-08f; /* 0xb3bbbd2e */

float atan2(float y, float x)
{
	float z;
	int32_t k,m,hx,hy,ix,iy;

	hx = getFloatWord(x);
	ix = hx&0x7fffffff;
	hy = getFloatWord(y);
	iy = hy&0x7fffffff;
	if((ix>0x7f800000)||
	   (iy>0x7f800000))	/* x or y is NaN */
	   return x+y;
	if(hx==0x3f800000) return atan(y);   /* x=1.0f */
	m = ((hy>>31)&1)|((hx>>30)&2);	/* 2*sign(x)+sign(y) */

    /* when y = 0 */
	if(iy==0) {
	    switch(m) {
		case 0:
		case 1: return y; 	/* atan(+-0,+anything)=+-0 */
		case 2: return  pi+tiny;/* atan(+0,-anything) = pi */
		case 3: return -pi-tiny;/* atan(-0,-anything) =-pi */
	    }
	}
    /* when x = 0 */
	if(ix==0) return (hy<0)?  -pi_o_2-tiny: pi_o_2+tiny;

    /* when x is INF */
	if(ix==0x7f800000) {
	    if(iy==0x7f800000) {
		switch(m) {
		    case 0: return  pi_o_4+tiny;/* atan(+INF,+INF) */
		    case 1: return -pi_o_4-tiny;/* atan(-INF,+INF) */
		    case 2: return  (float)3.0f*pi_o_4+tiny;/*atan(+INF,-INF)*/
		    case 3: return (float)-3.0f*pi_o_4-tiny;/*atan(-INF,-INF)*/
		}
	    } else {
		switch(m) {
		    case 0: return  zero  ;	/* atan(+...,+INF) */
		    case 1: return -zero  ;	/* atan(-...,+INF) */
		    case 2: return  pi+tiny  ;	/* atan(+...,-INF) */
		    case 3: return -pi-tiny  ;	/* atan(-...,-INF) */
		}
	    }
	}
    /* when y is INF */
	if(iy==0x7f800000) return (hy<0)? -pi_o_2-tiny: pi_o_2+tiny;

    /* compute y/x */
	k = (iy-ix)>>23;
	if(k > 60) z=pi_o_2+(float)0.5f*pi_lo; 	/* |y/x| >  2**60 */
	else if(hx<0&&k<-60) z=0.0f; 	/* |y|/x < -2**60 */
	else z=atan(abs(y/x));	/* safe to do y/x */
	switch (m) {
	    case 0: return       z  ;	/* atan(+,+) */
	    case 1: {
	    	  uint32_t zh;
		      zh = getFloatWord(z);
		      z = setFloatWord(zh ^ 0x80000000);
		    }
		    return       z  ;	/* atan(-,+) */
	    case 2: return  pi-(z-pi_lo);/* atan(+,-) */
	    default: /* case 3 */
	    	    return  (z-pi_lo)-pi;/* atan(-,-) */
	}
}

} // namespace strictmath
