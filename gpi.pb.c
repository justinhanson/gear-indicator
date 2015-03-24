/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.2 at Mon Mar  9 23:33:14 2015. */

#include "gpi.pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t GPIData_fields[5] = {
    PB_FIELD(  1, ENUM    , REQUIRED, STATIC  , FIRST, GPIData, type, type, 0),
    PB_FIELD(  2, MESSAGE , OPTIONAL, STATIC  , OTHER, GPIData, gearMessage, type, &GearMessage_fields),
    PB_FIELD(  3, MESSAGE , OPTIONAL, STATIC  , OTHER, GPIData, cutOffMessage, gearMessage, &CutOffMessage_fields),
    PB_FIELD(  4, MESSAGE , OPTIONAL, STATIC  , OTHER, GPIData, requestMessage, cutOffMessage, &RequestMessage_fields),
    PB_LAST_FIELD
};

const pb_field_t GearMessage_fields[2] = {
    PB_FIELD(  1, INT32   , OPTIONAL, STATIC  , FIRST, GearMessage, gear, gear, 0),
    PB_LAST_FIELD
};

const pb_field_t CutOffMessage_fields[7] = {
    PB_FIELD(  1, INT32   , OPTIONAL, STATIC  , FIRST, CutOffMessage, firstCutoff, firstCutoff, 0),
    PB_FIELD(  2, INT32   , OPTIONAL, STATIC  , OTHER, CutOffMessage, secondCutoff, firstCutoff, 0),
    PB_FIELD(  3, INT32   , OPTIONAL, STATIC  , OTHER, CutOffMessage, thirdCutoff, secondCutoff, 0),
    PB_FIELD(  4, INT32   , OPTIONAL, STATIC  , OTHER, CutOffMessage, fourthCutoff, thirdCutoff, 0),
    PB_FIELD(  5, INT32   , OPTIONAL, STATIC  , OTHER, CutOffMessage, fifthCutoff, fourthCutoff, 0),
    PB_FIELD(  6, INT32   , OPTIONAL, STATIC  , OTHER, CutOffMessage, sixthCutoff, fifthCutoff, 0),
    PB_LAST_FIELD
};

const pb_field_t RequestMessage_fields[2] = {
    PB_FIELD(  1, ENUM    , REQUIRED, STATIC  , FIRST, RequestMessage, type, type, 0),
    PB_LAST_FIELD
};


/* Check that field information fits in pb_field_t */
#if !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_32BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in 8 or 16 bit
 * field descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(GPIData, gearMessage) < 65536 && pb_membersize(GPIData, cutOffMessage) < 65536 && pb_membersize(GPIData, requestMessage) < 65536), YOU_MUST_DEFINE_PB_FIELD_32BIT_FOR_MESSAGES_GPIData_GearMessage_CutOffMessage_RequestMessage)
#endif

#if !defined(PB_FIELD_16BIT) && !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_16BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in the default
 * 8 bit descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(GPIData, gearMessage) < 256 && pb_membersize(GPIData, cutOffMessage) < 256 && pb_membersize(GPIData, requestMessage) < 256), YOU_MUST_DEFINE_PB_FIELD_16BIT_FOR_MESSAGES_GPIData_GearMessage_CutOffMessage_RequestMessage)
#endif


