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

#ifdef _LDBL_EQ_DBL
#define ALLOWED_ULP 5

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
            0.0625407617964913908006016368963807600413311640539283833188625L },
        { 0.0625000000000000000000000000000000000000000000000000000000000L +
            0.0468750000000000000000000000000000000000000000000000000000000L * (long double complex) I,
            0.0624718077425454327224542553404426375128518960148053460129198L +
            0.0469493692649333206435299611269511208092493663958490436780783L * (long double complex) I },
        { 0.0273437500000000000000000000000000000000000000000000000000000L +
            0.0937500000000000000000000000000000000000000000000000000000000L * (long double complex) I,
            0.0272276494603394966425980221233731664158628771449964870436807L +
            0.0936478206847519540207362209758492337703382440724722809504681L * (long double complex) I },
        { -0.0429687500000000000000000000000000000000000000000000000000000L +
            0.114257812500000000000000000000000000000000000000000000000000L * (long double complex) I,
            -0.0427034668959421934800021633233784491762971198358984509501357L +
            0.114114242795007357717305251402153908142554439364609332419162L * (long double complex) I },
        { -0.128662109375000000000000000000000000000000000000000000000000L +
            0.0820312500000000000000000000000000000000000000000000000000000L * (long double complex) I,
            -0.128578217628451801559750609891863955834619733609100729518184L +
            0.0826200105615420894872147908955762157137817134265173417118634L * (long double complex) I },

        { -0.190185546875000000000000000000000000000000000000000000000000L -
            0.0144653320312500000000000000000000000000000000000000000000000L * (long double complex) I,
            -0.191330114822507397325945264042542270549861839901744221415371L -
            0.0147336662339009185428957897587763987595506569013943627539039L * (long double complex) I },
        { -0.179336547851562500000000000000000000000000000000000000000000L -
            0.157104492187500000000000000000000000000000000000000000000000L * (long double complex) I,
            -0.178033471037655555493222789262478871516161011101592920152698L -
            0.158957335118334494228765223162209260331509677809361682688430L * (long double complex) I },
        { -0.0615081787109375000000000000000000000000000000000000000000000L -
            0.291606903076171875000000000000000000000000000000000000000000L * (long double complex) I,
            -0.0590750625694287802492486654258035196969550323134900945116796L -
            0.288113865313587744560574317590079077765868833364131041610961L * (long double complex) I },
        { 0.157196998596191406250000000000000000000000000000000000000000L -
            0.337738037109375000000000000000000000000000000000000000000000L * (long double complex) I,
            0.149314155045627732655691737740132287078853324742726749905583L -
            0.335224352651457879449054190252384196301417877280776594817746L * (long double complex) I },
        { 0.410500526428222656250000000000000000000000000000000000000000L -
            0.219840288162231445312500000000000000000000000000000000000000L * (long double complex) I,
            0.410628968739660731482501771303255786262182824435763861403664L -
            0.237532624183341263980280053112321651990092714303321970935053L * (long double complex) I },

        { 0.575380742549896240234375000000000000000000000000000000000000L +
            0.0880351066589355468750000000000000000000000000000000000000000L * (long double complex) I,
            0.609057472591026525454212825599220100578449915159185621468099L +
            0.107130292111860311974954794843410704575294000835179526826742L * (long double complex) I },
        { 0.509354412555694580078125000000000000000000000000000000000000L +
            0.519570663571357727050781250000000000000000000000000000000000L * (long double complex) I,
            0.456520257936854200553436638916965508604258609877831600963487L +
            0.550604137509262606277123063465067163942657165513112890335725L * (long double complex) I },
        { 0.119676414877176284790039062500000000000000000000000000000000L +
            0.901586472988128662109375000000000000000000000000000000000000L * (long double complex) I,
            0.0888437576853459997418187115217900202824074837609472560692625L +
            0.812694762329887698917574783877743495969005362240006836549805L * (long double complex) I },
        { -0.556513439863920211791992187500000000000000000000000000000000L +
            0.991343784146010875701904296875000000000000000000000000000000L * (long double complex) I,
            -0.389459667028493219807385451697716203344876752236387605870007L +
            0.931100797284786686020343266602037011534314729327994200350298L * (long double complex) I },
        { -1.30002127797342836856842041015625000000000000000000000000000L +
            0.573958704248070716857910156250000000000000000000000000000000L * (long double complex) I,
            -1.03812895638810821626919758052701065229821619401480378680402L +
            0.970517548068308175659147683754415640283578939938904725232023L * (long double complex) I },

        { -1.73049030615948140621185302734375000000000000000000000000000L -
            0.401057254232000559568405151367187500000000000000000000000000L * (long double complex) I,
            -1.29874330205511910682808600527094859120113727899090311445864L -
            1.19061294064461653003694224567555873974940097223866174230877L * (long double complex) I },
        { -1.42969736548548098653554916381835937500000000000000000000000L -
            1.69892498385161161422729492187500000000000000000000000000000L * (long double complex) I,
            -0.651231509598215706394518589082646388577526763569203439434628L -
            1.50289755557415964456307223212752967910455530545640926115979L * (long double complex) I },
        { -0.155503627596772275865077972412109375000000000000000000000000L -
            2.77119800796572235412895679473876953125000000000000000000000L * (long double complex) I,
            -0.0527422667376490490716489821298157884130946883273413221452403L -
            1.74480631307992967161515019786952803556594782513682055762706L * (long double complex) I },
        { 1.92289487837751948973163962364196777343750000000000000000000L -
            2.88782572866330156102776527404785156250000000000000000000000L * (long double complex) I,
            0.568753049232133032360782637453097120901947455670660411400936L -
            1.94557887006090469571587888739754131033292115437154999325047L * (long double complex) I },
        { 4.08876417487499566050246357917785644531250000000000000000000L -
            1.44565456988016194372903555631637573242187500000000000000000L * (long double complex) I,
            1.22232557519307154263044209176510720867597571208349451687592L -
            2.14989588764310903274416537936299325581259061012791602557495L * (long double complex) I },

        { 5.17300510228511711829924024641513824462890625000000000000000L +
            1.62091856127608480164781212806701660156250000000000000000000L * (long double complex) I,
            1.26218641117747713176053181792432014302372351130250416723553L +
            2.37640626473033531513038806593706504778071995914161268449826L * (long double complex) I },
        { 3.95731618132805351706338115036487579345703125000000000000000L +
            5.50067238798992264037224231287837028503417968750000000000000L * (long double complex) I,
            0.618508668787146587196775460624082047751927424420294941481336L +
            2.60833885501452957210325184377170236625837793149304392007205L * (long double complex) I },
        { -0.168188109664388463215800584293901920318603515625000000000000L +
            8.46865952398596277816977817565202713012695312500000000000000L * (long double complex) I,
            -0.0197205309643900761747830643901973327896311288488404157857677L +
            2.83318031469853011706661066862689986508639196532998132197685L * (long double complex) I },
        { -6.51968275265386054684313421603292226791381835937500000000000L +
            8.34251844173767143075792773743160068988800048828125000000000L * (long double complex) I,
            -0.661198111387839028339369923536528523428492230179475287531771L +
            3.05340562856528667503698010618750679183395583699174840689375L * (long double complex) I },
        { -12.7765715839571141199115800191066227853298187255859375000000L +
            3.45275637724727602062557707540690898895263671875000000000000L * (long double complex) I,
            -1.30613810023159514841659968855666837930611587459925269809971L +
            3.27476921384124168049187907752623924446019041278026973864393L * (long double complex) I },

        { -15.3661388668925711353807628256618045270442962646484375000000L -
            6.12967231072055956930810793892305810004472732543945312500000L * (long double complex) I,
            -1.19060191274891606212997397555762481317450591339866330521748L -
            3.49848453605236733079194156153990423768503891147913880618018L * (long double complex) I },
        { -10.7688846338521514583996818714695109520107507705688476562500L -
            17.6542764608899879208436800581694114953279495239257812500000L * (long double complex) I,
            -0.547211391832399601615465654352564475338889520661202546354557L -
            3.72255862413800388166568428049902385870357628710447391057424L * (long double complex) I },
        { 2.47182271181533948223307817215754766948521137237548828125000L -
            25.7309399362791015146434414617715447093360126018524169921875L * (long double complex) I,
            0.0956991945821138424701954206776981768379011882957163289265975L -
            3.94580145129634946294361411840539200222799708368623176690545L * (long double complex) I },
        { 21.7700276640246656182156592684862062014872208237648010253906L -
            23.8770729024175969029686328326533839572221040725708007812500L * (long double complex) I,
            0.739033033093438964603445421544928430294240209147725041065944L -
            4.16860003863762254723686248101342349034898686781634217911763L * (long double complex) I },
        { 39.6778323408378632954421338929762441694037988781929016113281L -
            7.54955215439909768930688838128872930610668845474720001220703L * (long double complex) I,
            1.38271625573470427604917826739957729578372423992592337196514L -
            4.39157887970071735256077772301954635573933870651797967939774L * (long double complex) I },

        { 45.3399964566371865624223001789427911489838152192533016204834L +
            22.2088221012292997822747120384434538209461607038974761962890L * (long double complex) I,
            1.11524147882261077491622049186015759964310581855771782304232L +
            4.61480487927959780505457980614241132045328699248786937586621L * (long double complex) I },
        /* casinl(5 + 5.500 * (long double complex) I) */
        { 5.0L +
              5.0L * (long double complex) I,
          0.78039857998538412279399934942730594974L +
              2.6491961778064711496078085316829813678L * (long double complex) I},
        /* casinl(5 + 0.001 * (long double complex) I) */
        { 5.0L +
              0.00100000000020372681319713592529296875L * (long double complex) I,
          1.5705922026526353507481515417984426789L +
              2.2924316908241090023360661700106176388L * (long double complex) I},
        /* casinl(5 - 0.001 * (long double complex) I) */
        { 5.0L +
              -0.00100000000020372681319713592529296875L * (long double complex) I,
          1.5705922026526353507481515417984426789L +
              -2.2924316908241090023360661700106176388L * (long double complex) I},
        /* casinl(5 - 5.005 * (long double complex) I) */
        { 5.0L +
              -5.00500000081956386566162109375L * (long double complex) I,
          0.7799039033573315491708776586146430015L +
              -2.6497010940877766623835819103116638246L * (long double complex) I},

        /* casinl(1 + 5.500 * (long double complex) I) */
        { 1.0L +
              5.5L * (long double complex) I,
          0.17709928865657500701574616561599207884L +
              2.4215734590506472832551764743408198521L * (long double complex) I},
        /* casinl(1 + 0.001 * (long double complex) I) */
        { 1.0L +
              0.00100000000020372681319713592529296875L * (long double complex) I,
          1.5391761860141264362817986763728995476L +
              0.031625411243185808119677973274798562706L * (long double complex) I},
        /* casinl(1 - 0.001 * (long double complex) I) */
        { 1.0L +
              -0.00100000000020372681319713592529296875L * (long double complex) I,
          1.5391761860141264362817986763728995476L +
              -0.031625411243185808119677973274798562706L * (long double complex) I},
        /* casinl(1 - 5.005 * (long double complex) I) */
        { 1.0L +
               -5.00500000081956386566162109375L * (long double complex) I,
           0.19361108350095466908008324661105735207L +
               -2.3319204172665009535854099220832561521L * (long double complex) I},

        /* casinl(1.010 + 0.010 * (long double complex) I) */
        { 1.00999999977648258209228515625L +
              0.00999999999839928932487964630126953125L * (long double complex) I,
          1.5066194333770467809224561186167656533L +
              0.15530130895851037533249923716679417024L * (long double complex) I},
        /* casinl(1.001 + 0.001 * (long double complex) I) */
        { 1.00100000016391277313232421875L +
              0.00100000000020372681319713592529296875L * (long double complex) I,
          1.5504498794209502900922829943924564204L +
              0.049132250821569752251467056475900506368L * (long double complex) I},
        /* casinl(0.999 - 0.001 * (long double complex) I) */
        { 0.99900000006891787052154541015625L +
              -0.00100000000020372681319713592529296875L * (long double complex) I,
          1.5216592829506333903849263470204566759L +
              -0.020358030197614237682124794531127573737L * (long double complex) I},
        /* casinl(0.990 - 0.010 * (long double complex) I) */
        { 0.98999999999068677425384521484375L +
              -0.00999999999839928932487964630126953125L * (long double complex) I,
          1.415343324846177448741960221704637353L +
              -0.064543122955644790186128464604536322608L * (long double complex) I},

        /* casinl(-1 + 5.500 * (long double complex) I) */
        { -1.0L +
              5.5L * (long double complex) I,
          -0.17709928865657500701574616561599207884L +
              2.4215734590506472832551764743408198521L * (long double complex) I},
        /* casinl(-1 + 0.001 * (long double complex) I) */
        { -1.0L +
              0.00100000000020372681319713592529296875L * (long double complex) I,
          -1.5391761860141264362817986763728995476L +
              0.031625411243185808119677973274798562706L * (long double complex) I},
        /* casinl(-1 - 0.001 * (long double complex) I) */
        { -1.0L +
              -0.00100000000020372681319713592529296875L * (long double complex) I,
          -1.5391761860141264362817986763728995476L +
              -0.031625411243185808119677973274798562706L * (long double complex) I},
        /* casinl(-1 - 5.500 * (long double complex) I) */
        { -1.0L +
              -5.5L * (long double complex) I,
          -0.17709928865657500701574616561599207884L +
              -2.4215734590506472832551764743408198521L * (long double complex) I},

        /* casinl(-1.010 + 0.010 * (long double complex) I) */
        { -1.00999999977648258209228515625L +
              0.00999999999839928932487964630126953125L * (long double complex) I,
          -1.5066194333770467809224561186167656533L +
              0.15530130895851037533249923716679417024L * (long double complex) I},
        /* casinl(-1.001 + 0.001 * (long double complex) I) */
        { -1.00100000016391277313232421875L +
              0.00100000000020372681319713592529296875L * (long double complex) I,
          -1.5504498794209502900922829943924564204L +
              0.049132250821569752251467056475900506368L * (long double complex) I},
        /* casinl(-0.999 + -0.001 * (long double complex) I) */
        { -0.99900000006891787052154541015625L +
              -0.00100000000020372681319713592529296875L * (long double complex) I,
          -1.5216592829506333903849263470204566759L +
              -0.020358030197614237682124794531127573737L * (long double complex) I},
        /* casinl(-0.990 + -0.010 * (long double complex) I) */
        { -0.98999999999068677425384521484375L +
              -0.00999999999839928932487964630126953125L * (long double complex) I,
          -1.415343324846177448741960221704637353L +
              -0.064543122955644790186128464604536322608L * (long double complex) I},

        /* casinl(-5 + 5.500 * (long double complex) I) */
        { -5.0L +
              5.5L * (long double complex) I,
          -0.73331675299678489263052876265049390879L +
              2.699541384316091448872442152519987505L * (long double complex) I},
        /* casinl(-5 + 0.001 * (long double complex) I) */
        { -5.0L +
              0.00100000000020372681319713592529296875L * (long double complex) I,
          -1.5705922026526353507481515417984426789L +
              2.2924316908241090023360661700106176388L * (long double complex) I},
        /* casinl(-5 - 0.001 * (long double complex) I) */
        { -5.0L +
              -0.00100000000020372681319713592529296875L * (long double complex) I,
          -1.5705922026526353507481515417984426789L +
              -2.2924316908241090023360661700106176388L * (long double complex) I},
        /* casinl(-5 - 5.005 * (long double complex) I) */
        { -5.0L +
              -5.00500000081956386566162109375L * (long double complex) I,
          -0.7799039033573315491708776586146430015L +
              -2.6497010940877766623835819103116638246L * (long double complex) I},
    };

    const int num_tests = sizeof(tests) / sizeof(tests[0]);
    int pass_count = 0;
    long double rel_prec = ALLOWED_ULP * LDBL_EPSILON;
    int totat_tests = 0;

    for (int i = 0; i < num_tests; ++i) {
        /* Normal Inputs*/
        long double complex result = casinl(tests[i].input);
        long double complex ref = tests[i].reference;

        int res_ok = within_rel_prec(result, ref, rel_prec);
        totat_tests++;

        if (res_ok) {
            pass_count++;
        } else {
        printf("Test %d FAILED:\n", i + 1);
        printf("  Input:       casinl(%La + %Lai)\n", creall(tests[i].input), cimagl(tests[i].input));
        printf("  Computed:    real = %La, imag = %La\n", creall(result), cimagl(result));
        printf("  Reference:   real = %La, imag = %La\n", creall(ref), cimagl(ref));
        printf("  |x/ref| = %.17Lg, error = %.1Le (allowed <= %.1Le)\n",
                cabsl(result / ref) , fabsl(cabsl(result / ref) - 1.0L), rel_prec);
        printf("\n");
        }

        /* Conjugate Inputs*/
        long double complex conj_result = casinl(conj(tests[i].input));
        long double complex conj_ref = conjl(tests[i].reference);

        res_ok = within_rel_prec(conj_result, conj_ref, rel_prec);
        totat_tests++;

        if (res_ok) {
            pass_count++;
        } else {
        printf("Test %d FAILED (Conjugate):\n", i + 1);
        printf("  Input:       casinl(%La + %Lai)\n", creall(tests[i].input), cimagl(tests[i].input));
        printf("  Computed:    real = %La, imag = %La\n", creall(conj_result), cimagl(conj_result));
        printf("  Reference:   real = %La, imag = %La\n", creall(conj_ref), cimagl(conj_ref));
        printf("  |x/ref| = %.17Lg, error = %.1Le (allowed <= %.1Le)\n",
                cabsl(conj_result / conj_ref) , fabsl(cabsl(conj_result / conj_ref) - 1.0L), rel_prec);
        printf("\n");
        }

        /* Negative Inputs*/
        long double complex neg_result = casinl(-tests[i].input);
        long double complex neg_ref = -tests[i].reference;

        res_ok = within_rel_prec(neg_result, neg_ref, rel_prec);
        totat_tests++;

        if (res_ok) {
            pass_count++;
        } else {

        printf("Test %d FAILED (Negative):\n", i + 1);
        printf("  Input:       casinl(%La + %Lai)\n", creall(tests[i].input), cimagl(tests[i].input));
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
