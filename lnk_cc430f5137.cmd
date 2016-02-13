/* ============================================================================ */
/* Copyright (c) 2013, Texas Instruments Incorporated                           */
/*  All rights reserved.                                                        */
/*                                                                              */
/*  Redistribution and use in source and binary forms, with or without          */
/*  modification, are permitted provided that the following conditions          */
/*  are met:                                                                    */
/*                                                                              */
/*  *  Redistributions of source code must retain the above copyright           */
/*     notice, this list of conditions and the following disclaimer.            */
/*                                                                              */
/*  *  Redistributions in binary form must reproduce the above copyright        */
/*     notice, this list of conditions and the following disclaimer in the      */
/*     documentation and/or other materials provided with the distribution.     */
/*                                                                              */
/*  *  Neither the name of Texas Instruments Incorporated nor the names of      */
/*     its contributors may be used to endorse or promote products derived      */
/*     from this software without specific prior written permission.            */
/*                                                                              */
/*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" */
/*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,       */
/*  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR      */
/*  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR            */
/*  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,       */
/*  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,         */
/*  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; */
/*  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,    */
/*  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR     */
/*  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,              */
/*  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                          */
/* ============================================================================ */

/******************************************************************************/
/* lnk_cc430f5137.cmd - LINKER COMMAND FILE FOR LINKING CC430F5137 PROGRAMS     */
/*                                                                            */
/*   Usage:  lnk430 <obj files...>    -o <out file> -m <map file> lnk.cmd     */
/*           cl430  <src files...> -z -o <out file> -m <map file> lnk.cmd     */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* These linker options are for command line linking only.  For IDE linking,  */
/* you should set your linker options in Project Properties                   */
/* -c                                               LINK USING C CONVENTIONS  */
/* -stack  0x0100                                   SOFTWARE STACK SIZE       */
/* -heap   0x0100                                   HEAP AREA SIZE            */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/****************************************************************************/
/* SPECIFY THE SYSTEM MEMORY MAP                                            */
/****************************************************************************/

MEMORY
{
    SFR                     : origin = 0x0000, length = 0x0010
    PERIPHERALS_8BIT        : origin = 0x0010, length = 0x00F0
    PERIPHERALS_16BIT       : origin = 0x0100, length = 0x0100
    RAM                     : origin = 0x1C00, length = 0x0FFE
    INFOA                   : origin = 0x1980, length = 0x0080
    INFOB                   : origin = 0x1900, length = 0x0080
    INFOC                   : origin = 0x1880, length = 0x0080
    INFOD                   : origin = 0x1800, length = 0x0080
    FLASH                   : origin = 0x8000, length = 0x6E00
    INT00                   : origin = 0xEF80, length = 0x0002
    INT01                   : origin = 0xEF82, length = 0x0002
    INT02                   : origin = 0xEF84, length = 0x0002
    INT03                   : origin = 0xEF86, length = 0x0002
    INT04                   : origin = 0xEF88, length = 0x0002
    INT05                   : origin = 0xEF8A, length = 0x0002
    INT06                   : origin = 0xEF8C, length = 0x0002
    INT07                   : origin = 0xEF8E, length = 0x0002
    INT08                   : origin = 0xEF90, length = 0x0002
    INT09                   : origin = 0xEF92, length = 0x0002
    INT10                   : origin = 0xEF94, length = 0x0002
    INT11                   : origin = 0xEF96, length = 0x0002
    INT12                   : origin = 0xEF98, length = 0x0002
    INT13                   : origin = 0xEF9A, length = 0x0002
    INT14                   : origin = 0xEF9C, length = 0x0002
    INT15                   : origin = 0xEF9E, length = 0x0002
    INT16                   : origin = 0xEFA0, length = 0x0002
    INT17                   : origin = 0xEFA2, length = 0x0002
    INT18                   : origin = 0xEFA4, length = 0x0002
    INT19                   : origin = 0xEFA6, length = 0x0002
    INT20                   : origin = 0xEFA8, length = 0x0002
    INT21                   : origin = 0xEFAA, length = 0x0002
    INT22                   : origin = 0xEFAC, length = 0x0002
    INT23                   : origin = 0xEFAE, length = 0x0002
    INT24                   : origin = 0xEFB0, length = 0x0002
    INT25                   : origin = 0xEFB2, length = 0x0002
    INT26                   : origin = 0xEFB4, length = 0x0002
    INT27                   : origin = 0xEFB6, length = 0x0002
    INT28                   : origin = 0xEFB8, length = 0x0002
    INT29                   : origin = 0xEFBA, length = 0x0002
    INT30                   : origin = 0xEFBC, length = 0x0002
    INT31                   : origin = 0xEFBE, length = 0x0002
    INT32                   : origin = 0xEFC0, length = 0x0002
    INT33                   : origin = 0xEFC2, length = 0x0002
    INT34                   : origin = 0xEFC4, length = 0x0002
    INT35                   : origin = 0xEFC6, length = 0x0002
    INT36                   : origin = 0xEFC8, length = 0x0002
    INT37                   : origin = 0xEFCA, length = 0x0002
    INT38                   : origin = 0xEFCC, length = 0x0002
    INT39                   : origin = 0xEFCE, length = 0x0002
    INT40                   : origin = 0xEFD0, length = 0x0002
    INT41                   : origin = 0xEFD2, length = 0x0002
    INT42                   : origin = 0xEFD4, length = 0x0002
    INT43                   : origin = 0xEFD6, length = 0x0002
    INT44                   : origin = 0xEFD8, length = 0x0002
    INT45                   : origin = 0xEFDA, length = 0x0002
    INT46                   : origin = 0xEFDC, length = 0x0002
    INT47                   : origin = 0xEFDE, length = 0x0002
    INT48                   : origin = 0xEFE0, length = 0x0002
    INT49                   : origin = 0xEFE2, length = 0x0002
    INT50                   : origin = 0xEFE4, length = 0x0002
    INT51                   : origin = 0xEFE6, length = 0x0002
    INT52                   : origin = 0xEFE8, length = 0x0002
    INT53                   : origin = 0xEFEA, length = 0x0002
    INT54                   : origin = 0xEFEC, length = 0x0002
    INT55                   : origin = 0xEFEE, length = 0x0002
    INT56                   : origin = 0xEFF0, length = 0x0002
    INT57                   : origin = 0xEFF2, length = 0x0002
    INT58                   : origin = 0xEFF4, length = 0x0002
    INT59                   : origin = 0xEFF6, length = 0x0002
    INT60                   : origin = 0xEFF8, length = 0x0002
    INT61                   : origin = 0xEFFA, length = 0x0002
    INT62                   : origin = 0xEFFC, length = 0x0002
    RESET                   : origin = 0xEFFE, length = 0x0002
}

/****************************************************************************/
/* SPECIFY THE SECTIONS ALLOCATION INTO MEMORY                              */
/****************************************************************************/

SECTIONS
{
    .bss        : {} > RAM                /* GLOBAL & STATIC VARS              */
    .data       : {} > RAM                /* GLOBAL & STATIC VARS              */
    .sysmem     : {} > RAM                /* DYNAMIC MEMORY ALLOCATION AREA    */
    .stack      : {} > RAM (HIGH)         /* SOFTWARE SYSTEM STACK             */

    .text       : {} > FLASH              /* CODE                              */
    .cinit      : {} > FLASH              /* INITIALIZATION TABLES             */
    .const      : {} > FLASH              /* CONSTANT DATA                     */
    .cio        : {} > RAM                /* C I/O BUFFER                      */

    .pinit      : {} > FLASH              /* C++ CONSTRUCTOR TABLES            */
    .init_array : {} > FLASH              /* C++ CONSTRUCTOR TABLES            */
    .mspabi.exidx : {} > FLASH            /* C++ CONSTRUCTOR TABLES            */
    .mspabi.extab : {} > FLASH            /* C++ CONSTRUCTOR TABLES            */

    .infoA     : {} > INFOA              /* MSP430 INFO FLASH MEMORY SEGMENTS */
    .infoB     : {} > INFOB
    .infoC     : {} > INFOC
    .infoD     : {} > INFOD

    /* MSP430 INTERRUPT VECTORS          */
    .int00       : {}               > INT00
    .int01       : {}               > INT01
    .int02       : {}               > INT02
    .int03       : {}               > INT03
    .int04       : {}               > INT04
    .int05       : {}               > INT05
    .int06       : {}               > INT06
    .int07       : {}               > INT07
    .int08       : {}               > INT08
    .int09       : {}               > INT09
    .int10       : {}               > INT10
    .int11       : {}               > INT11
    .int12       : {}               > INT12
    .int13       : {}               > INT13
    .int14       : {}               > INT14
    .int15       : {}               > INT15
    .int16       : {}               > INT16
    .int17       : {}               > INT17
    .int18       : {}               > INT18
    .int19       : {}               > INT19
    .int20       : {}               > INT20
    .int21       : {}               > INT21
    .int22       : {}               > INT22
    .int23       : {}               > INT23
    .int24       : {}               > INT24
    .int25       : {}               > INT25
    .int26       : {}               > INT26
    .int27       : {}               > INT27
    .int28       : {}               > INT28
    .int29       : {}               > INT29
    .int30       : {}               > INT30
    .int31       : {}               > INT31
    .int32       : {}               > INT32
    .int33       : {}               > INT33
    .int34       : {}               > INT34
    .int35       : {}               > INT35
    .int36       : {}               > INT36
    .int37       : {}               > INT37
    .int38       : {}               > INT38
    .int39       : {}               > INT39
    .int40       : {}               > INT40
    .int41       : {}               > INT41
    .int42       : {}               > INT42
    .int43       : {}               > INT43
    .int44       : {}               > INT44
    AES          : { * ( .int45 ) } > INT45 type = VECT_INIT
    RTC          : { * ( .int46 ) } > INT46 type = VECT_INIT
    .int47       : {}               > INT47
    PORT2        : { * ( .int48 ) } > INT48 type = VECT_INIT
    PORT1        : { * ( .int49 ) } > INT49 type = VECT_INIT
    TIMER1_A1    : { * ( .int50 ) } > INT50 type = VECT_INIT
    TIMER1_A0    : { * ( .int51 ) } > INT51 type = VECT_INIT
    DMA          : { * ( .int52 ) } > INT52 type = VECT_INIT
    CC1101       : { * ( .int53 ) } > INT53 type = VECT_INIT
    TIMER0_A1    : { * ( .int54 ) } > INT54 type = VECT_INIT
    TIMER0_A0    : { * ( .int55 ) } > INT55 type = VECT_INIT
    ADC12        : { * ( .int56 ) } > INT56 type = VECT_INIT
    USCI_B0      : { * ( .int57 ) } > INT57 type = VECT_INIT
    USCI_A0      : { * ( .int58 ) } > INT58 type = VECT_INIT
    WDT          : { * ( .int59 ) } > INT59 type = VECT_INIT
    COMP_B       : { * ( .int60 ) } > INT60 type = VECT_INIT
    UNMI         : { * ( .int61 ) } > INT61 type = VECT_INIT
    SYSNMI       : { * ( .int62 ) } > INT62 type = VECT_INIT
    .reset       : {}               > RESET  /* MSP430 RESET VECTOR         */ 
}

/****************************************************************************/
/* INCLUDE PERIPHERALS MEMORY MAP                                           */
/****************************************************************************/

-l cc430f5137.cmd

