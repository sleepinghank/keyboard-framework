#ifndef __INATECK_PRODUCT_CATALOG_H_
#define __INATECK_PRODUCT_CATALOG_H_

//---------------------------------------------------------产品型号表---------------------------------------------------------//
/*编码规则：高12bit为产品类型（0x001 键盘
                             0x002 鼠标
                             0x003 扫码枪
                             0x004 手写笔）
            低20bit为产品编码（其中高16bit为产品型号，低4bit为产品子类）（0x00010 KB09117_DE
                                                                      0x00011 KB09117_US
                                                                      0x00020 KB01104_DE
                                                                      0x0002F KB01104_ALL 注意：如果键盘只有一个固件通用多个语种的话则使用ALL而不再单独区分语种）

*/
#define _PRODUCT_KB09117_DE    (0x00100010)  //Inateck KB09117（键数：64键 触摸板：有 键芯：DE 系统：Mac）
#define _PRODUCT_KB09117_US    (0x00100011)  //Inateck KB09117（键数：64键 触摸板：有 键芯：US 系统：Mac）
#define _PRODUCT_KB01104_DE    (0x00100020)  //Inateck KB01104（键数：79键 触摸板：有 键芯：DE 系统：Mac Win Android）
#define _PRODUCT_KB01104_UK    (0x00100021)  //Inateck KB01104（键数：79键 触摸板：有 键芯：UK 系统：Mac Win Android）
#define _PRODUCT_KB01104_US    (0x00100022)  //Inateck KB01104（键数：79键 触摸板：有 键芯：US 系统：Mac Win Android）
#define _PRODUCT_BK2007_US     (0x00100030)  //Inateck KB2007 （键数：78键 触摸板：无 键芯：US 系统：IOS）
#define _PRODUCT_BK2007_DE     (0x00100031)  //Inateck KB2007 （键数：79键 触摸板：无 键芯：DE 系统：IOS）
#define _PRODUCT_BK2007_UK     (0x00100032)  //Inateck KB2007 （键数：79键 触摸板：无 键芯：UK 系统：IOS）
#define _PRODUCT_BK2011_US     (0x00100040)  //Inateck KB2001 （键数：78键 触摸板：无 键芯：US 系统：IOS）
#define _PRODUCT_BK2011_DE     (0x00100041)  //Inateck KB2001 （键数：78键 触摸板：无 键芯：DE 系统：IOS）
#define _PRODUCT_BK2011_UK     (0x00100042)  //Inateck KB2001 （键数：78键 触摸板：无 键芯：UK 系统：IOS）
#define _PRODUCT_KB06103_US    (0x00100050)  //Inateck KB06103 （键数：78键 触摸板：有 键芯：US 系统：Mac Win Android）
#define _PRODUCT_KB06103_DE    (0x00100051)  //Inateck KB06103 （键数：78键 触摸板：有 键芯：DE 系统：Mac Win Android）
#define _PRODUCT_KB09119_US    (0x00100060)  //Inateck KB09119 （键数：64键 触摸板：有 键芯：US 系统：IOS）
#define _PRODUCT_KB09119_DE    (0x00100061)  //Inateck KB09119 （键数：64键 触摸板：有 键芯：DE 系统：IOS）
#define _PRODUCT_KB09119_UK    (0x00100062)  //Inateck KB09119 （键数：64键 触摸板：有 键芯：UK 系统：IOS）
#define _PRODUCT_KB09119_JP    (0x00100063)  //Inateck KB09119 （键数：64键 触摸板：有 键芯：JP 系统：IOS）
#define _PRODUCT_KB09119_FR    (0x00100064)  //Inateck KB09119 （键数：64键 触摸板：有 键芯：FR 系统：IOS）
#define _PRODUCT_BK2007_RGB_US (0x00100070)  //Inateck KB2007-RGB （键数：78键 触摸板：无 键芯：US 系统：IOS）
#define _PRODUCT_BK2007_RGB_DE (0x00100071)  //Inateck KB2007-RGB （键数：78键 触摸板：无 键芯：DE 系统：IOS）
#define _PRODUCT_BK2007_RGB_UK (0x00100072)  //Inateck KB2007-RGB （键数：78键 触摸板：无 键芯：UK 系统：IOS）
#define _PRODUCT_KB02027_US    (0x00100080)  //Inateck KB2001-RGB （键数：78键 触摸板：有 键芯：US 系统：Win）
#define _PRODUCT_KB02027_DE    (0x00100081)  //Inateck KB2001-RGB （键数：78键 触摸板：有 键芯：DE 系统：Win）
#define _PRODUCT_KB02027_UK    (0x00100082)  //Inateck KB2001-RGB （键数：78键 触摸板：有 键芯：UK 系统：Win）
#define _PRODUCT_KB05113_US    (0x00100090)  //Inateck KB05113 （键数：78键 触摸板：有 键芯：US 系统：Win）
#define _PRODUCT_KB05113_DE    (0x00100091)  //Inateck KB05113 （键数：78键 触摸板：有 键芯：DE 系统：Win）
#define _PRODUCT_KB05113_UK    (0x00100092)  //Inateck KB05113 （键数：78键 触摸板：有 键芯：UK 系统：Win）
#define _PRODUCT_KB05114_US    (0x00100093)  //Inateck KB05114 （键数：78键 触摸板：有 键芯：US 系统：Win）
#define _PRODUCT_KB05114_DE    (0x00100094)  //Inateck KB05114 （键数：78键 触摸板：有 键芯：DE 系统：Win）
#define _PRODUCT_KB05114_UK    (0x00100095)  //Inateck KB05114 （键数：78键 触摸板：有 键芯：UK 系统：Win）
#define _PRODUCT_KB05114_ALL    (0x00100096)  //Inateck KB05114 （键数：78键 触摸板：有 键芯：DE/UK/US 系统：Win）
#define _PRODUCT_KB02028_US    (0x001000A0)  //Inateck KB02028 （键数：78键 触摸板：有 键芯：US 系统：Win）
#define _PRODUCT_KB02028_DE    (0x001000A1)  //Inateck KB02028 （键数：78键 触摸板：有 键芯：DE 系统：Win）
#define _PRODUCT_KB02028_UK    (0x001000A2)  //Inateck KB02028 （键数：78键 触摸板：有 键芯：UK 系统：Win）
#define _PRODUCT_KB06101_US    (0x001000B0)  //Inateck KB06101 （键数：78键 触摸板：有 键芯：UK 系统：Win）
#define _PRODUCT_KB06101_DE    (0x001000B1)  //Inateck KB06101 （键数：78键 触摸板：有 键芯：DE 系统：Win）
#define _PRODUCT_KB06101_UK    (0x001000B2)  //Inateck KB06101 （键数：78键 触摸板：有 键芯：US 系统：Win）
#define _PRODUCT_KB01002_US    (0x001000C0)  //Inateck KB06102 （键数：78键 触摸板：无 键芯：US 系统：All）
#define _PRODUCT_KB01002_DE    (0x001000C1)  //Inateck KB06102 （键数：78键 触摸板：无 键芯：DE 系统：All）
#define _PRODUCT_KB01002_UK    (0x001000C2)  //Inateck KB06102 （键数：78键 触摸板：无 键芯：UK 系统：All）
#define _PRODUCT_KB04111_US    (0x001000D0)  //Inateck KB04111 （键数：78键 触摸板：有 键芯：US 系统：IOS）
#define _PRODUCT_KB04111_DE    (0x001000D1)  //Inateck KB04111 （键数：78键 触摸板：有 键芯：DE 系统：IOS）
#define _PRODUCT_KB04111_UK    (0x001000D2)  //Inateck KB04111 （键数：78键 触摸板：有 键芯：UK 系统：IOS）
#define _PRODUCT_KB04111_11_US (0x001000D3)  //Inateck KB04111-11 （键数：78键 触摸板：有 键芯：US 系统：IOS）
#define _PRODUCT_KB04111_11_DE (0x001000D4)  //Inateck KB04111-11 （键数：78键 触摸板：有 键芯：DE 系统：IOS）
#define _PRODUCT_KB04111_11_UK (0x001000D5)  //Inateck KB04111-11 （键数：78键 触摸板：有 键芯：UK 系统：IOS）
#define _PRODUCT_BK2007_13_US  (0x001000E0)  //Inateck BK2007-13 （键数：78键 触摸板：无 键芯：US 系统：IOS）
#define _PRODUCT_BK2007_13_DE  (0x001000E1)  //Inateck BK2007-13 （键数：79键 触摸板：无 键芯：DE 系统：IOS）
#define _PRODUCT_BK2007_13_UK  (0x001000E2)  //Inateck BK2007-13 （键数：79键 触摸板：无 键芯：UK 系统：IOS）
#define _PRODUCT_KB09119_11_US (0x001000E3)  //KB09119_11 （键数：64键 触摸板：有 键芯：US 系统：IOS）
#define _PRODUCT_KB09119_11_DE (0x001000E4)  //KB09119_11 （键数：64键 触摸板：有 键芯：DE 系统：IOS）
#define _PRODUCT_KB09119_11_UK (0x001000E5)  //KB09119_11 （键数：64键 触摸板：有 键芯：UK 系统：IOS）
#define _PRODUCT_KB09119_11_JP (0x001000E6)  //KB09119_11 （键数：64键 触摸板：有 键芯：JP 系统：IOS）
#define _PRODUCT_KB09119_11_FR (0x001000E7)  //KB09119_11 （键数：64键 触摸板：有 键芯：FR 系统：IOS）
#define _PRODUCT_KB04122_US    (0x001000F1)  //KB04122     (键数：78键 触摸板：有 键芯：US 系统：IOS)
#define _PRODUCT_KB04122_DE_ES_FR_IT    (0x001000F2)  //KB04122     (键数：78键 触摸板：有 键芯：DE/ES/FR/IT 系统：IOS)
#define _PRODUCT_KB04122_UK    (0x001000F3)  //KB04122     (键数：78键 触摸板：有 键芯：UK 系统：IOS)
#define _PRODUCT_KB04122_JP    (0x001000F4)  //KB04122     (键数：78键 触摸板：有 键芯：JP 系统：IOS)
#define _PRODUCT_KB04122_11_US    (0x001000F5)  //KB04122_11     (键数：78键 触摸板：有 键芯：US 系统：IOS)
#define _PRODUCT_KB04122_11_DE_ES_FR_IT    (0x001000F6)  //KB04122_11     (键数：78键 触摸板：有 键芯：DE/ES/FR/IT 系统：IOS)
#define _PRODUCT_KB04122_11_UK    (0x001000F7)  //KB04122_11     (键数：78键 触摸板：有 键芯：UK 系统：IOS)
#define _PRODUCT_KB04122_11_JP    (0x001000F8)  //KB04122_11     (键数：78键 触摸板：有 键芯：JP 系统：IOS)
#define _PRODUCT_KB04122_13_A_US    (0x001000F9)  //KB04122_11     (键数：78键 触摸板：有 键芯：US 系统：IOS)
#define _PRODUCT_KB04122_13_A_DE    (0x001000FA)  //KB04122_11     (键数：78键 触摸板：有 键芯：DE 系统：IOS)
#define _PRODUCT_KB04122_13_A_UK    (0x001000FB)  //KB04122_11     (键数：78键 触摸板：有 键芯：UK 系统：IOS)
#define _PRODUCT_KB04122_13_A_JP    (0x001000FC)  //KB04122_11     (键数：78键 触摸板：有 键芯：JP 系统：IOS)
#define _PRODUCT_KB01105_US    (0x00100100)  //KB01105     (键数：78键 触摸板：有 键芯：US 系统：Mac Win Android)
#define _PRODUCT_KB01105_DE    (0x00100101)  //KB01105     (键数：78键 触摸板：有 键芯：DE 系统：Mac Win Android)
#define _PRODUCT_KB01105_UK    (0x00100102)  //KB01105     (键数：78键 触摸板：有 键芯：UK 系统：Mac Win Android)
#define _PRODUCT_KB01105_FR    (0x00100103)  //KB01105     (键数：78键 触摸板：有 键芯：FR 系统：Mac Win Android)
#define _PRODUCT_KB01105_IT    (0x00100104)  //KB01105     (键数：78键 触摸板：有 键芯：IT 系统：Mac Win Android)
#define _PRODUCT_KB06006_US    (0x00100110)  //KB06006     (键数：111键 触摸板：有 键芯：US 系统：Mac Win Android)
#define _PRODUCT_KB06006_DE    (0x00100111)  //KB06006     (键数：111键 触摸板：有 键芯：DE 系统：Mac Win Android)
#define _PRODUCT_KB06006_UK    (0x00100112)  //KB06006     (键数：111键 触摸板：有 键芯：UK 系统：Mac Win Android)
#define _PRODUCT_KB04112_US    (0x00100120)  //KB04112     (键数：78键 触摸板：有 键芯：US 系统：IOS)
#define _PRODUCT_KB04112_DE    (0x00100121)  //KB04112     (键数：78键 触摸板：有 键芯：DE 系统：IOS)
#define _PRODUCT_KB04112_UK    (0x00100122)  //KB04112     (键数：78键 触摸板：有 键芯：UK 系统：IOS)
#define _PRODUCT_KB04112_NOBG_US    (0x00100123)  //KB04112     (键数：78键 触摸板：有 键芯：US 系统：IOS)  无背光版本
#define _PRODUCT_KB04112_NOBG_DE    (0x00100124)  //KB04112     (键数：78键 触摸板：有 键芯：DE 系统：IOS)   无背光版本
#define _PRODUCT_KB04112_NOBG_UK    (0x00100125)  //KB04112     (键数：78键 触摸板：有 键芯：UK 系统：IOS)   无背光版本
#define _PRODUCT_KB06007_DE    (0x00100130)  //KB06007     (键数：111键 触摸板：有 键芯：DE 系统：Mac Win Android)
#define _PRODUCT_KB01103_DE    (0x00100140)  //KB01103     (键数：83键 触摸板：有 键芯：DE 系统：Mac Win Android)
#define _PRODUCT_KB01103_UK    (0x00100141)  //KB01103     (键数：83键 触摸板：有 键芯：UK 系统：Mac Win Android)
#define _PRODUCT_KB01103_US    (0x00100142)  //KB01103     (键数：83键 触摸板：有 键芯：US 系统：Mac Win Android)
#define _PRODUCT_KB002_US    (0x00100150)  //KB002     (键数：78键 触摸板：有 键芯：US 系统：ipad)
#define _PRODUCT_KB002_UK    (0x00100151)  //KB002     (键数：78键 触摸板：有 键芯：UK 系统：ipad)
#define _PRODUCT_KB002_DE    (0x00100152)  //KB002     (键数：78键 触摸板：有 键芯：DE 系统：ipad)
#define _PRODUCT_KB4002_DE    (0x00100160)  //KB4002     (键数：78键 触摸板：无 键芯：DE 系统：ipad)
#define _PRODUCT_KB4002_UK    (0x00100161)  //KB4002     (键数：78键 触摸板：无 键芯：UK 系统：ipad)
#define _PRODUCT_KB4002_US    (0x00100162)  //KB4002     (键数：78键 触摸板：无 键芯：US 系统：ipad)
#define _PRODUCT_KB403_11A_DE    (0x00100170)  //KB403_11A     (键数：78键 触摸板：无 键芯：DE 系统：ipad)
#define _PRODUCT_KB403_11A_UK    (0x00100171)  //KB403_11A     (键数：78键 触摸板：无 键芯：UK 系统：ipad)
#define _PRODUCT_KB403_11A_US    (0x00100172)  //KB403_11A     (键数：78键 触摸板：无 键芯：US 系统：ipad)
#define _PRODUCT_KB403_11A_JP    (0x00100173)  //KB403_11A     (键数：78键 触摸板：无 键芯：US 系统：ipad)
#define _PRODUCT_Inateck_N0046_US (0x00100180)  //Inateck N0046_13A (键数：78键 触摸板：有 键芯：US 系统：IOS)
#define _PRODUCT_Inateck_N0046_UK (0x00100181)  //Inateck N0046_13A (键数：78键 触摸板：有 键芯：UK 系统：IOS)
#define _PRODUCT_Inateck_N0046_DE (0x00100182)  //Inateck N0046_13A (键数：78键 触摸板：有 键芯：DE 系统：IOS)
#define _PRODUCT_Inateck_N0047_US (0x00100190)  //Inateck N0047_13A (键数：78键 触摸板：有 键芯：US 系统：IOS)
#define _PRODUCT_Inateck_N0047_UK (0x00100191)  //Inateck N0047_13A (键数：78键 触摸板：有 键芯：UK 系统：IOS)
#define _PRODUCT_Inateck_N0047_DE (0x00100192)  //Inateck N0047_13A (键数：78键 触摸板：有 键芯：DE 系统：IOS)
#define _PRODUCT_Inateck_N0001_US (0x001001a0)  //Inateck N0001 (键数：78键 触摸板：有 键芯：US 系统：ipad)
#define _PRODUCT_Inateck_N0001_UK (0x001001a1)  //Inateck N0001 (键数：78键 触摸板：有 键芯：UK 系统：ipad)
#define _PRODUCT_Inateck_N0001_DE (0x001001a2)  //Inateck N0001 (键数：78键 触摸板：有 键芯：DE 系统：ipad)
#define _PRODUCT_KB09119_10_US (0x001001b0)  //KB09119_10 （键数：64键 触摸板：有 键芯：US 系统：IOS）
#define _PRODUCT_KB09119_10_DE (0x001001b1)  //KB09119_10 （键数：64键 触摸板：有 键芯：DE 系统：IOS）
#define _PRODUCT_KB09119_10_UK (0x001001b2)  //KB09119_10 （键数：64键 触摸板：有 键芯：UK 系统：IOS）
#define _PRODUCT_KB09119_10_JP (0x001001b3)  //KB09119_10 （键数：64键 触摸板：有 键芯：JP 系统：IOS）
#define _PRODUCT_KB09119_10_FR (0x001001b4)  //KB09119_10 （键数：64键 触摸板：有 键芯：FR 系统：IOS）
#define _PRODUCT_KB403X_11A_DE    (0x001001c0)  //KB403X_11A     (键数：78键 触摸板：无 键芯：DE 系统：ipad)
#define _PRODUCT_KB403X_11A_UK    (0x001001c1)  //KB403X_11A     (键数：78键 触摸板：无 键芯：UK 系统：ipad)
#define _PRODUCT_KB403X_11A_US    (0x001001c2)  //KB403X_11A     (键数：78键 触摸板：无 键芯：US 系统：ipad)
#define _PRODUCT_KB403X_11A_JP    (0x001001c3)  //KB403X_11A     (键数：78键 触摸板：无 键芯：JP 系统：ipad)
#define _PRODUCT_inateck_NeoMagic_SE_US    (0x001001d0)  //Inateck inateck NeoMagic SE （键数：64键 触摸板：有 键芯：US 系统：IOS）
#define _PRODUCT_inateck_NeoMagic_SE_UK    (0x001001d1)  //Inateck inateck NeoMagic SE （键数：64键 触摸板：有 键芯：UK 系统：IOS）
#define _PRODUCT_inateck_NeoMagic_SE_DE    (0x001001d2)  //Inateck inateck NeoMagic SE （键数：64键 触摸板：有 键芯：DE 系统：IOS）
#define _PRODUCT_inateck_NeoMagic_SE_FR    (0x001001d3)  //Inateck inateck NeoMagic SE （键数：64键 触摸板：有 键芯：FR 系统：IOS）
#define _PRODUCT_inateck_NeoMagic_SE_IT    (0x001001d4)  //Inateck inateck NeoMagic SE （键数：64键 触摸板：有 键芯：IT 系统：IOS）
#define _PRODUCT_KB105_US         (0x001001f0)  //KB105     (键数：78键 触摸板：有 键芯：US 系统：ALL)
#define _PRODUCT_KB105_UK         (0x001001f1)  //KB105     (键数：78键 触摸板：有 键芯：UK 系统：ALL)
#define _PRODUCT_KB105_DE         (0x001001f2)  //KB105     (键数：78键 触摸板：有 键芯：DE 系统：ALL)
//IOS------------------------------------------------END------------------------------------------------------------//

//------------------------------------------------鼠标------------------------------------------------------------//
#define _PRODUCT_MS06006       (0x00200010)  //MS06006

//------------------------------------------------END------------------------------------------------------------//

//------------------------------------------------手写笔------------------------------------------------------------//
#define _PRODUCT_PCL004       (0x00400010)  //PCL004

//------------------------------------------------END------------------------------------------------------------//


//---------------------------------------------------------芯片型号表---------------------------------------------------------//
/*编码规则：高12bit为芯片厂商编码（0xC00 原相
                                0xC01 沁恒
                                ）
            低12bit为芯片型号
            最低8bit为芯片封装(0x01:QFN-48
                              0x02:QFN-32
                              0x03:QFN-20)
*/

/*
原相芯片型号
000：PAR2860
*/
#define _CHIP_PAR2860      (0xC0000001)  //原相PAR2860(蓝牙版本：5.0 封装：QFN-48)

/*
沁恒芯片型号
000：CH582
001：CH584
002：CH585
003：CH591
004: CH592
*/
#define _CHIP_CH582M       (0xC0100001)  //沁恒CH582M(蓝牙版本：5.4 封装：QFN-48T)
#define _CHIP_CH584F       (0xC0100102)  //沁恒CH584F(蓝牙版本：5.4 封装：QFN-32)
#define _CHIP_CH584M       (0xC0100101)  //沁恒CH584M(蓝牙版本：5.4 封装：QFN-48T)
#define _CHIP_CH585F       (0xC0100202)  //沁恒CH585F(蓝牙版本：5.4 封装：QFN-32)
#define _CHIP_CH585M       (0xC0100201)  //沁恒CH585M(蓝牙版本：5.4 封装：QFN-48T)
#define _CHIP_CH591D       (0xC0100303)  //沁恒CH591D(蓝牙版本：5.4 封装：QFN-20)
#define _CHIP_CH592D        (0xC0100403)  //沁恒CH592(蓝牙版本：5.4 封装：QFN-20)

//------------------------------------------------------------END------------------------------------------------------------//

#endif //_MODEL_TABLE_H_







