/*
* SPDX-License-Identifier: BSD-3-Clause
* 
* Copyright © 2025, Synopsys Inc.
* Copyright © 2025, Solid Sands B.V.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above
*    copyright notice, this list of conditions and the following
*    disclaimer in the documentation and/or other materials provided
*    with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>

#ifdef _LDBL_EQ_DBL
#if __STDC_IEC_559__
#define ALLOWED_ULP 2
#else
#define ALLOWED_ULP 4
#endif

typedef struct {
    long double complex input;
    long double complex reference;
} test_case;

static int within_rel_prec(long double complex x, long double complex ref, long double rel_prec) {
    if (cabsl(ref) == 0.0L) {
        return cabsl(x) == 0.0L;
    }
    long double ratio = cabsl(x / ref);
    return fabsl(ratio - 1.0L) <= rel_prec;
}

int main(void) {
    test_case tests[] = {
    { 0.0L, 0.0L },

    { 0.0625000000000000000000000000000000000000000000000000000000000L,
        0.0624593812555403088032090969419910779140198233574413056605194L },
    { 0.0625000000000000000000000000000000000000000000000000000000000L +
        0.0468750000000000000000000000000000000000000000000000000000000L * (long double complex) I,
        0.0625277569630907597786501757112239768666323561347854024018217L +
        0.0468005976234390405792696677083395297493314548689680792944458L * (long double complex) I },
    { 0.0273437500000000000000000000000000000000000000000000000000000L +
        0.0937500000000000000000000000000000000000000000000000000000000L * (long double complex) I,
        0.0274611675088708853864730622232975656024898508894147265916086L +
        0.0938523807707581810887806522350996180133478734485504681822068L * (long double complex) I },
    { -0.0429687500000000000000000000000000000000000000000000000000000L +
        0.114257812500000000000000000000000000000000000000000000000000L * (long double complex) I,
        -0.0432379938470243194722651981487468514596101554183129645893284L +
        0.114400463893750824253016662523198508237836910319039978909346L * (long double complex) I },
    { -0.128662109375000000000000000000000000000000000000000000000000L +
        0.0820312500000000000000000000000000000000000000000000000000000L * (long double complex) I,
        -0.128734162103663243785939630755422537819061686789986282594322L +
        0.0814461969133878397430193494680365228304757065595655769056968L * (long double complex) I },

    { -0.190185546875000000000000000000000000000000000000000000000000L -
        0.0144653320312500000000000000000000000000000000000000000000000L * (long double complex) I,
        -0.189076164981036285668877339239270867978887999337284573530421L -
        0.0142110403637310329965895316712720172140003886522189814225688L * (long double complex) I },
    { -0.179336547851562500000000000000000000000000000000000000000000L -
        0.157104492187500000000000000000000000000000000000000000000000L * (long double complex) I,
        -0.180535997859051734608351276533528417639100634755452699743914L -
        0.155200846111334323897940560825294730616484568126868882124504L * (long double complex) I },
    { -0.0615081787109375000000000000000000000000000000000000000000000L -
        0.291606903076171875000000000000000000000000000000000000000000L * (long double complex) I,
        -0.0642463928651769604080286199777230436708267313213612248999341L -
        0.295278301349239326187393816172546641139968429144292329513432L * (long double complex) I },
    { 0.157196998596191406250000000000000000000000000000000000000000L -
        0.337738037109375000000000000000000000000000000000000000000000L * (long double complex) I,
        0.165957653053747411964079185343414610530447291766262796433469L -
        0.339631685012985928311937778656222023264002619447310481860525L * (long double complex) I },
    { 0.410500526428222656250000000000000000000000000000000000000000L -
        0.219840288162231445312500000000000000000000000000000000000000L * (long double complex) I,
        0.407808878522601969739575148955506681493067222261111685264446L -
        0.204161528955985093839135478470331741194164606712149495235280L * (long double complex) I },

    { 0.575380742549896240234375000000000000000000000000000000000000L +
        0.0880351066589355468750000000000000000000000000000000000000000L * (long double complex) I,
        0.549055380535012934873231021801386661384739418448145495184983L +
        0.0763242783919902125835889680010980396255732104610962427907717L * (long double complex) I },
    { 0.509354412555694580078125000000000000000000000000000000000000L +
        0.519570663571357727050781250000000000000000000000000000000000L * (long double complex) I,
        0.543597731800714817209370313553582570129850490395282444075738L +
        0.468156584836746764060895514441756640995540084030490256001721L * (long double complex) I },
    { 0.119676414877176284790039062500000000000000000000000000000000L +
        0.901586472988128662109375000000000000000000000000000000000000L * (long double complex) I,
        0.244933101283648851281610529691616302508098287282652933352877L +
        1.06585383884676473156636110522911775356147889473014258281093L * (long double complex) I },
    { -0.556513439863920211791992187500000000000000000000000000000000L +
        0.991343784146010875701904296875000000000000000000000000000000L * (long double complex) I,
        -0.770852114485812033114950802346412967769850115755219606038709L +
        0.856466264313993592078514622702024246861586263477175259586937L * (long double complex) I },
    { -1.30002127797342836856842041015625000000000000000000000000000L +
        0.573958704248070716857910156250000000000000000000000000000000L * (long double complex) I,
        -1.12666913993171227715791832317669203245971409832746223293962L +
        0.343393768307227851805650860427750481936279910756843580862356L * (long double complex) I },

    { -1.73049030615948140621185302734375000000000000000000000000000L -
        0.401057254232000559568405151367187500000000000000000000000000L * (long double complex) I,
        -1.33347557631811752610790726661397283660446814538090208666234L -
        0.198984387535949738859129168480544054956439091292986361421749L * (long double complex) I },
    { -1.42969736548548098653554916381835937500000000000000000000000L -
        1.69892498385161161422729492187500000000000000000000000000000L * (long double complex) I,
        -1.48598792613283971441338764890464921484522002030333161166412L -
        0.820389158964413920573996975184072605493477489243969640330083L * (long double complex) I },
    { -0.155503627596772275865077972412109375000000000000000000000000L -
        2.77119800796572235412895679473876953125000000000000000000000L * (long double complex) I,
        -1.68009343502052616391364636101514477182551046939866722545442L -
        1.51071671057835217522201909541655199903004704182913982474789L * (long double complex) I },
    { 1.92289487837751948973163962364196777343750000000000000000000L -
        2.88782572866330156102776527404785156250000000000000000000000L * (long double complex) I,
        1.92961509182908190915620744599017602381020514992623642115800L -
        0.963740382781455034916833858797324199890876979501368934389002L * (long double complex) I },
    { 4.08876417487499566050246357917785644531250000000000000000000L -
        1.44565456988016194372903555631637573242187500000000000000000L * (long double complex) I,
        2.17056525791255989316392302225859836576762154760085334209684L -
        0.331746820763646449865848357586990202956155340626307714825967L * (long double complex) I },

    { 5.17300510228511711829924024641513824462890625000000000000000L +
        1.62091856127608480164781212806701660156250000000000000000000L * (long double complex) I,
        2.39037706627807577594362074862032096078058866216007080862749L +
        0.298896749908533638741479863249040865587654725719575386371247L * (long double complex) I },
    { 3.95731618132805351706338115036487579345703125000000000000000L +
        5.50067238798992264037224231287837028503417968750000000000000L * (long double complex) I,
        2.60487816718732063039708045202898996458250059738405003215450L +
        0.941964090512905342492951885903193992721666772896630666435571L * (long double complex) I },
    { -0.168188109664388463215800584293901920318603515625000000000000L +
        8.46865952398596277816977817565202713012695312500000000000000L * (long double complex) I,
        -2.82621654702177532623957119030140912479195294378678902656238L +
        1.55079906256194158045899806165763055623123325025709592593154L * (long double complex) I },
    { -6.51968275265386054684313421603292226791381835937500000000000L +
        8.34251844173767143075792773743160068988800048828125000000000L * (long double complex) I,
        -3.05232782533383564927545386113131088009371141505211636968965L +
        0.905270319017501053903726806638429770856723868253900146705689L * (long double complex) I },
    { -12.7765715839571141199115800191066227853298187255859375000000L +
        3.45275637724727602062557707540690898895263671875000000000000L * (long double complex) I,
        -3.27723515572059406514999477764685847058282207413502828012494L +
        0.263220405622396166698515927703973631166547899087866354950377L * (long double complex) I },

    { -15.3661388668925711353807628256618045270442962646484375000000L -
        6.12967231072055956930810793892305810004472732543945312500000L * (long double complex) I,
        -3.49980981486137419418758967632391593700303586007542936021206L -
        0.378936989377524413846013390185545104408553189066172450505140L * (long double complex) I },
    { -10.7688846338521514583996818714695109520107507705688476562500L -
        17.6542764608899879208436800581694114953279495239257812500000L * (long double complex) I,
        -3.72202355666775994369566412918421574093464479206466935181935L -
        1.02254535201955944295356874134275746010220626192701798719366L * (long double complex) I },
    { 2.47182271181533948223307817215754766948521137237548828125000L -
        25.7309399362791015146434414617715447093360126018524169921875L * (long double complex) I,
        3.94506684679819038148599958089026185302110636308948584196938L -
        1.47495467905731999529143488603730550186091654939752772742871L * (long double complex) I },
    { 21.7700276640246656182156592684862062014872208237648010253906L -
        23.8770729024175969029686328326533839572221040725708007812500L * (long double complex) I,
        4.16855592069120135382670571950288789985865733383377652037532L -
        0.831286425394456954092021969278422005318783670673387251810739L * (long double complex) I },
    { 39.6778323408378632954421338929762441694037988781929016113281L -
        7.54955215439909768930688838128872930610668845474720001220703L * (long double complex) I,
        4.39186396171243215104921911568429665537410087385821674993402L -
        0.187967510140470859953395095806582859105803147411803440014663L * (long double complex) I },

    { 45.3399964566371865624223001789427911489838152192533016204834L +
        22.2088221012292997822747120384434538209461607038974761962890L * (long double complex) I,
        4.61492512338423922736570061640072554778021152941534725418278L +
        0.455399864501031716423603351002125402621231606210687205770891L * (long double complex) I },
    
    /* casinhl(+5.500 + 5.0 * (long double complex) I) */
    { 5.5L +
        5.0L * (long double complex) I,
      2.699541384316091448872442152519987505L +
        0.73331675299678489263052876265049390879L * (long double complex) I},
        /* casinhl(+0.001 + 5.0 * (long double complex) I) */
    { 0.00100000000020372681319713592529296875L +
        5.0L * (long double complex) I,
      2.2924316908241090023360661700106176388L +
        1.5705922026526353507481515417984426789L * (long double complex) I},
        /* casinhl(+0.000 + 5.0 * (long double complex) I) */
    { 0.0L +
        5.0L * (long double complex) I,
      2.2924316695611776878007873113480154316L +
        1.5707963267948966192313216916397514421L * (long double complex) I},
        /* casinhl(-0.000 + 5.0 * (long double complex) I) */
    { -1.0 * (0.0L + -5.0L * (long double complex) I),
      -2.2924316695611776878007873113480154316L +
        1.5707963267948966192313216916397514421L * (long double complex) I},
        /* casinhl(-0.001 + 5.0 * (long double complex) I) */
    { -0.00100000000020372681319713592529296875L +
        5.0L * (long double complex) I,
      -2.2924316908241090023360661700106176388L +
        1.5705922026526353507481515417984426789L * (long double complex) I},
        /* casinhl(-5.005 + 5.0 * (long double complex) I) */
    { -5.00500000081956386566162109375L +
        5.0L * (long double complex) I,
      -2.6497010940877766623835819103116638246L +
        0.7799039033573315491708776586146430015L * (long double complex) I},

        /* casinhl(+5.500 + 1.0 * (long double complex) I) */
    { 5.5L +
        1.0L * (long double complex) I,
      2.4215734590506472832551764743408198521L +
        0.17709928865657500701574616561599207884L * (long double complex) I},
        /* casinhl(+0.001 + 1.0 * (long double complex) I) */
    { 0.00100000000020372681319713592529296875L +
        1.0L * (long double complex) I,
      0.031625411243185808119677973274798562706L +
        1.5391761860141264362817986763728995476L * (long double complex) I},
        /* casinhl(+0.000 + 1.0 * (long double complex) I) */
    { 0.0L +
        1.0L * (long double complex) I,
      0.0L +
        1.5707963267948966192313216916397514421L * (long double complex) I},
        /* casinhl(-0.000 + 1.0 * (long double complex) I) */
    { -0.0L +
        1.0L * (long double complex) I,
      -0.0L +
        1.5707963267948966192313216916397514421L * (long double complex) I},
        /* casinhl(-0.001 + 1.0 * (long double complex) I) */
    { -0.00100000000020372681319713592529296875L +
        1.0L * (long double complex) I,
      -0.031625411243185808119677973274798562706L +
        1.5391761860141264362817986763728995476L * (long double complex) I},
        /* casinhl(-5.005 + 1.0 * (long double complex) I) */
    { -5.00500000081956386566162109375L +
        1.0L * (long double complex) I,
      -2.3319204172665009535854099220832561521L +
        0.19361108350095466908008324661105735207L * (long double complex) I},

        /* casinhl(+0.010 + +1.010 * (long double complex) I) */
    { 0.00999999999839928932487964630126953125L +
        1.00999999977648258209228515625L * (long double complex) I,
      0.15530130895851037533249923716679417024L +
        1.5066194333770467809224561186167656533L * (long double complex) I},
        /* casinhl(+0.001 + +1.001 * (long double complex) I) */
    { 0.00100000000020372681319713592529296875L +
        1.00100000016391277313232421875L * (long double complex) I,
      0.049132250821569752251467056475900506368L +
        1.5504498794209502900922829943924564204L * (long double complex) I},
        /* casinhl(+0.000 + +1.001 * (long double complex) I) */
    { 0.0L +
        1.00100000016391277313232421875L * (long double complex) I,
      0.044717637272594233596467077872233378808L +
        1.5707963267948966192313216916397514421L * (long double complex) I},
        /* casinhl(+0.000 + +0.999 * (long double complex) I) */
    { 0.0L +
        0.99900000006891787052154541015625L * (long double complex) I,
      0.0L +
        1.5260712411675990551755823432327881636L * (long double complex) I},
        /* casinhl(-0.001 + +0.999 * (long double complex) I) */
    { -0.00100000000020372681319713592529296875L +
        0.99900000006891787052154541015625L * (long double complex) I,
      -0.020358030197614237682124794531127573737L +
        1.5216592829506333903849263470204566759L * (long double complex) I},
        /* casinhl(-0.010 + +0.999 * (long double complex) I) */
    { -0.00999999999839928932487964630126953125L +
        0.99900000006891787052154541015625L * (long double complex) I,
      -0.095226222878705825259796955825789701498L +
        1.4657486848547155179660578288581435168L * (long double complex) I},

        /* casinhl(+5.500 + -1.0 * (long double complex) I) */
    { 5.5L +
        -1.0L * (long double complex) I,
      2.4215734590506472832551764743408198521L +
        -0.17709928865657500701574616561599207884L * (long double complex) I},
        /* casinhl(+0.001 + -1.0 * (long double complex) I) */
    { 0.00100000000020372681319713592529296875L +
        -1.0L * (long double complex) I,
      0.031625411243185808119677973274798562706L +
        -1.5391761860141264362817986763728995476L * (long double complex) I},
        /* casinhl(+0.000 + -1.0 * (long double complex) I) */
    { 0.0L +
        -1.0L * (long double complex) I,
      0.0L +
        -1.5707963267948966192313216916397514421L * (long double complex) I},
        /* casinhl(-0.000 + -1.0 * (long double complex) I) */
    { 0.0L +
        -1.0L * (long double complex) I,
      0.0L +
        -1.5707963267948966192313216916397514421L * (long double complex) I},
        /* casinhl(-0.001 + -1.0 * (long double complex) I) */
    { -0.00100000000020372681319713592529296875L +
        -1.0L * (long double complex) I,
      -0.031625411243185808119677973274798562706L +
        -1.5391761860141264362817986763728995476L * (long double complex) I},
        /* casinhl(-5.005 + -1.0 * (long double complex) I) */
    { -5.00500000081956386566162109375L +
        -1.0L * (long double complex) I,
      -2.3319204172665009535854099220832561521L +
        -0.19361108350095466908008324661105735207L * (long double complex) I},

    /* casinhl(+0.010 + -1.010 * (long double complex) I) */
    { 0.00999999999839928932487964630126953125L +
        -1.00999999977648258209228515625L * (long double complex) I,
      0.15530130895851037533249923716679417024L +
        -1.5066194333770467809224561186167656533L * (long double complex) I},
        /* casinhl(+0.001 + -1.001 * (long double complex) I) */
    { 0.00100000000020372681319713592529296875L +
        -1.00100000016391277313232421875L * (long double complex) I,
      0.049132250821569752251467056475900506368L +
        -1.5504498794209502900922829943924564204L * (long double complex) I},
        /* casinhl(+0.000 + -1.001 * (long double complex) I) */
    { 0.0L +
        -1.00100000016391277313232421875L * (long double complex) I,
      0.044717637272594233596467077872233378808L +
        -1.5707963267948966192313216916397514421L * (long double complex) I},
        /* casinhl(+0.000 + -0.999 * (long double complex) I) */
    { 0.0L +
        -0.99900000006891787052154541015625L * (long double complex) I,
      0.0L +
        -1.5260712411675990551755823432327881636L * (long double complex) I},
        /* casinhl(-0.001 + -0.999 * (long double complex) I) */
    { -0.00100000000020372681319713592529296875L +
        -0.99900000006891787052154541015625L * (long double complex) I,
      -0.020358030197614237682124794531127573737L +
        -1.5216592829506333903849263470204566759L * (long double complex) I},
        /* casinhl(-0.010 + -0.999 * (long double complex) I) */
    { -0.00999999999839928932487964630126953125L +
        -0.99900000006891787052154541015625L * (long double complex) I,
      -0.095226222878705825259796955825789701498L +
        -1.4657486848547155179660578288581435168L * (long double complex) I},

        /* casinhl(+5.500 + -5.0 * (long double complex) I) */
    { 5.5L +
        -5.0L * (long double complex) I,
      2.699541384316091448872442152519987505L +
        -0.73331675299678489263052876265049390879L * (long double complex) I},
        /* casinhl(+0.001 + -5.0 * (long double complex) I) */
    { 0.00100000000020372681319713592529296875L +
        -5.0L * (long double complex) I,
      2.2924316908241090023360661700106176388L +
        -1.5705922026526353507481515417984426789L * (long double complex) I},
        /* casinhl(+0.000 + -5.0 * (long double complex) I) */
    { -1.0 * (-0.0L + 5.0L * (long double complex) I),
      -2.2924316695611776878007873113480154316L +
        -1.5707963267948966192313216916397514421L * (long double complex) I},
        /* casinhl(-0.000 + -5.0 * (long double complex) I) */
    { -0.0L +
        -5.0L * (long double complex) I,
      -2.2924316695611776878007873113480154316L +
        -1.5707963267948966192313216916397514421L * (long double complex) I},
        /* casinhl(-0.001 + -5.0 * (long double complex) I) */
    { -0.00100000000020372681319713592529296875L +
        -5.0L * (long double complex) I,
      -2.2924316908241090023360661700106176388L +
        -1.5705922026526353507481515417984426789L * (long double complex) I},
        /* casinhl(-5.005 + -5.0 * (long double complex) I) */
    { -5.00500000081956386566162109375L +
        -5.0L * (long double complex) I,
      -2.6497010940877766623835819103116638246L +
        -0.7799039033573315491708776586146430015L * (long double complex) I},

    };

    const int num_tests = sizeof(tests) / sizeof(tests[0]);
    int pass_count = 0;
    long double rel_prec = ALLOWED_ULP * DBL_EPSILON;
    int totat_tests = 0;

    for (int i = 0; i < num_tests; ++i) {
        /* Normal Inputs*/
        long double complex result = casinhl(tests[i].input);
        long double complex ref = tests[i].reference;

        int res_ok = within_rel_prec(result, ref, rel_prec);
        totat_tests++;

        if (res_ok) {
            pass_count++;
        } else {
        printf("Test %d FAILED:\n", i + 1);
        printf("  Input:       casinhl(%La + %Lai)\n", creall(tests[i].input), cimagl(tests[i].input));
        printf("  Computed:    real = %La, imag = %La\n", creall(result), cimagl(result));
        printf("  Reference:   real = %La, imag = %La\n", creall(ref), cimagl(ref));
        printf("  |x/ref| = %.17Lg, error = %.1Le (allowed <= %.1Le)\n",
                cabsl(result / ref) , fabsl(cabs(result / ref) - 1.0L), rel_prec);
        printf("\n");
        }

        /* Conjugate Inputs*/
        long double complex conj_result = casinhl(conjl(tests[i].input));
        long double complex conj_ref = conjl(tests[i].reference);

        res_ok = within_rel_prec(conj_result, conj_ref, rel_prec);
        totat_tests++;

        if (res_ok) {
            pass_count++;
        } else {
        printf("Test %d FAILED (Conjugate):\n", i + 1);
        printf("  Input:       casinhl(%La + %Lai)\n", creall(tests[i].input), cimagl(tests[i].input));
        printf("  Computed:    real = %La, imag = %La\n", creall(conj_result), cimagl(conj_result));
        printf("  Reference:   real = %La, imag = %La\n", creall(conj_ref), cimagl(conj_ref));
        printf("  |x/ref| = %.17Lg, error = %.1Le (allowed <= %.1Le)\n",
                cabsl(conj_result / conj_ref) , fabsl(cabsl(conj_result / conj_ref) - 1.0L), rel_prec);
        printf("\n");
        }

        /* Negative Inputs*/
        double complex neg_result = casinhl(-tests[i].input);
        double complex neg_ref = -tests[i].reference;

        res_ok = within_rel_prec(neg_result, neg_ref, rel_prec);
        totat_tests++;

        if (res_ok) {
            pass_count++;
        } else {

        printf("Test %d FAILED (Negative):\n", i + 1);
        printf("  Input:       casinhl(%La + %Lai)\n", creall(tests[i].input), cimagl(tests[i].input));
        printf("  Computed:    real = %La, imag = %La\n", creall(neg_result), cimagl(neg_result));
        printf("  Reference:   real = %La, imag = %La\n", creall(neg_ref), cimagl(neg_ref));
        printf("  |x/ref| = %.17Lg, error = %.1Le (allowed <= %.1Le)\n",
                cabsl(neg_result / neg_ref) , fabsl(cabsl(neg_result / neg_ref) - 1.0L), rel_prec);
        printf("\n");
        }
    }

    printf("Summary: %d/%d tests passed.\n", pass_count, totat_tests);
    return (pass_count == totat_tests) ? 0 : 1;
}
#else
int main(void) {
    printf( "Test skipped: for long double not equal to double" );
    return 0;
}
#endif
