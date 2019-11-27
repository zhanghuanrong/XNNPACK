// Copyright 2019 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <assert.h>
#include <math.h>
#include <stddef.h>

#include <arm_neon.h>

#include <xnnpack/math-stubs.h>


// Table of exp2(k / 2048) values, k = 0..2047
static const float exp2_table[2048] = {
  0x1.000000p+0f, 0x1.001630p+0f, 0x1.002C60p+0f, 0x1.004294p+0f,
  0x1.0058C8p+0f, 0x1.006F00p+0f, 0x1.008538p+0f, 0x1.009B72p+0f,
  0x1.00B1B0p+0f, 0x1.00C7EEp+0f, 0x1.00DE2Ep+0f, 0x1.00F472p+0f,
  0x1.010AB6p+0f, 0x1.0120FCp+0f, 0x1.013744p+0f, 0x1.014D8Ep+0f,
  0x1.0163DAp+0f, 0x1.017A28p+0f, 0x1.019078p+0f, 0x1.01A6CAp+0f,
  0x1.01BD1Ep+0f, 0x1.01D374p+0f, 0x1.01E9CCp+0f, 0x1.020026p+0f,
  0x1.021682p+0f, 0x1.022CDEp+0f, 0x1.02433Ep+0f, 0x1.0259A0p+0f,
  0x1.027004p+0f, 0x1.028668p+0f, 0x1.029CD0p+0f, 0x1.02B338p+0f,
  0x1.02C9A4p+0f, 0x1.02E010p+0f, 0x1.02F680p+0f, 0x1.030CF0p+0f,
  0x1.032364p+0f, 0x1.0339D8p+0f, 0x1.035050p+0f, 0x1.0366C8p+0f,
  0x1.037D42p+0f, 0x1.0393C0p+0f, 0x1.03AA3Ep+0f, 0x1.03C0BEp+0f,
  0x1.03D742p+0f, 0x1.03EDC6p+0f, 0x1.04044Cp+0f, 0x1.041AD4p+0f,
  0x1.04315Ep+0f, 0x1.0447EAp+0f, 0x1.045E78p+0f, 0x1.04750Ap+0f,
  0x1.048B9Cp+0f, 0x1.04A230p+0f, 0x1.04B8C6p+0f, 0x1.04CF5Ep+0f,
  0x1.04E5F8p+0f, 0x1.04FC94p+0f, 0x1.051330p+0f, 0x1.0529D0p+0f,
  0x1.054072p+0f, 0x1.055716p+0f, 0x1.056DBCp+0f, 0x1.058464p+0f,
  0x1.059B0Ep+0f, 0x1.05B1B8p+0f, 0x1.05C866p+0f, 0x1.05DF16p+0f,
  0x1.05F5C8p+0f, 0x1.060C7Ap+0f, 0x1.062330p+0f, 0x1.0639E8p+0f,
  0x1.0650A0p+0f, 0x1.06675Cp+0f, 0x1.067E1Ap+0f, 0x1.0694D8p+0f,
  0x1.06AB9Ap+0f, 0x1.06C25Ep+0f, 0x1.06D922p+0f, 0x1.06EFEAp+0f,
  0x1.0706B2p+0f, 0x1.071D7Ep+0f, 0x1.07344Ap+0f, 0x1.074B1Ap+0f,
  0x1.0761EAp+0f, 0x1.0778BEp+0f, 0x1.078F92p+0f, 0x1.07A66Ap+0f,
  0x1.07BD42p+0f, 0x1.07D41Ep+0f, 0x1.07EAFAp+0f, 0x1.0801DAp+0f,
  0x1.0818BAp+0f, 0x1.082F9Ep+0f, 0x1.084682p+0f, 0x1.085D68p+0f,
  0x1.087452p+0f, 0x1.088B3Cp+0f, 0x1.08A22Ap+0f, 0x1.08B918p+0f,
  0x1.08D008p+0f, 0x1.08E6FCp+0f, 0x1.08FDF0p+0f, 0x1.0914E6p+0f,
  0x1.092BE0p+0f, 0x1.0942DAp+0f, 0x1.0959D6p+0f, 0x1.0970D6p+0f,
  0x1.0987D6p+0f, 0x1.099ED8p+0f, 0x1.09B5DEp+0f, 0x1.09CCE4p+0f,
  0x1.09E3ECp+0f, 0x1.09FAF8p+0f, 0x1.0A1204p+0f, 0x1.0A2912p+0f,
  0x1.0A4024p+0f, 0x1.0A5736p+0f, 0x1.0A6E4Ap+0f, 0x1.0A8562p+0f,
  0x1.0A9C7Ap+0f, 0x1.0AB394p+0f, 0x1.0ACAB0p+0f, 0x1.0AE1D0p+0f,
  0x1.0AF8F0p+0f, 0x1.0B1012p+0f, 0x1.0B2738p+0f, 0x1.0B3E5Ep+0f,
  0x1.0B5586p+0f, 0x1.0B6CB2p+0f, 0x1.0B83DEp+0f, 0x1.0B9B0Cp+0f,
  0x1.0BB23Ep+0f, 0x1.0BC970p+0f, 0x1.0BE0A4p+0f, 0x1.0BF7DCp+0f,
  0x1.0C0F14p+0f, 0x1.0C2650p+0f, 0x1.0C3D8Cp+0f, 0x1.0C54CAp+0f,
  0x1.0C6C0Cp+0f, 0x1.0C834Ep+0f, 0x1.0C9A94p+0f, 0x1.0CB1DAp+0f,
  0x1.0CC922p+0f, 0x1.0CE06Ep+0f, 0x1.0CF7BAp+0f, 0x1.0D0F0Ap+0f,
  0x1.0D265Ap+0f, 0x1.0D3DAEp+0f, 0x1.0D5502p+0f, 0x1.0D6C5Ap+0f,
  0x1.0D83B2p+0f, 0x1.0D9B0Ep+0f, 0x1.0DB26Ap+0f, 0x1.0DC9CAp+0f,
  0x1.0DE12Ap+0f, 0x1.0DF88Ep+0f, 0x1.0E0FF2p+0f, 0x1.0E275Ap+0f,
  0x1.0E3EC4p+0f, 0x1.0E562Ep+0f, 0x1.0E6D9Cp+0f, 0x1.0E850Ap+0f,
  0x1.0E9C7Cp+0f, 0x1.0EB3F0p+0f, 0x1.0ECB66p+0f, 0x1.0EE2DCp+0f,
  0x1.0EFA56p+0f, 0x1.0F11D2p+0f, 0x1.0F2950p+0f, 0x1.0F40CEp+0f,
  0x1.0F5850p+0f, 0x1.0F6FD4p+0f, 0x1.0F875Ap+0f, 0x1.0F9EE2p+0f,
  0x1.0FB66Ap+0f, 0x1.0FCDF6p+0f, 0x1.0FE584p+0f, 0x1.0FFD14p+0f,
  0x1.1014A6p+0f, 0x1.102C3Ap+0f, 0x1.1043D0p+0f, 0x1.105B68p+0f,
  0x1.107302p+0f, 0x1.108A9Ep+0f, 0x1.10A23Cp+0f, 0x1.10B9DEp+0f,
  0x1.10D180p+0f, 0x1.10E924p+0f, 0x1.1100CAp+0f, 0x1.111872p+0f,
  0x1.11301Ep+0f, 0x1.1147CAp+0f, 0x1.115F78p+0f, 0x1.117728p+0f,
  0x1.118EDCp+0f, 0x1.11A690p+0f, 0x1.11BE46p+0f, 0x1.11D600p+0f,
  0x1.11EDBAp+0f, 0x1.120578p+0f, 0x1.121D36p+0f, 0x1.1234F8p+0f,
  0x1.124CBAp+0f, 0x1.126480p+0f, 0x1.127C48p+0f, 0x1.129410p+0f,
  0x1.12ABDCp+0f, 0x1.12C3AAp+0f, 0x1.12DB78p+0f, 0x1.12F34Ap+0f,
  0x1.130B1Ep+0f, 0x1.1322F4p+0f, 0x1.133ACCp+0f, 0x1.1352A6p+0f,
  0x1.136A82p+0f, 0x1.138260p+0f, 0x1.139A40p+0f, 0x1.13B222p+0f,
  0x1.13CA06p+0f, 0x1.13E1ECp+0f, 0x1.13F9D4p+0f, 0x1.1411BEp+0f,
  0x1.1429AAp+0f, 0x1.14419Ap+0f, 0x1.14598Ap+0f, 0x1.14717Cp+0f,
  0x1.148972p+0f, 0x1.14A168p+0f, 0x1.14B962p+0f, 0x1.14D15Cp+0f,
  0x1.14E95Ap+0f, 0x1.150158p+0f, 0x1.15195Ap+0f, 0x1.15315Cp+0f,
  0x1.154962p+0f, 0x1.15616Ap+0f, 0x1.157974p+0f, 0x1.15917Ep+0f,
  0x1.15A98Cp+0f, 0x1.15C19Cp+0f, 0x1.15D9AEp+0f, 0x1.15F1C2p+0f,
  0x1.1609D8p+0f, 0x1.1621F0p+0f, 0x1.163A0Ap+0f, 0x1.165226p+0f,
  0x1.166A46p+0f, 0x1.168266p+0f, 0x1.169A88p+0f, 0x1.16B2AEp+0f,
  0x1.16CAD4p+0f, 0x1.16E2FCp+0f, 0x1.16FB28p+0f, 0x1.171354p+0f,
  0x1.172B84p+0f, 0x1.1743B6p+0f, 0x1.175BE8p+0f, 0x1.17741Ep+0f,
  0x1.178C56p+0f, 0x1.17A48Ep+0f, 0x1.17BCCAp+0f, 0x1.17D508p+0f,
  0x1.17ED48p+0f, 0x1.18058Ap+0f, 0x1.181DCEp+0f, 0x1.183614p+0f,
  0x1.184E5Ep+0f, 0x1.1866A8p+0f, 0x1.187EF4p+0f, 0x1.189742p+0f,
  0x1.18AF94p+0f, 0x1.18C7E6p+0f, 0x1.18E03Cp+0f, 0x1.18F892p+0f,
  0x1.1910ECp+0f, 0x1.192946p+0f, 0x1.1941A4p+0f, 0x1.195A04p+0f,
  0x1.197266p+0f, 0x1.198ACAp+0f, 0x1.19A330p+0f, 0x1.19BB98p+0f,
  0x1.19D402p+0f, 0x1.19EC6Ep+0f, 0x1.1A04DCp+0f, 0x1.1A1D4Cp+0f,
  0x1.1A35BEp+0f, 0x1.1A4E34p+0f, 0x1.1A66AAp+0f, 0x1.1A7F24p+0f,
  0x1.1A979Ep+0f, 0x1.1AB01Cp+0f, 0x1.1AC89Ap+0f, 0x1.1AE11Cp+0f,
  0x1.1AF9A0p+0f, 0x1.1B1226p+0f, 0x1.1B2AACp+0f, 0x1.1B4336p+0f,
  0x1.1B5BC2p+0f, 0x1.1B7452p+0f, 0x1.1B8CE2p+0f, 0x1.1BA574p+0f,
  0x1.1BBE08p+0f, 0x1.1BD69Ep+0f, 0x1.1BEF38p+0f, 0x1.1C07D2p+0f,
  0x1.1C2070p+0f, 0x1.1C390Ep+0f, 0x1.1C51B0p+0f, 0x1.1C6A54p+0f,
  0x1.1C82FAp+0f, 0x1.1C9BA2p+0f, 0x1.1CB44Ap+0f, 0x1.1CCCF6p+0f,
  0x1.1CE5A6p+0f, 0x1.1CFE56p+0f, 0x1.1D1708p+0f, 0x1.1D2FBCp+0f,
  0x1.1D4874p+0f, 0x1.1D612Cp+0f, 0x1.1D79E6p+0f, 0x1.1D92A4p+0f,
  0x1.1DAB64p+0f, 0x1.1DC424p+0f, 0x1.1DDCE8p+0f, 0x1.1DF5AEp+0f,
  0x1.1E0E76p+0f, 0x1.1E2740p+0f, 0x1.1E400Cp+0f, 0x1.1E58DAp+0f,
  0x1.1E71AAp+0f, 0x1.1E8A7Ep+0f, 0x1.1EA352p+0f, 0x1.1EBC2Ap+0f,
  0x1.1ED502p+0f, 0x1.1EEDDEp+0f, 0x1.1F06BAp+0f, 0x1.1F1F9Ap+0f,
  0x1.1F387Cp+0f, 0x1.1F5160p+0f, 0x1.1F6A46p+0f, 0x1.1F832Ep+0f,
  0x1.1F9C18p+0f, 0x1.1FB504p+0f, 0x1.1FCDF4p+0f, 0x1.1FE6E4p+0f,
  0x1.1FFFD8p+0f, 0x1.2018CCp+0f, 0x1.2031C4p+0f, 0x1.204ABCp+0f,
  0x1.2063B8p+0f, 0x1.207CB6p+0f, 0x1.2095B6p+0f, 0x1.20AEB8p+0f,
  0x1.20C7BCp+0f, 0x1.20E0C4p+0f, 0x1.20F9CCp+0f, 0x1.2112D6p+0f,
  0x1.212BE4p+0f, 0x1.2144F2p+0f, 0x1.215E04p+0f, 0x1.217718p+0f,
  0x1.21902Cp+0f, 0x1.21A944p+0f, 0x1.21C25Ep+0f, 0x1.21DB7Ap+0f,
  0x1.21F49Ap+0f, 0x1.220DBAp+0f, 0x1.2226DCp+0f, 0x1.224002p+0f,
  0x1.225928p+0f, 0x1.227252p+0f, 0x1.228B7Cp+0f, 0x1.22A4AAp+0f,
  0x1.22BDDAp+0f, 0x1.22D70Cp+0f, 0x1.22F040p+0f, 0x1.230976p+0f,
  0x1.2322B0p+0f, 0x1.233BEAp+0f, 0x1.235526p+0f, 0x1.236E66p+0f,
  0x1.2387A6p+0f, 0x1.23A0EAp+0f, 0x1.23BA30p+0f, 0x1.23D378p+0f,
  0x1.23ECC2p+0f, 0x1.24060Ep+0f, 0x1.241F5Cp+0f, 0x1.2438ACp+0f,
  0x1.245200p+0f, 0x1.246B54p+0f, 0x1.2484ACp+0f, 0x1.249E06p+0f,
  0x1.24B760p+0f, 0x1.24D0BEp+0f, 0x1.24EA1Ep+0f, 0x1.250380p+0f,
  0x1.251CE4p+0f, 0x1.25364Cp+0f, 0x1.254FB4p+0f, 0x1.256920p+0f,
  0x1.25828Cp+0f, 0x1.259BFCp+0f, 0x1.25B56Ep+0f, 0x1.25CEE2p+0f,
  0x1.25E858p+0f, 0x1.2601D0p+0f, 0x1.261B4Ap+0f, 0x1.2634C6p+0f,
  0x1.264E46p+0f, 0x1.2667C6p+0f, 0x1.26814Ap+0f, 0x1.269ACEp+0f,
  0x1.26B456p+0f, 0x1.26CDE0p+0f, 0x1.26E76Cp+0f, 0x1.2700FAp+0f,
  0x1.271A8Cp+0f, 0x1.27341Ep+0f, 0x1.274DB2p+0f, 0x1.27674Ap+0f,
  0x1.2780E4p+0f, 0x1.279A7Ep+0f, 0x1.27B41Cp+0f, 0x1.27CDBCp+0f,
  0x1.27E75Ep+0f, 0x1.280104p+0f, 0x1.281AAAp+0f, 0x1.283452p+0f,
  0x1.284DFEp+0f, 0x1.2867ACp+0f, 0x1.28815Cp+0f, 0x1.289B0Cp+0f,
  0x1.28B4C0p+0f, 0x1.28CE78p+0f, 0x1.28E830p+0f, 0x1.2901EAp+0f,
  0x1.291BA8p+0f, 0x1.293566p+0f, 0x1.294F28p+0f, 0x1.2968ECp+0f,
  0x1.2982B2p+0f, 0x1.299C7Ap+0f, 0x1.29B644p+0f, 0x1.29D010p+0f,
  0x1.29E9E0p+0f, 0x1.2A03B0p+0f, 0x1.2A1D84p+0f, 0x1.2A375Ap+0f,
  0x1.2A5130p+0f, 0x1.2A6B0Ap+0f, 0x1.2A84E8p+0f, 0x1.2A9EC6p+0f,
  0x1.2AB8A6p+0f, 0x1.2AD28Ap+0f, 0x1.2AEC6Ep+0f, 0x1.2B0656p+0f,
  0x1.2B2040p+0f, 0x1.2B3A2Cp+0f, 0x1.2B541Ap+0f, 0x1.2B6E0Ap+0f,
  0x1.2B87FEp+0f, 0x1.2BA1F2p+0f, 0x1.2BBBEAp+0f, 0x1.2BD5E2p+0f,
  0x1.2BEFDEp+0f, 0x1.2C09DCp+0f, 0x1.2C23DCp+0f, 0x1.2C3DDEp+0f,
  0x1.2C57E4p+0f, 0x1.2C71EAp+0f, 0x1.2C8BF4p+0f, 0x1.2CA600p+0f,
  0x1.2CC00Cp+0f, 0x1.2CDA1Cp+0f, 0x1.2CF430p+0f, 0x1.2D0E44p+0f,
  0x1.2D285Ap+0f, 0x1.2D4274p+0f, 0x1.2D5C8Ep+0f, 0x1.2D76ACp+0f,
  0x1.2D90CCp+0f, 0x1.2DAAEEp+0f, 0x1.2DC512p+0f, 0x1.2DDF3Ap+0f,
  0x1.2DF962p+0f, 0x1.2E138Ep+0f, 0x1.2E2DBAp+0f, 0x1.2E47EAp+0f,
  0x1.2E621Cp+0f, 0x1.2E7C50p+0f, 0x1.2E9686p+0f, 0x1.2EB0C0p+0f,
  0x1.2ECAFAp+0f, 0x1.2EE538p+0f, 0x1.2EFF78p+0f, 0x1.2F19BAp+0f,
  0x1.2F33FEp+0f, 0x1.2F4E44p+0f, 0x1.2F688Cp+0f, 0x1.2F82D8p+0f,
  0x1.2F9D24p+0f, 0x1.2FB774p+0f, 0x1.2FD1C6p+0f, 0x1.2FEC1Ap+0f,
  0x1.300670p+0f, 0x1.3020CAp+0f, 0x1.303B24p+0f, 0x1.305582p+0f,
  0x1.306FE0p+0f, 0x1.308A42p+0f, 0x1.30A4A6p+0f, 0x1.30BF0Cp+0f,
  0x1.30D976p+0f, 0x1.30F3E0p+0f, 0x1.310E4Ep+0f, 0x1.3128BEp+0f,
  0x1.31432Ep+0f, 0x1.315DA2p+0f, 0x1.31781Ap+0f, 0x1.319292p+0f,
  0x1.31AD0Cp+0f, 0x1.31C78Ap+0f, 0x1.31E20Ap+0f, 0x1.31FC8Cp+0f,
  0x1.321710p+0f, 0x1.323196p+0f, 0x1.324C1Ep+0f, 0x1.3266AAp+0f,
  0x1.328138p+0f, 0x1.329BC6p+0f, 0x1.32B658p+0f, 0x1.32D0EEp+0f,
  0x1.32EB84p+0f, 0x1.33061Cp+0f, 0x1.3320B8p+0f, 0x1.333B56p+0f,
  0x1.3355F4p+0f, 0x1.337098p+0f, 0x1.338B3Cp+0f, 0x1.33A5E2p+0f,
  0x1.33C08Cp+0f, 0x1.33DB36p+0f, 0x1.33F5E4p+0f, 0x1.341094p+0f,
  0x1.342B46p+0f, 0x1.3445FAp+0f, 0x1.3460B2p+0f, 0x1.347B6Ap+0f,
  0x1.349626p+0f, 0x1.34B0E4p+0f, 0x1.34CBA4p+0f, 0x1.34E666p+0f,
  0x1.35012Cp+0f, 0x1.351BF2p+0f, 0x1.3536BCp+0f, 0x1.355188p+0f,
  0x1.356C56p+0f, 0x1.358726p+0f, 0x1.35A1FAp+0f, 0x1.35BCCEp+0f,
  0x1.35D7A6p+0f, 0x1.35F280p+0f, 0x1.360D5Cp+0f, 0x1.36283Ap+0f,
  0x1.36431Ap+0f, 0x1.365DFEp+0f, 0x1.3678E2p+0f, 0x1.3693CAp+0f,
  0x1.36AEB4p+0f, 0x1.36C9A0p+0f, 0x1.36E490p+0f, 0x1.36FF80p+0f,
  0x1.371A74p+0f, 0x1.37356Ap+0f, 0x1.375062p+0f, 0x1.376B5Cp+0f,
  0x1.378658p+0f, 0x1.37A158p+0f, 0x1.37BC58p+0f, 0x1.37D75Cp+0f,
  0x1.37F262p+0f, 0x1.380D6Ap+0f, 0x1.382876p+0f, 0x1.384382p+0f,
  0x1.385E92p+0f, 0x1.3879A4p+0f, 0x1.3894B8p+0f, 0x1.38AFCEp+0f,
  0x1.38CAE6p+0f, 0x1.38E602p+0f, 0x1.390120p+0f, 0x1.391C40p+0f,
  0x1.393762p+0f, 0x1.395286p+0f, 0x1.396DACp+0f, 0x1.3988D6p+0f,
  0x1.39A402p+0f, 0x1.39BF30p+0f, 0x1.39DA60p+0f, 0x1.39F592p+0f,
  0x1.3A10C8p+0f, 0x1.3A2C00p+0f, 0x1.3A4738p+0f, 0x1.3A6274p+0f,
  0x1.3A7DB4p+0f, 0x1.3A98F4p+0f, 0x1.3AB438p+0f, 0x1.3ACF7Cp+0f,
  0x1.3AEAC4p+0f, 0x1.3B0610p+0f, 0x1.3B215Cp+0f, 0x1.3B3CAAp+0f,
  0x1.3B57FCp+0f, 0x1.3B7350p+0f, 0x1.3B8EA6p+0f, 0x1.3BA9FEp+0f,
  0x1.3BC55Ap+0f, 0x1.3BE0B6p+0f, 0x1.3BFC16p+0f, 0x1.3C1778p+0f,
  0x1.3C32DCp+0f, 0x1.3C4E42p+0f, 0x1.3C69ACp+0f, 0x1.3C8518p+0f,
  0x1.3CA086p+0f, 0x1.3CBBF6p+0f, 0x1.3CD768p+0f, 0x1.3CF2DCp+0f,
  0x1.3D0E54p+0f, 0x1.3D29CEp+0f, 0x1.3D454Ap+0f, 0x1.3D60C8p+0f,
  0x1.3D7C4Ap+0f, 0x1.3D97CCp+0f, 0x1.3DB352p+0f, 0x1.3DCEDAp+0f,
  0x1.3DEA64p+0f, 0x1.3E05F2p+0f, 0x1.3E2180p+0f, 0x1.3E3D12p+0f,
  0x1.3E58A6p+0f, 0x1.3E743Cp+0f, 0x1.3E8FD6p+0f, 0x1.3EAB70p+0f,
  0x1.3EC70Ep+0f, 0x1.3EE2AEp+0f, 0x1.3EFE50p+0f, 0x1.3F19F4p+0f,
  0x1.3F359Cp+0f, 0x1.3F5146p+0f, 0x1.3F6CF2p+0f, 0x1.3F88A0p+0f,
  0x1.3FA450p+0f, 0x1.3FC004p+0f, 0x1.3FDBB8p+0f, 0x1.3FF770p+0f,
  0x1.40132Cp+0f, 0x1.402EE8p+0f, 0x1.404AA6p+0f, 0x1.406668p+0f,
  0x1.40822Cp+0f, 0x1.409DF2p+0f, 0x1.40B9BCp+0f, 0x1.40D586p+0f,
  0x1.40F154p+0f, 0x1.410D24p+0f, 0x1.4128F6p+0f, 0x1.4144CAp+0f,
  0x1.4160A2p+0f, 0x1.417C7Cp+0f, 0x1.419858p+0f, 0x1.41B436p+0f,
  0x1.41D016p+0f, 0x1.41EBFAp+0f, 0x1.4207E0p+0f, 0x1.4223C8p+0f,
  0x1.423FB2p+0f, 0x1.425BA0p+0f, 0x1.42778Ep+0f, 0x1.429380p+0f,
  0x1.42AF74p+0f, 0x1.42CB6Cp+0f, 0x1.42E764p+0f, 0x1.430360p+0f,
  0x1.431F5Ep+0f, 0x1.433B5Ep+0f, 0x1.435760p+0f, 0x1.437366p+0f,
  0x1.438F6Ep+0f, 0x1.43AB78p+0f, 0x1.43C784p+0f, 0x1.43E392p+0f,
  0x1.43FFA4p+0f, 0x1.441BB8p+0f, 0x1.4437CEp+0f, 0x1.4453E6p+0f,
  0x1.447002p+0f, 0x1.448C1Ep+0f, 0x1.44A83Ep+0f, 0x1.44C462p+0f,
  0x1.44E086p+0f, 0x1.44FCAEp+0f, 0x1.4518D6p+0f, 0x1.453504p+0f,
  0x1.455132p+0f, 0x1.456D62p+0f, 0x1.458996p+0f, 0x1.45A5CCp+0f,
  0x1.45C204p+0f, 0x1.45DE3Ep+0f, 0x1.45FA7Cp+0f, 0x1.4616BCp+0f,
  0x1.4632FEp+0f, 0x1.464F42p+0f, 0x1.466B8Ap+0f, 0x1.4687D2p+0f,
  0x1.46A41Ep+0f, 0x1.46C06Ep+0f, 0x1.46DCBEp+0f, 0x1.46F912p+0f,
  0x1.471566p+0f, 0x1.4731C0p+0f, 0x1.474E1Ap+0f, 0x1.476A76p+0f,
  0x1.4786D6p+0f, 0x1.47A338p+0f, 0x1.47BF9Cp+0f, 0x1.47DC04p+0f,
  0x1.47F86Ep+0f, 0x1.4814DAp+0f, 0x1.483148p+0f, 0x1.484DB8p+0f,
  0x1.486A2Cp+0f, 0x1.4886A2p+0f, 0x1.48A31Ap+0f, 0x1.48BF94p+0f,
  0x1.48DC10p+0f, 0x1.48F890p+0f, 0x1.491512p+0f, 0x1.493198p+0f,
  0x1.494E1Ep+0f, 0x1.496AA8p+0f, 0x1.498734p+0f, 0x1.49A3C2p+0f,
  0x1.49C052p+0f, 0x1.49DCE6p+0f, 0x1.49F97Cp+0f, 0x1.4A1614p+0f,
  0x1.4A32B0p+0f, 0x1.4A4F4Cp+0f, 0x1.4A6BECp+0f, 0x1.4A888Ep+0f,
  0x1.4AA532p+0f, 0x1.4AC1DAp+0f, 0x1.4ADE84p+0f, 0x1.4AFB30p+0f,
  0x1.4B17DEp+0f, 0x1.4B3490p+0f, 0x1.4B5144p+0f, 0x1.4B6DFAp+0f,
  0x1.4B8AB2p+0f, 0x1.4BA76Ep+0f, 0x1.4BC42Ap+0f, 0x1.4BE0EAp+0f,
  0x1.4BFDAEp+0f, 0x1.4C1A72p+0f, 0x1.4C373Ap+0f, 0x1.4C5404p+0f,
  0x1.4C70D0p+0f, 0x1.4C8DA0p+0f, 0x1.4CAA70p+0f, 0x1.4CC744p+0f,
  0x1.4CE41Cp+0f, 0x1.4D00F4p+0f, 0x1.4D1DD0p+0f, 0x1.4D3AAEp+0f,
  0x1.4D578Ep+0f, 0x1.4D7472p+0f, 0x1.4D9158p+0f, 0x1.4DAE40p+0f,
  0x1.4DCB2Ap+0f, 0x1.4DE816p+0f, 0x1.4E0506p+0f, 0x1.4E21F8p+0f,
  0x1.4E3EECp+0f, 0x1.4E5BE4p+0f, 0x1.4E78DEp+0f, 0x1.4E95DAp+0f,
  0x1.4EB2D8p+0f, 0x1.4ECFDAp+0f, 0x1.4EECDCp+0f, 0x1.4F09E2p+0f,
  0x1.4F26ECp+0f, 0x1.4F43F6p+0f, 0x1.4F6104p+0f, 0x1.4F7E14p+0f,
  0x1.4F9B28p+0f, 0x1.4FB83Cp+0f, 0x1.4FD554p+0f, 0x1.4FF26Ep+0f,
  0x1.500F8Cp+0f, 0x1.502CAAp+0f, 0x1.5049CCp+0f, 0x1.5066F2p+0f,
  0x1.508418p+0f, 0x1.50A142p+0f, 0x1.50BE6Ep+0f, 0x1.50DB9Cp+0f,
  0x1.50F8CCp+0f, 0x1.511600p+0f, 0x1.513336p+0f, 0x1.515070p+0f,
  0x1.516DAAp+0f, 0x1.518AE8p+0f, 0x1.51A828p+0f, 0x1.51C56Ap+0f,
  0x1.51E2B0p+0f, 0x1.51FFF8p+0f, 0x1.521D42p+0f, 0x1.523A90p+0f,
  0x1.5257DEp+0f, 0x1.527530p+0f, 0x1.529284p+0f, 0x1.52AFDCp+0f,
  0x1.52CD36p+0f, 0x1.52EA92p+0f, 0x1.5307F0p+0f, 0x1.532552p+0f,
  0x1.5342B6p+0f, 0x1.53601Cp+0f, 0x1.537D84p+0f, 0x1.539AF0p+0f,
  0x1.53B85Ep+0f, 0x1.53D5CEp+0f, 0x1.53F342p+0f, 0x1.5410B8p+0f,
  0x1.542E30p+0f, 0x1.544BAAp+0f, 0x1.546928p+0f, 0x1.5486A8p+0f,
  0x1.54A42Ap+0f, 0x1.54C1AEp+0f, 0x1.54DF36p+0f, 0x1.54FCC0p+0f,
  0x1.551A4Cp+0f, 0x1.5537DCp+0f, 0x1.55556Ep+0f, 0x1.557302p+0f,
  0x1.559098p+0f, 0x1.55AE32p+0f, 0x1.55CBCEp+0f, 0x1.55E96Cp+0f,
  0x1.56070Ep+0f, 0x1.5624B2p+0f, 0x1.564258p+0f, 0x1.566000p+0f,
  0x1.567DACp+0f, 0x1.569B5Ap+0f, 0x1.56B90Ap+0f, 0x1.56D6BEp+0f,
  0x1.56F474p+0f, 0x1.57122Cp+0f, 0x1.572FE6p+0f, 0x1.574DA4p+0f,
  0x1.576B64p+0f, 0x1.578926p+0f, 0x1.57A6ECp+0f, 0x1.57C4B4p+0f,
  0x1.57E27Ep+0f, 0x1.58004Ap+0f, 0x1.581E1Ap+0f, 0x1.583BECp+0f,
  0x1.5859C0p+0f, 0x1.587798p+0f, 0x1.589572p+0f, 0x1.58B34Ep+0f,
  0x1.58D12Ep+0f, 0x1.58EF0Ep+0f, 0x1.590CF4p+0f, 0x1.592ADAp+0f,
  0x1.5948C4p+0f, 0x1.5966B0p+0f, 0x1.59849Ep+0f, 0x1.59A28Ep+0f,
  0x1.59C082p+0f, 0x1.59DE78p+0f, 0x1.59FC72p+0f, 0x1.5A1A6Ep+0f,
  0x1.5A386Cp+0f, 0x1.5A566Cp+0f, 0x1.5A7470p+0f, 0x1.5A9276p+0f,
  0x1.5AB07Ep+0f, 0x1.5ACE88p+0f, 0x1.5AEC96p+0f, 0x1.5B0AA6p+0f,
  0x1.5B28BAp+0f, 0x1.5B46D0p+0f, 0x1.5B64E8p+0f, 0x1.5B8302p+0f,
  0x1.5BA120p+0f, 0x1.5BBF40p+0f, 0x1.5BDD62p+0f, 0x1.5BFB88p+0f,
  0x1.5C19B0p+0f, 0x1.5C37DAp+0f, 0x1.5C5606p+0f, 0x1.5C7436p+0f,
  0x1.5C9268p+0f, 0x1.5CB09Ep+0f, 0x1.5CCED6p+0f, 0x1.5CED10p+0f,
  0x1.5D0B4Cp+0f, 0x1.5D298Cp+0f, 0x1.5D47CEp+0f, 0x1.5D6612p+0f,
  0x1.5D845Ap+0f, 0x1.5DA2A2p+0f, 0x1.5DC0F0p+0f, 0x1.5DDF3Ep+0f,
  0x1.5DFD90p+0f, 0x1.5E1BE4p+0f, 0x1.5E3A3Cp+0f, 0x1.5E5896p+0f,
  0x1.5E76F2p+0f, 0x1.5E9550p+0f, 0x1.5EB3B2p+0f, 0x1.5ED216p+0f,
  0x1.5EF07Cp+0f, 0x1.5F0EE6p+0f, 0x1.5F2D52p+0f, 0x1.5F4BC0p+0f,
  0x1.5F6A32p+0f, 0x1.5F88A6p+0f, 0x1.5FA71Cp+0f, 0x1.5FC596p+0f,
  0x1.5FE412p+0f, 0x1.600290p+0f, 0x1.602112p+0f, 0x1.603F96p+0f,
  0x1.605E1Cp+0f, 0x1.607CA4p+0f, 0x1.609B30p+0f, 0x1.60B9BEp+0f,
  0x1.60D850p+0f, 0x1.60F6E4p+0f, 0x1.61157Ap+0f, 0x1.613412p+0f,
  0x1.6152AEp+0f, 0x1.61714Cp+0f, 0x1.618FEEp+0f, 0x1.61AE92p+0f,
  0x1.61CD38p+0f, 0x1.61EBE0p+0f, 0x1.620A8Cp+0f, 0x1.62293Ap+0f,
  0x1.6247ECp+0f, 0x1.62669Ep+0f, 0x1.628554p+0f, 0x1.62A40Ep+0f,
  0x1.62C2CAp+0f, 0x1.62E188p+0f, 0x1.630048p+0f, 0x1.631F0Cp+0f,
  0x1.633DD2p+0f, 0x1.635C9Ap+0f, 0x1.637B66p+0f, 0x1.639A34p+0f,
  0x1.63B906p+0f, 0x1.63D7D8p+0f, 0x1.63F6AEp+0f, 0x1.641588p+0f,
  0x1.643464p+0f, 0x1.645342p+0f, 0x1.647222p+0f, 0x1.649106p+0f,
  0x1.64AFECp+0f, 0x1.64CED6p+0f, 0x1.64EDC0p+0f, 0x1.650CAEp+0f,
  0x1.652BA0p+0f, 0x1.654A94p+0f, 0x1.65698Ap+0f, 0x1.658882p+0f,
  0x1.65A77Ep+0f, 0x1.65C67Cp+0f, 0x1.65E57Ep+0f, 0x1.660482p+0f,
  0x1.662388p+0f, 0x1.664292p+0f, 0x1.66619Ep+0f, 0x1.6680ACp+0f,
  0x1.669FBCp+0f, 0x1.66BED0p+0f, 0x1.66DDE8p+0f, 0x1.66FD00p+0f,
  0x1.671C1Cp+0f, 0x1.673B3Cp+0f, 0x1.675A5Cp+0f, 0x1.677980p+0f,
  0x1.6798A8p+0f, 0x1.67B7D0p+0f, 0x1.67D6FCp+0f, 0x1.67F62Cp+0f,
  0x1.68155Ep+0f, 0x1.683492p+0f, 0x1.6853C8p+0f, 0x1.687302p+0f,
  0x1.68923Ep+0f, 0x1.68B17Ep+0f, 0x1.68D0C0p+0f, 0x1.68F004p+0f,
  0x1.690F4Cp+0f, 0x1.692E96p+0f, 0x1.694DE2p+0f, 0x1.696D30p+0f,
  0x1.698C84p+0f, 0x1.69ABD8p+0f, 0x1.69CB30p+0f, 0x1.69EA8Ap+0f,
  0x1.6A09E6p+0f, 0x1.6A2946p+0f, 0x1.6A48A8p+0f, 0x1.6A680Ep+0f,
  0x1.6A8776p+0f, 0x1.6AA6E0p+0f, 0x1.6AC64Ep+0f, 0x1.6AE5BCp+0f,
  0x1.6B0530p+0f, 0x1.6B24A6p+0f, 0x1.6B441Ep+0f, 0x1.6B6398p+0f,
  0x1.6B8316p+0f, 0x1.6BA296p+0f, 0x1.6BC21Ap+0f, 0x1.6BE19Ep+0f,
  0x1.6C0128p+0f, 0x1.6C20B2p+0f, 0x1.6C4040p+0f, 0x1.6C5FD2p+0f,
  0x1.6C7F64p+0f, 0x1.6C9EFAp+0f, 0x1.6CBE94p+0f, 0x1.6CDE30p+0f,
  0x1.6CFDCEp+0f, 0x1.6D1D70p+0f, 0x1.6D3D12p+0f, 0x1.6D5CBAp+0f,
  0x1.6D7C62p+0f, 0x1.6D9C0Ep+0f, 0x1.6DBBBEp+0f, 0x1.6DDB70p+0f,
  0x1.6DFB24p+0f, 0x1.6E1ADAp+0f, 0x1.6E3A94p+0f, 0x1.6E5A52p+0f,
  0x1.6E7A10p+0f, 0x1.6E99D2p+0f, 0x1.6EB998p+0f, 0x1.6ED960p+0f,
  0x1.6EF92Ap+0f, 0x1.6F18F6p+0f, 0x1.6F38C6p+0f, 0x1.6F589Ap+0f,
  0x1.6F786Ep+0f, 0x1.6F9846p+0f, 0x1.6FB822p+0f, 0x1.6FD800p+0f,
  0x1.6FF7E0p+0f, 0x1.7017C2p+0f, 0x1.7037A8p+0f, 0x1.705792p+0f,
  0x1.70777Cp+0f, 0x1.70976Cp+0f, 0x1.70B75Cp+0f, 0x1.70D750p+0f,
  0x1.70F746p+0f, 0x1.711740p+0f, 0x1.71373Cp+0f, 0x1.71573Ap+0f,
  0x1.71773Cp+0f, 0x1.719740p+0f, 0x1.71B748p+0f, 0x1.71D752p+0f,
  0x1.71F75Ep+0f, 0x1.72176Ep+0f, 0x1.723780p+0f, 0x1.725796p+0f,
  0x1.7277AEp+0f, 0x1.7297C8p+0f, 0x1.72B7E6p+0f, 0x1.72D806p+0f,
  0x1.72F828p+0f, 0x1.73184Ep+0f, 0x1.733876p+0f, 0x1.7358A2p+0f,
  0x1.7378D0p+0f, 0x1.739902p+0f, 0x1.73B934p+0f, 0x1.73D96Cp+0f,
  0x1.73F9A4p+0f, 0x1.7419E0p+0f, 0x1.743A20p+0f, 0x1.745A62p+0f,
  0x1.747AA6p+0f, 0x1.749AECp+0f, 0x1.74BB36p+0f, 0x1.74DB84p+0f,
  0x1.74FBD4p+0f, 0x1.751C26p+0f, 0x1.753C7Cp+0f, 0x1.755CD4p+0f,
  0x1.757D2Ep+0f, 0x1.759D8Cp+0f, 0x1.75BDECp+0f, 0x1.75DE50p+0f,
  0x1.75FEB6p+0f, 0x1.761F1Ep+0f, 0x1.763F8Ap+0f, 0x1.765FF8p+0f,
  0x1.76806Ap+0f, 0x1.76A0DEp+0f, 0x1.76C154p+0f, 0x1.76E1CEp+0f,
  0x1.77024Cp+0f, 0x1.7722CAp+0f, 0x1.77434Cp+0f, 0x1.7763D2p+0f,
  0x1.77845Ap+0f, 0x1.77A4E4p+0f, 0x1.77C572p+0f, 0x1.77E602p+0f,
  0x1.780694p+0f, 0x1.78272Ap+0f, 0x1.7847C4p+0f, 0x1.786860p+0f,
  0x1.7888FEp+0f, 0x1.78A99Ep+0f, 0x1.78CA42p+0f, 0x1.78EAEAp+0f,
  0x1.790B94p+0f, 0x1.792C40p+0f, 0x1.794CF0p+0f, 0x1.796DA2p+0f,
  0x1.798E56p+0f, 0x1.79AF0Ep+0f, 0x1.79CFCAp+0f, 0x1.79F086p+0f,
  0x1.7A1148p+0f, 0x1.7A320Ap+0f, 0x1.7A52D0p+0f, 0x1.7A739Ap+0f,
  0x1.7A9466p+0f, 0x1.7AB534p+0f, 0x1.7AD606p+0f, 0x1.7AF6DAp+0f,
  0x1.7B17B0p+0f, 0x1.7B388Ap+0f, 0x1.7B5968p+0f, 0x1.7B7A48p+0f,
  0x1.7B9B2Ap+0f, 0x1.7BBC0Ep+0f, 0x1.7BDCF8p+0f, 0x1.7BFDE2p+0f,
  0x1.7C1ED0p+0f, 0x1.7C3FC0p+0f, 0x1.7C60B4p+0f, 0x1.7C81AAp+0f,
  0x1.7CA2A4p+0f, 0x1.7CC3A0p+0f, 0x1.7CE4A0p+0f, 0x1.7D05A2p+0f,
  0x1.7D26A6p+0f, 0x1.7D47AEp+0f, 0x1.7D68B8p+0f, 0x1.7D89C6p+0f,
  0x1.7DAAD6p+0f, 0x1.7DCBE8p+0f, 0x1.7DECFEp+0f, 0x1.7E0E18p+0f,
  0x1.7E2F34p+0f, 0x1.7E5052p+0f, 0x1.7E7174p+0f, 0x1.7E9298p+0f,
  0x1.7EB3BEp+0f, 0x1.7ED4E8p+0f, 0x1.7EF616p+0f, 0x1.7F1746p+0f,
  0x1.7F3878p+0f, 0x1.7F59AEp+0f, 0x1.7F7AE6p+0f, 0x1.7F9C22p+0f,
  0x1.7FBD60p+0f, 0x1.7FDEA0p+0f, 0x1.7FFFE4p+0f, 0x1.80212Cp+0f,
  0x1.804276p+0f, 0x1.8063C2p+0f, 0x1.808512p+0f, 0x1.80A664p+0f,
  0x1.80C7B8p+0f, 0x1.80E912p+0f, 0x1.810A6Cp+0f, 0x1.812BCAp+0f,
  0x1.814D2Ap+0f, 0x1.816E8Ep+0f, 0x1.818FF6p+0f, 0x1.81B15Ep+0f,
  0x1.81D2CCp+0f, 0x1.81F43Ap+0f, 0x1.8215ACp+0f, 0x1.823722p+0f,
  0x1.82589Ap+0f, 0x1.827A14p+0f, 0x1.829B92p+0f, 0x1.82BD12p+0f,
  0x1.82DE96p+0f, 0x1.83001Ep+0f, 0x1.8321A6p+0f, 0x1.834332p+0f,
  0x1.8364C2p+0f, 0x1.838654p+0f, 0x1.83A7EAp+0f, 0x1.83C982p+0f,
  0x1.83EB1Cp+0f, 0x1.840CBAp+0f, 0x1.842E5Ap+0f, 0x1.844FFEp+0f,
  0x1.8471A4p+0f, 0x1.84934Ep+0f, 0x1.84B4FAp+0f, 0x1.84D6AAp+0f,
  0x1.84F85Cp+0f, 0x1.851A10p+0f, 0x1.853BC8p+0f, 0x1.855D84p+0f,
  0x1.857F42p+0f, 0x1.85A102p+0f, 0x1.85C2C6p+0f, 0x1.85E48Cp+0f,
  0x1.860656p+0f, 0x1.862822p+0f, 0x1.8649F2p+0f, 0x1.866BC4p+0f,
  0x1.868D9Ap+0f, 0x1.86AF72p+0f, 0x1.86D14Ep+0f, 0x1.86F32Cp+0f,
  0x1.87150Cp+0f, 0x1.8736F0p+0f, 0x1.8758D6p+0f, 0x1.877AC0p+0f,
  0x1.879CAEp+0f, 0x1.87BE9Ep+0f, 0x1.87E090p+0f, 0x1.880286p+0f,
  0x1.88247Ep+0f, 0x1.88467Ap+0f, 0x1.886878p+0f, 0x1.888A7Ap+0f,
  0x1.88AC7Ep+0f, 0x1.88CE84p+0f, 0x1.88F090p+0f, 0x1.89129Cp+0f,
  0x1.8934ACp+0f, 0x1.8956C0p+0f, 0x1.8978D6p+0f, 0x1.899AEEp+0f,
  0x1.89BD0Ap+0f, 0x1.89DF2Ap+0f, 0x1.8A014Ap+0f, 0x1.8A2370p+0f,
  0x1.8A4598p+0f, 0x1.8A67C2p+0f, 0x1.8A89F0p+0f, 0x1.8AAC20p+0f,
  0x1.8ACE54p+0f, 0x1.8AF08Ap+0f, 0x1.8B12C4p+0f, 0x1.8B3500p+0f,
  0x1.8B5740p+0f, 0x1.8B7982p+0f, 0x1.8B9BC8p+0f, 0x1.8BBE10p+0f,
  0x1.8BE05Cp+0f, 0x1.8C02AAp+0f, 0x1.8C24FCp+0f, 0x1.8C4750p+0f,
  0x1.8C69A6p+0f, 0x1.8C8C00p+0f, 0x1.8CAE5Ep+0f, 0x1.8CD0BEp+0f,
  0x1.8CF322p+0f, 0x1.8D1588p+0f, 0x1.8D37F0p+0f, 0x1.8D5A5Cp+0f,
  0x1.8D7CCCp+0f, 0x1.8D9F3Ep+0f, 0x1.8DC1B2p+0f, 0x1.8DE42Ap+0f,
  0x1.8E06A6p+0f, 0x1.8E2924p+0f, 0x1.8E4BA4p+0f, 0x1.8E6E28p+0f,
  0x1.8E90B0p+0f, 0x1.8EB33Ap+0f, 0x1.8ED5C6p+0f, 0x1.8EF856p+0f,
  0x1.8F1AEAp+0f, 0x1.8F3D80p+0f, 0x1.8F6018p+0f, 0x1.8F82B4p+0f,
  0x1.8FA554p+0f, 0x1.8FC7F6p+0f, 0x1.8FEA9Ap+0f, 0x1.900D42p+0f,
  0x1.902FEEp+0f, 0x1.90529Ap+0f, 0x1.90754Cp+0f, 0x1.909800p+0f,
  0x1.90BAB6p+0f, 0x1.90DD70p+0f, 0x1.91002Ep+0f, 0x1.9122EEp+0f,
  0x1.9145B0p+0f, 0x1.916876p+0f, 0x1.918B40p+0f, 0x1.91AE0Cp+0f,
  0x1.91D0DAp+0f, 0x1.91F3ACp+0f, 0x1.921682p+0f, 0x1.92395Ap+0f,
  0x1.925C36p+0f, 0x1.927F14p+0f, 0x1.92A1F4p+0f, 0x1.92C4D8p+0f,
  0x1.92E7C0p+0f, 0x1.930AAAp+0f, 0x1.932D98p+0f, 0x1.935088p+0f,
  0x1.93737Cp+0f, 0x1.939672p+0f, 0x1.93B96Ap+0f, 0x1.93DC68p+0f,
  0x1.93FF66p+0f, 0x1.94226Ap+0f, 0x1.94456Ep+0f, 0x1.946878p+0f,
  0x1.948B82p+0f, 0x1.94AE92p+0f, 0x1.94D1A2p+0f, 0x1.94F4B8p+0f,
  0x1.9517D0p+0f, 0x1.953AEAp+0f, 0x1.955E08p+0f, 0x1.958128p+0f,
  0x1.95A44Cp+0f, 0x1.95C774p+0f, 0x1.95EA9Ep+0f, 0x1.960DCAp+0f,
  0x1.9630FAp+0f, 0x1.96542Ep+0f, 0x1.967764p+0f, 0x1.969A9Ep+0f,
  0x1.96BDDAp+0f, 0x1.96E118p+0f, 0x1.97045Cp+0f, 0x1.9727A0p+0f,
  0x1.974AEAp+0f, 0x1.976E34p+0f, 0x1.979184p+0f, 0x1.97B4D6p+0f,
  0x1.97D82Ap+0f, 0x1.97FB82p+0f, 0x1.981EDCp+0f, 0x1.98423Ap+0f,
  0x1.98659Cp+0f, 0x1.988900p+0f, 0x1.98AC66p+0f, 0x1.98CFD2p+0f,
  0x1.98F33Ep+0f, 0x1.9916AEp+0f, 0x1.993A22p+0f, 0x1.995D98p+0f,
  0x1.998112p+0f, 0x1.99A48Ep+0f, 0x1.99C80Ep+0f, 0x1.99EB92p+0f,
  0x1.9A0F18p+0f, 0x1.9A32A0p+0f, 0x1.9A562Cp+0f, 0x1.9A79BCp+0f,
  0x1.9A9D4Ep+0f, 0x1.9AC0E2p+0f, 0x1.9AE47Ap+0f, 0x1.9B0816p+0f,
  0x1.9B2BB4p+0f, 0x1.9B4F56p+0f, 0x1.9B72FCp+0f, 0x1.9B96A2p+0f,
  0x1.9BBA4Ep+0f, 0x1.9BDDFCp+0f, 0x1.9C01ACp+0f, 0x1.9C2560p+0f,
  0x1.9C4918p+0f, 0x1.9C6CD2p+0f, 0x1.9C9090p+0f, 0x1.9CB450p+0f,
  0x1.9CD814p+0f, 0x1.9CFBDAp+0f, 0x1.9D1FA4p+0f, 0x1.9D4372p+0f,
  0x1.9D6742p+0f, 0x1.9D8B14p+0f, 0x1.9DAEEAp+0f, 0x1.9DD2C4p+0f,
  0x1.9DF6A0p+0f, 0x1.9E1A80p+0f, 0x1.9E3E62p+0f, 0x1.9E6248p+0f,
  0x1.9E8632p+0f, 0x1.9EAA1Ep+0f, 0x1.9ECE0Cp+0f, 0x1.9EF1FEp+0f,
  0x1.9F15F4p+0f, 0x1.9F39ECp+0f, 0x1.9F5DE8p+0f, 0x1.9F81E8p+0f,
  0x1.9FA5E8p+0f, 0x1.9FC9EEp+0f, 0x1.9FEDF6p+0f, 0x1.A01200p+0f,
  0x1.A03610p+0f, 0x1.A05A20p+0f, 0x1.A07E36p+0f, 0x1.A0A24Cp+0f,
  0x1.A0C668p+0f, 0x1.A0EA86p+0f, 0x1.A10EA6p+0f, 0x1.A132CAp+0f,
  0x1.A156F2p+0f, 0x1.A17B1Cp+0f, 0x1.A19F4Ap+0f, 0x1.A1C37Ap+0f,
  0x1.A1E7AEp+0f, 0x1.A20BE6p+0f, 0x1.A23020p+0f, 0x1.A2545Ep+0f,
  0x1.A2789Ep+0f, 0x1.A29CE2p+0f, 0x1.A2C128p+0f, 0x1.A2E572p+0f,
  0x1.A309BEp+0f, 0x1.A32E0Ep+0f, 0x1.A35262p+0f, 0x1.A376B8p+0f,
  0x1.A39B12p+0f, 0x1.A3BF6Ep+0f, 0x1.A3E3CEp+0f, 0x1.A40832p+0f,
  0x1.A42C98p+0f, 0x1.A45102p+0f, 0x1.A4756Ep+0f, 0x1.A499DEp+0f,
  0x1.A4BE50p+0f, 0x1.A4E2C6p+0f, 0x1.A50740p+0f, 0x1.A52BBCp+0f,
  0x1.A5503Cp+0f, 0x1.A574BEp+0f, 0x1.A59944p+0f, 0x1.A5BDCCp+0f,
  0x1.A5E258p+0f, 0x1.A606E8p+0f, 0x1.A62B7Ap+0f, 0x1.A65010p+0f,
  0x1.A674A8p+0f, 0x1.A69944p+0f, 0x1.A6BDE4p+0f, 0x1.A6E286p+0f,
  0x1.A7072Cp+0f, 0x1.A72BD4p+0f, 0x1.A75080p+0f, 0x1.A77530p+0f,
  0x1.A799E2p+0f, 0x1.A7BE96p+0f, 0x1.A7E350p+0f, 0x1.A8080Ap+0f,
  0x1.A82CCAp+0f, 0x1.A8518Cp+0f, 0x1.A87652p+0f, 0x1.A89B1Ap+0f,
  0x1.A8BFE6p+0f, 0x1.A8E4B4p+0f, 0x1.A90986p+0f, 0x1.A92E5Cp+0f,
  0x1.A95334p+0f, 0x1.A97810p+0f, 0x1.A99CEEp+0f, 0x1.A9C1D0p+0f,
  0x1.A9E6B6p+0f, 0x1.AA0B9Ep+0f, 0x1.AA308Ap+0f, 0x1.AA5578p+0f,
  0x1.AA7A6Ap+0f, 0x1.AA9F60p+0f, 0x1.AAC458p+0f, 0x1.AAE954p+0f,
  0x1.AB0E52p+0f, 0x1.AB3354p+0f, 0x1.AB585Ap+0f, 0x1.AB7D62p+0f,
  0x1.ABA26Ep+0f, 0x1.ABC77Cp+0f, 0x1.ABEC8Ep+0f, 0x1.AC11A4p+0f,
  0x1.AC36BCp+0f, 0x1.AC5BD8p+0f, 0x1.AC80F6p+0f, 0x1.ACA618p+0f,
  0x1.ACCB3Ep+0f, 0x1.ACF066p+0f, 0x1.AD1592p+0f, 0x1.AD3AC2p+0f,
  0x1.AD5FF4p+0f, 0x1.AD852Ap+0f, 0x1.ADAA62p+0f, 0x1.ADCF9Ep+0f,
  0x1.ADF4DCp+0f, 0x1.AE1A20p+0f, 0x1.AE3F64p+0f, 0x1.AE64AEp+0f,
  0x1.AE89FAp+0f, 0x1.AEAF48p+0f, 0x1.AED49Cp+0f, 0x1.AEF9F2p+0f,
  0x1.AF1F4Ap+0f, 0x1.AF44A6p+0f, 0x1.AF6A06p+0f, 0x1.AF8F68p+0f,
  0x1.AFB4CEp+0f, 0x1.AFDA38p+0f, 0x1.AFFFA4p+0f, 0x1.B02514p+0f,
  0x1.B04A86p+0f, 0x1.B06FFCp+0f, 0x1.B09576p+0f, 0x1.B0BAF2p+0f,
  0x1.B0E072p+0f, 0x1.B105F6p+0f, 0x1.B12B7Cp+0f, 0x1.B15106p+0f,
  0x1.B17692p+0f, 0x1.B19C22p+0f, 0x1.B1C1B6p+0f, 0x1.B1E74Cp+0f,
  0x1.B20CE6p+0f, 0x1.B23284p+0f, 0x1.B25824p+0f, 0x1.B27DC8p+0f,
  0x1.B2A370p+0f, 0x1.B2C91Ap+0f, 0x1.B2EEC6p+0f, 0x1.B31478p+0f,
  0x1.B33A2Cp+0f, 0x1.B35FE2p+0f, 0x1.B3859Ep+0f, 0x1.B3AB5Cp+0f,
  0x1.B3D11Cp+0f, 0x1.B3F6E0p+0f, 0x1.B41CA8p+0f, 0x1.B44274p+0f,
  0x1.B46842p+0f, 0x1.B48E12p+0f, 0x1.B4B3E8p+0f, 0x1.B4D9C0p+0f,
  0x1.B4FF9Ap+0f, 0x1.B5257Ap+0f, 0x1.B54B5Cp+0f, 0x1.B57140p+0f,
  0x1.B59728p+0f, 0x1.B5BD14p+0f, 0x1.B5E304p+0f, 0x1.B608F6p+0f,
  0x1.B62EECp+0f, 0x1.B654E4p+0f, 0x1.B67AE0p+0f, 0x1.B6A0E0p+0f,
  0x1.B6C6E2p+0f, 0x1.B6ECE8p+0f, 0x1.B712F2p+0f, 0x1.B738FEp+0f,
  0x1.B75F0Ep+0f, 0x1.B78522p+0f, 0x1.B7AB38p+0f, 0x1.B7D152p+0f,
  0x1.B7F770p+0f, 0x1.B81D90p+0f, 0x1.B843B4p+0f, 0x1.B869DAp+0f,
  0x1.B89004p+0f, 0x1.B8B632p+0f, 0x1.B8DC64p+0f, 0x1.B90298p+0f,
  0x1.B928D0p+0f, 0x1.B94F0Ap+0f, 0x1.B97548p+0f, 0x1.B99B8Ap+0f,
  0x1.B9C1CEp+0f, 0x1.B9E816p+0f, 0x1.BA0E62p+0f, 0x1.BA34B0p+0f,
  0x1.BA5B04p+0f, 0x1.BA8158p+0f, 0x1.BAA7B2p+0f, 0x1.BACE0Ep+0f,
  0x1.BAF46Cp+0f, 0x1.BB1AD0p+0f, 0x1.BB4136p+0f, 0x1.BB679Ep+0f,
  0x1.BB8E0Cp+0f, 0x1.BBB47Cp+0f, 0x1.BBDAEEp+0f, 0x1.BC0166p+0f,
  0x1.BC27E0p+0f, 0x1.BC4E5Cp+0f, 0x1.BC74DEp+0f, 0x1.BC9B62p+0f,
  0x1.BCC1EAp+0f, 0x1.BCE874p+0f, 0x1.BD0F02p+0f, 0x1.BD3594p+0f,
  0x1.BD5C28p+0f, 0x1.BD82C0p+0f, 0x1.BDA95Cp+0f, 0x1.BDCFFAp+0f,
  0x1.BDF69Cp+0f, 0x1.BE1D42p+0f, 0x1.BE43EAp+0f, 0x1.BE6A96p+0f,
  0x1.BE9146p+0f, 0x1.BEB7FAp+0f, 0x1.BEDEB0p+0f, 0x1.BF0568p+0f,
  0x1.BF2C26p+0f, 0x1.BF52E6p+0f, 0x1.BF79AAp+0f, 0x1.BFA070p+0f,
  0x1.BFC73Cp+0f, 0x1.BFEE08p+0f, 0x1.C014DAp+0f, 0x1.C03BAEp+0f,
  0x1.C06286p+0f, 0x1.C08962p+0f, 0x1.C0B040p+0f, 0x1.C0D722p+0f,
  0x1.C0FE06p+0f, 0x1.C124F0p+0f, 0x1.C14BDCp+0f, 0x1.C172CCp+0f,
  0x1.C199BEp+0f, 0x1.C1C0B4p+0f, 0x1.C1E7AEp+0f, 0x1.C20EAAp+0f,
  0x1.C235AAp+0f, 0x1.C25CAEp+0f, 0x1.C283B6p+0f, 0x1.C2AAC0p+0f,
  0x1.C2D1CEp+0f, 0x1.C2F8DEp+0f, 0x1.C31FF4p+0f, 0x1.C3470Cp+0f,
  0x1.C36E26p+0f, 0x1.C39546p+0f, 0x1.C3BC68p+0f, 0x1.C3E38Ep+0f,
  0x1.C40AB6p+0f, 0x1.C431E2p+0f, 0x1.C45912p+0f, 0x1.C48046p+0f,
  0x1.C4A77Cp+0f, 0x1.C4CEB6p+0f, 0x1.C4F5F2p+0f, 0x1.C51D34p+0f,
  0x1.C54478p+0f, 0x1.C56BC0p+0f, 0x1.C5930Ap+0f, 0x1.C5BA58p+0f,
  0x1.C5E1AAp+0f, 0x1.C60900p+0f, 0x1.C63058p+0f, 0x1.C657B4p+0f,
  0x1.C67F12p+0f, 0x1.C6A676p+0f, 0x1.C6CDDCp+0f, 0x1.C6F546p+0f,
  0x1.C71CB2p+0f, 0x1.C74422p+0f, 0x1.C76B96p+0f, 0x1.C7930Ep+0f,
  0x1.C7BA88p+0f, 0x1.C7E206p+0f, 0x1.C80988p+0f, 0x1.C8310Ep+0f,
  0x1.C85896p+0f, 0x1.C88022p+0f, 0x1.C8A7B0p+0f, 0x1.C8CF44p+0f,
  0x1.C8F6DAp+0f, 0x1.C91E72p+0f, 0x1.C94610p+0f, 0x1.C96DB0p+0f,
  0x1.C99554p+0f, 0x1.C9BCFCp+0f, 0x1.C9E4A6p+0f, 0x1.CA0C54p+0f,
  0x1.CA3406p+0f, 0x1.CA5BBAp+0f, 0x1.CA8372p+0f, 0x1.CAAB2Ep+0f,
  0x1.CAD2EEp+0f, 0x1.CAFAB0p+0f, 0x1.CB2278p+0f, 0x1.CB4A40p+0f,
  0x1.CB720Ep+0f, 0x1.CB99DEp+0f, 0x1.CBC1B2p+0f, 0x1.CBE98Ap+0f,
  0x1.CC1164p+0f, 0x1.CC3944p+0f, 0x1.CC6124p+0f, 0x1.CC890Ap+0f,
  0x1.CCB0F2p+0f, 0x1.CCD8E0p+0f, 0x1.CD00CEp+0f, 0x1.CD28C2p+0f,
  0x1.CD50B8p+0f, 0x1.CD78B2p+0f, 0x1.CDA0B0p+0f, 0x1.CDC8B0p+0f,
  0x1.CDF0B6p+0f, 0x1.CE18BEp+0f, 0x1.CE40C8p+0f, 0x1.CE68D8p+0f,
  0x1.CE90EAp+0f, 0x1.CEB900p+0f, 0x1.CEE118p+0f, 0x1.CF0936p+0f,
  0x1.CF3156p+0f, 0x1.CF597Ap+0f, 0x1.CF81A0p+0f, 0x1.CFA9CCp+0f,
  0x1.CFD1FAp+0f, 0x1.CFFA2Ap+0f, 0x1.D02260p+0f, 0x1.D04A98p+0f,
  0x1.D072D4p+0f, 0x1.D09B14p+0f, 0x1.D0C358p+0f, 0x1.D0EB9Ep+0f,
  0x1.D113E8p+0f, 0x1.D13C36p+0f, 0x1.D16486p+0f, 0x1.D18CDAp+0f,
  0x1.D1B532p+0f, 0x1.D1DD8Ep+0f, 0x1.D205EEp+0f, 0x1.D22E50p+0f,
  0x1.D256B6p+0f, 0x1.D27F20p+0f, 0x1.D2A78Cp+0f, 0x1.D2CFFCp+0f,
  0x1.D2F870p+0f, 0x1.D320E8p+0f, 0x1.D34962p+0f, 0x1.D371E2p+0f,
  0x1.D39A64p+0f, 0x1.D3C2EAp+0f, 0x1.D3EB72p+0f, 0x1.D413FEp+0f,
  0x1.D43C8Ep+0f, 0x1.D46522p+0f, 0x1.D48DBAp+0f, 0x1.D4B654p+0f,
  0x1.D4DEF2p+0f, 0x1.D50794p+0f, 0x1.D53038p+0f, 0x1.D558E2p+0f,
  0x1.D5818Ep+0f, 0x1.D5AA3Ep+0f, 0x1.D5D2F0p+0f, 0x1.D5FBA8p+0f,
  0x1.D62462p+0f, 0x1.D64D20p+0f, 0x1.D675E2p+0f, 0x1.D69EA6p+0f,
  0x1.D6C76Ep+0f, 0x1.D6F03Ap+0f, 0x1.D7190Ap+0f, 0x1.D741DEp+0f,
  0x1.D76AB4p+0f, 0x1.D7938Ep+0f, 0x1.D7BC6Cp+0f, 0x1.D7E54Cp+0f,
  0x1.D80E32p+0f, 0x1.D8371Ap+0f, 0x1.D86006p+0f, 0x1.D888F4p+0f,
  0x1.D8B1E8p+0f, 0x1.D8DADEp+0f, 0x1.D903D8p+0f, 0x1.D92CD6p+0f,
  0x1.D955D8p+0f, 0x1.D97EDCp+0f, 0x1.D9A7E4p+0f, 0x1.D9D0F0p+0f,
  0x1.D9FA00p+0f, 0x1.DA2312p+0f, 0x1.DA4C28p+0f, 0x1.DA7542p+0f,
  0x1.DA9E60p+0f, 0x1.DAC782p+0f, 0x1.DAF0A6p+0f, 0x1.DB19CEp+0f,
  0x1.DB42FAp+0f, 0x1.DB6C2Ap+0f, 0x1.DB955Cp+0f, 0x1.DBBE94p+0f,
  0x1.DBE7CEp+0f, 0x1.DC110Cp+0f, 0x1.DC3A4Cp+0f, 0x1.DC6392p+0f,
  0x1.DC8CDAp+0f, 0x1.DCB626p+0f, 0x1.DCDF76p+0f, 0x1.DD08C8p+0f,
  0x1.DD3220p+0f, 0x1.DD5B7Ap+0f, 0x1.DD84D8p+0f, 0x1.DDAE38p+0f,
  0x1.DDD79Ep+0f, 0x1.DE0106p+0f, 0x1.DE2A72p+0f, 0x1.DE53E2p+0f,
  0x1.DE7D56p+0f, 0x1.DEA6CEp+0f, 0x1.DED048p+0f, 0x1.DEF9C6p+0f,
  0x1.DF2348p+0f, 0x1.DF4CCEp+0f, 0x1.DF7656p+0f, 0x1.DF9FE4p+0f,
  0x1.DFC974p+0f, 0x1.DFF308p+0f, 0x1.E01C9Ep+0f, 0x1.E0463Ap+0f,
  0x1.E06FD8p+0f, 0x1.E0997Ap+0f, 0x1.E0C320p+0f, 0x1.E0ECCAp+0f,
  0x1.E11676p+0f, 0x1.E14028p+0f, 0x1.E169DCp+0f, 0x1.E19394p+0f,
  0x1.E1BD50p+0f, 0x1.E1E70Ep+0f, 0x1.E210D0p+0f, 0x1.E23A98p+0f,
  0x1.E26462p+0f, 0x1.E28E2Ep+0f, 0x1.E2B800p+0f, 0x1.E2E1D6p+0f,
  0x1.E30BAEp+0f, 0x1.E3358Ap+0f, 0x1.E35F6Ap+0f, 0x1.E3894Cp+0f,
  0x1.E3B334p+0f, 0x1.E3DD1Ep+0f, 0x1.E4070Cp+0f, 0x1.E430FEp+0f,
  0x1.E45AF4p+0f, 0x1.E484EEp+0f, 0x1.E4AEEAp+0f, 0x1.E4D8EAp+0f,
  0x1.E502EEp+0f, 0x1.E52CF6p+0f, 0x1.E55702p+0f, 0x1.E58110p+0f,
  0x1.E5AB24p+0f, 0x1.E5D53Ap+0f, 0x1.E5FF54p+0f, 0x1.E62972p+0f,
  0x1.E65392p+0f, 0x1.E67DB8p+0f, 0x1.E6A7E0p+0f, 0x1.E6D20Cp+0f,
  0x1.E6FC3Cp+0f, 0x1.E72670p+0f, 0x1.E750A6p+0f, 0x1.E77AE2p+0f,
  0x1.E7A520p+0f, 0x1.E7CF62p+0f, 0x1.E7F9A8p+0f, 0x1.E823F2p+0f,
  0x1.E84E3Ep+0f, 0x1.E87890p+0f, 0x1.E8A2E4p+0f, 0x1.E8CD3Cp+0f,
  0x1.E8F798p+0f, 0x1.E921F6p+0f, 0x1.E94C5Ap+0f, 0x1.E976C0p+0f,
  0x1.E9A12Cp+0f, 0x1.E9CB9Ap+0f, 0x1.E9F60Cp+0f, 0x1.EA2080p+0f,
  0x1.EA4AFAp+0f, 0x1.EA7578p+0f, 0x1.EA9FF8p+0f, 0x1.EACA7Cp+0f,
  0x1.EAF504p+0f, 0x1.EB1F90p+0f, 0x1.EB4A1Ep+0f, 0x1.EB74B2p+0f,
  0x1.EB9F48p+0f, 0x1.EBC9E2p+0f, 0x1.EBF480p+0f, 0x1.EC1F22p+0f,
  0x1.EC49C8p+0f, 0x1.EC7472p+0f, 0x1.EC9F1Ep+0f, 0x1.ECC9CEp+0f,
  0x1.ECF482p+0f, 0x1.ED1F3Ap+0f, 0x1.ED49F6p+0f, 0x1.ED74B6p+0f,
  0x1.ED9F78p+0f, 0x1.EDCA40p+0f, 0x1.EDF50Ap+0f, 0x1.EE1FD8p+0f,
  0x1.EE4AAAp+0f, 0x1.EE7580p+0f, 0x1.EEA05Ap+0f, 0x1.EECB36p+0f,
  0x1.EEF616p+0f, 0x1.EF20FCp+0f, 0x1.EF4BE4p+0f, 0x1.EF76D0p+0f,
  0x1.EFA1BEp+0f, 0x1.EFCCB2p+0f, 0x1.EFF7AAp+0f, 0x1.F022A4p+0f,
  0x1.F04DA2p+0f, 0x1.F078A4p+0f, 0x1.F0A3AAp+0f, 0x1.F0CEB4p+0f,
  0x1.F0F9C2p+0f, 0x1.F124D2p+0f, 0x1.F14FE8p+0f, 0x1.F17B00p+0f,
  0x1.F1A61Cp+0f, 0x1.F1D13Cp+0f, 0x1.F1FC60p+0f, 0x1.F22788p+0f,
  0x1.F252B4p+0f, 0x1.F27DE2p+0f, 0x1.F2A916p+0f, 0x1.F2D44Cp+0f,
  0x1.F2FF86p+0f, 0x1.F32AC4p+0f, 0x1.F35606p+0f, 0x1.F3814Cp+0f,
  0x1.F3AC94p+0f, 0x1.F3D7E2p+0f, 0x1.F40332p+0f, 0x1.F42E86p+0f,
  0x1.F459E0p+0f, 0x1.F4853Cp+0f, 0x1.F4B09Ap+0f, 0x1.F4DBFEp+0f,
  0x1.F50766p+0f, 0x1.F532D0p+0f, 0x1.F55E40p+0f, 0x1.F589B2p+0f,
  0x1.F5B528p+0f, 0x1.F5E0A2p+0f, 0x1.F60C20p+0f, 0x1.F637A2p+0f,
  0x1.F66328p+0f, 0x1.F68EB0p+0f, 0x1.F6BA3Ep+0f, 0x1.F6E5CEp+0f,
  0x1.F71164p+0f, 0x1.F73CFCp+0f, 0x1.F76898p+0f, 0x1.F79438p+0f,
  0x1.F7BFDAp+0f, 0x1.F7EB82p+0f, 0x1.F8172Ep+0f, 0x1.F842DCp+0f,
  0x1.F86E90p+0f, 0x1.F89A46p+0f, 0x1.F8C600p+0f, 0x1.F8F1BEp+0f,
  0x1.F91D80p+0f, 0x1.F94946p+0f, 0x1.F97510p+0f, 0x1.F9A0DCp+0f,
  0x1.F9CCAEp+0f, 0x1.F9F882p+0f, 0x1.FA245Cp+0f, 0x1.FA5038p+0f,
  0x1.FA7C18p+0f, 0x1.FAA7FCp+0f, 0x1.FAD3E4p+0f, 0x1.FAFFD0p+0f,
  0x1.FB2BC0p+0f, 0x1.FB57B2p+0f, 0x1.FB83AAp+0f, 0x1.FBAFA4p+0f,
  0x1.FBDBA4p+0f, 0x1.FC07A6p+0f, 0x1.FC33ACp+0f, 0x1.FC5FB6p+0f,
  0x1.FC8BC4p+0f, 0x1.FCB7D6p+0f, 0x1.FCE3ECp+0f, 0x1.FD1006p+0f,
  0x1.FD3C22p+0f, 0x1.FD6844p+0f, 0x1.FD9468p+0f, 0x1.FDC092p+0f,
  0x1.FDECBEp+0f, 0x1.FE18EEp+0f, 0x1.FE4522p+0f, 0x1.FE715Ap+0f,
  0x1.FE9D96p+0f, 0x1.FEC9D6p+0f, 0x1.FEF61Ap+0f, 0x1.FF2262p+0f,
  0x1.FF4EACp+0f, 0x1.FF7AFCp+0f, 0x1.FFA74Ep+0f, 0x1.FFD3A6p+0f,
};

void xnn_math_f32_expminus__neonfma_lut2048_p1(
    size_t n,
    const float* input,
    float* output)
{
  assert(n % (4 * sizeof(float)) == 0);

  const float32x4_t vmagic_bias = vmovq_n_f32(0x1.800000p23f);
  // The smallest x for which expf(x) is normalized.
  const float32x4_t vdenorm_cutoff = vmovq_n_f32(-0x1.5D589Ep6f);
  const float32x4_t vlog2e_x2048  = vmovq_n_f32(0x1.715476p11f);
  const float32x4_t vminus_ln2_o2048_hi = vmovq_n_f32(-0x1.62e43p-12f);
  const float32x4_t vminus_ln2_o2048_lo = vmovq_n_f32(0x1.05c61p-40f);

  const float32x4_t vc1 = vmovq_n_f32(0x1.FFFFFEp-1f);

  const int32x4_t vindex_mask = vmovq_n_s32(INT32_C(0x7FF));

  for (; n != 0; n -= 4 * sizeof(float)) {
    const float32x4_t vx = vld1q_f32(input); input += 4;

    // Compute reduced argument n := round(x * 2048 / log(2)).
    // We do it by adding a large number (magic bias), which cause rounding of the result to an integer, then subtracing
    // the large number back. The first addition is combined with multiplication by log2e into a single FMA instruction.
    // The trick with adding large number is valid only within certain bounds (|x * 2048 / log(2)| <= 2**22, i.e.
    // |x| <= 0x1.62E43p+10 = 1419.5654296875), but that is acceptable, because inputs outside of [-87.336540, 0.0]
    // result in denormalized or underflown expf(x). We fixup the result for such inputs at the very end of the
    // algorithm.
    float32x4_t vn = vfmaq_f32(vmagic_bias, vx, vlog2e_x2048);

    // Create a floating-point number s (scale) such that s := 2**(n / 2048) for such inputs that expf(x) is normalized,
    // i.e. -87.33642 <= x <= 0.0. As n has 11 fractional bits, we split s == 2**(n / 2048) = 2**e * 2**(n / 2048 - e),
    // where e := int(n / 2048). We create s in two steps:
    // 1. Fetch 2**(n / 2048 - e) = 2**(n % 2048) from exp2_table using the 6 low bits of n, as integer. Note that the
    //    fetched values are in the [1.0, 2.0) range, i.e. their floating-point exponent is 0.
    // 2. Adjust fecthed value by addition of e to its floating-point exponent. The result is always a normalized
    //    number, because for -87.33642 <= x <= 0.0 (inputs for which expf(x) is normalized) we have -126 <= e <= 0,
    //    and thus the adjusted exponent is not lower than -126.
    //
    // Extract e from bits 11:19 of n and shift it into bits 23:31 (position of floating-point exponent).
    const int32x4_t ve = vshlq_n_s32(vbicq_s32(vreinterpretq_s32_f32(vn), vmovq_n_s32(INT32_C(0x7FF))), 12);

    // Use bits 0:11 bits of n, as integer, as an index for table lookup of l := 2**(n % 2048).
    const uint64x2_t vidx = vreinterpretq_u64_s32(vandq_s32(vreinterpretq_s32_f32(vn), vindex_mask));
    const uint64_t vidx01 = vgetq_lane_u64(vidx, 0);
    const uint64_t vidx23 = vgetq_lane_u64(vidx, 1);
    float32x2_t vl01 = vld1_dup_f32(&exp2_table[(uint32_t) vidx01]);
    float32x2_t vl23 = vld1_dup_f32(&exp2_table[(uint32_t) vidx23]);
    vl01 = vld1_lane_f32(&exp2_table[(uint32_t) (vidx01 >> 32)], vl01, 1);
    vl23 = vld1_lane_f32(&exp2_table[(uint32_t) (vidx23 >> 32)], vl23, 1);
    const float32x4_t vl = vcombine_f32(vl01, vl23);
    // Adjust exponent of the value l fetched from the exp2_table to get the final s value.
    const float32x4_t vs = vreinterpretq_f32_s32(vaddq_s32(vreinterpretq_s32_f32(vl), ve));

    // Subtract the large number back to get final n := round(x * 2048 / log(2)) as a floating-point number.
    vn = vsubq_f32(vn, vmagic_bias);

    // Compute reduced argument t := x - n * log(2) / 2048.
    // Use Cody-Waite range reduction method (note the two constants representing log(2) / 2048) to improve accuracy.
    float32x4_t vt = vfmaq_f32(vx, vn, vminus_ln2_o2048_hi);
    vt = vfmaq_f32(vt, vn, vminus_ln2_o2048_lo);

    // Compute degree-1 polynomial approxiatmion for exp(t) on [-log(2)/2048, log(2)/2048].
    const float32x4_t vp = vmulq_f32(vt, vc1);

    // Reconstruct the final f value:
    //   f = s * (1 + t * c1)
    //     = s + s * (t * c1))
    //     = s + s * p
    float32x4_t vf = vfmaq_f32(vs, vs, vp);

    // For inputs below denormal cutoff, replace output with +0.0f.
    // Note that for NaN inputs, comparison result is false, and outputs are left unchanged.
    vf = vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(vf), vcltq_f32(vx, vdenorm_cutoff)));
    vst1q_f32(output, vf); output += 4;
  }
}