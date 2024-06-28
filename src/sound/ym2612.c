/* COPYRIGHT (C) HARRY CLARK 2024 */

/* SEGA MEGA DRIVE EMULATOR */

/* THIS FILE PERTAINS TOWARDS THE FUNCTIONALITY SURROUNDING THE YM2612 */
/* AND THE CORRESPODENCE ASSOCIATED WITH FM AUDIO INTEGRATION */ 

/* NESTED INCLUDES */

#include "common.h"
#include "md.h"
#include "ym2612.h"

#undef USE_FM_CHANNELS

/* INITIALISE THE CORE STATE MACHINEE FOR THE YM2612 */
/* AND ANY OF IT'S CORRESPONDENCE ASSOCIATED */

/* THIS WILL BE DONE BY EVALUATING HOW MANY FM CHANNELS */
/* THE AUDIO SYSTEM WILL CONSIST OF */

void YM2612_INIT(struct YM2612* YM2612)
{
    U8 INDEX = 0;

    /* ITERATE THORUGH EACH INSTANCE OF THE FM CHANNEL CONFIGURATION */
    /* TO DETERMINE THEIR RESPECTIVE FUNCTIONALITY */

    for (YM2612->FM_CHANNEL = 0; YM2612->FM_CHANNEL; YM2612->FM_CHANNEL++)
    {
        YM2612->CHANNEL.PAN_LEFT = true;
        YM2612->CHANNEL.PAN_RIGHT = true;
    }
    
    YM2612->STATE.DAC = 0;
    YM2612->DAC_CHANNEL_DISABLED = true;
    YM2612->TIMER[INDEX].VALUE = 0;

    /* FOR EACH SUBSEQUENT INSTANCE OF THE FM AUDIO TIMER */
    /* EACH CORRESPONDENCE WILL BE ITERATED IN ORDER TO DETERMINE THE SAMPLE RATE */

    for (INDEX = 0; INDEX < sizeof(&YM2612->TIMER); INDEX++)
    {
        YM2612->TIMER[INDEX].VALUE = FM_SR_DIV;
        YM2612->TIMER[INDEX].COUNTER = FM_SR_DIV;
        YM2612->TIMER[INDEX].ENABLED = false;
    }
}
