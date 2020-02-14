///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2013
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  -  pll_measure.c
//
//!   @brief -  icmd support for measuring pll's against XUSB clock
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <grg_pll.h>
#include <imath.h>
#include "grg_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static GRG_PllMeasurementT _GRG_PLLMeasureFreq(enum GRG_PllSelect pllSelection, uint16 *cxmRet, uint16 * xusbRet);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: GRG_MeasurePllInMHz()
*
* @brief  - Measure a PLL clock in megahertz
*
* @return - The value in MHz
*
* @note   - Rounds up to the nearest whole MHz value
*/
uint16 GRG_MeasurePllInMHz(enum GRG_PllSelect pllSelection)
{
    uint16 usbCount;
    uint16 pllCount;
    uint16 numerator;
    uint16 denominator;
    uint16 remainder;
    uint16 frequency;

    // The clock being measured is compared to our USB clock running at 60 MHz
    // Two 8-bit counters are set to run, and whichever counter maxes out
    // first determines when the measurement stops. If the clock being measured
    // is less than 60MHz, the USB counter will be at 255. If the measured clock
    // is faster, its counter will be at 255, with the USB count being less than
    // that.


    // Get value of counters for our reference clock (USB)
    // and the clock to be measured
    _GRG_PLLMeasureFreq(pllSelection, &pllCount, &usbCount);

    // If the usbCount is 255 we have a pll clock less than 60MHz
    if (usbCount == 255)
    {
        // 255 cycles at 60MHz is 4.25uS, so our frequency is simply
        // the number of counts from the PLL divided by 4.25uS

        // Our period is defined as the actual value * 100, so we multiply
        // everything by 100 to make our integer math work
        numerator = GRG_int16Multiply(100, pllCount);
        // Adding 50 is like adding 0.5 to round fractional values up
        numerator += 50;
        denominator = PLL_MEASURE_PERIOD;
    }
    else
    {
        // Our measured clock is faster, so we are using the formula
        // (max possible freq) / (USB cycles measured)
        numerator = PLL_MAX_FREQUENCY;
        denominator = usbCount;
    }

    GRG_int16Divide(numerator, denominator, &frequency, &remainder);
    return(frequency);
}


GRG_PllMeasurementT GRG_PllMeasureFreq(enum GRG_PllSelect pllSelection)
{
    return _GRG_PLLMeasureFreq(pllSelection, NULL, NULL);
}

boolT GRG_PllMeasureIs125Mhz(GRG_PllMeasurementT pllDiff)
{
    // The RGMII standard permits clock periods between 7.2 and 8.8 ns.
    // GMII is slightly more strict (7.5 to 8.5 ns). Just use the RGMII
    // values.
    return (pllDiff >= 120 && pllDiff <= 145);
}


boolT GRG_PllMeasureIs25Mhz(GRG_PllMeasurementT pllDiff)
{
    // The RMII standard permits clock periods between 36 and 44 ns.
    // The MII standard is unclear. Just use the RMII values.
    return (pllDiff >= -159 && pllDiff <= -136);
}

/**
* FUNCTION NAME: _GRG_PLLMeasureFreq()
*
* @brief  - Find the difference between our XUSB clock & a PLL
*
* @return - The difference between the 2 clocks
*
* @note   - not re-entrant
*         - this could later be used by a test harness
*         - updated measurement algorithm to avoid metastability (see
*           http://lexington/wiki/index.php/Goldenears_ASIC_Developement#Frequency_Measurement):
*           1. Read GRG.PLL.FreqTest
*           2. Assert all Reset fields, set the Select field to the clock to be measured and write
*              GRG.PLL.FreqTest.
*           3. Assert the Clear field and write GRG.PLL.FreqTest.
*           4. De-assert all Reset fields, de-assert the Clear field and write GRG.PLL.FreqTest.
*           5. Assert the Go field and write GRG.PLL.FreqTest.
*           6. Read GRG.PLL.FreqTest until GRG.PLL.FreqTest.Go is de-asserted.
*           7. Measured Frequency = (60 * GRG.PLL.FreqTest.Cxm) / GRG.PLL.FreqTest.Xusb
*/
static GRG_PllMeasurementT _GRG_PLLMeasureFreq
(
    enum GRG_PllSelect pllSelection, // The PLL to measure
    uint16 *cxmRet,             // Will be filled in with the number of counts on PLL
    uint16 *xusbRet             // Will be filled in with the number of counds on the USB clock
)
{
    LEON_TimerValueT startTime;
    uint16 cxmCount;
    uint16 xusbCount;



    // initial read
    uint32 reg = GRG_PLL_FREQTEST_READ_REG(GRG_BASE_ADDR);

    // select pll and assert all reset bits
    reg = GRG_PLL_FREQTEST_SELECT_SET_BF(reg, pllSelection);
    reg = GRG_PLL_FREQTEST_RESET_SET_BF(reg, ~0);
    GRG_PLL_FREQTEST_WRITE_REG(GRG_BASE_ADDR, reg);

    // clear all counts
    reg = GRG_PLL_FREQTEST_CLEAR_SET_BF(reg, 1);
    GRG_PLL_FREQTEST_WRITE_REG(GRG_BASE_ADDR, reg);

    // de-assert clear and all reset bits
    reg = GRG_PLL_FREQTEST_CLEAR_SET_BF(reg, 0);
    reg = GRG_PLL_FREQTEST_RESET_SET_BF(reg, 0);
    GRG_PLL_FREQTEST_WRITE_REG(GRG_BASE_ADDR, reg);

    // go
    reg = GRG_PLL_FREQTEST_GO_SET_BF(reg, 1);
    GRG_PLL_FREQTEST_WRITE_REG(GRG_BASE_ADDR, reg);
    startTime = LEON_TimerRead();

    // read reg until go is cleared, or 6 microseconds has passed
    do {
        reg = GRG_PLL_FREQTEST_READ_REG(GRG_BASE_ADDR);
    } while (GRG_PLL_FREQTEST_GO_GET_BF(reg) && (LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < 6));

    // grab the counts
    cxmCount = GRG_PLL_FREQTEST_CXM_GET_BF(reg);
    xusbCount = GRG_PLL_FREQTEST_XUSB_GET_BF(reg);
    ilog_GRG_COMPONENT_3(ILOG_DEBUG, FREQ_MEASURE, pllSelection, xusbCount, cxmCount);

    if (cxmRet)
        *cxmRet = cxmCount;

    if (xusbRet)
        *xusbRet = xusbCount;

    if (GRG_PLL_FREQTEST_GO_GET_BF(reg))
    {
        // Go bit is still stuck, presumably dead CxM clock
        return GRG_PLL_MEASUREMENT_FAIL;
    }
    else
    {
        return cxmCount - xusbCount;
    }
}
#if 0 // {
For values of cxmCount - xusbCount, which are > 0, frequency is 60 * 255/(255 - diff)
For values of cxmCount - xusbCount, which are < 0, frequency is 60 * (255 + diff)/255
// NOTE: #define GRG_PLL_MEASUREMENT_FAIL ((sint16)(-1))
-254: 0.235294
-253: 0.470588
-252: 0.705882
-251: 0.941176
-250: 1.176471
-249: 1.411765
-248: 1.647059
-247: 1.882353
-246: 2.117647
-245: 2.352941
-244: 2.588235
-243: 2.823529
-242: 3.058824
-241: 3.294118
-240: 3.529412
-239: 3.764706
-238: 4.000000
-237: 4.235294
-236: 4.470588
-235: 4.705882
-234: 4.941176
-233: 5.176471
-232: 5.411765
-231: 5.647059
-230: 5.882353
-229: 6.117647
-228: 6.352941
-227: 6.588235
-226: 6.823529
-225: 7.058824
-224: 7.294118
-223: 7.529412
-222: 7.764706
-221: 8.000000
-220: 8.235294
-219: 8.470588
-218: 8.705882
-217: 8.941176
-216: 9.176471
-215: 9.411765
-214: 9.647059
-213: 9.882353
-212: 10.117647
-211: 10.352941
-210: 10.588235
-209: 10.823529
-208: 11.058824
-207: 11.294118
-206: 11.529412
-205: 11.764706
-204: 12.000000
-203: 12.235294
-202: 12.470588
-201: 12.705882
-200: 12.941176
-199: 13.176471
-198: 13.411765
-197: 13.647059
-196: 13.882353
-195: 14.117647
-194: 14.352941
-193: 14.588235
-192: 14.823529
-191: 15.058824
-190: 15.294118
-189: 15.529412
-188: 15.764706
-187: 16.000000
-186: 16.235294
-185: 16.470588
-184: 16.705882
-183: 16.941176
-182: 17.176471
-181: 17.411765
-180: 17.647059
-179: 17.882353
-178: 18.117647
-177: 18.352941
-176: 18.588235
-175: 18.823529
-174: 19.058824
-173: 19.294118
-172: 19.529412
-171: 19.764706
-170: 20.000000
-169: 20.235294
-168: 20.470588
-167: 20.705882
-166: 20.941176
-165: 21.176471
-164: 21.411765
-163: 21.647059
-162: 21.882353
-161: 22.117647
-160: 22.352941
-159: 22.588235
-158: 22.823529
-157: 23.058824
-156: 23.294118
-155: 23.529412
-154: 23.764706
-153: 24.000000
-152: 24.235294
-151: 24.470588
-150: 24.705882
-149: 24.941176 <--- MII clock
-148: 25.176471 <--- MII clock
-147: 25.411765
-146: 25.647059
-145: 25.882353
-144: 26.117647
-143: 26.352941
-142: 26.588235
-141: 26.823529
-140: 27.058824
-139: 27.294118
-138: 27.529412
-137: 27.764706
-136: 28.000000
-135: 28.235294
-134: 28.470588
-133: 28.705882
-132: 28.941176
-131: 29.176471
-130: 29.411765
-129: 29.647059
-128: 29.882353
-127: 30.117647
-126: 30.352941
-125: 30.588235
-124: 30.823529
-123: 31.058824
-122: 31.294118
-121: 31.529412
-120: 31.764706
-119: 32.000000
-118: 32.235294
-117: 32.470588
-116: 32.705882
-115: 32.941176
-114: 33.176471
-113: 33.411765
-112: 33.647059
-111: 33.882353
-110: 34.117647
-109: 34.352941
-108: 34.588235
-107: 34.823529
-106: 35.058824
-105: 35.294118
-104: 35.529412
-103: 35.764706
-102: 36.000000
-101: 36.235294
-100: 36.470588
-99: 36.705882
-98: 36.941176
-97: 37.176471
-96: 37.411765
-95: 37.647059
-94: 37.882353
-93: 38.117647
-92: 38.352941
-91: 38.588235
-90: 38.823529
-89: 39.058824
-88: 39.294118
-87: 39.529412
-86: 39.764706
-85: 40.000000
-84: 40.235294
-83: 40.470588
-82: 40.705882
-81: 40.941176
-80: 41.176471
-79: 41.411765
-78: 41.647059
-77: 41.882353
-76: 42.117647
-75: 42.352941
-74: 42.588235
-73: 42.823529
-72: 43.058824
-71: 43.294118
-70: 43.529412
-69: 43.764706
-68: 44.000000
-67: 44.235294
-66: 44.470588
-65: 44.705882
-64: 44.941176
-63: 45.176471
-62: 45.411765
-61: 45.647059
-60: 45.882353
-59: 46.117647
-58: 46.352941
-57: 46.588235
-56: 46.823529
-55: 47.058824
-54: 47.294118
-53: 47.529412
-52: 47.764706
-51: 48.000000
-50: 48.235294
-49: 48.470588
-48: 48.705882
-47: 48.941176
-46: 49.176471
-45: 49.411765
-44: 49.647059
-43: 49.882353 <--- RMII clock
-42: 50.117647 <--- RMII clock
-41: 50.352941
-40: 50.588235
-39: 50.823529
-38: 51.058824
-37: 51.294118
-36: 51.529412
-35: 51.764706
-34: 52.000000
-33: 52.235294
-32: 52.470588
-31: 52.705882
-30: 52.941176
-29: 53.176471
-28: 53.411765
-27: 53.647059
-26: 53.882353
-25: 54.117647
-24: 54.352941
-23: 54.588235
-22: 54.823529
-21: 55.058824
-20: 55.294118
-19: 55.529412
-18: 55.764706
-17: 56.000000
-16: 56.235294
-15: 56.470588
-14: 56.705882
-13: 56.941176
-12: 57.176471
-11: 57.411765
-10: 57.647059
-9: 57.882353
-8: 58.117647
-7: 58.352941
-6: 58.588235
-5: 58.823529
-4: 59.058824
-3: 59.294118
-2: 59.529412
-1: 59.764706
0: 60.000000
1: 60.236220
2: 60.474308
3: 60.714286
4: 60.956175
5: 61.200000
6: 61.445783
7: 61.693548
8: 61.943320
9: 62.195122
10: 62.448980
11: 62.704918
12: 62.962963
13: 63.223140
14: 63.485477
15: 63.750000
16: 64.016736
17: 64.285714
18: 64.556962
19: 64.830508
20: 65.106383
21: 65.384615
22: 65.665236
23: 65.948276
24: 66.233766
25: 66.521739
26: 66.812227
27: 67.105263
28: 67.400881
29: 67.699115
30: 68.000000
31: 68.303571
32: 68.609865
33: 68.918919
34: 69.230769
35: 69.545455
36: 69.863014
37: 70.183486
38: 70.506912
39: 70.833333
40: 71.162791
41: 71.495327
42: 71.830986
43: 72.169811
44: 72.511848
45: 72.857143
46: 73.205742
47: 73.557692
48: 73.913043
49: 74.271845
50: 74.634146
51: 75.000000
52: 75.369458
53: 75.742574
54: 76.119403
55: 76.500000
56: 76.884422
57: 77.272727
58: 77.664975
59: 78.061224
60: 78.461538
61: 78.865979
62: 79.274611
63: 79.687500
64: 80.104712
65: 80.526316
66: 80.952381
67: 81.382979
68: 81.818182
69: 82.258065
70: 82.702703
71: 83.152174
72: 83.606557
73: 84.065934
74: 84.530387
75: 85.000000
76: 85.474860
77: 85.955056
78: 86.440678
79: 86.931818
80: 87.428571
81: 87.931034
82: 88.439306
83: 88.953488
84: 89.473684
85: 90.000000
86: 90.532544
87: 91.071429
88: 91.616766
89: 92.168675
90: 92.727273
91: 93.292683
92: 93.865031
93: 94.444444
94: 95.031056
95: 95.625000
96: 96.226415
97: 96.835443
98: 97.452229
99: 98.076923
100: 98.709677
101: 99.350649
102: 100.000000
103: 100.657895
104: 101.324503
105: 102.000000
106: 102.684564
107: 103.378378
108: 104.081633
109: 104.794521
110: 105.517241
111: 106.250000
112: 106.993007
113: 107.746479
114: 108.510638
115: 109.285714
116: 110.071942
117: 110.869565
118: 111.678832
119: 112.500000
120: 113.333333
121: 114.179104
122: 115.037594
123: 115.909091
124: 116.793893
125: 117.692308
126: 118.604651
127: 119.531250
128: 120.472441
129: 121.428571
130: 122.400000
131: 123.387097
132: 124.390244 <--- GMII/RGMII clock
133: 125.409836 <--- GMII/RGMII clock
134: 126.446281
135: 127.500000
136: 128.571429
137: 129.661017
138: 130.769231
139: 131.896552
140: 133.043478
141: 134.210526
142: 135.398230
143: 136.607143
144: 137.837838
145: 139.090909
146: 140.366972
147: 141.666667
148: 142.990654
149: 144.339623
150: 145.714286
151: 147.115385
152: 148.543689
153: 150.000000
154: 151.485149
155: 153.000000
156: 154.545455
157: 156.122449
158: 157.731959
159: 159.375000
160: 161.052632
161: 162.765957
162: 164.516129
163: 166.304348
164: 168.131868
165: 170.000000
166: 171.910112
167: 173.863636
168: 175.862069
169: 177.906977
170: 180.000000
171: 182.142857
172: 184.337349
173: 186.585366
174: 188.888889
175: 191.250000
176: 193.670886
177: 196.153846
178: 198.701299
179: 201.315789
180: 204.000000
181: 206.756757
182: 209.589041
183: 212.500000
184: 215.492958
185: 218.571429
186: 221.739130
187: 225.000000
188: 228.358209
189: 231.818182
190: 235.384615
191: 239.062500
192: 242.857143
193: 246.774194
194: 250.819672
195: 255.000000
196: 259.322034
197: 263.793103
198: 268.421053
199: 273.214286
200: 278.181818
201: 283.333333
202: 288.679245
203: 294.230769
204: 300.000000
205: 306.000000
206: 312.244898
207: 318.750000
208: 325.531915
209: 332.608696
210: 340.000000
211: 347.727273
212: 355.813953
213: 364.285714
214: 373.170732
215: 382.500000
216: 392.307692
217: 402.631579
218: 413.513514
219: 425.000000
220: 437.142857
221: 450.000000
222: 463.636364
223: 478.125000
224: 493.548387
225: 510.000000
226: 527.586207
227: 546.428571
228: 566.666667
229: 588.461538
230: 612.000000
231: 637.500000
232: 665.217391
233: 695.454545
234: 728.571429
235: 765.000000
236: 805.263158
237: 850.000000
238: 900.000000
239: 956.250000
240: 1020.000000
241: 1092.857143
242: 1176.923077
243: 1275.000000
244: 1390.909091
245: 1530.000000
246: 1700.000000
247: 1912.500000
248: 2185.714286
249: 2550.000000
250: 3060.000000
251: 3825.000000
252: 5100.000000
253: 7650.000000
254: 15300.000000

#endif // }

/**
* FUNCTION NAME: icmdPLLMeasure()
*
* @brief  - An icmd function for measuring PLL's
*
* @return - void
*
* @note   -
*
*/
void icmdPLLMeasure
(
    uint8 pll   // The PLL to measure.  see enum GRG_PllSelect for valid entries
)
{
    const enum GRG_PllSelect pllType = CAST(pll, uint8, enum GRG_PllSelect);

    if (    (pllType != GRG_PllSelectCrmPhyClk)
        &&  (pllType != GRG_PllSelectCtmPhyClk)
        &&  (pllType != GRG_PllSelectCrmRefClk)
        &&  (pllType != GRG_PllSelectCtmRefClk)
        &&  (pllType != GRG_PllSelectClmClk1))
    {
        ilog_GRG_COMPONENT_1(ILOG_USER_LOG, INVALID_ICMD_ARG, pll);
    }
    else
    {
        uint16 cxmCount;
        uint16 xusbCount;
        sint16 pllDiff = _GRG_PLLMeasureFreq(pllType, &cxmCount, &xusbCount);
        ilog_GRG_COMPONENT_3(ILOG_USER_LOG, PLL_MEASUREMENT, pllDiff, xusbCount, cxmCount);
    }
}
/**
* FUNCTION NAME: icmdPLLFreq()
*
* @brief  - An icmd function for measuring PLL's
*
* @return - void
*
* @note   - Sends an ilog with the frequency in MHz rather than as an abstract
*           counter value
*/
void icmdPLLFreq
(
    uint8 pll   // The PLL to measure.  see enum GRG_PllSelect for valid entries
)
{
    const enum GRG_PllSelect pllType = CAST(pll, uint8, enum GRG_PllSelect);
    uint16 frequency;

    if (    (pllType != GRG_PllSelectCrmPhyClk)
        &&  (pllType != GRG_PllSelectCtmPhyClk)
        &&  (pllType != GRG_PllSelectCrmRefClk)
        &&  (pllType != GRG_PllSelectCtmRefClk)
        &&  (pllType != GRG_PllSelectClmClk1))
    {
        ilog_GRG_COMPONENT_1(ILOG_USER_LOG, INVALID_PLL, pll);
    }
    else
    {
        frequency = GRG_MeasurePllInMHz(pllType);
        ilog_GRG_COMPONENT_2(ILOG_USER_LOG, MEAS_PLL_FREQ, pll, frequency);
    }
}

