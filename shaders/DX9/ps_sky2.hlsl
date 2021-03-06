#include "common.h"
#include "atmospheric.h"
struct         v2p
{
        float4        	factor:     COLOR0          ;        // for SM3 - factor.rgb - tonemap-prescaled
        float3        	tc0:        TEXCOORD0       ;
        float3        	tc1:        TEXCOORD1		;
		float3			pos_vertex: 	TEXCOORD2		;		
};
struct        _out
{
        float4        	low: 		COLOR0;
        float4        	high:		COLOR1;
};


uniform samplerCUBE     s_sky0      : register(s0)	;
uniform samplerCUBE     s_sky1      : register(s1)	;

//////////////////////////////////////////////////////////////////////////////////////////
// Pixel
_out         main               ( v2p I )
{
        float3         	s0  	= texCUBE        (s_sky0,I.tc0);
        float3         	s1      = texCUBE        (s_sky1,I.tc1);
#ifndef USE_PROC_SKY
	float3	sky		= I.factor*lerp( s0, s1, I.factor.w );
			sky		*= 0.5;	
#else	
	float3	sky   = compute_scattering(I.pos_vertex);
#endif	
        // final tone-mapping
        _out        	o;
#ifdef USE_VTF
	o.low = sky.xyzz;
	o.high = o.low / def_hdr;
#else
        float    scale   = tex2D(s_tonemap,float2(.5h,.5h)).x;
        tonemap        	(o.low, o.high, sky, scale*1.7 );
#endif
        return        	o;
}